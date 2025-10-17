#pragma once

#include "ProjectMap.h"
#include DOMAIN_STATE_NAVIGATION
#include PRESENTATION_BASE_COMPONENT
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <thread>
#include <condition_variable>

namespace domain::state {

/**
 * @brief HUD 실시간 상태 관리 스토어
 * 
 * 중앙화된 상태 관리를 통해 모든 HUD 컴포넌트들이 일관된 상태를 유지하도록 합니다.
 * Observer 패턴과 스레드 안전성을 보장하여 멀티스레드 환경에서 안정적으로 동작합니다.
 */
class HudStateStore : public presentation::components::BaseComponent<presentation::components::NoWxBase> {
public:
    using SubscriberPtr = std::weak_ptr<StateSubscriber>;
    using SubscriberSharedPtr = std::shared_ptr<StateSubscriber>;
    
    explicit HudStateStore();
    ~HudStateStore() override;
    
    // === BaseComponent 오버라이드 ===
    void Update(double deltaTime) override;
    void Render(wxDC& dc) override {} // 상태 스토어는 렌더링하지 않음
    
    // === 상태 관리 ===
    
    /**
     * @brief 위치 상태 업데이트
     */
    void UpdateLocationState(const ExtendedLocationState& locationState);
    
    /**
     * @brief 네비게이션 상태 업데이트
     */
    void UpdateNavigationState(const NavigationState& navState);
    
    /**
     * @brief 현재 위치 상태 조회
     */
    ExtendedLocationState GetLocationState() const;
    
    /**
     * @brief 현재 네비게이션 상태 조회
     */
    NavigationState GetNavigationState() const;
    
    // === 구독자 관리 ===
    
    /**
     * @brief 상태 구독자 등록
     */
    void Subscribe(SubscriberSharedPtr subscriber);
    
    /**
     * @brief 상태 구독자 해제
     */
    void Unsubscribe(SubscriberSharedPtr subscriber);
    
    /**
     * @brief 특정 이벤트 타입으로 구독자 등록
     */
    void Subscribe(SubscriberSharedPtr subscriber, const std::vector<HudStateEventType>& eventTypes);
    
    // === 성능 및 디버깅 ===
    
    /**
     * @brief 구독자 수 반환
     */
    size_t GetSubscriberCount() const;
    
    /**
     * @brief 상태 업데이트 빈도 설정 (fps)
     */
    void SetUpdateFrequency(double fps);
    
    /**
     * @brief 디버그 정보 조회
     */
    std::string GetDebugInfo() const override;
    
    /**
     * @brief 상태 스토어 리셋
     */
    void Reset();

private:
    // === 내부 상태 ===
    mutable std::mutex stateMutex_;
    ExtendedLocationState currentLocationState_;
    NavigationState currentNavigationState_;
    
    // === 구독자 관리 ===
    mutable std::mutex subscribersMutex_;
    std::vector<SubscriberPtr> subscribers_;
    std::unordered_map<HudStateEventType, std::vector<SubscriberPtr>> eventSubscribers_;
    
    // === 업데이트 제어 ===
    std::atomic<bool> isRunning_{true};
    std::atomic<double> updateFrequency_{60.0}; // 기본 60fps
    std::chrono::steady_clock::time_point lastUpdateTime_;
    
    // === 성능 추적 ===
    std::atomic<size_t> locationUpdateCount_{0};
    std::atomic<size_t> navigationUpdateCount_{0};
    std::atomic<size_t> notificationCount_{0};
    
    // === 내부 메서드 ===
    
    /**
     * @brief 구독자들에게 위치 상태 변경 알림
     */
    void NotifyLocationStateChanged(const ExtendedLocationState& locationState);
    
    /**
     * @brief 구독자들에게 네비게이션 상태 변경 알림
     */
    void NotifyNavigationStateChanged(const NavigationState& navState);
    
    /**
     * @brief 특정 이벤트 타입 구독자들에게 알림
     */
    void NotifySubscribers(HudStateEventType eventType, 
                          std::function<void(SubscriberSharedPtr)> notifyFn);
    
    /**
     * @brief 만료된 구독자 정리
     */
    void CleanupExpiredSubscribers();
    
    /**
     * @brief 업데이트 주기 확인
     */
    bool ShouldUpdate() const;
    
    /**
     * @brief 상태 유효성 검사
     */
    bool ValidateLocationState(const ExtendedLocationState& locationState) const;
    bool ValidateNavigationState(const NavigationState& navState) const;
};

/**
 * @brief 싱글톤 HUD 상태 스토어 인스턴스
 */
class HudStateManager {
public:
    static HudStateStore& GetInstance();
    static void DestroyInstance();
    
private:
    static std::unique_ptr<HudStateStore> instance_;
    static std::mutex instanceMutex_;
};

} // namespace domain::state