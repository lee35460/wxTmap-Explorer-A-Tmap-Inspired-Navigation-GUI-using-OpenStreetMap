#pragma once
#include "render/RenderPipeline.h"
#include "ui/PolylineStyler.h"
#include <vector>

namespace ui {

// 폴리라인 하이라이트 테마 설정
struct PolylineTheme {
    int highlightColor = 0x00FF00;   // 하이라이트 색상 (녹색)
    int normalColor = 0x0080FF;      // 일반 색상 (파란색)
    float highlightThickness = 8.0f; // 하이라이트 두께
    float normalThickness = 4.0f;    // 일반 두께
};

// 폴리라인 하이라이트 렌더러
class PolylineHighlightRenderer {
public:
    PolylineHighlightRenderer(RenderPipeline& pipeline, const PolylineTheme& theme);
    
    // 하이라이트된 폴리라인 렌더링
    void renderHighlightedPolyline(const std::vector<LonLat>& route, double progress);
    
    // 스타일 테마 업데이트
    void updateTheme(const PolylineTheme& theme);
    
    // 성능 메트릭 접근
    double getLastRenderTimeUs() const { return last_render_time_us_; }
    size_t getLastSegmentCount() const { return last_segment_count_; }
    
private:
    RenderPipeline& pipeline_;
    PolylineTheme theme_;
    
    // 성능 메트릭 저장
    double last_render_time_us_{0.0};
    size_t last_segment_count_{0};
    
    void renderSegment(const std::vector<LonLat>& segment, int color);
};

} // namespace ui