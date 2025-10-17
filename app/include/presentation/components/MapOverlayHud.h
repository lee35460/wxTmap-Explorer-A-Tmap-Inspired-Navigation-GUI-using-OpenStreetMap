#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <functional>
#include <string>
#include <chrono>
#include <mutex>

namespace presentation::components {

enum class HudTheme { Day, Night, HighContrast };

struct HudState {
    bool visible = true;                       // HUD 표시/숨김
    double speed_kmh = 0.0;                    // 현재 속도
    double speed_limit_kmh = 0.0;              // 제한 속도
    double distance_remain_m = 0.0;            // 남은 거리(m)
    std::chrono::system_clock::time_point eta; // 도착 예상 시간
    HudTheme theme = HudTheme::Day;            // 테마: 주간/야간/고대비
    double font_scale = 1.0;                   // 접근성(크기 확장)
};

// 메트릭 훅: (key, value)
using MetricFn = std::function<void(const std::string&, double)>;

class MapOverlayHud : public wxPanel {
public:
    explicit MapOverlayHud(wxWindow* parent,
                           wxWindowID id = wxID_ANY,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize);

    void SetState(const HudState& s);          // 상태 바인딩(스레드 안전)
    void ToggleVisible();                      // 표시/숨김 토글
    void SetMetricHook(MetricFn fn);           // 메트릭 콜백 주입

    static void DrawHudStatic(wxDC& dc, const HudState& state, int dpi_scale);

protected:
    void OnPaint(wxPaintEvent& e);
    void OnSize(wxSizeEvent& e);

private:
    // 렌더 보조
    void DrawHud(wxBufferedPaintDC& dc);
    void ResolveTheme(wxColour& fg, wxColour& fgSecondary, wxColour& bg, wxColour& accent) const;
    int  Px(double logicalPx) const;           // HiDPI 스케일 적용
    wxString FormatSpeed(double kmh) const;
    wxString FormatSpeedLimit(double kmh) const;
    wxString FormatDistance(double m) const;
    wxString FormatETA(std::chrono::system_clock::time_point eta) const;

private:
    mutable std::mutex mtx_;
    HudState state_;
    MetricFn metric_;

    // 페인트 메트릭
    std::chrono::steady_clock::time_point last_paint_start_{};
    bool first_paint_done_{false};

    // 레이아웃 캐시
    int dpi_scale_ = 1;

    wxDECLARE_EVENT_TABLE();
};

} // namespace presentation::components