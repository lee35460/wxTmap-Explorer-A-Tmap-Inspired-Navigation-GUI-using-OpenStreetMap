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
    void beginFrame();                                      // start frame timing
    void submitPolyline(const std::vector<LonLat>& line);   // accept polyline input
    void endFrame();                                        // end frame timing, record FPS
    double fpsAverage() const noexcept;                     // average FPS over window
    double lastPolylineDrawMs() const noexcept { return last_draw_ms_; }
    std::size_t drawCalls() const noexcept { return draw_calls_; }
    void resetDrawStats() noexcept { last_draw_ms_ = 0.0; draw_calls_ = 0; }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point t0_{};
    std::vector<double> fps_samples_;
    double last_draw_ms_{0.0};
    std::size_t draw_calls_{0};
};