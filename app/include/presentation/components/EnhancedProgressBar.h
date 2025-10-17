#pragma once

#include <wx/wx.h>
#include <memory>
#include <chrono>
#include "ProjectMap.h"
#include PRESENTATION_NAVIGATION_PROGRESS
#include PRESENTATION_TRANSITION_ANIMATOR
#include PRESENTATION_VISUAL_FEEDBACK
#include DOMAIN_STATE_ROUTE_PROGRESS

namespace presentation::components {

/**
 * @brief 경로 진행률 기반 향상된 진행률 바
 * 
 * RouteProgress 기반의 동적 시각적 피드백을 제공하는 wxPanel 기반 컨트롤입니다.
 */
class EnhancedProgressBar : public wxPanel, public IVisualFeedback {
public:
    // === 생성자/소멸자 ===
    EnhancedProgressBar(wxWindow* parent, 
                       wxWindowID id = wxID_ANY,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize);
    
    virtual ~EnhancedProgressBar() = default;
    
    // === IVisualFeedback 인터페이스 구현 ===
    void applyProgressFeedback(const domain::state::RouteProgress& progress) override;
    void resetFeedback() override;
    void setFeedbackEnabled(bool enabled) override;
    bool isFeedbackEnabled() const override;
    FeedbackType getFeedbackType() const override;
    
    // === 향상된 진행률 바 기능 ===
    
    /**
     * @brief 진행률 색상 모드 설정
     */
    enum class ColorMode {
        STATIC,          // 고정 색상
        GRADIENT,        // 그라디언트 (시작→끝)
        DYNAMIC,         // 동적 색상 (속도/상태 기반)
        MILESTONE_BASED  // 이정표 기반 색상 변화
    };
    
    /**
     * @brief 색상 모드 설정
     * @param mode 색상 모드
     */
    void setColorMode(ColorMode mode);
    
    /**
     * @brief 현재 색상 모드 반환
     * @return 색상 모드
     */
    ColorMode getColorMode() const;
    
    /**
     * @brief 그라디언트 색상 설정
     * @param startColor 시작 색상
     * @param endColor 끝 색상
     */
    void setGradientColors(const wxColour& startColor, const wxColour& endColor);
    
    /**
     * @brief 이정표 색상 설정
     * @param milestoneColors 이정표별 색상 배열
     */
    void setMilestoneColors(const std::vector<wxColour>& milestoneColors);
    
    /**
     * @brief 애니메이션 속도 설정
     * @param speed 애니메이션 속도 (1.0 = 기본, >1.0 = 빠름)
     */
    void setAnimationSpeed(double speed);
    
    /**
     * @brief 부드러운 애니메이션 활성화/비활성화
     * @param enabled 애니메이션 활성화 여부
     */
    void setSmoothAnimation(bool enabled);
    
    /**
     * @brief ETA 표시 활성화/비활성화
     * @param enabled ETA 표시 여부
     */
    void setETADisplayEnabled(bool enabled);
    
    /**
     * @brief 진행률 텍스트 형식 설정
     * @param format 텍스트 형식 ("%d%%" 등)
     */
    void setProgressTextFormat(const wxString& format);

protected:
    // === 렌더링 메서드 ===
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    
private:
    // === 내부 상태 ===
    bool feedbackEnabled_ = true;
    ColorMode colorMode_ = ColorMode::GRADIENT;
    double animationSpeed_ = 1.0;
    bool smoothAnimation_ = true;
    bool etaDisplayEnabled_ = true;
    wxString progressTextFormat_ = wxT("%d%%");
    
    // 색상 설정
    wxColour startColor_ = wxColour(0, 150, 255);    // 파란색
    wxColour endColor_ = wxColour(0, 255, 150);      // 초록색
    std::vector<wxColour> milestoneColors_;
    
    // 진행률 상태
    domain::state::RouteProgress currentProgress_;
    double displayedProgress_ = 0.0;  // 현재 화면에 표시되는 진행률
    double targetProgress_ = 0.0;     // 목표 진행률
    
    // 애니메이션
    std::unique_ptr<TransitionAnimator> progressAnimator_;
    std::chrono::steady_clock::time_point lastUpdateTime_;
    
    // === 내부 헬퍼 메서드들 ===
    
    /**
     * @brief 진행률에 따른 색상 계산
     * @param progress 진행률 (0.0 ~ 1.0)
     * @return 계산된 색상
     */
    wxColour calculateProgressColor(double progress) const;
    
    /**
     * @brief 그라디언트 색상 계산
     * @param progress 진행률 (0.0 ~ 1.0)
     * @return 그라디언트 색상
     */
    wxColour calculateGradientColor(double progress) const;
    
    /**
     * @brief 동적 색상 계산 (속도 기반)
     * @param progress 진행률 정보
     * @return 동적 색상
     */
    wxColour calculateDynamicColor(const domain::state::RouteProgress& progress) const;
    
    /**
     * @brief 이정표 기반 색상 계산
     * @param progress 진행률 (0.0 ~ 1.0)
     * @return 이정표 색상
     */
    wxColour calculateMilestoneColor(double progress) const;
    
    /**
     * @brief 진행률 바 렌더링
     * @param dc 그리기 컨텍스트
     * @param rect 그리기 영역
     */
    void renderProgressBar(wxDC& dc, const wxRect& rect);
    
    /**
     * @brief 진행률 텍스트 렌더링
     * @param dc 그리기 컨텍스트
     * @param rect 그리기 영역
     */
    void renderProgressText(wxDC& dc, const wxRect& rect);
    
    /**
     * @brief ETA 텍스트 렌더링
     * @param dc 그리기 컨텍스트
     * @param rect 그리기 영역
     */
    void renderETAText(wxDC& dc, const wxRect& rect);
    
    /**
     * @brief 부드러운 진행률 애니메이션 업데이트
     */
    void updateProgressAnimation();
    
    /**
     * @brief 애니메이션 완료 콜백
     * @param finalValue 최종 값
     */
    void onAnimationComplete(double finalValue);
    
    /**
     * @brief ETA를 사람이 읽기 쉬운 형식으로 변환
     * @param seconds 초 단위 시간
     * @return 형식화된 문자열
     */
    wxString formatETA(double seconds) const;
    
    // === 이벤트 핸들러 ===
    DECLARE_EVENT_TABLE()
};

// === 구현부 ===

inline EnhancedProgressBar::EnhancedProgressBar(wxWindow* parent, 
                                               wxWindowID id,
                                               const wxPoint& pos,
                                               const wxSize& size)
    : wxPanel(parent, id, pos, size) {
    
    // TransitionAnimator 초기화
    progressAnimator_ = std::make_unique<TransitionAnimator>();
    
    // 기본 이정표 색상 설정
    milestoneColors_ = {
        wxColour(255, 100, 100),  // 25% - 빨간색
        wxColour(255, 200, 100),  // 50% - 주황색
        wxColour(100, 255, 100),  // 75% - 초록색
        wxColour(100, 255, 255)   // 100% - 청록색
    };
    
    lastUpdateTime_ = std::chrono::steady_clock::now();
}

inline void EnhancedProgressBar::applyProgressFeedback(
    const domain::state::RouteProgress& progress
) {
    if (!feedbackEnabled_ || !progress.isValid()) {
        return;
    }
    
    currentProgress_ = progress;
    targetProgress_ = progress.completionPercentage;
    
    if (smoothAnimation_) {
        // 부드러운 애니메이션으로 진행률 변경 (간단한 구현)
        displayedProgress_ = targetProgress_;
        Refresh();
    } else {
        // 즉시 변경
        displayedProgress_ = targetProgress_;
        Refresh();
    }
}

inline void EnhancedProgressBar::resetFeedback() {
    currentProgress_ = domain::state::RouteProgress{};
    displayedProgress_ = 0.0;
    targetProgress_ = 0.0;
    Refresh();
}

inline FeedbackType EnhancedProgressBar::getFeedbackType() const {
    return FeedbackType::PROGRESS_BAR;
}

inline wxColour EnhancedProgressBar::calculateProgressColor(double progress) const {
    switch (colorMode_) {
        case ColorMode::STATIC:
            return startColor_;
            
        case ColorMode::GRADIENT:
            return calculateGradientColor(progress);
            
        case ColorMode::DYNAMIC:
            return calculateDynamicColor(currentProgress_);
            
        case ColorMode::MILESTONE_BASED:
            return calculateMilestoneColor(progress);
            
        default:
            return startColor_;
    }
}

inline wxColour EnhancedProgressBar::calculateGradientColor(double progress) const {
    // 시작 색상에서 끝 색상으로 선형 보간
    double factor = std::clamp(progress, 0.0, 1.0);
    
    int r = static_cast<int>(startColor_.Red() + factor * (endColor_.Red() - startColor_.Red()));
    int g = static_cast<int>(startColor_.Green() + factor * (endColor_.Green() - startColor_.Green()));
    int b = static_cast<int>(startColor_.Blue() + factor * (endColor_.Blue() - startColor_.Blue()));
    
    return wxColour(r, g, b);
}

inline wxString EnhancedProgressBar::formatETA(double seconds) const {
    if (seconds <= 0) return wxT("도착");
    
    int totalMinutes = static_cast<int>(seconds / 60);
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;
    
    if (hours > 0) {
        return wxString::Format(wxT("%d시간 %d분"), hours, minutes);
    } else if (minutes > 0) {
        return wxString::Format(wxT("%d분"), minutes);
    } else {
        return wxT("1분 이내");
    }
}

} // namespace presentation::components