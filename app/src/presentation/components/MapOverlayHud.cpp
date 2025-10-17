#include "ProjectMap.h" 
#include PRESENTATION_MAP_OVERLAY_HUD
#include <wx/datetime.h>
#include <iomanip>
#include <sstream>


namespace presentation::components {

wxBEGIN_EVENT_TABLE(MapOverlayHud, wxPanel)
    EVT_PAINT(MapOverlayHud::OnPaint)
    EVT_SIZE(MapOverlayHud::OnSize)
wxEND_EVENT_TABLE()

MapOverlayHud::MapOverlayHud(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT); // 더블버퍼 페인트
#if defined(__WXMAC__)
    dpi_scale_ = std::max(1, int(std::round(GetContentScaleFactor())));
#else
    dpi_scale_ = 1;
#endif
}

void MapOverlayHud::SetState(const HudState& s) {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        state_ = s;
    }
    Refresh(); // 다시 그리기
}

void MapOverlayHud::ToggleVisible() {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        state_.visible = !state_.visible;
    }
    if (metric_) metric_("hud_visible", state_.visible ? 1.0 : 0.0);
    Refresh();
}

void MapOverlayHud::SetMetricHook(MetricFn fn) {
    std::lock_guard<std::mutex> lk(mtx_);
    metric_ = std::move(fn);
}

void MapOverlayHud::DrawHudStatic(wxDC& dc, const HudState& state, int dpi_scale) {
    wxColour fg, fg2, bg, accent;
    switch (state.theme) {
        case HudTheme::Day:
            bg = wxColour(255,255,255,230);
            fg = *wxBLACK;
            fg2 = wxColour(70,70,70);
            accent = wxColour(0,122,255);
            break;
        case HudTheme::Night:
            bg = wxColour(20,20,20,200);
            fg = *wxWHITE;
            fg2 = wxColour(180,180,180);
            accent = wxColour(10,132,255);
            break;
        case HudTheme::HighContrast:
            bg = wxColour(0,0,0,230);
            fg = *wxWHITE;
            fg2 = wxColour(220,220,0);
            accent = wxColour(255,215,0);
            break;
    }

    // 배경
    wxSize sz = dc.GetSize();
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(bg));
    dc.DrawRectangle({0,0}, sz);

    if (!state.visible) {
        // 숨김 상태면 아무것도 그리지 않음
        return;
    }

    // HiDPI 및 접근성 스케일 적용
    auto Px = [&](double logicalPx) {
        return int(std::round(logicalPx * dpi_scale * std::max(0.75, state.font_scale)));
    };

    // 텍스트(큰 정보와 보조 정보 계층화)
    wxFont big(Px(32), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    wxFont small(Px(16), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    dc.SetTextForeground(fg);

    // 속도/제한속도
    int speed_v = int(std::round(state.speed_kmh));
    wxString speed = wxString::Format("%d km/h", speed_v);

    wxString limit = (state.speed_limit_kmh <= 0.0)
        ? "—"
        : wxString::Format("제한 %d", int(std::round(state.speed_limit_kmh)));

    dc.SetFont(big);
    dc.DrawText(speed, Px(16), Px(12));

    dc.SetFont(small);
    dc.SetTextForeground(fg2);
    dc.DrawText(limit, Px(16), Px(52));

    // 하단: ETA / 남은 거리
    dc.SetTextForeground(fg);
    dc.SetFont(small);

    wxString etaStr;
    if (state.eta.time_since_epoch().count() == 0) {
        etaStr = "ETA —";
    } else {
        std::time_t t = std::chrono::system_clock::to_time_t(state.eta);
        wxDateTime dt((time_t)t);
        etaStr = wxString::Format("ETA %s", dt.FormatISOTime());
    }

    wxString distStr;
    if (state.distance_remain_m >= 1000.0) {
        double km = state.distance_remain_m / 1000.0;
        distStr = wxString::Format("%.1f km", km);
    } else {
        distStr = wxString::Format("%d m", int(std::round(state.distance_remain_m)));
    }

    auto eta_size  = dc.GetTextExtent(etaStr);
    auto dist_size = dc.GetTextExtent(distStr);

    int padding = Px(12);
    int rightX_eta  = sz.x - eta_size.x - padding;
    int rightX_dist = sz.x - dist_size.x - padding;

    dc.DrawText(etaStr,  rightX_eta,  sz.y - eta_size.y  - padding);
    dc.DrawText(distStr, rightX_dist, sz.y - dist_size.y - padding - eta_size.y - Px(8));

    // 속도 강조용 얇은 밑줄(액센트)
    dc.SetPen(wxPen(accent, Px(2)));
    int speed_w, speed_h;
    dc.GetTextExtent(speed, &speed_w, &speed_h);
    dc.DrawLine(Px(16), Px(12 + 32 + 6), Px(16 + speed_w), Px(12 + 32 + 6));
}

void MapOverlayHud::OnSize(wxSizeEvent& e) {
    Refresh();
    e.Skip();
}

int MapOverlayHud::Px(double logicalPx) const {
    return int(std::round(logicalPx * dpi_scale_ * std::max(0.75, state_.font_scale)));
}

wxString MapOverlayHud::FormatSpeed(double kmh) const {
    int v = int(std::round(kmh));
    return wxString::Format("%d km/h", v);
}

wxString MapOverlayHud::FormatSpeedLimit(double kmh) const {
    if (kmh <= 0.0) return "—";
    int v = int(std::round(kmh));
    return wxString::Format("제한 %d", v);
}

wxString MapOverlayHud::FormatDistance(double m) const {
    if (m >= 1000.0) {
        double km = m / 1000.0;
        return wxString::Format("%.1f km", km);
    }
    return wxString::Format("%d m", int(std::round(m)));
}

wxString MapOverlayHud::FormatETA(std::chrono::system_clock::time_point eta) const {
    if (eta.time_since_epoch().count() == 0) return "ETA —";
    std::time_t t = std::chrono::system_clock::to_time_t(eta);
    wxDateTime dt((time_t)t);
    return wxString::Format("ETA %s", dt.FormatISOTime());
}

void MapOverlayHud::ResolveTheme(wxColour& fg, wxColour& fgSecondary, wxColour& bg, wxColour& accent) const {
    switch (state_.theme) {
        case HudTheme::Day:
            bg = wxColour(255,255,255,230);
            fg = *wxBLACK;
            fgSecondary = wxColour(70,70,70);
            accent = wxColour(0,122,255);
            break;
        case HudTheme::Night:
            bg = wxColour(20,20,20,200);
            fg = *wxWHITE;
            fgSecondary = wxColour(180,180,180);
            accent = wxColour(10,132,255);
            break;
        case HudTheme::HighContrast:
            bg = wxColour(0,0,0,230);
            fg = *wxWHITE;
            fgSecondary = wxColour(220,220,0);
            accent = wxColour(255,215,0);
            break;
    }
}

void MapOverlayHud::OnPaint(wxPaintEvent& e) {
    auto paint_start = std::chrono::steady_clock::now();
    wxAutoBufferedPaintDC auto_dc(this);
    wxBufferedPaintDC dc(this);

    HudState s;
    MetricFn mh;
    {
        std::lock_guard<std::mutex> lk(mtx_);
        s = state_;
        mh = metric_;
    }

    wxColour fg, fg2, bg, accent;
    ResolveTheme(fg, fg2, bg, accent);

    wxSize sz = GetClientSize();
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(bg));
    dc.DrawRectangle({0,0}, sz);

    if (!s.visible) {
        if (mh) mh("hud_visible", 0.0);
        if (mh) {
            auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - paint_start).count();
            mh("hud_paint_ms", ms);
        }
        return;
    }

    wxFont big(Px(32), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    wxFont small(Px(16), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    dc.SetTextForeground(fg);

    wxString speed = FormatSpeed(s.speed_kmh);
    wxString limit = FormatSpeedLimit(s.speed_limit_kmh);

    dc.SetFont(big);
    dc.DrawText(speed, Px(16), Px(12));

    dc.SetFont(small);
    dc.SetTextForeground(fg2);
    dc.DrawText(limit, Px(16), Px(52));

    dc.SetTextForeground(fg);
    dc.SetFont(small);

    wxString etaStr = FormatETA(s.eta);
    wxString distStr = FormatDistance(s.distance_remain_m);

    auto eta_size  = dc.GetTextExtent(etaStr);
    auto dist_size = dc.GetTextExtent(distStr);

    int padding = Px(12);
    int rightX_eta  = sz.x - eta_size.x - padding;
    int rightX_dist = sz.x - dist_size.x - padding;

    dc.DrawText(etaStr,  rightX_eta,  sz.y - eta_size.y  - padding);
    dc.DrawText(distStr, rightX_dist, sz.y - dist_size.y - padding - eta_size.y - Px(8));

    dc.SetPen(wxPen(accent, Px(2)));
    int speed_w, speed_h;
    dc.GetTextExtent(speed, &speed_w, &speed_h);
    dc.DrawLine(Px(16), Px(12 + 32 + 6), Px(16 + speed_w), Px(12 + 32 + 6));

    if (mh) mh("hud_visible", 1.0);
    if (mh) {
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - paint_start).count();
        mh("hud_paint_ms", ms);
    }

    if (!first_paint_done_) first_paint_done_ = true;
}

} // namespace ui
