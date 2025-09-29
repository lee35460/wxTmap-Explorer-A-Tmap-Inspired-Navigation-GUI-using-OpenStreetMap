#include "render/RenderPipeline.h"
#include <numeric>
#include <algorithm>
#include <chrono>

void RenderPipeline::beginFrame() {
    t0_ = Clock::now();
}

void RenderPipeline::submitPolyline(std::vector<LonLat> /*line*/) {
    // TODO: polyline pre-processing (simplify/clip)
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