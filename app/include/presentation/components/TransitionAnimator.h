#pragma once
#include "ProjectMap.h"
#include DOMAIN_TYPES
#include "presentation/components/BaseComponent.h"
#include <functional>
#include <chrono>

namespace presentation::components {

/**
 * WXT-60: 카메라 애니메이션 전환 엔진
 * BaseComponent 기반으로 애니메이션 상태 관리 및 보간 처리
 */
class TransitionAnimator : public AnimatedComponent<NoWxBase> {
public:
    explicit TransitionAnimator();
    
    // === BaseComponent 오버라이드 ===
    void Render(wxDC& dc) override {} // TransitionAnimator는 렌더링하지 않음
    void Render(wxDC& dc, const CoordTransformFn& coordToPixel) override {} // RenderableComponent 오버라이드
    void Update(double deltaTime) override;
    
    // === AnimatedComponent 오버라이드 ===
    void OnAnimationComplete() override;
    
    // === 애니메이션 제어 ===
    
    // 카메라 전환 애니메이션 시작
    void StartTransition(const CameraViewport& from, const CameraViewport& to, 
                        const CameraAnimationConfig& config = CameraAnimationConfig());
    
    // 애니메이션 제어
    void PauseAnimation();
    void ResumeAnimation();
    void StopAnimation() override; // BaseComponent의 StopAnimation 오버라이드
    bool IsTransitioning() const;
    
    // 진행률 및 상태
    double GetProgress() const;
    AnimationState GetAnimationState() const { return animationState_; }
    CameraViewport GetCurrentViewport() const;
    
    // 콜백 설정
    void SetUpdateCallback(std::function<void(const CameraViewport&)> callback);
    void SetCompleteCallback(std::function<void()> callback);
    void SetCompletionCallback(std::function<void()> callback); // 테스트 호환성
    
    // 인터랙션 처리
    void OnUserInteraction(); // 사용자 인터랙션 시 호출
    
private:
    // 애니메이션 상태
    AnimationState animationState_{AnimationState::Idle};
    CameraViewport fromViewport_;
    CameraViewport toViewport_;
    CameraViewport currentViewport_;
    CameraAnimationConfig config_;
    
    // 타이밍
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::steady_clock::time_point pauseTime_;
    double elapsedTime_{0.0}; // ms
    
    // 콜백
    std::function<void(const CameraViewport&)> updateCallback_;
    std::function<void()> completeCallback_;
    
    // 이징 함수들
    double ApplyEasing(double t, EasingType type) const;
    double EaseLinear(double t) const;
    double EaseIn(double t) const;
    double EaseOut(double t) const;
    double EaseInOut(double t) const;
    
    // 보간 함수들
    CameraViewport InterpolateViewport(const CameraViewport& from, 
                                      const CameraViewport& to, 
                                      double t) const;
    LonLat InterpolateCoordinate(const LonLat& from, const LonLat& to, double t) const;
    double InterpolateValue(double from, double to, double t) const;
    double InterpolateBearing(double from, double to, double t) const; // 각도 보간 (최단 경로)
    
    // 상태 관리
    void UpdateAnimationState();
    bool ShouldInterruptAnimation() const;
};

} // namespace presentation::components