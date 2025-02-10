#include "InfrastTask.h"

#include "Utils/Logger.hpp"

#include "Task/Infrast/DronesForShamareTaskPlugin.h"
#include "Task/Infrast/InfrastControlTask.h"
#include "Task/Infrast/InfrastDormTask.h"
#include "Task/Infrast/InfrastInfoTask.h"
#include "Task/Infrast/InfrastMfgTask.h"
#include "Task/Infrast/InfrastOfficeTask.h"
#include "Task/Infrast/InfrastPowerTask.h"
#include "Task/Infrast/InfrastProcessingTask.h"
#include "Task/Infrast/InfrastReceptionTask.h"
#include "Task/Infrast/InfrastTradeTask.h"
#include "Task/Infrast/InfrastTrainingTask.h"
#include "Task/Infrast/ReplenishOriginiumShardTaskPlugin.h"
#include "Task/ProcessTask.h"

asst::InfrastTask::InfrastTask(const AsstCallback& callback, Assistant* inst) :
    InterfaceTask(callback, inst, TaskType),
    m_infrast_begin_task_ptr(std::make_shared<ProcessTask>(callback, inst, TaskType)),
    m_info_task_ptr(std::make_shared<InfrastInfoTask>(callback, inst, TaskType)),
    m_mfg_task_ptr(std::make_shared<InfrastMfgTask>(callback, inst, TaskType)),
    m_trade_task_ptr(std::make_shared<InfrastTradeTask>(callback, inst, TaskType)),
    m_power_task_ptr(std::make_shared<InfrastPowerTask>(callback, inst, TaskType)),
    m_control_task_ptr(std::make_shared<InfrastControlTask>(callback, inst, TaskType)),
    m_reception_task_ptr(std::make_shared<InfrastReceptionTask>(callback, inst, TaskType)),
    m_office_task_ptr(std::make_shared<InfrastOfficeTask>(callback, inst, TaskType)),
    m_processing_task_ptr(std::make_shared<InfrastProcessingTask>(callback, inst, TaskType)),
    m_training_task_ptr(std::make_shared<InfrastTrainingTask>(callback, inst, TaskType)),
    m_dorm_task_ptr(std::make_shared<InfrastDormTask>(callback, inst, TaskType))
{
    LogTraceFunction;

    m_infrast_begin_task_ptr->set_tasks({ "InfrastBegin" }).set_ignore_error(false);
    m_replenish_task_ptr = m_mfg_task_ptr->register_plugin<ReplenishOriginiumShardTaskPlugin>();
    m_info_task_ptr->set_ignore_error(true);
    m_mfg_task_ptr->set_ignore_error(true);
    m_trade_task_ptr->set_ignore_error(true);
    m_power_task_ptr->set_ignore_error(true);
    m_control_task_ptr->set_ignore_error(true);
    m_reception_task_ptr->set_ignore_error(true);
    m_office_task_ptr->set_ignore_error(true);
    m_training_task_ptr->set_ignore_error(true);
    m_processing_task_ptr->set_ignore_error(true);
    m_dorm_task_ptr->set_ignore_error(true);

    m_subtasks.emplace_back(m_infrast_begin_task_ptr);
}

bool asst::InfrastTask::set_params(const json::value& params)
{
    LogTraceFunction;

    int mode = params.get("mode", 0);
    bool is_custom = static_cast<Mode>(mode) == Mode::Custom;

    if (!m_running) {
        auto facility_opt = params.find<json::array>("facility");
        if (!facility_opt) {
            return false;
        }

        auto append_infrast_begin = [&]() {
            m_subtasks.emplace_back(m_infrast_begin_task_ptr);
        };

        m_subtasks.clear();
        append_infrast_begin();
        m_subtasks.emplace_back(m_info_task_ptr);

        for (const auto& facility_json : facility_opt.value()) {
            if (!facility_json.is_string()) {
                m_subtasks.clear();
                append_infrast_begin();
                return false;
            }

            std::string facility = facility_json.as_string();
            if (facility == "Dorm") {
                m_subtasks.emplace_back(m_dorm_task_ptr);
            }
            else if (facility == "Mfg") {
                m_subtasks.emplace_back(m_mfg_task_ptr);
            }
            else if (facility == "Trade") {
                m_subtasks.emplace_back(m_trade_task_ptr);
            }
            else if (facility == "Power") {
                m_subtasks.emplace_back(m_power_task_ptr);
            }
            else if (facility == "Office") {
                m_subtasks.emplace_back(m_office_task_ptr);
            }
            else if (facility == "Reception") {
                m_subtasks.emplace_back(m_reception_task_ptr);
            }
            else if (facility == "Control") {
                m_subtasks.emplace_back(m_control_task_ptr);
            }
            else if (facility == "Processing") {
                m_subtasks.emplace_back(m_processing_task_ptr);
            }
            else if (facility == "Training") {
                m_subtasks.emplace_back(m_training_task_ptr);
            }
            else {
                Log.error(__FUNCTION__, "| Unknown facility", facility);
                m_subtasks.clear();
                append_infrast_begin();
                return false;
            }
            append_infrast_begin();
        }
    }

    bool continue_training = params.get("continue_training", false);
    m_training_task_ptr->set_continue_training(continue_training);

    if (!is_custom) {
        std::string drones = params.get("drones", "_NotUse");
        m_mfg_task_ptr->set_uses_of_drone(drones);
        m_trade_task_ptr->set_uses_of_drone(drones);
        m_trade_task_ptr->register_plugin<DronesForShamareTaskPlugin>()->set_retry_times(0);
    }

    double threshold = params.get("threshold", 0.3);
    m_info_task_ptr->set_mood_threshold(threshold);
    m_mfg_task_ptr->set_mood_threshold(threshold);
    m_trade_task_ptr->set_mood_threshold(threshold);
    m_power_task_ptr->set_mood_threshold(threshold);
    m_control_task_ptr->set_mood_threshold(threshold);
    m_reception_task_ptr->set_mood_threshold(threshold);
    m_office_task_ptr->set_mood_threshold(threshold);
    m_processing_task_ptr->set_mood_threshold(threshold);
    m_dorm_task_ptr->set_mood_threshold(threshold);

    bool dorm_notstationed_enabled = params.get("dorm_notstationed_enabled", false);
    m_dorm_task_ptr->set_notstationed_enabled(dorm_notstationed_enabled);

    bool dorm_trust_enabled = params.get("dorm_trust_enabled", false);
    m_dorm_task_ptr->set_trust_enabled(dorm_trust_enabled);

    bool replenish = params.get("replenish", false);
    m_replenish_task_ptr->set_enable(replenish);

    bool prioritize_sending_clue = params.get("prioritize_sending_clue", false);
    int amount_of_clue_to_send = params.get("amount_of_clue_to_send", 3);
    bool send_clue_to_ocr = params.get("send_clue_to_ocr", false);
    bool only_send_clue_to_ocr = params.get("only_send_clue_to_ocr", false);
    std::string send_clue_list = params.get("send_clue_list", "");
    m_reception_task_ptr->set_clue_sending_config(
        prioritize_sending_clue,
        amount_of_clue_to_send,
        send_clue_to_ocr,
        only_send_clue_to_ocr,
        send_clue_list);

    if (is_custom && !m_running) {
        auto filename_opt = params.find<std::string>("filename");
        if (!filename_opt) {
            Log.error("filename is not set while custom mode is enabled");
            return false;
        }
        std::string filename = filename_opt.value();
        int index = params.get("plan_index", 0);

        try {
            return parse_and_set_custom_config(utils::path(filename), index);
        }
        catch (const json::exception& e) {
            Log.error("Json parse failed", utils::path(filename), e.what());
            return false;
        }
        catch (const std::exception& e) {
            Log.error("Json parse failed", utils::path(filename), e.what());
            return false;
        }
    }

    return true;
}

bool asst::InfrastTask::parse_and_set_custom_config(const std::filesystem::path& path, int index)
{
    LogTraceFunction;

    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
        Log.error("custom infrast file does not exist:", path);
        return false;
    }

    auto custom_json_opt = json::open(path);
    if (!custom_json_opt) {
        Log.error("failed to open json file:", path);
        return false;
    }
    auto& custom_json = custom_json_opt.value();
    Log.trace(__FUNCTION__, "| custom json:", custom_json.to_string());

    auto& all_plans = custom_json.at("plans").as_array();
    if (index < 0 || index >= int(all_plans.size())) {
        Log.error("index is out of range, plans size:", all_plans.size(), ", index:", index);
        return false;
    }
    auto& cur_plan = all_plans.at(index);

    // 录入干员编组
    std::unordered_map<std::string, std::vector<std::string>> ori_operator_groups;
    if (auto opt = cur_plan.find<json::array>("groups")) {
        for (const auto& group_info : opt.value()) {
            std::vector<std::string> oper_group;
            for (const auto& oper_info : group_info.at("operators").as_array()) {
                oper_group.emplace_back(oper_info.as_string());
            }
            ori_operator_groups.emplace(group_info.at("name").as_string(), std::move(oper_group));
        }
    }

    for (const auto& [facility, facility_info] : cur_plan.at("rooms").as_object()) {
        infrast::CustomFacilityConfig facility_config;

        for (const auto& room_info : facility_info.as_array()) {
            infrast::CustomRoomConfig room_config;
            room_config.skip = room_info.get("skip", false);
            room_config.autofill = room_info.get("autofill", false);
            room_config.sort = room_info.get("sort", false);
            room_config.use_operator_groups = room_info.get("use_operator_groups", false);

            static std::unordered_map<std::string, infrast::CustomRoomConfig::Product> ProductNames = {
                { "Battle Record", infrast::CustomRoomConfig::Product::BattleRecord },
                { "Pure Gold", infrast::CustomRoomConfig::Product::PureGold },
                { "Dualchip", infrast::CustomRoomConfig::Product::Dualchip },
                { "Originium Shard", infrast::CustomRoomConfig::Product::OriginiumShard },
                { "LMD", infrast::CustomRoomConfig::Product::LMD },
                { "Orundum", infrast::CustomRoomConfig::Product::Orundum },
            };
            std::string product = room_info.get("product", std::string());
            if (!product.empty()) {
                if (auto iter = ProductNames.find(product); iter != ProductNames.cend()) {
                    room_config.product = iter->second;
                }
                else {
                    Log.error("Unknown product", product);
                    return false;
                }
            }

            // 设置干员
            if (auto opers_opt = room_info.find<json::array>("operators")) {
                for (const auto& oper_name : opers_opt.value()) {
                    std::string name = oper_name.as_string();
                    if (name.empty()) {
                        Log.warn("operators.name is empty");
                        continue;
                    }
                    room_config.names.emplace_back(std::move(name));
                }
                if (room_config.use_operator_groups) {
                    // 将引用了的干员编组，装载到对应房间配置
                    // name数组此后可以作废
                    std::set<std::string> name_set;
                    name_set.insert(room_config.names.begin(), room_config.names.end());
                    ranges::for_each(
                        ori_operator_groups,
                        [name_set, &room_config](std::pair<std::string, std::vector<std::string>> pair) {
                            if (name_set.contains(pair.first)) {
                                room_config.operator_groups[pair.first] = std::move(pair.second);
                            }
                        });
                }
            }

            // 备选干员
            if (auto candidates_opt = room_info.find<json::array>("candidates")) {
                for (const auto& candidate_name : candidates_opt.value()) {
                    std::string name = candidate_name.as_string();
                    if (name.empty()) {
                        Log.warn("operators.candidates is empty");
                        continue;
                    }
                    room_config.candidates.emplace_back(std::move(name));
                }
            }
            facility_config.emplace_back(std::move(room_config));
        }

        // 不同类型建筑配置
        if (facility == "control") {
            m_control_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "manufacture") {
            m_mfg_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "trading") {
            m_trade_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "power") {
            m_power_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "meeting") {
            m_reception_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "hire") {
            m_office_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "processing") {
            m_processing_task_ptr->set_custom_config(facility_config);
        }
        else if (facility == "dormitory") {
            m_dorm_task_ptr->set_custom_config(facility_config);
        }
        else {
            Log.error(__FUNCTION__, "unknown facility", facility);
            return false;
        }
    }

    // 菲亚梅塔
    bool Fia_is_pre = false;
    do {
        auto Fia_opt = cur_plan.find<json::object>("Fiammetta");
        if (!Fia_opt) {
            break;
        }
        const auto& Fia_json = Fia_opt.value();
        bool enable = Fia_json.get("enable", true);
        if (!enable) {
            break;
        }
        std::string target = Fia_json.get("target", std::string());
        if (target.empty()) {
            Log.warn("Fiammetta's target is unsetted or empty");
            break;
        }
        Log.trace("Fiammetta's target:", target);

        static const std::string FiaName = "菲亚梅塔";

        // 如果肥鸭在换班前就在宿舍的第二个位置，直接换并不会让她的技能生效（位置没变，还在第二个位置）
        // 所以强制给他换个顺序
        infrast::CustomFacilityConfig pre_facility_config(1);
        auto& pre_room_config = pre_facility_config.front();
        pre_room_config.names = { target };
        auto pre_task_ptr = std::make_shared<InfrastDormTask>(m_callback, m_inst, TaskType);
        pre_task_ptr->set_custom_config(std::move(pre_facility_config));

        infrast::CustomFacilityConfig facility_config(1);
        auto& room_config = facility_config.front();
        room_config.names = { target, FiaName };
        room_config.sort = true;

        auto Fia_task_ptr = std::make_shared<InfrastDormTask>(m_callback, m_inst, TaskType);
        Fia_task_ptr->set_custom_config(std::move(facility_config));

        if (Fia_json.get("order", "pre") != "post") {
            m_subtasks.insert(m_subtasks.begin(), { m_infrast_begin_task_ptr, pre_task_ptr, Fia_task_ptr });
            Fia_is_pre = true;
        }
        else {
            m_subtasks.insert(m_subtasks.end(), { pre_task_ptr, Fia_task_ptr, m_infrast_begin_task_ptr });
        }
    } while (false);

    // 无人机
    do {
        auto drones_opt = cur_plan.find<json::object>("drones");
        if (!drones_opt) {
            break;
        }
        const auto& drones_json = drones_opt.value();
        bool enable = drones_json.get("enable", true);
        if (!enable) {
            break;
        }
        infrast::CustomDronesConfig drones_config;
        drones_config.index = drones_json.get("index", 1) - 1;
        drones_config.order = drones_json.get("order", "pre") == "post" ? infrast::CustomDronesConfig::Order::Post
                                                                        : infrast::CustomDronesConfig::Order::Pre;
        bool additional_advance_drones = Fia_is_pre && drones_config.order == infrast::CustomDronesConfig::Order::Pre;
        std::string room = drones_json.get("room", std::string());
        if (room.empty()) {
            Log.warn("drones room is unsetted or empty");
            break;
        }
        else if (room != "trading" && room != "manufacture") {
            Log.error("error drones config, unknown room", room);
            return false;
        }

        // 无人机和肥鸭同时为true时，优先使用无人机
        if (additional_advance_drones) {
            std::shared_ptr<InfrastProductionTask> drones_only_task_ptr = nullptr;
            if (room == "trading") {
                drones_only_task_ptr = std::make_shared<InfrastTradeTask>(m_callback, m_inst, TaskType);
            }
            else if (room == "manufacture") {
                drones_only_task_ptr = std::make_shared<InfrastMfgTask>(m_callback, m_inst, TaskType);
            }

            drones_only_task_ptr->set_custom_config(
                infrast::CustomFacilityConfig(drones_config.index + 1, infrast::CustomRoomConfig { .skip = true }));
            drones_only_task_ptr->set_custom_drones_config(std::move(drones_config));
            m_subtasks.insert(m_subtasks.begin(), { m_infrast_begin_task_ptr, drones_only_task_ptr });
        }
        else {
            if (room == "trading") {
                m_trade_task_ptr->set_custom_drones_config(std::move(drones_config));
            }
            else if (room == "manufacture") {
                m_mfg_task_ptr->set_custom_drones_config(std::move(drones_config));
            }
        }
    } while (false);

    return true;
}
