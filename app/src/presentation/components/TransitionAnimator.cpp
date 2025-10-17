#include "ProjectMap.h"
#include PRESENTATION_TRANSITION_ANIMATOR
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

namespace presentation::components {

TransitionAnimator::TransitionAnimator() 
    : AnimatedComponent<NoWxBase>() {
    spdlog::debug("TransitionAnimator 생성됨");
}

// === BaseComponent 오버라이드 ===

void TransitionAnimator::Update(double deltaTime) {
    // 부모 클래스의 Update 호출
    AnimatedComponent<NoWxBase>::Update(deltaTime);
    
    if (animationState_ != AnimationState::Running) {
        return;
    }
    
    // 경과 시간 계산
    auto now = std::chrono::steady_clock::now();
    elapsedTime_ = std::chrono::duration<double, std::milli>(now - startTime_).count();
    
    // 진행률 계산
    double progress = std::min(elapsedTime_ / config_.duration, 1.0);
    
    // 이징 적용
    double easedProgress = ApplyEasing(progress, config_.easing);
    
    // 뷰포트 보간
    currentViewport_ = InterpolateViewport(fromViewport_, toViewport_, easedProgress);
    
    // 업데이트 콜백 호출
    if (updateCallback_) {
        updateCallback_(currentViewport_);
    }
    
    // 완료 체크
    if (progress >= 1.0) {
        animationState_ = AnimationState::Completed;
        OnAnimationComplete();
    }
    
    spdlog::trace("애니메이션 업데이트: 진행률 {:.2f}%, 경과시간 {:.1f}ms", 
                  progress * 100, elapsedTime_);
}

void TransitionAnimator::OnAnimationComplete() {
    spdlog::info("카메라 전환 애니메이션 완료");
    
    // 최종 위치로 정확히 설정
    currentViewport_ = toViewport_;
    
    // 완료 콜백 호출
    if (completeCallback_) {
        completeCallback_();
    }
    
    // 상태 초기화
    animationState_ = AnimationState::Idle;
}

// === 애니메이션 제어 ===

void TransitionAnimator::StartTransition(const CameraViewport& from, 
                                        const CameraViewport& to, 
                                        const CameraAnimationConfig& config) {
    // 이전 애니메이션 중단
    if (IsTransitioning()) {
        StopAnimation();
    }
    
    fromViewport_ = from;
    toViewport_ = to;
    currentViewport_ = from;
    config_ = config;
    
    // 타이밍 초기화
    startTime_ = std::chrono::steady_clock::now();
    elapsedTime_ = 0.0;
    
    // 상태 설정
    animationState_ = AnimationState::Running;
    
    spdlog::info("카메라 전환 애니메이션 시작: {:.6f},{:.6f} -> {:.6f},{:.6f}, 지속시간 {:.0f}ms",
                 from.center.lon, from.center.lat, to.center.lon, to.center.lat, config.duration);
}

void TransitionAnimator::PauseAnimation() {
    if (animationState_ == AnimationState::Running) {
        animationState_ = AnimationState::Paused;
        pauseTime_ = std::chrono::steady_clock::now();
        spdlog::debug("애니메이션 일시정지");
    }
}

void TransitionAnimator::ResumeAnimation() {
    if (animationState_ == AnimationState::Paused) {
        // 일시정지된 시간만큼 시작 시간 조정
        auto pauseDuration = std::chrono::steady_clock::now() - pauseTime_;
        startTime_ += pauseDuration;
        
        animationState_ = AnimationState::Running;
        spdlog::debug("애니메이션 재개");
    }
}

void TransitionAnimator::StopAnimation() {
    if (IsTransitioning()) {
        animationState_ = AnimationState::Idle;
        spdlog::debug("애니메이션 중단");
    }
}

bool TransitionAnimator::IsTransitioning() const {
    return animationState_ == AnimationState::Running || 
           animationState_ == AnimationState::Paused;
}

double TransitionAnimator::GetProgress() const {
    if (animationState_ == AnimationState::Idle) return 0.0;
    if (animationState_ == AnimationState::Completed) return 1.0;
    
    return std::min(elapsedTime_ / config_.duration, 1.0);
}

CameraViewport TransitionAnimator::GetCurrentViewport() const {
    return currentViewport_;
}

// === 콜백 설정 ===

void TransitionAnimator::SetUpdateCallback(std::function<void(const CameraViewport&)> callback) {
    updateCallback_ = std::move(callback);
}

void TransitionAnimator::SetCompleteCallback(std::function<void()> callback) {
    completeCallback_ = std::move(callback);
}

void TransitionAnimator::SetCompletionCallback(std::function<void()> callback) {
    // 테스트 호환성을 위한 alias
    SetCompleteCallback(std::move(callback));
}

// === 인터랙션 처리 ===

void TransitionAnimator::OnUserInteraction() {
    if (config_.interruptible && IsTransitioning()) {
        spdlog::debug("사용자 인터랙션으로 애니메이션 중단");
        StopAnimation();
    }
}

// === 이징 함수들 ===

double TransitionAnimator::ApplyEasing(double t, EasingType type) const {
    switch (type) {
        case EasingType::Linear:   return EaseLinear(t);
        case EasingType::EaseIn:   return EaseIn(t);
        case EasingType::EaseOut:  return EaseOut(t);
        case EasingType::EaseInOut: return EaseInOut(t);
        default: return EaseLinear(t);
    }
}

double TransitionAnimator::EaseLinear(double t) const {
    return t;
}

double TransitionAnimator::EaseIn(double t) const {
    return t * t;
}

double TransitionAnimator::EaseOut(double t) const {
    return 1.0 - (1.0 - t) * (1.0 - t);
}

double TransitionAnimator::EaseInOut(double t) const {
    if (t < 0.5) {
        return 2.0 * t * t;
    } else {
        return 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
    }
}

// === 보간 함수들 ===

CameraViewport TransitionAnimator::InterpolateViewport(const CameraViewport& from, 
                                                       const CameraViewport& to, 
                                                       double t) const {
    CameraViewport result;
    result.center = InterpolateCoordinate(from.center, to.center, t);
    result.zoom = InterpolateValue(from.zoom, to.zoom, t);
    result.bearing = InterpolateBearing(from.bearing, to.bearing, t);
    return result;
}

LonLat TransitionAnimator::InterpolateCoordinate(const LonLat& from, const LonLat& to, double t) const {
    // 단순 선형 보간 (대원호 보간은 더 복잡함)
    return LonLat(
        InterpolateValue(from.lon, to.lon, t),
        InterpolateValue(from.lat, to.lat, t)
    );
}

double TransitionAnimator::InterpolateValue(double from, double to, double t) const {
    return from + (to - from) * t;
}

double TransitionAnimator::InterpolateBearing(double from, double to, double t) const {
    // 각도 보간 - 최단 경로로 회전
    double diff = to - from;
    
    // 360도 범위 내에서 최단 경로 계산
    if (diff > 180.0) {
        diff -= 360.0;
    } else if (diff < -180.0) {
        diff += 360.0;
    }
    
    double result = from + diff * t;
    
    // 0-360 범위로 정규화
    while (result < 0.0) result += 360.0;
    while (result >= 360.0) result -= 360.0;
    
    return result;
}

} // namespace presentation::components