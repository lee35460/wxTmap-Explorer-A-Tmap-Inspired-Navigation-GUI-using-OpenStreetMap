
#pragma once
#include "ProjectMap.h" 
#include DOMAIN_TYPES
#include <functional>
#include <chrono>
#include "presentation/components/BaseComponent.h"

namespace presentation::components {

// 카메라 follow 로직 담당 (wxWidgets 불필요)
class CameraController : public BaseComponent<NoWxBase> {
public:
    explicit CameraController(wxWindow* parent = nullptr);
    
    // === BaseComponent 오버라이드 ===
    void Render(wxDC& dc) override {} // CameraController는 렌더링하지 않음
    
    // === CameraController 특화 메서드들 ===
    void SetFollowMode(CameraFollowMode mode);
    CameraFollowMode GetFollowMode() const { return followMode_; }
    
    // 위치 업데이트시 카메라 이동 계산
    void UpdateLocation(const LocationState& location);
    
    // 카메라 이동 콜백 설정
    void SetCameraMoveCallback(std::function<void(const LonLat&, double)> callback);
    
    // 설정
    void SetFollowThreshold(double meters); // 이동 임계값
    void SetAnimationDuration(double ms);   // 카메라 애니메이션 시간
    
private:
    CameraFollowMode followMode_{CameraFollowMode::Off};
    LocationState lastLocation_;
    
    // 콜백
    std::function<void(const LonLat&, double)> cameraMoveCallback_;
    
    // 설정값
    double followThreshold_{5.0};     // 5m 이상 이동시 follow
    double animationDuration_{500.0}; // 500ms 애니메이션
    
    // 내부 메서드
    bool ShouldFollowLocation(const LocationState& newLocation) const;
    double CalculateCameraBearing(const LocationState& location) const;
};

} // namespace presentation::components