#include "InterfaceTask.h"

#include "Config/GeneralConfig.h"
#include "Utils/Logger.hpp"

bool asst::PackageTask::run()
{
    if (!m_enable) {
        Log.info("task disabled, pass", basic_info().to_string());
        return true;
    }
    m_running = true;

    const int task_delay = Config.get_options().task_delay;

    for (size_t i = 0; i != m_subtasks.size(); ++i) {
        if (need_exit()) {
            return false;
        }

        auto task_ptr = m_subtasks.at(i);
        if (!task_ptr->get_enable()) {
            continue;
        }
        Log.trace(__FUNCTION__, "| run subtask", i + 1, "/", m_subtasks.size(), task_ptr->basic_info().to_string());

        task_ptr->set_task_id(m_task_id);

        if (!task_ptr->run() && !task_ptr->get_ignore_error()) {
            return false;
        }

        if (i != m_subtasks.size() - 1) {
            sleep(task_delay);
        }
    }
    return true;
}

asst::AbstractTask& asst::PackageTask::set_retry_times(int times) noexcept
{
    AbstractTask::set_retry_times(times);
    for (auto&& sub : m_subtasks) {
        sub->set_retry_times(times);
    }
    return *this;
}

asst::AbstractTask& asst::PackageTask::set_task_id(int task_id) noexcept
{
    AbstractTask::set_task_id(task_id);
    for (auto&& sub : m_subtasks) {
        sub->set_task_id(task_id);
    }
    return *this;
}
