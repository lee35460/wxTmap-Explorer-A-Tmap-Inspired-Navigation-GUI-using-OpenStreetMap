#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <optional>
#include "ProjectMap.h"
#include DOMAIN_TYPES
#include DOMAIN_STATE_ROUTE_PROGRESS

namespace domain::services {

// Type aliases
using RoutePoint = LocationState;
using Route = std::vector<RoutePoint>;

/**
 * @brief 경로 진행률 계산 서비스
 * 
 * 현재 위치와 경로 정보를 기반으로 정확한 진행률을 계산하고,
 * 남은 거리, 예상 소요 시간 등을 제공합니다.
 */
class RouteProgressCalculator {
public:
    // === 생성자 ===
    RouteProgressCalculator() = default;
    virtual ~RouteProgressCalculator() = default;
    
    // === 핵심 계산 메서드 ===
    
    /**
     * @brief 현재 위치와 경로를 기반으로 진행률 계산
     * @param currentLocation 현재 위치 정보
     * @param route 전체 경로 정보
     * @param previousProgress 이전 진행률 (추세 분석용)
     * @return 계산된 진행률 정보
     */
    state::RouteProgress calculateProgress(
        const LocationState& currentLocation,
        const Route& route,
        const std::optional<state::RouteProgress>& previousProgress = std::nullopt
    ) const;
    
    /**
     * @brief 경로 완료율 계산 (0.0 ~ 1.0)
     * @param route 전체 경로 포인트
     * @param currentLocation 현재 위치
     * @return 완료율 (0.0 = 시작, 1.0 = 완료)
     */
    double calculateCompletionRatio(
        const std::vector<RoutePoint>& route,
        const LocationState& currentLocation
    ) const;
    
    /**
     * @brief 남은 거리 계산 (미터)
     * @param route 전체 경로 포인트
     * @param currentLocation 현재 위치
     * @param closestPointIndex 가장 가까운 경로 포인트 인덱스
     * @return 남은 거리 (미터)
     */
    double calculateRemainingDistance(
        const std::vector<RoutePoint>& route,
        const LocationState& currentLocation,
        size_t closestPointIndex
    ) const;
    
    /**
     * @brief 예상 도착 시간 계산 (초)
     * @param remainingDistance 남은 거리 (미터)
     * @param currentSpeed 현재 속도 (m/s)
     * @param averageSpeed 평균 속도 (m/s)
     * @param trafficFactor 교통 상황 배율 (1.0 = 정상, >1.0 = 지연)
     * @return 예상 소요 시간 (초)
     */
    double calculateEstimatedTimeOfArrival(
        double remainingDistance,
        double currentSpeed,
        double averageSpeed,
        double trafficFactor = 1.0
    ) const;

private:
    // === 내부 계산 헬퍼 메서드들 ===
    
    /**
     * @brief 현재 위치에서 가장 가까운 경로 포인트 찾기
     * @param route 경로 포인트들
     * @param currentLocation 현재 위치
     * @return {가장 가까운 포인트 인덱스, 거리}
     */
    std::pair<size_t, double> findClosestRoutePoint(
        const std::vector<RoutePoint>& route,
        const LocationState& currentLocation
    ) const;
    
    /**
     * @brief 현재 경로 구간 정보 계산
     * @param route 전체 경로
     * @param currentLocation 현재 위치
     * @param closestIndex 가장 가까운 포인트 인덱스
     * @return 현재 구간 정보
     */
    state::RouteProgress::ProgressSegment calculateCurrentSegment(
        const std::vector<RoutePoint>& route,
        const LocationState& currentLocation,
        size_t closestIndex
    ) const;
    
    /**
     * @brief 두 지점 간의 거리 계산 (Haversine 공식)
     * @param lat1, lon1 첫 번째 지점 (위도, 경도)
     * @param lat2, lon2 두 번째 지점 (위도, 경도)
     * @return 거리 (미터)
     */
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) const;
    
    /**
     * @brief 전체 경로의 총 거리 계산
     * @param route 경로 포인트들
     * @return 총 거리 (미터)
     */
    double calculateTotalRouteDistance(const std::vector<RoutePoint>& route) const;
    
    /**
     * @brief 지나온 거리 계산
     * @param route 경로 포인트들
     * @param closestIndex 현재 위치에서 가장 가까운 포인트 인덱스
     * @param distanceToClosest 가장 가까운 포인트까지의 거리
     * @return 지나온 거리 (미터)
     */
    double calculateTraveledDistance(
        const std::vector<RoutePoint>& route,
        size_t closestIndex,
        double distanceToClosest
    ) const;
    
    /**
     * @brief 속도 기반 ETA 계산 (고급 버전)
     * @param remainingDistance 남은 거리
     * @param currentSpeed 현재 속도
     * @param averageSpeed 평균 속도
     * @param speedTrend 속도 변화 추세
     * @return 예상 소요 시간 (초)
     */
    double calculateAdvancedETA(
        double remainingDistance,
        double currentSpeed,
        double averageSpeed,
        double speedTrend
    ) const;
    
    /**
     * @brief 진행률 추세 분석
     * @param currentProgress 현재 진행률
     * @param previousProgress 이전 진행률
     * @param deltaTime 시간 차이 (초)
     * @return 업데이트된 추세 정보
     */
    state::RouteProgress::ProgressTrends analyzeTrends(
        const state::RouteProgress& currentProgress,
        const std::optional<state::RouteProgress>& previousProgress,
        double deltaTime
    ) const;
    
    // === 상수 정의 ===
    static constexpr double EARTH_RADIUS_M = 6371000.0;  // 지구 반지름 (미터)
    static constexpr double MIN_SPEED_THRESHOLD = 0.5;   // 최소 속도 임계값 (m/s)
    static constexpr double MAX_REASONABLE_SPEED = 50.0;  // 최대 합리적 속도 (m/s)
    static constexpr double DEFAULT_AVERAGE_SPEED = 13.89; // 기본 평균 속도 (50km/h → m/s)
    static constexpr double ROUTE_TOLERANCE_M = 50.0;    // 경로 허용 오차 (미터)
};

// === 구현부 ===

inline state::RouteProgress RouteProgressCalculator::calculateProgress(
    const LocationState& currentLocation,
    const Route& route,
    const std::optional<state::RouteProgress>& previousProgress
) const {
    state::RouteProgress progress;
    
    // Route가 유효하지 않으면 기본값 반환
    if (route.empty()) {
        return progress;
    }
    
    // 현재 위치가 유효하지 않으면 기본값 반환
    if (!currentLocation.isValid) {
        return progress;
    }
    
    // 경로 포인트들은 이미 RoutePoint 배열이므로 직접 사용
    const std::vector<RoutePoint>& routePoints = route;
    
    // 1. 가장 가까운 경로 포인트 찾기
    auto [closestIndex, distanceToClosest] = findClosestRoutePoint(routePoints, currentLocation);
    
    // 2. 현재 구간 정보 계산
    progress.currentSegment = calculateCurrentSegment(routePoints, currentLocation, closestIndex);
    
    // 3. 완료율 계산
    progress.completionPercentage = calculateCompletionRatio(routePoints, currentLocation);
    
    // 4. 남은 거리 계산
    progress.remainingDistance = calculateRemainingDistance(routePoints, currentLocation, closestIndex);
    
    // 5. 속도 정보 업데이트 (기본 속도 사용)
    double currentSpeed = DEFAULT_AVERAGE_SPEED; // LocationState에 speed 필드가 없으므로 기본값 사용
    progress.updateSpeed(currentSpeed);
    
    // 6. ETA 계산
    progress.estimatedTimeRemaining = calculateAdvancedETA(
        progress.remainingDistance,
        currentSpeed,
        progress.trends.averageSpeed,
        progress.trends.speedTrend
    );
    
    // 7. 추세 분석 (이전 데이터가 있는 경우)
    if (previousProgress.has_value()) {
        auto now = std::chrono::system_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - previousProgress.value().lastUpdateTime
        ).count() / 1000.0; // 초 단위
        
        progress.trends = analyzeTrends(progress, previousProgress, deltaTime);
    }
    
    // 8. 마지막 업데이트 시간 설정
    progress.lastUpdateTime = std::chrono::system_clock::now();
    
    return progress;
}

inline double RouteProgressCalculator::calculateCompletionRatio(
    const std::vector<RoutePoint>& route,
    const LocationState& currentLocation
) const {
    if (route.size() < 2) return 0.0;
    
    // 전체 경로 거리
    double totalDistance = calculateTotalRouteDistance(route);
    if (totalDistance <= 0.0) return 0.0;
    
    // 가장 가까운 포인트 찾기
    auto [closestIndex, distanceToClosest] = findClosestRoutePoint(route, currentLocation);
    
    // 지나온 거리 계산
    double traveledDistance = calculateTraveledDistance(route, closestIndex, distanceToClosest);
    
    // 완료율 계산
    double ratio = traveledDistance / totalDistance;
    return std::clamp(ratio, 0.0, 1.0);
}

inline double RouteProgressCalculator::calculateRemainingDistance(
    const std::vector<RoutePoint>& route,
    const LocationState& currentLocation,
    size_t closestPointIndex
) const {
    if (route.empty() || closestPointIndex >= route.size()) {
        return 0.0;
    }
    
    double remainingDistance = 0.0;
    
    // 현재 위치에서 가장 가까운 경로 포인트까지의 거리
    const auto& closestPoint = route[closestPointIndex];
    remainingDistance += calculateDistance(
        currentLocation.coordinates.lat, currentLocation.coordinates.lon,
        closestPoint.coordinates.lat, closestPoint.coordinates.lon
    );
    
    // 나머지 경로 구간들의 거리 합계
    for (size_t i = closestPointIndex; i < route.size() - 1; ++i) {
        remainingDistance += calculateDistance(
            route[i].coordinates.lat, route[i].coordinates.lon,
            route[i + 1].coordinates.lat, route[i + 1].coordinates.lon
        );
    }
    
    return remainingDistance;
}

inline double RouteProgressCalculator::calculateEstimatedTimeOfArrival(
    double remainingDistance,
    double currentSpeed,
    double averageSpeed,
    double trafficFactor
) const {
    if (remainingDistance <= 0.0) return 0.0;
    
    // 현재 속도가 너무 낮으면 평균 속도 사용
    double effectiveSpeed = (currentSpeed < MIN_SPEED_THRESHOLD) ? 
                           averageSpeed : 
                           (currentSpeed + averageSpeed) / 2.0;
    
    // 교통 상황 반영
    effectiveSpeed = std::max(MIN_SPEED_THRESHOLD, effectiveSpeed / trafficFactor);
    
    return remainingDistance / effectiveSpeed;
}

// 나머지 헬퍼 메서드들은 파일이 너무 길어지지 않도록 별도 구현...

} // namespace domain::services