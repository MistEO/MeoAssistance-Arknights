// <copyright file="DataHelper.cs" company="MaaAssistantArknights">
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
using MaaWpfGui.Constants;
using MaaWpfGui.ViewModels.UI;
using MaaWpfGui.ViewModels.UserControl.Settings;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace MaaWpfGui.Helper
{
    public static class DataHelper
    {
        public static readonly Dictionary<string, string> ClientDirectoryMapper = new()
        {
            { "zh-tw", "txwy" },
            { "en-us", "YoStarEN" },
            { "ja-jp", "YoStarJP" },
            { "ko-kr", "YoStarKR" },
        };

        public static readonly Dictionary<string, string> ClientLanguageMapper = new()
        {
            { string.Empty, "zh-cn" },
            { "Official", "zh-cn" },
            { "Bilibili", "zh-cn" },
            { "YoStarEN", "en-us" },
            { "YoStarJP", "ja-jp" },
            { "YoStarKR", "ko-kr" },
            { "txwy", "zh-tw" },
        };

        // 储存角色信息的字典
        public static Dictionary<string, CharacterInfo> Characters { get; } = new();

        public static HashSet<string> CharacterNames { get; } = [];

        public static Dictionary<string, (string DisplayName, string ClientName)> RecruitTags { get; private set; } = [];

        public static List<MapInfo> MapData { get; private set; } = [];

        static DataHelper()
        {
            InitRecruitTag();
            Reload();
        }

        public static void Reload()
        {
            LoadBattleData();
            LoadMapData();
        }

        private static void LoadBattleData()
        {
            const string FilePath = "resource/battle_data.json";
            if (!File.Exists(FilePath))
            {
                return;
            }

            string jsonText = File.ReadAllText(FilePath);
            var characterData = JsonConvert.DeserializeObject<Dictionary<string, CharacterInfo>>(JObject.Parse(jsonText)["chars"]?.ToString() ?? string.Empty) ?? new Dictionary<string, CharacterInfo>();

            var characterNamesLangAdd = GetCharacterNamesAddAction(ConfigurationHelper.GetGlobalValue(ConfigurationKeys.Localization, LocalizationHelper.DefaultLanguage));
            var characterNamesClientAdd = GetCharacterNamesAddAction(ConfigurationHelper.GetValue(ConfigurationKeys.ClientType, string.Empty));

            Characters.Clear();
            CharacterNames.Clear();
            foreach (var (key, value) in characterData)
            {
                Characters.Add(key, value);
                if (!key.StartsWith("char_"))
                {
                    continue;
                }

                characterNamesLangAdd.Invoke(value);
                characterNamesClientAdd.Invoke(value);
            }
        }

        private static void InitRecruitTag()
        {
            var clientType = GameSettingsUserControlModel.Instance.ClientType;
            var clientPath = clientType switch
            {
                "" or "Official" or "Bilibili" => string.Empty,
                _ => Path.Combine("global", clientType, "resource"),
            };

            var displayLanguage = GuiSettingsUserControlModel.Instance.Language;
            var displayPath = displayLanguage switch
            {
                "zh-tw" or "en-us" or "ja-jp" or "ko-kr" => Path.Combine("global", ClientDirectoryMapper[displayLanguage], "resource"),
                _ => string.Empty,
            };

            var clientTags = ParseRecruit(Path.Combine("resource", clientPath, "recruitment.json"));
            var displayTags = ParseRecruit(Path.Combine("resource", displayPath, "recruitment.json"));

            RecruitTags = clientTags.Keys
                .Select(key => new KeyValuePair<string, (string DisplayName, string ClientName)>(
                    key,
                    (displayTags.TryGetValue(key, out var displayTag)
                        ? displayTag
                        : string.Empty,
                    clientTags.TryGetValue(key, out var clientTag)
                        ? clientTag
                        : string.Empty)))
                .Where(i => !string.IsNullOrEmpty(i.Value.ClientName))
                .Select(i => !string.IsNullOrEmpty(i.Value.DisplayName) ? i : new(i.Key, (i.Value.ClientName, i.Value.ClientName)))
                .ToDictionary();
            return;

            static Dictionary<string, string> ParseRecruit(string path)
            {
                Dictionary<string, string> clientTags = [];
                if (!File.Exists(path))
                {
                    return clientTags;
                }

                var jObj = (JObject?)JsonConvert.DeserializeObject(File.ReadAllText(path));
                if (jObj is null || !jObj.ContainsKey("tags") || jObj["tags"] is not JObject tags)
                {
                    return clientTags;
                }

                foreach (var item in tags)
                {
                    clientTags.Add(item.Key, item.Value?.ToString() ?? string.Empty);
                }

                return clientTags.Where(i => !string.IsNullOrEmpty(i.Value)).ToDictionary();
            }
        }

        private static void LoadMapData()
        {
            MapData = [];
            var path = Path.Combine("resource", "Arknights-Tile-Pos", "overview.json");
            if (!File.Exists(path))
            {
                return;
            }

            try
            {
                var jObj = JsonConvert.DeserializeObject<Dictionary<string, MapInfo>>(File.ReadAllText(path));
                if (jObj is not null && jObj.Count > 0)
                {
                    MapData = [.. jObj.Values];
                }
            }
            catch
            {
            }
        }

        public static MapInfo? FindMap(string mapId)
            => MapData.FirstOrDefault(map => map.Code == mapId || map.Name == mapId || map.StageId == mapId || map.LevelId == mapId);

        private static Action<CharacterInfo> GetCharacterNamesAddAction(string str)
        {
            return str switch
            {
                "zh-cn" or "Official" or "Bilibili" =>
                    v => CharacterNames.Add(v.Name ?? string.Empty),
                "zh-tw" or "txwy" =>
                    v => CharacterNames.Add(v.NameTw ?? string.Empty),
                "en-us" or "YoStarEN" =>
                    v => CharacterNames.Add(v.NameEn ?? string.Empty),
                "ja-jp" or "YoStarJP" =>
                    v => CharacterNames.Add(v.NameJp ?? string.Empty),
                "ko-kr" or "YoStarKR" =>
                    v => CharacterNames.Add(v.NameKr ?? string.Empty),
                _ =>
                    v => CharacterNames.Add(v.Name ?? string.Empty),
            };
        }

        public static CharacterInfo? GetCharacterByNameOrAlias(string characterName)
        {
            return Characters.Values.FirstOrDefault(
                character => new[] { character.Name, character.NameEn, character.NameJp, character.NameKr, character.NameTw, }
                    .Any(name => name?.Equals(characterName, StringComparison.OrdinalIgnoreCase) ?? false));
        }

        public static string? GetLocalizedCharacterName(string? characterName, string? language = null)
        {
            if (string.IsNullOrEmpty(characterName))
            {
                return null;
            }

            var characterInfo = GetCharacterByNameOrAlias(characterName);
            if (characterInfo?.Name == null)
            {
                return null;
            }

            language ??= SettingsViewModel.GuiSettings.OperNameLocalization;

            return language switch
            {
                "zh-cn" => characterInfo.Name,
                "zh-tw" => characterInfo.NameTw ?? characterInfo.Name,
                "en-us" => characterInfo.NameEn ?? characterInfo.Name,
                "ja-jp" => characterInfo.NameJp ?? characterInfo.Name,
                "ko-kr" => characterInfo.NameKr ?? characterInfo.Name,
                _ => characterInfo.Name,
            };
        }

        public static bool IsCharacterAvailableInClient(CharacterInfo? character, string clientType)
        {
            if (character is null)
            {
                return false;
            }

            return clientType switch
            {
                "zh-tw" or "txwy" => !character.NameTwUnavailable,
                "en-us" or "YoStarEN" => !character.NameEnUnavailable,
                "ja-jp" or "YoStarJP" => !character.NameJpUnavailable,
                "ko-kr" or "YoStarKR" => !character.NameKrUnavailable,
                _ => true,
            };
        }

        public static bool IsCharacterAvailableInClient(string characterName, string clientType)
        {
            var character = GetCharacterByNameOrAlias(characterName);
            return character != null && IsCharacterAvailableInClient(character, clientType);
        }

        public class CharacterInfo
        {
            [JsonProperty("name")]
            public string? Name { get; set; }

            [JsonProperty("name_en")]
            public string? NameEn { get; set; }

            [JsonProperty("name_en_unavailable")]
            public bool NameEnUnavailable { get; set; } = false;

            [JsonProperty("name_jp")]
            public string? NameJp { get; set; }

            [JsonProperty("name_jp_unavailable")]
            public bool NameJpUnavailable { get; set; } = false;

            [JsonProperty("name_kr")]
            public string? NameKr { get; set; }

            [JsonProperty("name_kr_unavailable")]
            public bool NameKrUnavailable { get; set; } = false;

            [JsonProperty("name_tw")]
            public string? NameTw { get; set; }

            [JsonProperty("name_tw_unavailable")]
            public bool NameTwUnavailable { get; set; } = false;

            [JsonProperty("position")]
            public string? Position { get; set; }

            [JsonProperty("profession")]
            public string? Profession { get; set; }

            [JsonProperty("rangeId")]
            public List<string>? RangeId { get; set; }

            [JsonProperty("rarity")]
            public int Rarity { get; set; }
        }

        public class MapInfo
        {
            // 1-7
            [JsonProperty("code")]
            public string? Code { get; set; }

            // main_01-07#f#-obt-main-level_main_01-07.json
            [JsonProperty("filename")]
            public string? Filename { get; set; }

            // obt/main/level_main_01-07
            [JsonProperty("levelId")]
            public string? LevelId { get; set; }

            // 暴君
            [JsonProperty("name")]
            public string? Name { get; set; }

            // main_01-07#f#, #f#是突袭关卡
            [JsonProperty("stageId")]
            public string? StageId { get; set; }

            [JsonProperty("height")]
            public int Height { get; set; }

            [JsonProperty("width")]
            public int Width { get; set; }
        }
    }
}
