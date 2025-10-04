#include "ui/TurnBanner.h"
#include <spdlog/spdlog.h>
#include <wx/dcbuffer.h> // Required for wxAutoBufferedPaintDC

using namespace ui;

wxBEGIN_EVENT_TABLE(TurnBanner, wxPanel)
    EVT_PAINT(TurnBanner::OnPaint)
    EVT_TIMER(wxID_ANY, TurnBanner::OnAnimTick)
wxEND_EVENT_TABLE()

TurnBanner::TurnBanner(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(400,80),
              wxBORDER_NONE | wxTRANSPARENT_WINDOW),
      animTimer_(this)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    icon_ = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    text_ = new wxStaticText(this, wxID_ANY, "");
    progress_ = new wxGauge(this, wxID_ANY, 100);

    sizer->Add(icon_, 0, wxALL|wxALIGN_CENTER, 5);
    sizer->Add(text_, 1, wxALL|wxALIGN_CENTER, 5);
    sizer->Add(progress_, 1, wxALL|wxALIGN_CENTER, 5);

    SetSizer(sizer);
    animTimer_.Start(1000/30); // 30fps 애니메이션

    Hide();
}

void TurnBanner::UpdateState(const TurnBannerState& newState) {
    state_ = newState;
    if (state_.visible) {
        text_->SetLabel(wxString::Format("%s (%.0fm)",
            state_.instruction, state_.distance_m));
        progress_->SetValue(int(state_.progress * 100));
        Show();
    } else {
        Hide();
    }
    Refresh();
}

void TurnBanner::OnPaint(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    // 향후 아이콘 로딩/렌더링 추가
}

void TurnBanner::OnAnimTick(wxTimerEvent& evt) {
    // 메트릭 기록
    static auto last = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(now-last).count();
    last = now;
    spdlog::info("[metric] turn_banner_update_ms={}", ms);
}