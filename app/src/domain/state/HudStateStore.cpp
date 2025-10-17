#include "ProjectMap.h"
#include DOMAIN_STATE_HUD_STORE
#include <spdlog/spdlog.h>
#include <algorithm>
#include <sstream>

namespace domain::state {

// === HudStateStore 구현 ===

HudStateStore::HudStateStore() {
    lastUpdateTime_ = std::chrono::steady_clock::now();
    spdlog::info("HudStateStore 초기화 완료");
}

HudStateStore::~HudStateStore() {
    isRunning_ = false;
    {
        std::lock_guard<std::mutex> lock(subscribersMutex_);
        subscribers_.clear();
        eventSubscribers_.clear();
    }
    spdlog::info("HudStateStore 소멸 완료");
}

void HudStateStore::Update(double deltaTime) {
    if (!ShouldUpdate()) {
        return;
    }
    
    // 만료된 구독자 정리
    CleanupExpiredSubscribers();
    
    lastUpdateTime_ = std::chrono::steady_clock::now();
}

// === 상태 관리 ===

void HudStateStore::UpdateLocationState(const ExtendedLocationState& locationState) {
    if (!ValidateLocationState(locationState)) {
        spdlog::warn("유효하지 않은 위치 상태 업데이트 시도");
        return;
    }
    
    ExtendedLocationState oldState;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        oldState = currentLocationState_;
        currentLocationState_ = locationState;
        // 타임스탬프는 ExtendedLocationState 생성자에서 자동 설정됨
    }
    
    locationUpdateCount_++;
    
    // 상태가 실제로 변경된 경우에만 알림
    if (oldState.coordinates.lon != locationState.coordinates.lon ||
        oldState.coordinates.lat != locationState.coordinates.lat ||
        oldState.accuracy != locationState.accuracy ||
        oldState.isMoving != locationState.isMoving) {
        
        NotifyLocationStateChanged(currentLocationState_);
        spdlog::debug("위치 상태 업데이트: ({:.6f}, {:.6f}), 정확도: {:.1f}m", 
                     locationState.coordinates.lon, locationState.coordinates.lat, locationState.accuracy);
    }
}

void HudStateStore::UpdateNavigationState(const NavigationState& navState) {
    if (!ValidateNavigationState(navState)) {
        spdlog::warn("유효하지 않은 네비게이션 상태 업데이트 시도");
        return;
    }
    
    NavigationState oldState;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        oldState = currentNavigationState_;
        currentNavigationState_ = navState;
        // 타임스탬프는 NavigationState 생성자에서 자동 설정됨
    }
    
    navigationUpdateCount_++;
    
    // 주요 상태 변경 감지
    bool significantChange = 
        std::abs(oldState.currentSpeed - navState.currentSpeed) > 1.0 ||
        std::abs(oldState.remainingDistance - navState.remainingDistance) > 0.1 ||
        oldState.isNavigating != navState.isNavigating ||
        oldState.isOffRoute != navState.isOffRoute;
    
    if (significantChange) {
        NotifyNavigationStateChanged(currentNavigationState_);
        spdlog::debug("네비게이션 상태 업데이트: 속도 {:.1f}km/h, 남은거리 {:.1f}km", 
                     navState.currentSpeed, navState.remainingDistance);
    }
}

ExtendedLocationState HudStateStore::GetLocationState() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return currentLocationState_;
}

NavigationState HudStateStore::GetNavigationState() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return currentNavigationState_;
}

// === 구독자 관리 ===

void HudStateStore::Subscribe(SubscriberSharedPtr subscriber) {
    if (!subscriber) {
        spdlog::warn("nullptr 구독자 등록 시도");
        return;
    }
    
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    subscribers_.emplace_back(subscriber);
    
    // 이벤트 타입별 구독도 등록
    auto eventTypes = subscriber->GetSubscribedEvents();
    for (auto eventType : eventTypes) {
        eventSubscribers_[eventType].emplace_back(subscriber);
    }
    
    spdlog::info("구독자 등록: {} (총 {}개)", subscriber->GetSubscriberId(), subscribers_.size());
}

void HudStateStore::Unsubscribe(SubscriberSharedPtr subscriber) {
    if (!subscriber) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    
    // 일반 구독자 목록에서 제거
    subscribers_.erase(
        std::remove_if(subscribers_.begin(), subscribers_.end(),
                      [&subscriber](const SubscriberPtr& weak_sub) {
                          return weak_sub.expired() || weak_sub.lock() == subscriber;
                      }),
        subscribers_.end());
    
    // 이벤트별 구독자 목록에서도 제거
    for (auto& [eventType, subscribers] : eventSubscribers_) {
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                          [&subscriber](const SubscriberPtr& weak_sub) {
                              return weak_sub.expired() || weak_sub.lock() == subscriber;
                          }),
            subscribers.end());
    }
    
    spdlog::info("구독자 해제: {} (총 {}개)", subscriber->GetSubscriberId(), subscribers_.size());
}

void HudStateStore::Subscribe(SubscriberSharedPtr subscriber, const std::vector<HudStateEventType>& eventTypes) {
    if (!subscriber) {
        spdlog::warn("nullptr 구독자 등록 시도");
        return;
    }
    
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    
    for (auto eventType : eventTypes) {
        eventSubscribers_[eventType].emplace_back(subscriber);
    }
    
    spdlog::info("특정 이벤트 구독자 등록: {} (이벤트 {}개)", 
                subscriber->GetSubscriberId(), eventTypes.size());
}

// === 성능 및 디버깅 ===

size_t HudStateStore::GetSubscriberCount() const {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    return subscribers_.size();
}

void HudStateStore::SetUpdateFrequency(double fps) {
    updateFrequency_ = std::max(1.0, std::min(fps, 120.0)); // 1-120 fps 제한
    spdlog::info("업데이트 주파수 설정: {:.1f} fps", updateFrequency_.load());
}

std::string HudStateStore::GetDebugInfo() const {
    std::ostringstream oss;
    
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        oss << "=== HUD State Store Debug Info ===\n";
        oss << "구독자 수: " << GetSubscriberCount() << "\n";
        oss << "업데이트 주파수: " << updateFrequency_.load() << " fps\n";
        oss << "위치 업데이트 횟수: " << locationUpdateCount_.load() << "\n";
        oss << "네비게이션 업데이트 횟수: " << navigationUpdateCount_.load() << "\n";
        oss << "알림 횟수: " << notificationCount_.load() << "\n";
        
        oss << "\n=== 현재 위치 상태 ===\n";
        oss << "좌표: (" << currentLocationState_.coordinates.lon << ", " << currentLocationState_.coordinates.lat << ")\n";
        oss << "정확도: " << currentLocationState_.accuracy << "m\n";
        oss << "움직임: " << (currentLocationState_.isMoving ? "예" : "아니오") << "\n";
        oss << "신호 강도: " << currentLocationState_.signalStrength << "%\n";
        
        oss << "\n=== 현재 네비게이션 상태 ===\n";
        oss << "현재 속도: " << currentNavigationState_.currentSpeed << " km/h\n";
        oss << "남은 거리: " << currentNavigationState_.remainingDistance << " km\n";
        oss << "완료율: " << currentNavigationState_.completionPercentage << "%\n";
        oss << "네비게이션 활성: " << (currentNavigationState_.isNavigating ? "예" : "아니오") << "\n";
    }
    
    return oss.str();
}

void HudStateStore::Reset() {
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        currentLocationState_ = ExtendedLocationState();
        currentNavigationState_ = NavigationState();
    }
    
    locationUpdateCount_ = 0;
    navigationUpdateCount_ = 0;
    notificationCount_ = 0;
    
    spdlog::info("HudStateStore 리셋 완료");
}

// === 내부 메서드 ===

void HudStateStore::NotifyLocationStateChanged(const ExtendedLocationState& locationState) {
    NotifySubscribers(HudStateEventType::LocationUpdate, 
                     [&locationState](SubscriberSharedPtr sub) {
                         sub->OnLocationStateChanged(locationState);
                     });
}

void HudStateStore::NotifyNavigationStateChanged(const NavigationState& navState) {
    NotifySubscribers(HudStateEventType::NavigationUpdate,
                     [&navState](SubscriberSharedPtr sub) {
                         sub->OnNavigationStateChanged(navState);
                     });
}

void HudStateStore::NotifySubscribers(HudStateEventType eventType, 
                                     std::function<void(SubscriberSharedPtr)> notifyFn) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    
    // 해당 이벤트 타입 구독자들에게 알림
    auto it = eventSubscribers_.find(eventType);
    if (it != eventSubscribers_.end()) {
        for (auto& weakSub : it->second) {
            if (auto sub = weakSub.lock()) {
                try {
                    notifyFn(sub);
                    notificationCount_++;
                } catch (const std::exception& e) {
                    spdlog::error("구독자 알림 중 오류: {}", e.what());
                }
            }
        }
    }
    
    // 전체 구독자들에게도 알림 (All 이벤트 구독자)
    auto allIt = eventSubscribers_.find(HudStateEventType::All);
    if (allIt != eventSubscribers_.end()) {
        for (auto& weakSub : allIt->second) {
            if (auto sub = weakSub.lock()) {
                try {
                    notifyFn(sub);
                    notificationCount_++;
                } catch (const std::exception& e) {
                    spdlog::error("전체 구독자 알림 중 오류: {}", e.what());
                }
            }
        }
    }
}

void HudStateStore::CleanupExpiredSubscribers() {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    
    // 일반 구독자 목록 정리
    size_t beforeCount = subscribers_.size();
    subscribers_.erase(
        std::remove_if(subscribers_.begin(), subscribers_.end(),
                      [](const SubscriberPtr& weak_sub) {
                          return weak_sub.expired();
                      }),
        subscribers_.end());
    
    // 이벤트별 구독자 목록 정리
    for (auto& [eventType, subscribers] : eventSubscribers_) {
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                          [](const SubscriberPtr& weak_sub) {
                              return weak_sub.expired();
                          }),
            subscribers.end());
    }
    
    size_t afterCount = subscribers_.size();
    if (beforeCount != afterCount) {
        spdlog::debug("만료된 구독자 {} 개 정리 완료", beforeCount - afterCount);
    }
}

bool HudStateStore::ShouldUpdate() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime_);
    auto updateInterval = std::chrono::milliseconds(static_cast<int>(1000.0 / updateFrequency_.load()));
    
    return elapsed >= updateInterval;
}

bool HudStateStore::ValidateLocationState(const ExtendedLocationState& locationState) const {
    return locationState.IsValid() && 
           locationState.accuracy >= 0.0 &&
           locationState.signalStrength >= 0 && locationState.signalStrength <= 100;
}

bool HudStateStore::ValidateNavigationState(const NavigationState& navState) const {
    return navState.IsValid();
}

// === HudStateManager 싱글톤 구현 ===

std::unique_ptr<HudStateStore> HudStateManager::instance_;
std::mutex HudStateManager::instanceMutex_;

HudStateStore& HudStateManager::GetInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (!instance_) {
        instance_ = std::make_unique<HudStateStore>();
    }
    return *instance_;
}

void HudStateManager::DestroyInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex_);
    instance_.reset();
}

} // namespace domain::state