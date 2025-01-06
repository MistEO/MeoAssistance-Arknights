#include "RoguelikeIterateMonthlySquadPlugin.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"

bool asst::RoguelikeIterateMonthlySquadPlugin::load_params([[maybe_unused]] const json::value& params)
{
    LogTraceFunction;

    checkComms = params.find<bool>("monthly_squad_check_comms").value_or(false);
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
    const auto& mode = m_config->get_mode();
    const std::string& task = details.get("details", "task", "");
    std::string_view task_view = task;
    if (task_view.starts_with(roguelike_name)) {
        task_view.remove_prefix(roguelike_name.length());
    }
    if (task_view == "Roguelike@StartExplore" && mode == RoguelikeMode::Squad) {
        return true;
    }
    else {
        return false;
    }
}

bool asst::RoguelikeIterateMonthlySquadPlugin::_run()
{
    LogTraceFunction;

    completed = true;
    if (monthlySquadCount[m_config->get_theme()] > 0) {
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@ChooseMonthlySquad" }).run();
    }

    for (int i = 0; i < monthlySquadCount[m_config->get_theme()]; i++) {
        if (checkComms) {
            ProcessTask(*this, { m_config->get_theme() + "@Roguelike@MonthlySquadComms" }).run();
            if (!ProcessTask(*this, { m_config->get_theme() + "@Roguelike@MonthlySquadCommsMiss" }).run()) {
                ProcessTask(*this, { "Roguelike@MonthlySquadCommsBackTwice" }).run();
                completed = false;
                break;
            }
        }

        if (!ProcessTask(*this, { m_config->get_theme() + "@Roguelike@MonthlySquadRewardMiss" }).run()) {
            completed = false;
            break;
        }
    }
    if (completed) {
        ProcessTask(*this, { m_config->get_theme() + "@Roguelike@MonthlySquadComplain" }).run();
        m_task_ptr->set_enable(false);
    }

    return true;
}
