#pragma once
#include <vector>
#include <chrono>

// Basic geographic coordinate struct
struct LonLat {
    double lon{};  // longitude in degrees
    double lat{};  // latitude in degrees
};

// Sliding window size for FPS averaging
constexpr size_t MAX_SAMPLES = 120;

class RenderPipeline {
public:
    void beginFrame();                              // start frame timing
    void submitPolyline(std::vector<LonLat> line);  // accept polyline input
    void endFrame();                                // end frame timing, record FPS
    double fpsAverage() const noexcept;             // average FPS over window

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point t0_{};
    std::vector<double> fps_samples_;
};