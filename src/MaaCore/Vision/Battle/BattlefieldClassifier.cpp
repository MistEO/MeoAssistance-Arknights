#include "BattlefieldClassifier.h"

#include "Utils/NoWarningCV.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "Config/OnnxSessions.h"
#include "Config/TaskData.h"
#include "Utils/ImageIo.hpp"
#include "Utils/Logger.hpp"

using namespace asst;

BattlefieldClassifier::ResultOpt BattlefieldClassifier::analyze() const
{
    Result result { .object_of_interest = m_object_of_interest };
    bool analyzed = false;

    if (m_object_of_interest.skill_ready) {
        result.skill_ready = skill_ready_analyze();
        analyzed = true;
    }

    if (m_object_of_interest.deploy_direction) {
        result.deploy_direction = deploy_direction_analyze();
        analyzed = true;
    }

    if (!analyzed) {
        return std::nullopt;
    }

    return result;
}

BattlefieldClassifier::SkillReadyResult BattlefieldClassifier::skill_ready_analyze() const
{
    auto task_ptr = Task.get<MatchTaskInfo>("BattleSkillReady");
    const Rect& skill_roi_move = task_ptr->rect_move;
    Rect roi = Rect(m_base_point.x, m_base_point.y, 0, 0).move(skill_roi_move);

    cv::Mat image = make_roi(m_image, correct_rect(roi, m_image));

    // 1. 图像大小调整(推理慢可不做)
    cv::Mat resized_image;
    cv::resize(image, resized_image, cv::Size(72, 72));

    // 2. 中心裁剪(推理慢可不做)
    int crop_size = 64;
    int x = (resized_image.cols - crop_size) / 2;
    int y = (resized_image.rows - crop_size) / 2;
    cv::Rect crop_roi(x, y, crop_size, crop_size);
    cv::Mat cropped_image = resized_image(crop_roi);

    // 3. 图像转换为 tensor
    std::vector<float> input = image_to_tensor(cropped_image);

    // 4. 归一化
    float mean[] = { 0.485f, 0.456f, 0.406f };
    float std[] = { 0.229f, 0.224f, 0.225f };
    for (size_t i = 0; i < input.size(); i++) {
        int channel = i % 3;
        input[i] = (input[i] - mean[channel]) / std[channel];
    }

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    constexpr int64_t batch_size = 1;

    auto& session = OnnxSessions::get_instance().get("skill_ready_cls");

    std::array<int64_t, 4> input_shape { batch_size, cropped_image.channels(), cropped_image.cols, cropped_image.rows };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input.data(),
        input.size(),
        input_shape.data(),
        input_shape.size());

    SkillReadyResult::Raw raw_results;
    std::array<int64_t, 2> output_shape { batch_size, SkillReadyResult::ClsSize };
    Ort::Value output_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        raw_results.data(),
        raw_results.size(),
        output_shape.data(),
        output_shape.size());

    // 这俩是hardcode在模型里的
    constexpr const char* input_names[] = { "input" };   // session.GetInputName()
    constexpr const char* output_names[] = { "output" }; // session.GetOutputName()

    Ort::RunOptions run_options;
    session.Run(run_options, input_names, &input_tensor, 1, output_names, &output_tensor, 1);
    Log.info(__FUNCTION__, "raw results:", raw_results);

    SkillReadyResult::Prob prob = softmax(raw_results);
    Log.info(__FUNCTION__, "prob:", prob);
    // 类别顺序为 c, n, y
    size_t class_id = ranges::max_element(prob) - prob.begin();
    bool ready = (class_id == 2); // 只有当class_id为2（代表y）时，才认为是ready
    float score = prob[class_id];

#ifdef ASST_DEBUG
    // 在调试模式下，根据不同类别绘制不同颜色的标记
    if (class_id == 2) {
        // y类别：橙色
        rectangle(m_image_draw, make_rect<cv::Rect>(roi), cv::Scalar(0, 165, 255), 2);
        putText(m_image_draw, std::to_string(score), cv::Point(roi.x, roi.y - 10), 1, 1.2, cv::Scalar(0, 165, 255), 2);
    }
    else if (class_id == 0) { // c类别的特殊处理
        // 使用蓝色（BGR：255,0,0）标记c类别
        rectangle(m_image_draw, make_rect<cv::Rect>(roi), cv::Scalar(255, 0, 0), 2);
        putText(m_image_draw, std::to_string(score), cv::Point(roi.x, roi.y - 10), 1, 1.2, cv::Scalar(255, 0, 0), 2);
    }
#endif

    const auto result = SkillReadyResult {
        .ready = ready,
        .rect = roi,
        .score = score,
        .raw = raw_results,
        .prob = prob,
        .base_point = m_base_point,
    };

    if (!std::filesystem::exists("DEBUG_skill_ready.txt")) {
        return result;
    }

    // 为重新训练模型截图
    static Point last_base_point = { -1, -1 };
    static int last_class = -1; // 记录上一次的分类结果
    static auto last_save_time = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    const auto duration_since_last_save =
        std::chrono::duration_cast<std::chrono::seconds>(now - last_save_time).count();

    auto need_save = false;
    // 如果相同点且分类结果变化了，则保存
    if (last_base_point == m_base_point && last_class != static_cast<int>(class_id)) {
        need_save = true;
    }
    // 如果检测到新的基准点且结果为ready（y）或c类别，也保存
    else if (last_base_point != m_base_point && (class_id == 2 || class_id == 0)) {
        need_save = true;
    }
    // 来点随机截图
    else if (duration_since_last_save > 10) {
        last_save_time = now;
        need_save = true;
    }

    if (need_save) {
        std::string base_filename = utils::get_time_filestem() + "_" + std::to_string(m_base_point.x) + "_" +
                                    std::to_string(m_base_point.y) + "(c" + std::to_string(prob[0]) + ")(n" +
                                    std::to_string(prob[1]) + ")(y" + std::to_string(prob[2]) + ").png";
        std::string subfolder;
        switch (class_id) {
        case 2:
            subfolder = "y";
            break;
        case 1:
            subfolder = "n";
            break;
        case 0:
            subfolder = "c";
            break;
        default:
            subfolder = "unknown";
            break;
        }

        std::filesystem::path relative_path =
            utils::path("debug") / utils::path("skill_ready") / utils::path(subfolder) / base_filename;
        last_base_point = m_base_point;
        last_class = static_cast<int>(class_id);
        Log.trace("Save image", relative_path);
        asst::imwrite(relative_path, image);
    }

    return result;
}

BattlefieldClassifier::DeployDirectionResult BattlefieldClassifier::deploy_direction_analyze() const
{
    const auto& task_ptr = Task.get<MatchTaskInfo>("BattleDeployDirectionRectMove");
    const Rect& roi_move = task_ptr->rect_move;
    Rect roi = Rect(m_base_point.x, m_base_point.y, 0, 0).move(roi_move);

    cv::Mat image = make_roi(m_image, correct_rect(roi, m_image));
    std::vector<float> input = image_to_tensor(image);

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    constexpr int64_t batch_size = 1;
    std::array<int64_t, 4> input_shape { batch_size, image.channels(), image.cols, image.rows };

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input.data(),
        input.size(),
        input_shape.data(),
        input_shape.size());

    DeployDirectionResult::Raw raw_results;
    std::array<int64_t, 2> output_shape { batch_size, DeployDirectionResult::ClsSize };
    Ort::Value output_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        raw_results.data(),
        raw_results.size(),
        output_shape.data(),
        output_shape.size());

    auto& session = OnnxSessions::get_instance().get("deploy_direction_cls");
    // 这俩是hardcode在模型里的
    constexpr const char* input_names[] = { "input" };   // session.GetInputName()
    constexpr const char* output_names[] = { "output" }; // session.GetOutputName()

    Ort::RunOptions run_options;
    session.Run(run_options, input_names, &input_tensor, 1, output_names, &output_tensor, 1);
    Log.info(__FUNCTION__, "raw result:", raw_results);

    DeployDirectionResult::Prob prob = softmax(raw_results);
    Log.info(__FUNCTION__, "after softmax:", prob);

    size_t class_id = std::max_element(prob.begin(), prob.end()) - prob.begin();

#ifdef ASST_DEBUG
    static const std::unordered_map<size_t, std::string> ClassNames = {
        { 0, "Right" },
        { 1, "Down" },
        { 2, "Left" },
        { 3, "Up" },
    };
    if (ClassNames.size() != prob.size()) {
        Log.error("ClassNames.size() != prob.size()", ClassNames.size(), prob.size());
        throw std::runtime_error("ClassNames.size() != prob.size()");
    }
    cv::putText(
        m_image_draw,
        ClassNames.at(class_id),
        cv::Point(roi.x, roi.y + roi.height),
        cv::FONT_HERSHEY_PLAIN,
        1.2,
        cv::Scalar(0, 255, 0),
        2);
    cv::putText(
        m_image_draw,
        std::to_string(prob[class_id]),
        cv::Point(roi.x, roi.y + roi.height + 20),
        cv::FONT_HERSHEY_PLAIN,
        1.2,
        cv::Scalar(0, 255, 0),
        2);
#endif

    return DeployDirectionResult {
        .direction = static_cast<battle::DeployDirection>(class_id),
        .rect = roi,
        .score = prob[class_id],
        .raw = raw_results,
        .prob = prob,
        .base_point = m_base_point,
    };
}
