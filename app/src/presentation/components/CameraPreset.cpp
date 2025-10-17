#include "ProjectMap.h"
#include PRESENTATION_CAMERA_PRESET
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

namespace presentation::components {

CameraPreset::CameraPreset() 
    : BaseComponent<NoWxBase>() {
    spdlog::debug("CameraPreset 생성됨");
}

::CameraPreset CameraPreset::CreateDefaultPreset() const {
    CameraViewport viewport = CameraPresets::Seoul();
    return ::CameraPreset(CameraPresetType::Default, viewport, "Default Seoul View");
}

::CameraPreset CameraPreset::CreateUserLocationPreset(const LonLat& location) const {
    if (!IsValidCoordinate(location)) {
        spdlog::warn("유효하지 않은 사용자 위치: {}, {}", location.lon, location.lat);
        return CreateDefaultPreset(); // 폴백
    }
    
    CameraViewport viewport = CameraPresets::UserLocationView(location);
    return ::CameraPreset(CameraPresetType::UserLocation, viewport, "User Location View");
}

::CameraPreset CameraPreset::CreateRoutePreset(const std::vector<LonLat>& routePoints) const {
    if (routePoints.empty()) {
        spdlog::warn("빈 경로 포인트로 프리셋 생성 시도");
        return CreateDefaultPreset();
    }
    
    // 모든 포인트가 유효한지 확인
    for (const auto& point : routePoints) {
        if (!IsValidCoordinate(point)) {
            spdlog::warn("유효하지 않은 경로 포인트: {}, {}", point.lon, point.lat);
            return CreateDefaultPreset();
        }
    }
    
    CameraViewport viewport = CameraPresets::RouteView(routePoints);
    return ::CameraPreset(CameraPresetType::Route, viewport, "Route View");
}

// === 커스텀 프리셋 관리 ===

bool CameraPreset::SaveCustomPreset(const std::string& name, const ::CameraPreset& preset) {
    if (name.empty()) {
        spdlog::warn("빈 이름으로 커스텀 프리셋 저장 시도");
        return false;
    }
    
    if (!IsValidViewport(preset.viewport)) {
        spdlog::warn("유효하지 않은 뷰포트로 커스텀 프리셋 저장 시도: {}", name);
        return false;
    }
    
    customPresets_[name] = preset.viewport;
    spdlog::info("커스텀 프리셋 저장됨: {}", name);
    return true;
}

std::optional<::CameraPreset> CameraPreset::LoadCustomPreset(const std::string& name) const {
    auto it = customPresets_.find(name);
    if (it == customPresets_.end()) {
        spdlog::warn("존재하지 않는 커스텀 프리셋 요청: {}", name);
        return std::nullopt;
    }
    
    return ::CameraPreset(CameraPresetType::Custom, it->second, name);
}

bool CameraPreset::DeleteCustomPreset(const std::string& name) {
    auto removed = customPresets_.erase(name);
    if (removed > 0) {
        spdlog::info("커스텀 프리셋 삭제됨: {}", name);
        return true;
    } else {
        spdlog::warn("삭제할 커스텀 프리셋을 찾을 수 없음: {}", name);
        return false;
    }
}

void CameraPreset::SaveCustomPreset(const std::string& name, const CameraViewport& viewport) {
    if (name.empty()) {
        spdlog::warn("빈 이름으로 커스텀 프리셋 저장 시도");
        return;
    }
    
    if (!IsValidViewport(viewport)) {
        spdlog::warn("유효하지 않은 뷰포트로 커스텀 프리셋 저장 시도: {}", name);
        return;
    }
    
    customPresets_[name] = viewport;
    spdlog::info("커스텀 프리셋 저장됨: {}", name);
}

::CameraPreset CameraPreset::GetCustomPreset(const std::string& name) const {
    auto it = customPresets_.find(name);
    if (it == customPresets_.end()) {
        spdlog::warn("존재하지 않는 커스텀 프리셋 요청: {}", name);
        return CreateDefaultPreset();
    }
    
    return ::CameraPreset(CameraPresetType::Custom, it->second, name);
}

bool CameraPreset::HasCustomPreset(const std::string& name) const {
    return customPresets_.find(name) != customPresets_.end();
}

void CameraPreset::RemoveCustomPreset(const std::string& name) {
    auto removed = customPresets_.erase(name);
    if (removed > 0) {
        spdlog::info("커스텀 프리셋 삭제됨: {}", name);
    } else {
        spdlog::warn("삭제할 커스텀 프리셋을 찾을 수 없음: {}", name);
    }
}

std::vector<std::string> CameraPreset::GetCustomPresetNames() const {
    std::vector<std::string> names;
    names.reserve(customPresets_.size());
    
    for (const auto& pair : customPresets_) {
        names.push_back(pair.first);
    }
    
    std::sort(names.begin(), names.end());
    return names;
}

// === 프리셋 검증 ===

bool CameraPreset::IsValidPreset(const ::CameraPreset& preset) const {
    return IsValidViewport(preset.viewport);
}

bool CameraPreset::IsValidViewport(const CameraViewport& viewport) const {
    return IsValidCoordinate(viewport.center) && 
           IsValidZoom(viewport.zoom) && 
           IsValidBearing(viewport.bearing);
}

// === 화면 크기 최적화 ===

CameraViewport CameraPreset::OptimizeForScreenSize(const CameraViewport& viewport, 
                                                   int screenWidth, int screenHeight) const {
    if (screenWidth <= 0 || screenHeight <= 0) {
        spdlog::warn("유효하지 않은 화면 크기: {}x{}", screenWidth, screenHeight);
        return viewport;
    }
    
    CameraViewport optimized = viewport;
    
    // 화면 비율에 따른 줌 조정
    double aspectRatio = static_cast<double>(screenWidth) / screenHeight;
    double zoomAdjustment = 0.0;
    
    // 세로 화면 (모바일)에서는 줌 아웃
    if (aspectRatio < 1.0) {
        zoomAdjustment = -0.5;
    }
    // 매우 와이드 화면에서는 줌 인
    else if (aspectRatio > 2.0) {
        zoomAdjustment = 0.3;
    }
    
    optimized.zoom = std::clamp(viewport.zoom + zoomAdjustment, 1.0, 20.0);
    
    spdlog::debug("화면 크기 최적화: {}x{} (비율: {:.2f}) -> 줌 {:.1f} -> {:.1f}", 
                  screenWidth, screenHeight, aspectRatio, viewport.zoom, optimized.zoom);
    
    return optimized;
}

// === 유효성 검증 헬퍼 ===

bool CameraPreset::IsValidCoordinate(const LonLat& coord) const {
    return coord.lon >= -180.0 && coord.lon <= 180.0 &&
           coord.lat >= -90.0 && coord.lat <= 90.0;
}

bool CameraPreset::IsValidZoom(double zoom) const {
    return zoom >= 1.0 && zoom <= 20.0;
}

bool CameraPreset::IsValidBearing(double bearing) const {
    return bearing >= 0.0 && bearing < 360.0;
}

double CameraPreset::CalculateOptimalZoom(const CameraViewport& viewport, 
                                         int screenWidth, int screenHeight) const {
    // 기본적인 줌 최적화 로직
    // 실제로는 더 복잡한 계산이 필요할 수 있음
    double baseZoom = viewport.zoom;
    double aspectRatio = static_cast<double>(screenWidth) / screenHeight;
    
    // 화면 크기에 따른 조정
    if (screenWidth < 800 || screenHeight < 600) {
        baseZoom -= 0.5; // 작은 화면에서는 더 넓게
    }
    
    return std::clamp(baseZoom, 1.0, 20.0);
}

} // namespace presentation::components