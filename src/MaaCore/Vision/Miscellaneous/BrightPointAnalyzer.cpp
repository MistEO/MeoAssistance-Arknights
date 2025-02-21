#include "BrightPointAnalyzer.h"

#include "Utils/Logger.hpp"
#include "Utils/NoWarningCV.h"
#include "Utils/Ranges.hpp"

using namespace asst;

BrightPointAnalyzer::ResultsVecOpt BrightPointAnalyzer::analyze()
{
    const cv::Mat croppedImage = make_roi(m_image, m_roi);
    cv::Mat binaryImage;
    if (m_rgb_mode) {
        cv::Mat rgbImage;
        cv::cvtColor(croppedImage, rgbImage, cv::COLOR_BGR2RGB);
        cv::inRange(rgbImage, m_rgb_lb, m_rgb_ub, binaryImage);
    }
    else {
        cv::Mat grayImage;
        cv::cvtColor(croppedImage, grayImage, cv::COLOR_BGR2GRAY);
        cv::threshold(grayImage, binaryImage, m_lb, m_ub, cv::THRESH_BINARY);
    }
    std::vector<cv::Point> brightPoints;
    cv::findNonZero(binaryImage, brightPoints);

    if (brightPoints.empty()) {
        return std::nullopt;
    }

    auto transform_view = brightPoints | views::transform([](const cv::Point& p) { return Point(p.x, p.y); });
    ResultsVec results(ranges::begin(transform_view), ranges::end(transform_view));

    if (m_log_tracing) {
        Log.trace("analyze_bright_points | num:", results.size());
    }

    // FIXME: 老接口太难重构了，先弄个这玩意兼容下，后续慢慢全删掉
    m_result = std::move(results);
    return m_result;
}
