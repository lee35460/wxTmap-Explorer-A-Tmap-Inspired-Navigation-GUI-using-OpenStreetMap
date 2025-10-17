#include "ProjectMap.h"  // 🗺️ 프로젝트 파일 지도 사용
#include PRESENTATION_CAMERA_CONTROLLER  // "presentation/components/CameraController.h"로 자동 확장
#include PRESENTATION_CAMERA_PRESET
#include PRESENTATION_TRANSITION_ANIMATOR
#include <cmath>
#include <spdlog/spdlog.h>

using namespace presentation::components;

namespace presentation::components {

CameraController::CameraController(wxWindow* parent) 
    : BaseComponent<NoWxBase>() {
    InitializeComponents();
    spdlog::info("CameraController 생성됨 (WXT-60 확장)");
}

CameraController::~CameraController() {
    // unique_ptr은 자동으로 소멸됨
}

// === BaseComponent 오버라이드 ===

void CameraController::Update(double deltaTime) {
    BaseComponent<NoWxBase>::Update(deltaTime);
    
    // 애니메이터 업데이트
    if (animator_) {
        animator_->Update(deltaTime);
    }
}

// === 기존 Follow 기능 (WXT-59) ===

void CameraController::SetFollowMode(CameraFollowMode mode) {
    followMode_ = mode;
    spdlog::debug("Follow 모드 변경: {}", static_cast<int>(mode));
}

void CameraController::UpdateLocation(const LocationState& location) {
    if (followMode_ == CameraFollowMode::Off) return;
    if (!location.isValid) return;
    
    if (ShouldFollowLocation(location)) {
        // WXT-60: 새로운 애니메이션 시스템 사용
        LonLat targetCenter = location.coordinates;
        double targetBearing = CalculateCameraBearing(location);
        
        // zoom이 초기화되지 않은 경우 기본값 사용
        double targetZoom = (currentViewport_.zoom > 0) ? currentViewport_.zoom : 15.0;
        CameraViewport targetViewport(targetCenter, targetZoom, targetBearing);
        
        // 애니메이션으로 전환
        CameraAnimationConfig config(animationDuration_, EasingType::EaseOut);
        StartTransitionTo(targetViewport, config);
        
        // 기존 콜백도 호출 (하위 호환성)
        if (cameraMoveCallback_) {
            cameraMoveCallback_(targetCenter, targetBearing);
        }
    }
    lastLocation_ = location;
}

void CameraController::SetCameraMoveCallback(std::function<void(const LonLat&, double)> callback) {
    cameraMoveCallback_ = std::move(callback);
}

void CameraController::SetFollowThreshold(double meters) {
    followThreshold_ = meters;
}

void CameraController::SetAnimationDuration(double ms) {
    animationDuration_ = ms;
}

// === WXT-60: 새로운 프리셋 및 애니메이션 기능 ===

void CameraController::InitializeCamera(const CameraViewport& initialViewport) {
    if (initialViewport.center.lon == 0.0 && initialViewport.center.lat == 0.0) {
        // 기본 서울 뷰로 초기화
        currentViewport_ = CameraPresets::Seoul();
    } else {
        currentViewport_ = initialViewport;
    }
    
    isInitialized_ = true;
    
    // 초기 뷰 설정 (애니메이션 없이)
    SetCurrentViewport(currentViewport_, true);
    
    spdlog::info("카메라 초기화 완료: {:.6f},{:.6f} 줌={:.1f}", 
                 currentViewport_.center.lon, currentViewport_.center.lat, currentViewport_.zoom);
}

void CameraController::ApplyPreset(const ::CameraPreset& preset, bool animated) {
    if (!presetManager_ || !presetManager_->IsValidPreset(preset)) {
        spdlog::warn("유효하지 않은 프리셋 적용 시도");
        return;
    }
    
    if (animated && isInitialized_) {
        CameraAnimationConfig config(1000.0, EasingType::EaseInOut);
        StartTransitionTo(preset.viewport, config);
    } else {
        SetCurrentViewport(preset.viewport);
    }
    
    spdlog::info("프리셋 적용: {} -> {:.6f},{:.6f}", 
                 preset.name, preset.viewport.center.lon, preset.viewport.center.lat);
}

void CameraController::SetToDefaultView(bool animated) {
    if (presetManager_) {
        auto defaultPreset = presetManager_->CreateDefaultPreset();
        ApplyPreset(defaultPreset, animated);
    }
}

void CameraController::SetToUserLocation(const LonLat& location, bool animated) {
    if (presetManager_) {
        auto userPreset = presetManager_->CreateUserLocationPreset(location);
        ApplyPreset(userPreset, animated);
    }
}

void CameraController::FitToRoute(const std::vector<LonLat>& routePoints, bool animated) {
    if (presetManager_) {
        auto routePreset = presetManager_->CreateRoutePreset(routePoints);
        ApplyPreset(routePreset, animated);
    }
}

// === 커스텀 프리셋 관리 ===

void CameraController::SaveCurrentViewAsPreset(const std::string& name) {
    if (presetManager_) {
        presetManager_->SaveCustomPreset(name, currentViewport_);
        spdlog::info("현재 뷰를 프리셋으로 저장: {}", name);
    }
}

void CameraController::LoadCustomPreset(const std::string& name, bool animated) {
    if (presetManager_ && presetManager_->HasCustomPreset(name)) {
        auto preset = presetManager_->GetCustomPreset(name);
        ApplyPreset(preset, animated);
    }
}

std::vector<std::string> CameraController::GetCustomPresetNames() const {
    if (presetManager_) {
        return presetManager_->GetCustomPresetNames();
    }
    return {};
}

// === 애니메이션 제어 ===

void CameraController::StartTransitionTo(const CameraViewport& target, 
                                        const CameraAnimationConfig& config) {
    if (animator_) {
        animator_->StartTransition(currentViewport_, target, config);
    }
}

void CameraController::PauseTransition() {
    if (animator_) {
        animator_->PauseAnimation();
    }
}

void CameraController::ResumeTransition() {
    if (animator_) {
        animator_->ResumeAnimation();
    }
}

void CameraController::StopTransition() {
    if (animator_) {
        animator_->StopAnimation();
    }
}

bool CameraController::IsTransitioning() const {
    return animator_ && animator_->IsTransitioning();
}

double CameraController::GetTransitionProgress() const {
    return animator_ ? animator_->GetProgress() : 0.0;
}

// === 현재 뷰포트 상태 ===

CameraViewport CameraController::GetCurrentViewport() const {
    return currentViewport_;
}

void CameraController::SetCurrentViewport(const CameraViewport& viewport, bool updateCallback) {
    currentViewport_ = viewport;
    
    if (updateCallback && cameraMoveCallback_) {
        cameraMoveCallback_(viewport.center, viewport.bearing);
    }
}

void CameraController::OptimizeForScreenSize(int width, int height) {
    if (presetManager_) {
        CameraViewport optimized = presetManager_->OptimizeForScreenSize(currentViewport_, width, height);
        SetCurrentViewport(optimized);
    }
}

// === 사용자 인터랙션 처리 ===

void CameraController::OnUserPan() {
    if (animator_) {
        animator_->OnUserInteraction();
    }
}

void CameraController::OnUserZoom() {
    if (animator_) {
        animator_->OnUserInteraction();
    }
}

void CameraController::OnUserRotate() {
    if (animator_) {
        animator_->OnUserInteraction();
    }
}

// === 기존 Follow 내부 메서드 ===

bool CameraController::ShouldFollowLocation(const LocationState& newLocation) const {
    if (!lastLocation_.isValid) return true; // 첫 번째 위치
    
    double distance = CalculateDistance(lastLocation_.coordinates, newLocation.coordinates);
    return distance >= followThreshold_;
}

double CameraController::CalculateCameraBearing(const LocationState& location) const {
    if (followMode_ == CameraFollowMode::Bearing && location.hasBearing) {
        return location.bearing;
    }
    return 0.0; // 기본 북쪽 방향
}

// === WXT-60: 새로운 내부 메서드들 ===

void CameraController::InitializeComponents() {
    // CameraPreset 컴포넌트 생성
    presetManager_ = std::make_unique<CameraPreset>();
    
    // TransitionAnimator 컴포넌트 생성
    animator_ = std::make_unique<TransitionAnimator>();
    
    // 애니메이션 콜백 설정
    SetupAnimationCallbacks();
}

void CameraController::SetupAnimationCallbacks() {
    if (animator_) {
        // 애니메이션 업데이트시 뷰포트 동기화
        animator_->SetUpdateCallback([this](const CameraViewport& viewport) {
            OnAnimationUpdate(viewport);
        });
        
        // 애니메이션 완료시 처리
        animator_->SetCompleteCallback([this]() {
            OnAnimationComplete();
        });
    }
}

::CameraPreset CameraController::CreateCurrentPreset() const {
    return ::CameraPreset(CameraPresetType::Custom, currentViewport_, "Current View");
}

void CameraController::OnAnimationUpdate(const CameraViewport& viewport) {
    SetCurrentViewport(viewport, true);
}

void CameraController::OnAnimationComplete() {
    spdlog::debug("카메라 전환 완료");
}

bool CameraController::ShouldInterruptAnimation() const {
    // 필요시 추가 로직 구현
    return false;
}

} // namespace presentation::components