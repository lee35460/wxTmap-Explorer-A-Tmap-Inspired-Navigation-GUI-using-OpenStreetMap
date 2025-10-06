#include "render/RenderPipeline.h"
#include <numeric>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>

void RenderPipeline::beginFrame() {
    t0_ = Clock::now();
}

void RenderPipeline::submitPolyline(const std::vector<LonLat>& line) {
    const auto tStart = Clock::now();
    double length = 0.0;
    for (std::size_t i = 1; i < line.size(); ++i) {
        const double dx = line[i].lon - line[i - 1].lon;
        const double dy = line[i].lat - line[i - 1].lat;
        length += std::hypot(dx, dy);
    }
    (void)length; // placeholder for future use
    last_draw_ms_ = std::chrono::duration<double, std::milli>(Clock::now() - tStart).count();
    ++draw_calls_;
}

void RenderPipeline::endFrame() {
    const auto dt = std::chrono::duration<double>(Clock::now() - t0_).count();
    if (dt > 0.0) {
        const double fps = 1.0 / dt;
        fps_samples_.push_back(fps);
        if (fps_samples_.size() > MAX_SAMPLES) {
            fps_samples_.erase(fps_samples_.begin());
        }
    }
}

double RenderPipeline::fpsAverage() const noexcept {
    if (fps_samples_.empty()) return 0.0;
    const double sum = std::accumulate(fps_samples_.begin(), fps_samples_.end(), 0.0);
    return sum / static_cast<double>(fps_samples_.size());
}