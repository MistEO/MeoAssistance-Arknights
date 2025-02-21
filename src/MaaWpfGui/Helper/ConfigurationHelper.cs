// <copyright file="ConfigurationHelper.cs" company="MaaAssistantArknights">
// MaaWpfGui - A part of the MaaCoreArknights project
// Copyright (C) 2021 MistEO and Contributors
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License v3.0 only as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY
// </copyright>

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using MaaWpfGui.Constants;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Serilog;

namespace MaaWpfGui.Helper
{
    public class ConfigurationHelper
    {
        private static readonly string _configurationFile = Path.Combine(Environment.CurrentDirectory, "config/gui.json");
        private static readonly string _configurationBakFile = Path.Combine(Environment.CurrentDirectory, "config/gui.json.bak");
        private static readonly string _configurationErrorFile = Path.Combine(Environment.CurrentDirectory, "config/gui.error.json");
        private static ConcurrentDictionary<string, ConcurrentDictionary<string, string>> _kvsMap;
        private static string _current = ConfigurationKeys.DefaultConfiguration;
        private static ConcurrentDictionary<string, string> _kvs;
        private static ConcurrentDictionary<string, string> _globalKvs;

        private static readonly object _lock = new();

        private static readonly ILogger _logger = Log.ForContext<ConfigurationHelper>();

        public delegate void ConfigurationUpdateEventHandler(string key, string oldValue, string newValue);

        public static event ConfigurationUpdateEventHandler ConfigurationUpdateEvent;

        private static bool Released { get; set; }

        /// <summary>
        /// Get a configuration value
        /// </summary>
        /// <param name="key">The config key</param>
        /// <param name="defaultValue">The default value to return if the key is not existed</param>
        /// <returns>The config value</returns>
        public static string GetValue(string key, string defaultValue)
        {
            var hasValue = _kvs.TryGetValue(key, out var value);

            // _logger.Debug("Read configuration key {Key} with default value {DefaultValue}, configuration hit: {HasValue}, configuration value {Value}", key, defaultValue, hasValue, value);
            if (hasValue)
            {
                return value;
            }

            hasValue = _globalKvs.TryGetValue(key, out value);
            if (hasValue)
            {
                _logger.Information("Read configuration key {Key} with global configuration value {Value}, configuration hit: {HasValue}, configuration value {Value}", key, value, true, value);
                SetValue(key, value);
                return value;
            }

            // return hasValue ? value : defaultValue;
            SetValue(key, defaultValue);
            return defaultValue;
        }

        /// <summary>
        /// Get a configuration value
        /// </summary>
        /// <param name="key">The config key</param>
        /// <param name="defaultValue">The default value to return if the key is not existed</param>
        /// <returns>The config value</returns>
        public static int GetValue(string key, int defaultValue)
        {
            var value = GetValue(key, defaultValue.ToString());
            return int.TryParse(value, out var result) ? result : defaultValue;
        }

        public static string GetGlobalValue(string key, string defaultValue)
        {
            var hasValue = _globalKvs.TryGetValue(key, out var value);

            // _logger.Debug("Read global configuration key {Key} with default value {DefaultValue}, configuration hit: {HasValue}, configuration value {Value}", key, defaultValue, hasValue, value);
            if (hasValue)
            {
                return value;
            }

            hasValue = _kvs.TryGetValue(key, out value);
            if (hasValue)
            {
                _logger.Information("Read global configuration key {Key} with current configuration value {Value}, configuration hit: {HasValue}, configuration value {Value}", key, value, true, value);
                SetGlobalValue(key, value);
                DeleteValue(key);
                return value;
            }

            // 保证有全局配置
            SetGlobalValue(key, defaultValue);
            return defaultValue;
        }

        /// <summary>
        /// Set a configuration value
        /// </summary>
        /// <param name="key">The config key</param>
        /// <param name="value">The config value</param>
        /// <returns>The return value of <see cref="Save"/></returns>
        public static bool SetValue(string key, string value)
        {
            var old = string.Empty;
            if (!_kvs.TryAdd(key, value))
            {
                old = _kvs[key];
                _kvs[key] = value;
            }

            var result = Save();
            if (result)
            {
                ConfigurationUpdateEvent?.Invoke(key, old, value);
                _logger.Debug("Configuration {Key} has been set to {Value}", key, value);
            }
            else
            {
                _logger.Warning("Failed to save configuration {Key} to {Value}", key, value);
            }

            return result;
        }

        public static bool SetGlobalValue(string key, string value)
        {
            var old = string.Empty;
            if (!_globalKvs.TryAdd(key, value))
            {
                old = _globalKvs[key];
                _globalKvs[key] = value;
            }

            var result = Save();
            if (result)
            {
                ConfigurationUpdateEvent?.Invoke(key, old, value);
                _logger.Debug("Global configuration {Key} has been set to {Value}", key, value);
            }
            else
            {
                _logger.Warning("Failed to save global configuration {Key} to {Value}", key, value);
            }

            return result;
        }

        public static bool ContainsKey(string key, bool isGlobal = false) => isGlobal ? _globalKvs.ContainsKey(key) : _kvs.ContainsKey(key);

        /// <summary>
        /// Deletes a configuration
        /// </summary>
        /// <param name="key">The configuration key.</param>
        /// <returns>The return value of <see cref="Save"/>.</returns>
        // ReSharper disable once UnusedMember.Global
        public static bool DeleteValue(string key)
        {
            var old = string.Empty;
            if (_kvs.TryGetValue(key, out var kv))
            {
                old = kv;
            }

            _kvs.Remove(key, out _);
            var result = Save();
            if (result)
            {
                ConfigurationUpdateEvent?.Invoke(key, old, string.Empty);
                _logger.Debug("Configuration {Key} has been deleted", key);
            }
            else
            {
                _logger.Warning("Failed to save configuration file when deleted {Key}", key);
            }

            return result;
        }

        private static void EnsureConfigDirectoryExists()
        {
            if (Directory.Exists("config") is false)
            {
                Directory.CreateDirectory("config");
            }
        }

        private static bool ParseConfig(out JObject parsed)
        {
            parsed = ParseJsonFile(_configurationFile);
            if (parsed is not null)
            {
                return true;
            }

            if (File.Exists(_configurationFile))
            {
                _logger.Warning("Failed to load configuration file, copying configuration file to error file");
                File.Copy(_configurationFile, _configurationErrorFile, true);
            }

            if (File.Exists(_configurationBakFile))
            {
                _logger.Information("trying to use backup file");
                parsed = ParseJsonFile(_configurationBakFile);
                if (parsed is not null)
                {
                    _logger.Information("Backup file loaded successfully, copying backup file to configuration file");
                    File.Copy(_configurationBakFile, _configurationFile, true);
                    return true;
                }
            }

            _logger.Warning("Failed to load configuration file and/or backup file, using default settings");

            _kvsMap = [];
            _current = ConfigurationKeys.DefaultConfiguration;
            _kvsMap[_current] = [];
            _kvs = _kvsMap[_current];
            _globalKvs = [];
            return false;
        }

        /// <summary>
        /// Load configuration file
        /// </summary>
        /// <returns>True if success, false if failed</returns>
        public static bool Load()
        {
            EnsureConfigDirectoryExists();

            // Load configuration file
            if (!ParseConfig(out var parsed))
            {
                return false;
            }

            SetKvOrMigrate(parsed);

            return true;
        }

        private static void SetKvOrMigrate(JObject parsed)
        {
            bool hasConfigMap = parsed.ContainsKey(ConfigurationKeys.ConfigurationMap);
            bool hasGlobalConfig = parsed.ContainsKey(ConfigurationKeys.GlobalConfiguration);
            if (hasConfigMap)
            {
                // new version
                _kvsMap = parsed[ConfigurationKeys.ConfigurationMap]?.ToObject<ConcurrentDictionary<string, ConcurrentDictionary<string, string>>>()
                          ?? [];
                _current = parsed[ConfigurationKeys.CurrentConfiguration]?.ToString()
                           ?? ConfigurationKeys.DefaultConfiguration;
            }
            else
            {
                // old version
                _logger.Information("Configuration file is in old version, migrating to new version");

                _kvsMap = [];
                _current = ConfigurationKeys.DefaultConfiguration;
                _kvsMap[_current] = parsed.ToObject<ConcurrentDictionary<string, string>>();
            }

            _kvs = _kvsMap[_current];

            _globalKvs = hasGlobalConfig
                ? parsed[ConfigurationKeys.GlobalConfiguration]?.ToObject<ConcurrentDictionary<string, string>>()
                : new ConcurrentDictionary<string, string>();
        }

        /// <summary>
        /// Save configuration file
        /// </summary>
        /// <returns>The result of saving process</returns>
        private static bool Save(string file = null)
        {
            lock (_lock)
            {
                if (Released)
                {
                    _logger.Warning("Attempting to save configuration file after release, this is not allowed");
                    return false;
                }

                try
                {
                    var sortedKvsMap = new SortedDictionary<string, SortedDictionary<string, string>>(
                        _kvsMap.ToDictionary(
                            kvp => kvp.Key,
                            kvp => new SortedDictionary<string, string>(kvp.Value)));

                    var jsonStr = JsonConvert.SerializeObject(
                        new SortedDictionary<string, object>
                        {
                            { ConfigurationKeys.ConfigurationMap, sortedKvsMap },
                            { ConfigurationKeys.CurrentConfiguration, _current },
                            { ConfigurationKeys.GlobalConfiguration, new SortedDictionary<string, string>(_globalKvs) },
                        },
                        Formatting.Indented);

                    File.WriteAllText(file ?? _configurationFile, jsonStr);
                }
                catch (Exception e)
                {
                    _logger.Error(e, "Failed to save configuration file.");
                    return false;
                }

                return true;
            }
        }

        public static string GetCheckedStorage(string storageKey, string name, string defaultValue)
        {
            return GetValue(storageKey + name + ".IsChecked", defaultValue);
        }

        public static bool SetCheckedStorage(string storageKey, string name, string value)
        {
            return SetValue(storageKey + name + ".IsChecked", value);
        }

        public static string GetFacilityOrder(string facility, string defaultValue)
        {
            return GetValue("Infrast.Order." + facility, defaultValue);
        }

        public static bool SetFacilityOrder(string facility, string value)
        {
            return SetValue("Infrast.Order." + facility, value);
        }

        public static string GetTimer(int i, string defaultValue)
        {
            return GetGlobalValue($"Timer.Timer{i + 1}", defaultValue);
        }

        public static bool SetTimer(int i, string value)
        {
            return SetGlobalValue($"Timer.Timer{i + 1}", value);
        }

        public static string GetTimerHour(int i, string defaultValue)
        {
            return GetGlobalValue($"Timer.Timer{i + 1}Hour", defaultValue);
        }

        public static bool SetTimerHour(int i, string value)
        {
            return SetGlobalValue($"Timer.Timer{i + 1}Hour", value);
        }

        public static string GetTimerMin(int i, string defaultValue)
        {
            return GetGlobalValue($"Timer.Timer{i + 1}Min", defaultValue);
        }

        public static bool SetTimerMin(int i, string value)
        {
            return SetGlobalValue($"Timer.Timer{i + 1}Min", value);
        }

        public static string GetTimerConfig(int i, string defaultValue)
        {
            return GetGlobalValue($"Timer.Timer{i + 1}.Config", defaultValue);
        }

        public static bool SetTimerConfig(int i, string value)
        {
            return SetGlobalValue($"Timer.Timer{i + 1}.Config", value);
        }

        public static string GetTaskOrder(string task, string defaultValue)
        {
            return GetValue("TaskQueue.Order." + task, defaultValue);
        }

        public static bool SetTaskOrder(string task, string value)
        {
            return SetValue("TaskQueue.Order." + task, value);
        }

        public static void Release()
        {
            lock (_lock)
            {
                if (Released)
                {
                    return;
                }

                if (Save())
                {
                    _logger.Information($"{_configurationFile} saved");
                }
                else
                {
                    _logger.Warning($"{_configurationFile} save failed");
                }

                if (Save(_configurationBakFile))
                {
                    _logger.Information($"{_configurationBakFile} saved");
                }
                else
                {
                    _logger.Warning($"{_configurationBakFile} save failed");
                }
            }

            Released = true;
        }

        private static JObject ParseJsonFile(string filePath)
        {
            if (File.Exists(filePath) is false)
            {
                return null;
            }

            var str = File.ReadAllText(filePath);

            try
            {
                var obj = (JObject)JsonConvert.DeserializeObject(str);
                return obj ?? throw new Exception("Failed to parse json file");
            }
            catch (Exception ex)
            {
                _logger.Error(ex, "Failed to deserialize json file: {FilePath}", filePath);
            }

            return null;
        }

        public static bool SwitchConfiguration(string configName)
        {
            if (_kvsMap.ContainsKey(configName) is false)
            {
                _logger.Warning("Configuration {ConfigName} does not exist", configName);
                return false;
            }

            _current = configName;
            _kvs = _kvsMap[_current];
            return true;
        }

        public static bool AddConfiguration(string configName, string copyFrom = null)
        {
            if (string.IsNullOrEmpty(configName))
            {
                return false;
            }

            if (_kvsMap.ContainsKey(configName))
            {
                _logger.Warning("Configuration {ConfigName} already exists", configName);
                return false;
            }

            if (copyFrom is null)
            {
                _kvsMap[configName] = [];
            }
            else
            {
                if (_kvsMap.ContainsKey(copyFrom) is false)
                {
                    _logger.Warning("Configuration {ConfigName} does not exist", copyFrom);
                    return false;
                }

                _kvsMap[configName] = new ConcurrentDictionary<string, string>(_kvsMap[copyFrom]);
            }

            return true;
        }

        public static bool DeleteConfiguration(string configName)
        {
            if (_kvsMap.ContainsKey(configName) is false)
            {
                _logger.Warning("Configuration {ConfigName} does not exist", configName);
                return false;
            }

            if (_current == configName)
            {
                _logger.Warning("Configuration {ConfigName} is current configuration, cannot delete", configName);
                return false;
            }

            _kvsMap.Remove(configName, out _);
            return true;
        }

        public static List<string> GetConfigurationList() => [.. _kvsMap.Keys.OrderBy(key => key)];

        public static string GetCurrentConfiguration() => _current;
    }
}
