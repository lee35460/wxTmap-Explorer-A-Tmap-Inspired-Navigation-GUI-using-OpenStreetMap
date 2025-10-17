#include "ProjectMap.h"
#include DOMAIN_SERVICES_ROUTE_PROGRESS_CALCULATOR
#include <cmath>
#include <algorithm>

namespace domain::services {

// ============================================================================
// Haversine 거리 계산 구현 (헤더에서 선언된 calculateDistance만 구현)
// ============================================================================

double RouteProgressCalculator::calculateDistance(
    double lat1, double lon1, double lat2, double lon2
) const {
    // 상수 정의
    const double EARTH_RADIUS_M = 6371000.0;  // 지구 반지름 (m)
    const double DEG_TO_RAD = M_PI / 180.0;   // 도->라디안 변환 상수
    
    // 각도를 라디안으로 변환
    double lat1_rad = lat1 * DEG_TO_RAD;
    double lon1_rad = lon1 * DEG_TO_RAD;
    double lat2_rad = lat2 * DEG_TO_RAD;
    double lon2_rad = lon2 * DEG_TO_RAD;
    
    // 차이값 계산
    double dlat = lat2_rad - lat1_rad;
    double dlon = lon2_rad - lon1_rad;
    
    // Haversine 공식
    double a = std::sin(dlat/2) * std::sin(dlat/2) + 
               std::cos(lat1_rad) * std::cos(lat2_rad) * 
               std::sin(dlon/2) * std::sin(dlon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    
    return EARTH_RADIUS_M * c; // 거리 (미터)
}

// ============================================================================
// 경로 분석 및 계산 구현
// ============================================================================

std::pair<size_t, double> RouteProgressCalculator::findClosestRoutePoint(
    const std::vector<RoutePoint>& route,
    const LocationState& currentLocation
) const {
    if (route.empty()) {
        return {0, 0.0};
    }
    
    size_t closestIndex = 0;
    double minDistance = calculateDistance(
        currentLocation.coordinates.lat, currentLocation.coordinates.lon,
        route[0].coordinates.lat, route[0].coordinates.lon
    );
    
    for (size_t i = 1; i < route.size(); ++i) {
        double distance = calculateDistance(
            currentLocation.coordinates.lat, currentLocation.coordinates.lon,
            route[i].coordinates.lat, route[i].coordinates.lon
        );
        
        if (distance < minDistance) {
            minDistance = distance;
            closestIndex = i;
        }
    }
    
    return {closestIndex, minDistance};
}

double RouteProgressCalculator::calculateTotalRouteDistance(
    const std::vector<RoutePoint>& route
) const {
    if (route.size() < 2) {
        return 0.0;
    }
    
    double totalDistance = 0.0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        totalDistance += calculateDistance(
            route[i].coordinates.lat, route[i].coordinates.lon,
            route[i + 1].coordinates.lat, route[i + 1].coordinates.lon
        );
    }
    
    return totalDistance;
}

// 헤더에 선언된 시그니처와 일치하도록 수정
double RouteProgressCalculator::calculateTraveledDistance(
    const std::vector<RoutePoint>& route,
    size_t closestIndex,
    double distanceToClosest
) const {
    if (route.empty() || closestIndex >= route.size()) {
        return 0.0;
    }
    
    double traveledDistance = 0.0;
    
    // 시작점부터 가장 가까운 포인트의 이전 포인트까지의 거리
    for (size_t i = 0; i < closestIndex; ++i) {
        traveledDistance += calculateDistance(
            route[i].coordinates.lat, route[i].coordinates.lon,
            route[i + 1].coordinates.lat, route[i + 1].coordinates.lon
        );
    }
    
    // 가장 가까운 포인트까지의 거리는 제외하고 현재 구간 내에서의 진행도는 별도로 계산
    return traveledDistance;
}

// 헤더에 선언된 시그니처와 일치하도록 수정
domain::state::RouteProgress::ProgressSegment RouteProgressCalculator::calculateCurrentSegment(
    const std::vector<RoutePoint>& route,
    const LocationState& currentLocation,
    size_t closestIndex
) const {
    domain::state::RouteProgress::ProgressSegment segment;
    
    if (route.size() < 2 || closestIndex >= route.size()) {
        return segment;
    }
    
    segment.segmentIndex = closestIndex;
    
    // 현재 구간 정의: closestIndex와 그 다음 포인트 사이
    if (closestIndex < route.size() - 1) {
        segment.segmentStart = route[closestIndex];
        segment.segmentEnd = route[closestIndex + 1];
        
        segment.segmentLength = calculateDistance(
            segment.segmentStart.coordinates.lat, segment.segmentStart.coordinates.lon,
            segment.segmentEnd.coordinates.lat, segment.segmentEnd.coordinates.lon
        );
        
        // 현재 위치에서 구간 시작점까지의 거리
        segment.distanceIntoSegment = calculateDistance(
            segment.segmentStart.coordinates.lat, segment.segmentStart.coordinates.lon,
            currentLocation.coordinates.lat, currentLocation.coordinates.lon
        );
        
        // 구간 길이를 초과하지 않도록 제한
        segment.distanceIntoSegment = std::min(segment.distanceIntoSegment, segment.segmentLength);
    } else {
        // 마지막 포인트에 도달한 경우
        segment.segmentStart = route[closestIndex];
        segment.segmentEnd = route[closestIndex];
        segment.segmentLength = 0.0;
        segment.distanceIntoSegment = 0.0;
    }
    
    return segment;
}

// ============================================================================
// 고급 ETA 계산
// ============================================================================

double RouteProgressCalculator::calculateAdvancedETA(
    double remainingDistance,
    double currentSpeed,
    double averageSpeed,
    double speedTrend
) const {
    if (remainingDistance <= 0.0) {
        return 0.0;
    }
    
    // 기본 ETA 계산
    double baseETA = calculateEstimatedTimeOfArrival(
        remainingDistance, currentSpeed, averageSpeed
    );
    
    // 속도 추세를 반영한 조정
    double trendFactor = 1.0;
    if (speedTrend > 0.1) {
        // 속도 증가 추세: ETA 단축
        trendFactor = 0.95;
    } else if (speedTrend < -0.1) {
        // 속도 감소 추세: ETA 증가
        trendFactor = 1.05;
    }
    
    return baseETA * trendFactor;
}

// ============================================================================
// 추세 분석
// ============================================================================

domain::state::RouteProgress::ProgressTrends RouteProgressCalculator::analyzeTrends(
    const domain::state::RouteProgress& currentProgress,
    const std::optional<domain::state::RouteProgress>& previousProgress,
    double deltaTime
) const {
    auto trends = currentProgress.trends;
    
    if (!previousProgress.has_value() || deltaTime <= 0.0) {
        return trends;
    }
    
    // 진행률 변화 계산
    double progressChange = currentProgress.completionPercentage - 
                           previousProgress->completionPercentage;
    
    // ETA 정확도 업데이트 (단순한 방법)
    if (deltaTime > 0.0) {
        double expectedProgress = deltaTime * currentProgress.trends.averageSpeed / 
                                std::max(1.0, currentProgress.remainingDistance);
        double accuracyRatio = std::abs(progressChange) / std::max(0.001, expectedProgress);
        
        // ETA 정확도 조정 (0.8 ~ 1.0 범위)
        trends.etaAccuracy = std::clamp(1.0 / accuracyRatio, 0.8, 1.0);
    }
    
    return trends;
}

} // namespace domain::services