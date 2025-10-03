#pragma once
#include <wx/wx.h>
#include <vector>
#include "render/RenderPipeline.h"  // LonLat 타입 사용을 위해
#include "ui/MapOverlayHud.h"

class MapPanel : public wxPanel {
public:
    explicit MapPanel(wxWindow* parent);
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
    ui::MapOverlayHud* hud_;
};