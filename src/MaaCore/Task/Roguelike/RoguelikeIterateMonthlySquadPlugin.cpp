#include "RoguelikeIterateMonthlySquadPlugin.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"

bool asst::RoguelikeIterateMonthlySquadPlugin::load_params([[maybe_unused]] const json::value& params)
{
    LogTraceFunction;

    auto iterateMS = params.find<bool>("monthly_squad_auto_iterate");
    return iterateMS.value_or(false);
}

bool asst::RoguelikeIterateMonthlySquadPlugin::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    if (!RoguelikeConfig::is_valid_theme(m_config->get_theme())) {
        Log.error("Roguelike name doesn't exist!");
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

bool asst::RoguelikeIterateMonthlySquadPlugin::_run()
{
    LogTraceFunction;

    // 检查主题，读月度小队数量
    if (m_config->get_theme() == "Phantom" || m_config->get_theme() == "Mizuki" || m_config->get_theme() == "Sami") {
        monthly_squad_count = 8;
    }
    else {
        monthly_squad_count = 1;
    }

    if (monthly_squad_count > 0) {
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseMonthlySquad" }).run();
    }

    // todo: 检查月度小队通信
    while (monthly_squad_count > 0) {
        monthly_squad_count--;
        if (!ProcessTask(*this, { m_config->get_theme() + "@Roguelike@MonthlySquadRewardMiss" }).run()) {
            break;
        }
    }

    // todo: 全部结束直接停止
    // todo: actually complain in wpf
    ProcessTask(*this, { m_config->get_theme() + "@Roguelike@MonthlySquadComplete" }).run();

    return true;
}
