#include "ui/NavigationProgressBar.h"
#include <wx/dcbuffer.h>
#include <spdlog/spdlog.h>
#include <cmath>

using namespace ui;

wxBEGIN_EVENT_TABLE(NavigationProgressBar, wxPanel)
    EVT_PAINT(NavigationProgressBar::OnPaint)
    EVT_SIZE(NavigationProgressBar::OnSize)
wxEND_EVENT_TABLE()

NavigationProgressBar::NavigationProgressBar(wxWindow* parent, wxWindowID id,
                                           const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    last_update_ = std::chrono::steady_clock::now();
}

void NavigationProgressBar::UpdateProgress(const NavigationProgress& progress) {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(now - last_update_).count();
    
    // 성능 메트릭 기록
    spdlog::debug("[metric] progress_update_ms={}", elapsed);
    
    progress_ = progress;
    animated_progress_ = progress.completion_ratio;
    last_update_ = now;
    
    if (progress_.visible) {
        Show();
        Refresh();
    } else {
        Hide();
    }
}

void NavigationProgressBar::SetVisible(bool visible) {
    progress_.visible = visible;
    if (visible) {
        Show();
    } else {
        Hide();
    }
}

void NavigationProgressBar::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();
    
    // 배경 클리어
    dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
    dc.Clear();
    
    if (!progress_.visible || size.GetWidth() <= 0 || size.GetHeight() <= 0) {
        return;
    }
    
    wxRect rect(0, 0, size.GetWidth(), size.GetHeight());
    DrawProgressBar(dc, rect);
    DrawProgressText(dc, rect);
}

void NavigationProgressBar::OnSize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void NavigationProgressBar::DrawProgressBar(wxDC& dc, const wxRect& rect) {
    // 진행 바 배경
    dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
    dc.SetPen(wxPen(wxColour(150, 150, 150)));
    dc.DrawRoundedRectangle(rect, 5);
    
    // 진행 바 내용
    if (animated_progress_ > 0.0) {
        int progress_width = static_cast<int>(rect.width * animated_progress_);
        wxRect progress_rect(rect.x, rect.y, progress_width, rect.height);
        
        dc.SetBrush(wxBrush(wxColour(50, 150, 250))); // 파란색
        dc.SetPen(wxPen(wxColour(30, 120, 200)));
        dc.DrawRoundedRectangle(progress_rect, 5);
    }
}

void NavigationProgressBar::DrawProgressText(wxDC& dc, const wxRect& rect) {
    // 진행률 텍스트
    wxString progress_text = wxString::Format("%.1f%%", animated_progress_ * 100.0);
    
    dc.SetTextForeground(wxColour(50, 50, 50));
    dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    
    wxSize text_size = dc.GetTextExtent(progress_text);
    int text_x = rect.x + (rect.width - text_size.GetWidth()) / 2;
    int text_y = rect.y + (rect.height - text_size.GetHeight()) / 2;
    
    dc.DrawText(progress_text, text_x, text_y);
}

void NavigationProgressBar::DrawProgressBarStatic(wxDC& dc, const NavigationProgress& progress,
                                                int width, int height) {
    wxRect rect(0, 0, width, height);
    
    // 배경
    dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
    dc.SetPen(wxPen(wxColour(150, 150, 150)));
    dc.DrawRoundedRectangle(rect, 5);
    
    // 진행 바
    if (progress.completion_ratio > 0.0) {
        int progress_width = static_cast<int>(width * progress.completion_ratio);
        wxRect progress_rect(0, 0, progress_width, height);
        
        dc.SetBrush(wxBrush(wxColour(50, 150, 250)));
        dc.SetPen(wxPen(wxColour(30, 120, 200)));
        dc.DrawRoundedRectangle(progress_rect, 5);
    }
    
    // 텍스트
    wxString text = wxString::Format("%.1f%%", progress.completion_ratio * 100.0);
    dc.SetTextForeground(wxColour(50, 50, 50));
    dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    
    wxSize text_size = dc.GetTextExtent(text);
    int text_x = (width - text_size.GetWidth()) / 2;
    int text_y = (height - text_size.GetHeight()) / 2;
    
    dc.DrawText(text, text_x, text_y);
}