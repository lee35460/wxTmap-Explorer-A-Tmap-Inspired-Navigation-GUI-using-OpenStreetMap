#include "ProjectMap.h"  // 🗺️ 프로젝트 파일 지도 사용
#include PRESENTATION_POLYLINE_HIGHLIGHT  // "presentation/components/PolylineHighlight.h"로 자동 확장
#include PRESENTATION_POLYLINE_STYLER
#include <chrono>

namespace presentation::components {

PolylineHighlightRenderer::PolylineHighlightRenderer(RenderPipeline& pipeline, const PolylineTheme& theme)
    : pipeline_(pipeline), theme_(theme) {
}

void PolylineHighlightRenderer::renderHighlightedPolyline(const std::vector<LonLat>& route, double progress) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 진행률에 따라 경로 분할
    auto [completed_segment, remaining_segment] = SplitPolylineByProgress(route, progress);
    
    // 완료된 구간을 하이라이트 색상으로 렌더링
    if (!completed_segment.empty()) {
        renderSegment(completed_segment, theme_.doneColor.GetRGB());
    }
    
    // 남은 구간을 일반 색상으로 렌더링
    if (!remaining_segment.empty()) {
        renderSegment(remaining_segment, theme_.remainColor.GetRGB());
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // 성능 메트릭 저장 (RenderPipeline에 메트릭 기능이 없으므로 로컬 변수로 저장)
    last_render_time_us_ = static_cast<double>(duration.count());
    last_segment_count_ = completed_segment.size() + remaining_segment.size();
}

void PolylineHighlightRenderer::updateTheme(const PolylineTheme& theme) {
    theme_ = theme;
}

void PolylineHighlightRenderer::renderSegment(const std::vector<LonLat>& segment, int color) {
    if (segment.empty()) return;
    
    // RenderPipeline의 submitPolyline 메서드 사용
    pipeline_.submitPolyline(segment);
}

} // namespace ui