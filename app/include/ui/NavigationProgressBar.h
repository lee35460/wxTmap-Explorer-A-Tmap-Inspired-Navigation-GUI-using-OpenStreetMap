#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <chrono>

namespace ui {

struct NavigationProgress {
    double total_distance = 0.0;      // 전체 거리 (m)
    double remaining_distance = 0.0;  // 남은 거리 (m)
    double completion_ratio = 0.0;    // 완료 비율 (0.0 ~ 1.0)
    std::chrono::system_clock::time_point eta; // 예상 도착 시간
    bool visible = true;
};

class NavigationProgressBar : public wxPanel {
public:
    explicit NavigationProgressBar(wxWindow* parent,
                                 wxWindowID id = wxID_ANY,
                                 const wxPoint& pos = wxDefaultPosition,
                                 const wxSize& size = wxDefaultSize);

    void UpdateProgress(const NavigationProgress& progress);
    void SetVisible(bool visible);
    
    // 정적 렌더링 메서드 (테스트용)
    static void DrawProgressBarStatic(wxDC& dc, const NavigationProgress& progress,
                                    int width, int height);

protected:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);

private:
    void DrawProgressBar(wxDC& dc, const wxRect& rect);
    void DrawProgressText(wxDC& dc, const wxRect& rect);
    void AnimateProgress();

private:
    NavigationProgress progress_;
    std::chrono::steady_clock::time_point last_update_;
    double animated_progress_ = 0.0;
    bool animation_active_ = false;

    wxDECLARE_EVENT_TABLE();
};

} // namespace ui