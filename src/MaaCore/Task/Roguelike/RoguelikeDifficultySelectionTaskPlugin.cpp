#include "RoguelikeDifficultySelectionTaskPlugin.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Status.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/OCRer.h"

bool asst::RoguelikeDifficultySelectionTaskPlugin::load_params([[maybe_unused]] const json::value& params)
{
    LogTraceFunction;

    // 集成战略 <傀影与猩红孤钻> 的难度选项没有数字标注，暂不支持难度选择功能
    if (m_config->get_theme() == RoguelikeTheme::Phantom) {
        return false;
    }

    auto opt = params.find<int>("difficulty");
    return opt && *opt != -1;
}

bool asst::RoguelikeDifficultySelectionTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    if (!RoguelikeConfig::is_valid_theme(m_config->get_theme())) {
        Log.error("Roguelike name doesn't exist!");
        return false;
    }

    const auto& mode = m_config->get_mode();
    if (mode == RoguelikeMode::Exploration || mode == RoguelikeMode::Squad) {
        return false;
    }

    const std::string roguelike_name = m_config->get_theme() + "@";
    const std::string& task = details.get("details", "task", "");

    std::string_view task_view = task;
    if (task_view.starts_with(roguelike_name)) {
        task_view.remove_prefix(roguelike_name.length());
    }
    if (task_view == "Roguelike@StartExplore") {
        return true;
    }
    else {
        return false;
    }
}

bool asst::RoguelikeDifficultySelectionTaskPlugin::_run()
{
    LogTraceFunction;

    if (m_config->get_run_for_collectible()) {
        Log.info(__FUNCTION__, "| Running for collectible");
    }

    const int difficulty = m_config->get_run_for_collectible() ? 0 : m_config->get_difficulty();
    Log.info(__FUNCTION__, "| current_difficulty:", m_current_difficulty, "next difficulty:", difficulty);

    // 仅在插件记录的当前难度与目标难度不一致时重新选择难度
    if (m_current_difficulty != difficulty) {
        select_difficulty(difficulty);
    }

    return true;
}

bool asst::RoguelikeDifficultySelectionTaskPlugin::select_difficulty(const int difficulty)
{
    LogTraceFunction;

    if (difficulty == INT_MAX) {
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseDifficulty_Hardest" }).run();
    }
    else if (difficulty == 0) {
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseDifficulty_Easiest" }).run();
    }
    else {
        // 从最高难度依次点下来
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseDifficulty_Hardest" }).run();
        std::vector<std::string> difficulty_list;
        for (int i = 20; i >= difficulty; --i) { // 难度识别内容为 20 ~ difficulty
            difficulty_list.push_back(std::to_string(i));
        }
        Task.get<OcrTaskInfo>(m_config->get_theme() + "@Roguelike@ChooseDifficulty_Specified")->text = difficulty_list;
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseDifficulty_Specified", "Stop" }).run();
    }

    // 识别当前难度
    const cv::Mat image = ctrler()->get_image();
    OCRer current_difficulty_analyzer(image);
    current_difficulty_analyzer.set_task_info("Roguelike@ChooseDifficulty_AnalyzeCurrentDifficulty");
    if (current_difficulty_analyzer.analyze()) {
        const std::string current_difficulty_text = current_difficulty_analyzer.get_result().front().text;
        Log.info(__FUNCTION__, "| Current difficulty text is", current_difficulty_text);
        if (!utils::chars_to_number(current_difficulty_text, m_current_difficulty)) {
            Log.error("Fail to convert current difficulty text to int, reset current difficulty to 0");
            m_current_difficulty = 0;
        }
    }
    else {
        Log.error(__FUNCTION__, "| Fail to detect current difficulty, reset current difficulty to 0");
        m_current_difficulty = 0;
    }
    Log.info(__FUNCTION__, "| Current difficulty is", m_current_difficulty);

    ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseDifficultyConfirm" }).run();

    return true;
}
