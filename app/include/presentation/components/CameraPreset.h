#pragma once
#include "ProjectMap.h"
#include DOMAIN_TYPES
#include "presentation/components/BaseComponent.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

namespace presentation::components {

/**
 * WXT-60: 카메라 초기화 프리셋 관리 클래스
 * BaseComponent를 상속받아 통일된 컴포넌트 시스템 내에서 동작
 */
class CameraPreset : public BaseComponent<NoWxBase> {
public:
    explicit CameraPreset();
    
    // === BaseComponent 오버라이드 ===
    void Render(wxDC& dc) override {} // CameraPreset은 렌더링하지 않음
    
    // === 카메라 프리셋 관리 ===
    
    // 기본 프리셋 생성
    ::CameraPreset CreateDefaultPreset() const;
    ::CameraPreset CreateUserLocationPreset(const LonLat& location) const;
    ::CameraPreset CreateRoutePreset(const std::vector<LonLat>& routePoints) const;
    
    // 커스텀 프리셋 관리
    bool SaveCustomPreset(const std::string& name, const ::CameraPreset& preset);
    std::optional<::CameraPreset> LoadCustomPreset(const std::string& name) const;
    bool DeleteCustomPreset(const std::string& name);
    void SaveCustomPreset(const std::string& name, const CameraViewport& viewport);
    ::CameraPreset GetCustomPreset(const std::string& name) const;
    bool HasCustomPreset(const std::string& name) const;
    void RemoveCustomPreset(const std::string& name);
    std::vector<std::string> GetCustomPresetNames() const;
    
    // 프리셋 검증
    bool IsValidPreset(const ::CameraPreset& preset) const;
    bool IsValidViewport(const CameraViewport& viewport) const;
    
    // 프리셋 최적화
    CameraViewport OptimizeForScreenSize(const CameraViewport& viewport, 
                                        int screenWidth, int screenHeight) const;
    
private:
    // 커스텀 프리셋 저장소
    std::unordered_map<std::string, CameraViewport> customPresets_;
    
    // 유효성 검증 헬퍼
    bool IsValidCoordinate(const LonLat& coord) const;
    bool IsValidZoom(double zoom) const;
    bool IsValidBearing(double bearing) const;
    
    // 화면 크기 기반 최적화 헬퍼
    double CalculateOptimalZoom(const CameraViewport& viewport, 
                               int screenWidth, int screenHeight) const;
};

} // namespace presentation::components