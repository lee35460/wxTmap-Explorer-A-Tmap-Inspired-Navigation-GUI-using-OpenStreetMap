#include "ui/CameraController.h"
#include <cmath>

namespace ui {

CameraController::CameraController() = default;

void CameraController::SetFollowMode(CameraFollowMode mode) {
    followMode_ = mode;
}

void CameraController::UpdateLocation(const LocationState& location) {
    if (followMode_ == CameraFollowMode::Off) return;
    if (!location.isValid) return;
    
    if (ShouldFollowLocation(location)) {
        if (cameraMoveCallback_) {
            double bearing = (followMode_ == CameraFollowMode::Bearing && location.hasBearing) 
                             ? location.bearing : 0.0;
            cameraMoveCallback_(location.coordinates, bearing);
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

} // namespace ui