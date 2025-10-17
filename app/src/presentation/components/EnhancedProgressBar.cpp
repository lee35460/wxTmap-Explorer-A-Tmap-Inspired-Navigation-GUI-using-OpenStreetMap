#include "ProjectMap.h"
#include PRESENTATION_ENHANCED_PROGRESS
#include <wx/dcbuffer.h>

namespace presentation::components {

// ============================================================================
// 이벤트 테이블
// ============================================================================
wxBEGIN_EVENT_TABLE(EnhancedProgressBar, wxPanel)
    EVT_PAINT(EnhancedProgressBar::OnPaint)
    EVT_SIZE(EnhancedProgressBar::OnSize)
wxEND_EVENT_TABLE()

// ============================================================================
// IVisualFeedback 인터페이스 구현
// ============================================================================

void EnhancedProgressBar::setFeedbackEnabled(bool enabled) {
    feedbackEnabled_ = enabled;
    if (!enabled) {
        resetFeedback();
    }
}

bool EnhancedProgressBar::isFeedbackEnabled() const {
    return feedbackEnabled_;
}

// ============================================================================
// 향상된 기능 구현
// ============================================================================

void EnhancedProgressBar::setColorMode(ColorMode mode) {
    colorMode_ = mode;
    Refresh();
}

EnhancedProgressBar::ColorMode EnhancedProgressBar::getColorMode() const {
    return colorMode_;
}

void EnhancedProgressBar::setGradientColors(const wxColour& startColor, const wxColour& endColor) {
    startColor_ = startColor;
    endColor_ = endColor;
    if (colorMode_ == ColorMode::GRADIENT) {
        Refresh();
    }
}

void EnhancedProgressBar::setMilestoneColors(const std::vector<wxColour>& milestoneColors) {
    milestoneColors_ = milestoneColors;
    if (colorMode_ == ColorMode::MILESTONE_BASED) {
        Refresh();
    }
}

void EnhancedProgressBar::setAnimationSpeed(double speed) {
    animationSpeed_ = std::clamp(speed, 0.1, 5.0);
}

void EnhancedProgressBar::setSmoothAnimation(bool enabled) {
    smoothAnimation_ = enabled;
}

void EnhancedProgressBar::setETADisplayEnabled(bool enabled) {
    etaDisplayEnabled_ = enabled;
    Refresh();
}

void EnhancedProgressBar::setProgressTextFormat(const wxString& format) {
    progressTextFormat_ = format;
    Refresh();
}

// ============================================================================
// 렌더링 구현
// ============================================================================

void EnhancedProgressBar::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    wxRect rect = GetClientRect();
    
    // 배경 지우기
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    
    if (rect.width <= 0 || rect.height <= 0) {
        return;
    }
    
    // 애니메이션 업데이트
    updateProgressAnimation();
    
    // 진행률 바 렌더링
    renderProgressBar(dc, rect);
    
    // 텍스트 렌더링
    renderProgressText(dc, rect);
    
    if (etaDisplayEnabled_) {
        renderETAText(dc, rect);
    }
}

void EnhancedProgressBar::OnSize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void EnhancedProgressBar::renderProgressBar(wxDC& dc, const wxRect& rect) {
    // 진행률 바 영역 계산 (텍스트를 위한 공간 확보)
    wxRect barRect = rect;
    barRect.Deflate(5); // 여백
    barRect.height = std::max(20, barRect.height / 3); // 높이 제한
    
    // 배경 (회색 테두리)
    dc.SetPen(wxPen(wxColour(200, 200, 200), 2));
    dc.SetBrush(wxBrush(wxColour(240, 240, 240)));
    dc.DrawRoundedRectangle(barRect, 5);
    
    // 진행률 바
    if (displayedProgress_ > 0.0) {
        wxRect progressRect = barRect;
        progressRect.Deflate(2); // 테두리와의 간격
        progressRect.width = static_cast<int>(progressRect.width * displayedProgress_);
        
        wxColour progressColor = calculateProgressColor(displayedProgress_);
        dc.SetPen(wxPen(progressColor, 1));
        dc.SetBrush(wxBrush(progressColor));
        dc.DrawRoundedRectangle(progressRect, 3);
    }
}

void EnhancedProgressBar::renderProgressText(wxDC& dc, const wxRect& rect) {
    int percentage = static_cast<int>(displayedProgress_ * 100);
    wxString text = wxString::Format(progressTextFormat_, percentage);
    
    dc.SetTextForeground(wxColour(50, 50, 50));
    dc.SetFont(GetFont());
    
    wxSize textSize = dc.GetTextExtent(text);
    wxPoint textPos;
    textPos.x = rect.x + (rect.width - textSize.GetWidth()) / 2;
    textPos.y = rect.y + rect.height / 3 + 10; // 진행률 바 아래
    
    dc.DrawText(text, textPos);
}

void EnhancedProgressBar::renderETAText(wxDC& dc, const wxRect& rect) {
    if (!currentProgress_.isValid() || currentProgress_.estimatedTimeRemaining <= 0) {
        return;
    }
    
    wxString etaText = wxT("도착예정: ") + formatETA(currentProgress_.estimatedTimeRemaining);
    
    dc.SetTextForeground(wxColour(100, 100, 100));
    wxFont smallFont = GetFont();
    smallFont.SetPointSize(smallFont.GetPointSize() - 2);
    dc.SetFont(smallFont);
    
    wxSize textSize = dc.GetTextExtent(etaText);
    wxPoint textPos;
    textPos.x = rect.x + (rect.width - textSize.GetWidth()) / 2;
    textPos.y = rect.y + rect.height * 2 / 3; // 하단
    
    dc.DrawText(etaText, textPos);
}

// ============================================================================
// 색상 계산 구현
// ============================================================================

wxColour EnhancedProgressBar::calculateDynamicColor(
    const domain::state::RouteProgress& progress
) const {
    // 속도 기반 색상 계산
    double speedRatio = std::clamp(progress.trends.instantaneousSpeed / 20.0, 0.0, 1.0); // 20m/s = 72km/h 기준
    
    if (speedRatio < 0.3) {
        // 느림: 빨간색
        return wxColour(255, 100, 100);
    } else if (speedRatio < 0.7) {
        // 보통: 주황색
        return wxColour(255, 200, 100);
    } else {
        // 빠름: 초록색
        return wxColour(100, 255, 100);
    }
}

wxColour EnhancedProgressBar::calculateMilestoneColor(double progress) const {
    if (milestoneColors_.empty()) {
        return startColor_;
    }
    
    // 이정표 기반 색상 선택
    size_t colorIndex = static_cast<size_t>(progress * milestoneColors_.size());
    colorIndex = std::min(colorIndex, milestoneColors_.size() - 1);
    
    return milestoneColors_[colorIndex];
}

void EnhancedProgressBar::updateProgressAnimation() {
    if (!smoothAnimation_ || !progressAnimator_) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastUpdateTime_
    ).count();
    
    if (deltaTime > 16) { // ~60fps
        // TransitionAnimator 업데이트 (간소화된 버전)
        if (progressAnimator_->IsTransitioning()) {
            double newValue = progressAnimator_->GetProgress();
            if (std::abs(newValue - displayedProgress_) > 0.001) {
                displayedProgress_ = newValue;
                Refresh();
            }
            
            if (progressAnimator_->GetAnimationState() == AnimationState::Completed) {
                onAnimationComplete(targetProgress_);
            }
        }
        
        lastUpdateTime_ = now;
    }
}

void EnhancedProgressBar::onAnimationComplete(double finalValue) {
    displayedProgress_ = finalValue;
    Refresh();
}

} // namespace presentation::components