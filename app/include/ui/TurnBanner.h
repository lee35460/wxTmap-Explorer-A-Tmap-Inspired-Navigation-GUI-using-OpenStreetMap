#pragma once
#include <wx/wx.h>
#include <chrono>
#include <string>

namespace ui {

struct TurnBannerState {
    bool visible = false;
    std::string instruction;     // "좌회전", "우회전" 등
    std::string icon;            // 아이콘 리소스 키 (예: "left", "right")
    double distance_m = 0.0;     // 남은 거리
    double progress = 0.0;       // 0.0~1.0
};

class TurnBanner : public wxPanel {
public:
    TurnBanner(wxWindow* parent);

    void UpdateState(const TurnBannerState& state);

private:
    TurnBannerState state_;
    wxStaticBitmap* icon_;
    wxStaticText* text_;
    wxGauge* progress_;  // 진행 바
    wxTimer animTimer_;

    void OnPaint(wxPaintEvent& evt);
    void OnAnimTick(wxTimerEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

} // namespace ui