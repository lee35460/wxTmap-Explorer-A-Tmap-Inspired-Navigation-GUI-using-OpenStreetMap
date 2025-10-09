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
    spdlog::debug("[metric] turn_banner_update_ms={}", ms);
}

void TurnBanner::DrawTurnBannerStatic(wxDC& dc, const TurnBannerState& state,
                                    int width, int height) {
    if (!state.visible || width <= 0 || height <= 0) {
        return;
    }
    
    // 배경
    dc.SetBrush(wxBrush(wxColour(240, 240, 240)));
    dc.SetPen(wxPen(wxColour(200, 200, 200)));
    dc.DrawRoundedRectangle(0, 0, width, height, 8);
    
    // 지시 텍스트
    dc.SetTextForeground(wxColour(50, 50, 50));
    dc.SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    
    wxString instruction_text = wxString::Format("%s (%.0fm)", 
                                                state.instruction, state.distance_m);
    wxSize text_size = dc.GetTextExtent(instruction_text);
    int text_x = (width - text_size.GetWidth()) / 2;
    int text_y = height / 4;
    
    dc.DrawText(instruction_text, text_x, text_y);
    
    // 진행 바
    if (state.progress > 0.0) {
        int bar_y = height * 2 / 3;
        int bar_width = width - 40;
        int bar_height = 8;
        int bar_x = (width - bar_width) / 2;
        
        // 진행 바 배경
        dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
        dc.SetPen(wxPen(wxColour(150, 150, 150)));
        dc.DrawRoundedRectangle(bar_x, bar_y, bar_width, bar_height, 4);
        
        // 진행 바 내용
        int progress_width = static_cast<int>(bar_width * state.progress);
        dc.SetBrush(wxBrush(wxColour(50, 200, 100))); // 초록색
        dc.SetPen(wxPen(wxColour(30, 150, 80)));
        dc.DrawRoundedRectangle(bar_x, bar_y, progress_width, bar_height, 4);
    }  
}