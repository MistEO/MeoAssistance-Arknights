// <copyright file="VersionUpdateSettingsUserControlModel.cs" company="MaaAssistantArknights">
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

#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows;
using MaaWpfGui.Constants;
using MaaWpfGui.Extensions;
using MaaWpfGui.Helper;
using MaaWpfGui.Main;
using MaaWpfGui.Models;
using MaaWpfGui.Properties;
using MaaWpfGui.Services;
using MaaWpfGui.Utilities.ValueType;
using MaaWpfGui.ViewModels.UI;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Stylet;

namespace MaaWpfGui.ViewModels.UserControl.Settings;

/// <summary>
/// 软件更新设置
/// </summary>
public class VersionUpdateSettingsUserControlModel : PropertyChangedBase
{
    static VersionUpdateSettingsUserControlModel()
    {
        Instance = new();
    }

    public static VersionUpdateSettingsUserControlModel Instance { get; }

    public enum UpdateVersionType
    {
        /// <summary>
        /// 测试版
        /// </summary>
        Nightly,

        /// <summary>
        /// 开发版
        /// </summary>
        Beta,

        /// <summary>
        /// 稳定版
        /// </summary>
        Stable,
    }

    /// <summary>
    /// Gets the core version.
    /// </summary>
    public static string CoreVersion { get; } = Marshal.PtrToStringAnsi(MaaService.AsstGetVersion()) ?? "0.0.1";

    public static string CoreVersionDisplay => string.Join("\u200B", CoreVersion.ToCharArray());

    private static readonly string _uiVersion = Assembly.GetExecutingAssembly().GetCustomAttribute<AssemblyInformationalVersionAttribute>()?.InformationalVersion.Split('+')[0] ?? "0.0.1";

    /// <summary>
    /// Gets the UI version.
    /// </summary>
    public static string UiVersion { get; } = _uiVersion == "0.0.1" ? "DEBUG VERSION" : _uiVersion;

    public static string UiVersionDisplay => string.Join("\u200B", UiVersion.ToCharArray());

    public static DateTime BuildDateTime { get; } = Assembly.GetExecutingAssembly().GetCustomAttribute<BuildDateTimeAttribute>()?.BuildDateTime ?? DateTime.MinValue;

    public static string BuildDateTimeCurrentCultureString => BuildDateTime.ToLocalTimeString();

    private static (DateTime DateTime, string VersionName) _resourceInfo = GetResourceVersionByClientType(SettingsViewModel.GameSettings.ClientType);

    public (DateTime DateTime, string VersionName) ResourceInfo
    {
        get => _resourceInfo;
        set => SetAndNotify(ref _resourceInfo, value);
    }

    private static string _resourceVersion = _resourceInfo.VersionName;

    /// <summary>
    /// Gets or sets the resource version.
    /// </summary>
    public string ResourceVersion
    {
        get => _resourceVersion;
        set => SetAndNotify(ref _resourceVersion, value);
    }

    private static DateTime _resourceDateTime = _resourceInfo.DateTime;

    public DateTime ResourceDateTime
    {
        get => _resourceDateTime;
        set => SetAndNotify(ref _resourceDateTime, value);
    }

    private string _resourceDateTimeCurrentCultureString = _resourceDateTime.ToLocalTimeString();

    public string ResourceDateTimeCurrentCultureString
    {
        get => _resourceDateTimeCurrentCultureString;
        set => SetAndNotify(ref _resourceDateTimeCurrentCultureString, value);
    }

    public void ResourceInfoUpdate()
    {
        ResourceInfo = GetResourceVersionByClientType(SettingsViewModel.GameSettings.ClientType);
        ResourceVersion = ResourceInfo.VersionName;
        ResourceDateTime = ResourceInfo.DateTime;
        ResourceDateTimeCurrentCultureString = ResourceDateTime.ToLocalTimeString();
        Instances.SettingsViewModel.UpdateWindowTitle();
    }

    public static (DateTime DateTime, string VersionName) GetResourceVersionByClientType(string clientType)
    {
        bool isDefaultClient = new HashSet<string> { string.Empty, "Official", "Bilibili" }.Contains(clientType);

        const string DefaultJsonPath = "resource/version.json";
        var jsonPath = isDefaultClient
            ? DefaultJsonPath
            : $"resource/global/{clientType}/resource/version.json";

        string versionName;
        if (!File.Exists(DefaultJsonPath) || (!isDefaultClient && !File.Exists(jsonPath)))
        {
            return (DateTime.MinValue, string.Empty);
        }

        var versionJson = LoadJson(jsonPath);
        var currentTime = (ulong)DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        var poolTime = (ulong?)versionJson?["gacha"]?["time"]; // 卡池的开始时间
        var activityTime = (ulong?)versionJson?["activity"]?["time"]; // 活动的开始时间
        var lastUpdated = isDefaultClient
            ? (string?)versionJson?["last_updated"]
            : (string?)LoadJson(DefaultJsonPath)?["last_updated"];

        var dateTime = lastUpdated == null
            ? DateTime.MinValue
            : DateTime.ParseExact(lastUpdated, "yyyy-MM-dd HH:mm:ss.fff", null);

        if (currentTime < poolTime && currentTime < activityTime)
        {
            versionName = string.Empty;
        }
        else if (currentTime >= poolTime && currentTime < activityTime)
        {
            versionName = versionJson?["gacha"]?["pool"]?.ToString() ?? string.Empty;
        }
        else if (currentTime < poolTime && currentTime >= activityTime)
        {
            versionName = versionJson?["activity"]?["name"]?.ToString() ?? string.Empty;
        }
        else if (poolTime > activityTime)
        {
            versionName = versionJson?["gacha"]?["pool"]?.ToString() ?? string.Empty;
        }
        else
        {
            versionName = versionJson?["activity"]?["name"]?.ToString() ?? string.Empty;
        }

        return (dateTime, versionName);

        static JObject? LoadJson(string path)
        {
            return JsonConvert.DeserializeObject<JObject>(File.ReadAllText(path));
        }
    }

    private UpdateVersionType _versionType = (UpdateVersionType)Enum.Parse(
        typeof(UpdateVersionType),
        ConfigurationHelper.GetGlobalValue(ConfigurationKeys.VersionType, UpdateVersionType.Stable.ToString()));

    /// <summary>
    /// Gets or sets the type of version to update.
    /// </summary>
    public UpdateVersionType VersionType
    {
        get => _versionType;
        set
        {
            SetAndNotify(ref _versionType, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.VersionType, value.ToString());
        }
    }

    /// <summary>
    /// Gets the list of the version type.
    /// </summary>
    public List<GenericCombinedData<UpdateVersionType>> AllVersionTypeList { get; } =
        [
            new() { Display = LocalizationHelper.GetString("UpdateCheckNightly"), Value = UpdateVersionType.Nightly },
            new() { Display = LocalizationHelper.GetString("UpdateCheckBeta"), Value = UpdateVersionType.Beta },
            new() { Display = LocalizationHelper.GetString("UpdateCheckStable"), Value = UpdateVersionType.Stable },
        ];

    public List<GenericCombinedData<UpdateVersionType>> VersionTypeList
    {
        get => AllVersionTypeList.Where(v => AllowNightlyUpdates || v.Value != UpdateVersionType.Nightly).ToList();
    }

    public bool AllowNightlyUpdates { get; set; } = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.AllowNightlyUpdates, bool.FalseString));

    private bool _hasAcknowledgedNightlyWarning = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.HasAcknowledgedNightlyWarning, bool.FalseString));

    public bool HasAcknowledgedNightlyWarning
    {
        get => _hasAcknowledgedNightlyWarning;
        set
        {
            SetAndNotify(ref _hasAcknowledgedNightlyWarning, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.HasAcknowledgedNightlyWarning, value.ToString());
        }
    }

    public List<GenericCombinedData<string>> UpdateSourceList { get; } = [
        new() { Display = LocalizationHelper.GetString("GlobalSource"), Value = "Github" },
        new() { Display = LocalizationHelper.GetString("MirrorChyan"), Value = "MirrorChyan" },
    ];

    private string _updateSource = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.UpdateSource, "Github");

    /// <summary>
    /// Gets or sets the type of version to update.
    /// </summary>
    public string UpdateSource
    {
        get => _updateSource;
        set
        {
            SetAndNotify(ref _updateSource, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.UpdateSource, value);
        }
    }

    private string _mirrorChyanCdk = SimpleEncryptionHelper.Decrypt(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.MirrorChyanCdk, string.Empty));

    public string MirrorChyanCdk
    {
        get => _mirrorChyanCdk;
        set
        {
            if (!SetAndNotify(ref _mirrorChyanCdk, value))
            {
                return;
            }

            value = SimpleEncryptionHelper.Encrypt(value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.MirrorChyanCdk, value);
        }
    }

    private bool _startupUpdateCheck = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.StartupUpdateCheck, bool.TrueString));

    /// <summary>
    /// Gets or sets a value indicating whether to check update.
    /// </summary>
    public bool StartupUpdateCheck
    {
        get => _startupUpdateCheck;
        set
        {
            SetAndNotify(ref _startupUpdateCheck, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.StartupUpdateCheck, value.ToString());
        }
    }

    private bool _updateAutoCheck = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.UpdateAutoCheck, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to check update.
    /// </summary>
    public bool UpdateAutoCheck
    {
        get => _updateAutoCheck;
        set
        {
            SetAndNotify(ref _updateAutoCheck, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.UpdateAutoCheck, value.ToString());
        }
    }

    private string _proxy = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.UpdateProxy, string.Empty);

    /// <summary>
    /// Gets or sets the proxy settings.
    /// </summary>
    public string Proxy
    {
        get => _proxy;
        set
        {
            SetAndNotify(ref _proxy, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.UpdateProxy, value);
        }
    }

    public List<CombinedData> ProxyTypeList { get; } =
        [
            new() { Display = "HTTP Proxy", Value = "http" },
            new() { Display = "SOCKS5 Proxy", Value = "socks5" },
        ];

    private string _proxyType = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.ProxyType, "http");

    public string ProxyType
    {
        get => _proxyType;
        set
        {
            SetAndNotify(ref _proxyType, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.ProxyType, value);
        }
    }

    private bool _isCheckingForUpdates;

    /// <summary>
    /// Gets or sets a value indicating whether the update is being checked.
    /// </summary>
    public bool IsCheckingForUpdates
    {
        get => _isCheckingForUpdates;
        set
        {
            SetAndNotify(ref _isCheckingForUpdates, value);
        }
    }

    private bool _autoDownloadUpdatePackage = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.AutoDownloadUpdatePackage, bool.TrueString));

    /// <summary>
    /// Gets or sets a value indicating whether to auto download update package.
    /// </summary>
    public bool AutoDownloadUpdatePackage
    {
        get => _autoDownloadUpdatePackage;
        set
        {
            SetAndNotify(ref _autoDownloadUpdatePackage, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.AutoDownloadUpdatePackage, value.ToString());
        }
    }

    private bool _autoInstallUpdatePackage = Convert.ToBoolean(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.AutoInstallUpdatePackage, bool.FalseString));

    /// <summary>
    /// Gets or sets a value indicating whether to auto install update package.
    /// </summary>
    public bool AutoInstallUpdatePackage
    {
        get => _autoInstallUpdatePackage;
        set
        {
            SetAndNotify(ref _autoInstallUpdatePackage, value);
            ConfigurationHelper.SetGlobalValue(ConfigurationKeys.AutoInstallUpdatePackage, value.ToString());
        }
    }

    /// <summary>
    /// Updates manually.
    /// </summary>
    /// <returns>A <see cref="Task"/> representing the asynchronous operation.</returns>
    public async Task ManualUpdate()
    {
        var ret = await Instances.VersionUpdateViewModel.CheckAndDownloadVersionUpdate();

        var toastMessage = ret switch
        {
            VersionUpdateViewModel.CheckUpdateRetT.NoNeedToUpdate => string.Empty,
            VersionUpdateViewModel.CheckUpdateRetT.NoNeedToUpdateDebugVersion => LocalizationHelper.GetString("NoNeedToUpdateDebugVersion"),
            VersionUpdateViewModel.CheckUpdateRetT.AlreadyLatest => LocalizationHelper.GetString("AlreadyLatest"),
            VersionUpdateViewModel.CheckUpdateRetT.UnknownError => LocalizationHelper.GetString("NewVersionDetectFailedTitle"),
            VersionUpdateViewModel.CheckUpdateRetT.NetworkError => LocalizationHelper.GetString("CheckNetworking"),
            VersionUpdateViewModel.CheckUpdateRetT.FailedToGetInfo => LocalizationHelper.GetString("GetReleaseNoteFailed"),
            VersionUpdateViewModel.CheckUpdateRetT.OK => string.Empty,
            VersionUpdateViewModel.CheckUpdateRetT.NewVersionIsBeingBuilt => LocalizationHelper.GetString("NewVersionIsBeingBuilt"),
            VersionUpdateViewModel.CheckUpdateRetT.OnlyGameResourceUpdated => LocalizationHelper.GetString("GameResourceUpdated"),
            VersionUpdateViewModel.CheckUpdateRetT.NoMirrorChyanCdk => LocalizationHelper.GetString("MirrorChyanSoftwareUpdateTip"),
            _ => string.Empty,
        };

        if (toastMessage != string.Empty)
        {
            ToastNotification.ShowDirect(toastMessage);
        }

        if (ret == VersionUpdateViewModel.CheckUpdateRetT.OK)
        {
            _ = Instances.VersionUpdateViewModel.AskToRestart();
        }
    }

    public async Task ManualUpdateResource()
    {
        IsCheckingForUpdates = true;

        var (ret, uri) = await ResourceUpdater.CheckFromMirrorChyanAsync();
        var toastMessage = ret switch
        {
            VersionUpdateViewModel.CheckUpdateRetT.NoNeedToUpdate => string.Empty,
            VersionUpdateViewModel.CheckUpdateRetT.NoNeedToUpdateDebugVersion => LocalizationHelper.GetString("NoNeedToUpdateDebugVersion"),
            VersionUpdateViewModel.CheckUpdateRetT.AlreadyLatest => LocalizationHelper.GetString("AlreadyLatest"),
            VersionUpdateViewModel.CheckUpdateRetT.UnknownError => LocalizationHelper.GetString("NewVersionDetectFailedTitle"),
            VersionUpdateViewModel.CheckUpdateRetT.NetworkError => LocalizationHelper.GetString("CheckNetworking"),
            VersionUpdateViewModel.CheckUpdateRetT.FailedToGetInfo => LocalizationHelper.GetString("GetReleaseNoteFailed"),
            VersionUpdateViewModel.CheckUpdateRetT.OK => string.Empty,
            VersionUpdateViewModel.CheckUpdateRetT.NewVersionIsBeingBuilt => LocalizationHelper.GetString("NewVersionIsBeingBuilt"),
            VersionUpdateViewModel.CheckUpdateRetT.OnlyGameResourceUpdated => LocalizationHelper.GetString("GameResourceUpdated"),
            VersionUpdateViewModel.CheckUpdateRetT.NoMirrorChyanCdk => LocalizationHelper.GetString("MirrorChyanSoftwareUpdateTip"),
            _ => string.Empty,
        };

        if (toastMessage != string.Empty)
        {
            ToastNotification.ShowDirect(toastMessage);
        }

        if (ret == VersionUpdateViewModel.CheckUpdateRetT.AlreadyLatest)
        {
            SettingsViewModel.VersionUpdateSettings.IsCheckingForUpdates = false;
            return;
        }

        bool success = UpdateSource switch
        {
            "Github" => await ResourceUpdater.UpdateFromGithubAsync(),
            "MirrorChyan" => (ret == VersionUpdateViewModel.CheckUpdateRetT.OK) && await ResourceUpdater.DownloadFromMirrorChyanAsync(uri),
            _ => await ResourceUpdater.UpdateFromGithubAsync(),
        };

        if (success)
        {
            Instances.AsstProxy.LoadResource();
            DataHelper.Reload();
            SettingsViewModel.VersionUpdateSettings.ResourceInfoUpdate();
            ToastNotification.ShowDirect(LocalizationHelper.GetString("GameResourceUpdated"));
        }

        IsCheckingForUpdates = false;
    }

    // UI 绑定的方法
    // ReSharper disable once UnusedMember.Global
    public void ShowChangelog()
    {
        Instances.WindowManager.ShowWindow(Instances.VersionUpdateViewModel);
    }
}
