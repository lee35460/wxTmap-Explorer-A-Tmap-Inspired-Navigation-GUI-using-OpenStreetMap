#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include <memory>
#include "infrastructure/rendering/RenderPipeline.h"  // Clean layer access
#include "presentation/components/MapOverlayHud.h"
#include "presentation/components/MapOverlayTheme.h"
#include "presentation/components/PolylineStyler.h"

// Forward declarations for UI components  
namespace presentation::components {
    class TurnBanner;
    class NavigationProgressBar;
    class WaypointListPanel;
    class LocationPuck;
    class PolylineHighlightRenderer;
    class CameraController;
}

// 맵 패널: WebView + HUD + Polyline 렌더링
class MapPanel : public wxPanel {
public:
    explicit MapPanel(wxWindow* parent); // explicit 생성자: 단일 인자 생성자 방지
    void DrawPolyline(const std::vector<LonLat>& coords);

    // HUD 바인딩 및 제어
    void BindHudState(const presentation::components::HudState& state) {
        hud_->SetState(state); // 스레드 안전
    }
    void SetHudMetricHook(presentation::components::MetricFn fn) {
        hud_->SetMetricHook(std::move(fn));
    }
    void ToggleHudVisible() {
        hud_->ToggleVisible();
    }
private:
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    presentation::components::MapOverlayHud* hud_{nullptr}; // wxPanel 상속 - HUD 오버레이 렌더링
    presentation::components::TurnBanner* turnBanner_{nullptr}; // wxPanel 상속 - 방향 안내 배너
    presentation::components::NavigationProgressBar* progressBar_{nullptr}; // wxPanel 상속 - 진행률 표시
    presentation::components::WaypointListPanel* waypointPanel_{nullptr}; // wxPanel 상속 - 경유지 리스트 UI

    // === 순수 비즈니스 로직 클래스들 (wxWidgets 비의존적) ===
    std::unique_ptr<presentation::components::LocationPuck> locationPuck_; // 위치 마커 렌더링
    std::unique_ptr<presentation::components::CameraController> cameraController_; // 카메라 follow 로직
    std::unique_ptr<presentation::components::PolylineHighlightRenderer> polylineHighlight_; // 경로 하이라이트
    
    // === 데이터 멤버들 ===
    std::vector<LonLat> currentPolyline_;
    double routeProgress_{0.0};
    int zoomLevel_{15};
    double dpiScale_{1.0};
    presentation::components::PolylineTheme polylineTheme_;

    void UpdateProgressFromGauge();
    void PublishPolylineStyle();
    std::string Hex(const wxColour& c) const;
};