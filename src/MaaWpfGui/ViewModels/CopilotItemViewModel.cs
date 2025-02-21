// <copyright file="CopilotItemViewModel.cs" company="MaaAssistantArknights">
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

using MaaWpfGui.Helper;
using Newtonsoft.Json;
using Stylet;

namespace MaaWpfGui.ViewModels
{
    [JsonObject(MemberSerialization.OptIn)]
    public class CopilotItemViewModel : PropertyChangedBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CopilotItemViewModel"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="filePath">The original Name of file</param>
        /// <param name="isRaid">是否为突袭关</param>
        /// <param name="copilotId">作业站对应id，本地作业应为默认值0</param>
        /// <param name="isChecked">isChecked</param>
        public CopilotItemViewModel(string name, string filePath, bool isRaid = false, int copilotId = 0, bool isChecked = true)
        {
            Name = name;
            FilePath = filePath;
            _isRaid = isRaid;
            CopilotId = copilotId;
            _isChecked = isChecked;
        }

        /// <summary>
        /// Gets the name.
        /// </summary>
        [JsonProperty("name")]
        public string Name { get; }

        /// <summary>
        /// Gets the original_name.
        /// </summary>
        [JsonProperty("file_path")]
        public string FilePath { get; }

        /// <summary>
        /// Gets or sets 作业站对应id，本地作业应为默认值0
        /// </summary>
        [JsonProperty("copilot_id")]
        public int CopilotId { get; set; }

        [JsonProperty("is_raid")]
        private bool _isRaid;

        /// <summary>
        /// Gets or sets a value indicating whether 突袭关
        /// </summary>
        public bool IsRaid
        {
            get => _isRaid;
            set
            {
                SetAndNotify(ref _isRaid, value);
                Instances.CopilotViewModel.SaveCopilotTask();
            }
        }

        [JsonProperty("is_checked")]
        private bool _isChecked;

        /// <summary>
        /// Gets or sets a value indicating whether gets or sets whether the key is checked.
        /// </summary>
        public bool IsChecked
        {
            get => _isChecked;
            set
            {
                SetAndNotify(ref _isChecked, value);
                Instances.CopilotViewModel.SaveCopilotTask();
            }
        }

        private int _index;

        public int Index
        {
            get => _index;
            set => SetAndNotify(ref _index, value);
        }
    }
}
