#pragma once

#include "ProjectMap.h"
#include DOMAIN_TYPES
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>

namespace domain::state {

/**
 * @brief 네비게이션 관련 상태 데이터
 */
struct NavigationState {
    double currentSpeed{0.0};           // 현재 속도 (km/h)
    double speedLimit{0.0};             // 제한 속도 (km/h)
    double remainingDistance{0.0};      // 남은 거리 (km)
    double totalDistance{0.0};          // 전체 거리 (km)
    std::chrono::seconds estimatedTimeArrival{0}; // 예상 도착 시간
    std::chrono::seconds remainingTime{0};        // 남은 시간
    double completionPercentage{0.0};   // 완료 백분율 (0-100)
    std::string nextTurnDirection;      // 다음 회전 방향
    double nextTurnDistance{0.0};       // 다음 회전까지 거리 (m)
    bool isNavigating{false};           // 네비게이션 활성 상태
    bool isOffRoute{false};             // 경로 이탈 상태
    
    // 타임스탬프
    std::chrono::steady_clock::time_point lastUpdated;
    
    NavigationState() {
        lastUpdated = std::chrono::steady_clock::now();
    }
    
    // 유효성 검사
    bool IsValid() const {
        return currentSpeed >= 0.0 && 
               remainingDistance >= 0.0 && 
               totalDistance >= 0.0 &&
               completionPercentage >= 0.0 && 
               completionPercentage <= 100.0;
    }
    
    // 업데이트 시간 갱신
    void UpdateTimestamp() {
        lastUpdated = std::chrono::steady_clock::now();
    }
};

/**
 * @brief 확장된 위치 상태 데이터
 */
struct ExtendedLocationState : public LocationState {
    double accuracy{0.0};               // 위치 정확도 (m)
    int signalStrength{0};              // GPS 신호 강도 (0-100)
    bool isMoving{false};               // 움직임 감지
    double movementSpeed{0.0};          // 움직임 속도 (m/s)
    std::chrono::steady_clock::time_point lastMovement; // 마지막 움직임 시간
    
    ExtendedLocationState() : LocationState() {
        lastMovement = std::chrono::steady_clock::now();
    }
    
    explicit ExtendedLocationState(const LocationState& base) 
        : LocationState(base) {
        lastMovement = std::chrono::steady_clock::now();
    }
    
    // GPS 신호 품질 평가
    bool HasGoodSignal() const {
        return signalStrength >= 50 && accuracy <= 10.0;
    }
    
    // 움직임 감지 업데이트
    void UpdateMovement(double speed) {
        movementSpeed = speed;
        isMoving = speed > 0.5; // 0.5 m/s 이상이면 움직임으로 판단
        if (isMoving) {
            lastMovement = std::chrono::steady_clock::now();
        }
    }
};

/**
 * @brief HUD 상태 변경 이벤트 타입
 */
enum class HudStateEventType {
    LocationUpdate,
    NavigationUpdate,
    SpeedUpdate,
    RouteUpdate,
    All
};

/**
 * @brief 상태 구독자 인터페이스
 */
class StateSubscriber {
public:
    virtual ~StateSubscriber() = default;
    
    // 위치 상태 업데이트 콜백
    virtual void OnLocationStateChanged(const ExtendedLocationState& locationState) {}
    
    // 네비게이션 상태 업데이트 콜백  
    virtual void OnNavigationStateChanged(const NavigationState& navState) {}
    
    // 구독자 ID 반환 (디버깅용)
    virtual std::string GetSubscriberId() const = 0;
    
    // 구독할 이벤트 타입 반환
    virtual std::vector<HudStateEventType> GetSubscribedEvents() const = 0;
};

} // namespace domain::state