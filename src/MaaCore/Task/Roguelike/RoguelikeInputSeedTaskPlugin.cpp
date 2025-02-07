#include "RoguelikeInputSeedTaskPlugin.h"

#include "Config/Roguelike/RoguelikeStageEncounterConfig.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"

bool asst::RoguelikeInputSeedTaskPlugin::load_params([[maybe_unused]] const json::value& params)
{
    // 本插件暂仅用于萨卡兹种子刷钱
    return m_config->get_theme() == RoguelikeTheme::Sarkaz && params.get("start_with_seed", false);
}

bool asst::RoguelikeInputSeedTaskPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
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
    return false;
}

bool asst::RoguelikeInputSeedTaskPlugin::_run()
{
    return ProcessTask(*this, { m_config->get_theme() + "@Roguelike@StartExploreWithSeed" })
        .set_times_limit("Roguelike@StartExplore", 0)
        .run();
}
