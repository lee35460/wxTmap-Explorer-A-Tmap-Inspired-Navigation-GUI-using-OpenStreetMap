
#pragma once
#include "ProjectMap.h" 
#include DOMAIN_TYPES
#include <functional>
#include <chrono>
#include <memory>
#include "presentation/components/BaseComponent.h"

// Forward declarations for WXT-60
namespace presentation::components {
    class CameraPreset;
    class TransitionAnimator;
}

namespace presentation::components {

/**
 * WXT-60: 확장된 카메라 컨트롤러
 * 기존 follow 기능 + 새로운 프리셋 및 애니메이션 시스템 통합
 */
class CameraController : public BaseComponent<NoWxBase> {
public:
    explicit CameraController(wxWindow* parent = nullptr);
    ~CameraController(); // 구현 파일로 이동
    
    // === BaseComponent 오버라이드 ===
    void Render(wxDC& dc) override {} // CameraController는 렌더링하지 않음
    void Update(double deltaTime) override;
    
    // === 기존 Follow 기능 (WXT-59) ===
    void SetFollowMode(CameraFollowMode mode);
    CameraFollowMode GetFollowMode() const { return followMode_; }
    void UpdateLocation(const LocationState& location);
    void SetCameraMoveCallback(std::function<void(const LonLat&, double)> callback);
    void SetFollowThreshold(double meters);
    void SetAnimationDuration(double ms);
    
    // === WXT-60: 새로운 프리셋 및 애니메이션 기능 ===
    
    // 카메라 초기화 및 프리셋
    void InitializeCamera(const CameraViewport& initialViewport = CameraViewport());
    void ApplyPreset(const ::CameraPreset& preset, bool animated = true);
    void SetToDefaultView(bool animated = true);
    void SetToUserLocation(const LonLat& location, bool animated = true);
    void FitToRoute(const std::vector<LonLat>& routePoints, bool animated = true);
    
    // 커스텀 프리셋 관리
    void SaveCurrentViewAsPreset(const std::string& name);
    void LoadCustomPreset(const std::string& name, bool animated = true);
    std::vector<std::string> GetCustomPresetNames() const;
    
    // 애니메이션 제어
    void StartTransitionTo(const CameraViewport& target, 
                          const CameraAnimationConfig& config = CameraAnimationConfig());
    void PauseTransition();
    void ResumeTransition();
    void StopTransition();
    bool IsTransitioning() const;
    double GetTransitionProgress() const;
    
    // 현재 뷰포트 상태
    CameraViewport GetCurrentViewport() const;
    void SetCurrentViewport(const CameraViewport& viewport, bool updateCallback = true);
    
    // 화면 크기 최적화
    void OptimizeForScreenSize(int width, int height);
    
    // 사용자 인터랙션 처리
    void OnUserPan();
    void OnUserZoom();
    void OnUserRotate();
    
private:
    // === 기존 Follow 관련 (WXT-59) ===
    CameraFollowMode followMode_{CameraFollowMode::Off};
    LocationState lastLocation_;
    std::function<void(const LonLat&, double)> cameraMoveCallback_;
    double followThreshold_{5.0};
    double animationDuration_{500.0};
    
    // === WXT-60: 새로운 컴포넌트들 ===
    std::unique_ptr<CameraPreset> presetManager_;
    std::unique_ptr<TransitionAnimator> animator_;
    
    // 현재 상태
    CameraViewport currentViewport_;
    bool isInitialized_{false};
    
    // === 내부 메서드 ===
    
    // 기존 Follow 관련
    bool ShouldFollowLocation(const LocationState& newLocation) const;
    double CalculateCameraBearing(const LocationState& location) const;
    
    // WXT-60: 새로운 메서드들
    void InitializeComponents();
    void SetupAnimationCallbacks();
    ::CameraPreset CreateCurrentPreset() const;
    void OnAnimationUpdate(const CameraViewport& viewport);
    void OnAnimationComplete();
    bool ShouldInterruptAnimation() const;
};

} // namespace presentation::components