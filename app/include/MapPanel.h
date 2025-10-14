#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include <memory>
#include "render/RenderPipeline.h"  // LonLat 타입 사용을 위해
#include "ui/MapOverlayHud.h"
#include "ui/MapOverlayTheme.h"
#include "ui/PolylineStyler.h"

// Forward declarations for UI components
namespace ui {
    class TurnBanner;
    class NavigationProgressBar;
    class WaypointListPanel;
    class LocationPuck;
    class PolylineHighlightRenderer;
    class CameraController;
    struct PolylineTheme;
}

// 맵 패널: WebView + HUD + Polyline 렌더링
class MapPanel : public wxPanel {
public:
    explicit MapPanel(wxWindow* parent); // explicit 생성자: 단일 인자 생성자 방지
    void DrawPolyline(const std::vector<LonLat>& coords);

    // HUD 바인딩 및 제어
    void BindHudState(const ui::HudState& state) {
        hud_->SetState(state); // 스레드 안전
    }
    void SetHudMetricHook(ui::MetricFn fn) {
        hud_->SetMetricHook(std::move(fn));
    }
    void ToggleHudVisible() {
        hud_->ToggleVisible();
    }
private:
    // === wxWidgets 상속 UI 컴포넌트들 (화면 렌더링/사용자 입력 처리) ===
    ui::MapOverlayHud* hud_{nullptr}; // wxPanel 상속 - HUD 오버레이 렌더링
    ui::TurnBanner* turnBanner_{nullptr}; // wxPanel 상속 - 방향 안내 배너
    ui::NavigationProgressBar* progressBar_{nullptr}; // wxPanel 상속 - 진행률 표시
    ui::WaypointListPanel* waypointPanel_{nullptr}; // wxPanel 상속 - 경유지 리스트 UI

    // === 순수 비즈니스 로직 클래스들 (wxWidgets 비의존적) ===
    std::unique_ptr<ui::LocationPuck> locationPuck_; // 위치 마커 렌더링
    std::unique_ptr<ui::CameraController> cameraController_; // 카메라 follow 로직
    std::unique_ptr<ui::PolylineHighlightRenderer> polylineHighlight_; // 경로 하이라이트
    
    // === 데이터 멤버들 ===
    std::vector<LonLat> currentPolyline_;
    double routeProgress_{0.0};
    int zoomLevel_{15};
    double dpiScale_{1.0};
    ui::PolylineTheme polylineTheme_;

    void UpdateProgressFromGauge();
    void PublishPolylineStyle();
    std::string Hex(const wxColour& c) const;
};