#pragma once
#include "render/RenderPipeline.h"
#include "ui/PolylineStyler.h"
#include "ui/MapOverlayTheme.h"
#include <vector>

namespace ui {

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