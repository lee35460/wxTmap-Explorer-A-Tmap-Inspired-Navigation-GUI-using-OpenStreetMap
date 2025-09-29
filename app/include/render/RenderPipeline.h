#pragma once
#include <vector>
#include <chrono>

#include <chrono>
struct LonLat {
    double lon{}; // longitude in degrees
    double lat{}; // latitude in degrees
};

constexpr size_t MAX_SAMPLES = 120;

class RenderPipeline {
public:
  void beginFrame();                              // 프레임 계측 시작
  void submitPolyline(std::vector<LonLat> line);  // 렌더 입력
  void endFrame();                                // 첫 페인트/프레임 시간 기록
  double fpsAverage() const noexcept;             // 평균 FPS 반환

private:
  using Clock = std::chrono::steady_clock;
  Clock::time_point t0_{};
  std::vector<double> fps_samples_;               // 최근 N 프레임 성능 샘플
};