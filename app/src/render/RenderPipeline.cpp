#include "render/RenderPipeline.h"
#include <numeric>
#include <algorithm>

void RenderPipeline::beginFrame(){
    t0_ = Clock::now();
}

void RenderPipeline::submitPolyline(std::vector<LonLat> /*line*/){
    auto dt = std::chrono::duration<double>(Clock::now() - t0_).count();
    if (dt > 0.0) {
        fps_samples_.push_back(1.0 / dt);
        if (fps_samples_.size() > MAX_SAMPLES ) fps_samples_.erase(fps_samples_.begin());
    }
}

double RenderPipeline::fpsAverage() const noexcept {
    if (fps_samples_.empty()) return 0.0;
    double sum = std::accumulate(fps_samples_.begin(), fps_samples_.end(), 0.0);
    return sum / fps_samples_.size();
}

#ifdef UNIT_TEST_RENDER
#include <thread>
#include <cassert>
#include <iostream>

int main() {
    RenderPipeline rp;
    for (int i = 0; i < 100; ++i) {
        rp.beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rp.endFrame();
    }
    double avg = rp.fpsAverage();
    std::cout << "[TEST] avg_fps=" << avg << "\n";
    assert(avg >= 45.0);
    return 0;
}
#endif