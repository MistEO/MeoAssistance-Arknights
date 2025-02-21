// <copyright file="CopilotHelper.cs" company="MaaAssistantArknights">
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
using System.Threading.Tasks;
using MaaWpfGui.Constants;
using MaaWpfGui.Models;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace MaaWpfGui.Helper;

public static class CopilotHelper
{
    public static async Task<(PrtsStatus Status, PrtsCopilotModel? Copilot)> RequestCopilotAsync(int copilotId)
    {
        var jsonResponse = await Instances.HttpService.GetStringAsync(new Uri(MaaUrls.PrtsPlusCopilotGet + copilotId)) ?? string.Empty;
        if (jsonResponse is null)
        {
            return (PrtsStatus.NetworkError, null);
        }

        var json = JsonConvert.DeserializeObject<PrtsCopilotModel>(jsonResponse);
        if (json != null && json.StatusCode == 200)
        {
            return (PrtsStatus.Success, json);
        }

        return (PrtsStatus.NotFound, null);
    }

    public static async Task<(PrtsStatus Status, PrtsCopilotSetModel? CopilotSet)> RequestCopilotSetAsync(int copilotId)
    {
        var jsonResponse = await Instances.HttpService.GetStringAsync(new Uri(MaaUrls.PrtsPlusCopilotSetGet + copilotId)) ?? string.Empty;
        if (jsonResponse is null)
        {
            return (PrtsStatus.NetworkError, null);
        }

        var json = JsonConvert.DeserializeObject<PrtsCopilotSetModel>(jsonResponse);
        if (json != null && json.StatusCode == 200)
        {
            return (PrtsStatus.Success, json);
        }

        return (PrtsStatus.NotFound, null);
    }

    public static async Task<PrtsStatus> RateWebJsonAsync(int copilotId, string rating)
    {
        var response = await Instances.HttpService.PostAsJsonAsync(new Uri(MaaUrls.PrtsPlusCopilotRating), new { id = copilotId, rating });
        if (response == null)
        {
            return PrtsStatus.NetworkError;
        }

        return PrtsStatus.Success;
    }

    public enum PrtsStatus
    {
        /// <summary>
        /// 成功
        /// </summary>
        Success,

        /// <summary>
        /// 未找到
        /// </summary>
        NotFound,

        /// <summary>
        /// 网络错误
        /// </summary>
        NetworkError,
    }

    public class PrtsCopilotModel
    {
        [JsonProperty("status_code")]
        public int StatusCode { get; set; }

        [JsonProperty("data")]
        public CopilotData? Data { get; set; } = new();

        public class CopilotData
        {
            [JsonProperty("id")]
            public int Id { get; set; }

            [JsonProperty("upload_time")]
            public DateTime UploadTime { get; set; }

            [JsonProperty("uploader_id")]
            public string UploaderId { get; set; } = string.Empty;

            [JsonProperty("uploader")]
            public string Uploader { get; set; } = string.Empty;

            [JsonProperty("views")]
            public int Views { get; set; }

            [JsonProperty("hot_score")]
            public double HotScore { get; set; }

            [JsonProperty("available")]
            public bool Available { get; set; }

            [JsonProperty("rating_level")]
            public int RatingLevel { get; set; }

            [JsonProperty("not_enough_rating")]
            public bool NotEnoughRating { get; set; }

            [JsonProperty("rating_ratio")]
            public double RatingRatio { get; set; }

            [JsonProperty("rating_type")]
            public int RatingType { get; set; }

            [JsonProperty("comments_count")]
            public int CommentsCount { get; set; }

            [JsonProperty("like")]
            public int Like { get; set; }

            [JsonProperty("dislike")]
            public int Dislike { get; set; }

            [JsonProperty("content")]
            [JsonConverter(typeof(CopilotContentConverter))]
            public object? Content { get; set; } = new();
        }
    }

    public class PrtsCopilotSetModel
    {
        [JsonProperty("status_code")]
        public int StatusCode { get; set; }

        /// <summary>
        /// Gets or sets 输出提示，code 200时不存在该项
        /// <para>e.g.: code=400, msg=作业不存在</para>
        /// </summary>
        [JsonProperty("message")]
        public string? Message { get; set; }

        [JsonProperty("data")]
        public CopilotSetData? Data { get; set; } = new();

        public class CopilotSetData
        {
            [JsonProperty("id")]
            public int Id { get; set; }

            [JsonProperty("name")]
            public string Name { get; set; } = string.Empty;

            [JsonProperty("description")]
            public string Description { get; set; } = string.Empty;

            [JsonProperty("copilot_ids")]
            public List<int> CopilotIds { get; set; } = [];

            [JsonProperty("creator_id")]
            public string CreatorId { get; set; } = string.Empty;

            [JsonProperty("creator")]
            public string Creator { get; set; } = string.Empty;

            [JsonProperty("create_time")]
            public DateTime CreatedTime { get; set; }

            [JsonProperty("update_time")]
            public DateTime UpdatedTime { get; set; }

            [JsonProperty("status")]
            public string Status { get; set; } = string.Empty;
        }
    }

    public class CopilotContentConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType) => objectType == typeof(object);

        public override object? ReadJson(JsonReader reader, Type objectType, object? existingValue, JsonSerializer serializer)
        {
            // 处理字符串或对象两种输入形式
            JToken token = JToken.Load(reader);
            if (token.Type == JTokenType.String)
            {
                // 如果 Content 是字符串，解析为嵌套 JSON
                string jsonString = token.Value<string>() ?? string.Empty;
                token = JToken.Parse(jsonString);
            }

            if (token is JObject obj)
            {
                // 根据 "type" 字段判断类型
                if (obj.TryGetValue("type", StringComparison.OrdinalIgnoreCase, out var typeToken) && typeToken.ToString() == "SSS")
                {
                    return obj.ToObject<SSSCopilotModel>(serializer);
                }
                else
                {
                    return obj.ToObject<CopilotModel>(serializer);
                }
            }

            throw new JsonSerializationException("Unsupported JSON structure for Content");
        }

        public override void WriteJson(JsonWriter writer, object? value, JsonSerializer serializer)
        {
            writer.WriteValue(JsonConvert.SerializeObject(value));
        }
    }
}
