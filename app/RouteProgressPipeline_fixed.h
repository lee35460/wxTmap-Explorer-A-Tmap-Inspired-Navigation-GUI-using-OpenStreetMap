#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include "ProjectMap.h"
#include DOMAIN_TYPES
#include DOMAIN_STATE_HUD_STORE
#include DOMAIN_SERVICES_ROUTE_PROGRESS
#include PRESENTATION_VISUAL_FEEDBACK
#include DOMAIN_STATE_ROUTE_PROGRESS

namespace presentation::components {

// RoutePoint와 Route 타입 정의
using RoutePoint = LocationState;
using Route = std::vector<RoutePoint>;

/**
 * @brief 경로 진행률 실시간 업데이트 파이프라인
 * 
 * HudStateStore의 위치 업데이트를 감지하여 RouteProgressCalculator로 진행률을 계산하고,
 * VisualFeedbackManager를 통해 모든 시각적 피드백 컴포넌트에 업데이트를 전파합니다.
 */
class RouteProgressPipeline {
public:
    // === 생성자/소멸자 ===
    RouteProgressPipeline();
    virtual ~RouteProgressPipeline();
    
    // === 초기화 및 종료 ===
    
    /**
     * @brief 파이프라인 초기화
     * @param stateStore HUD 상태 저장소
     * @param feedbackManager 시각적 피드백 매니저
     * @return 초기화 성공 여부
     */
    bool initialize(std::weak_ptr<domain::state::HudStateStore> stateStore,
                   std::shared_ptr<VisualFeedbackManager> feedbackManager);
    
    /**
     * @brief 파이프라인 종료 및 정리
     */
    void shutdown();
    
    // === 진행률 추적 제어 ===
    
    /**
     * @brief 경로 진행률 추적 시작
     * @param route 추적할 경로
     */
    void startProgressTracking(const Route& route);
    
    /**
     * @brief 경로 진행률 추적 중지
     */
    void stopProgressTracking();
    
    /**
     * @brief 현재 추적 상태 확인
     * @return 추적 중 여부
     */
    bool isTracking() const;
    
    // === 설정 및 제어 ===
    
    /**
     * @brief 업데이트 간격 설정
     * @param interval 업데이트 간격 (밀리초)
     */
    void setUpdateInterval(std::chrono::milliseconds interval);
    
    /**
     * @brief 현재 업데이트 간격 반환
     * @return 업데이트 간격
     */
    std::chrono::milliseconds getUpdateInterval() const;
    
    /**
     * @brief 적응형 업데이트 간격 활성화/비활성화
     * @param enabled 적응형 간격 사용 여부
     */
    void setAdaptiveInterval(bool enabled);
    
    /**
     * @brief 진행률 변화 임계값 설정
     * @param threshold 변화 임계값 (0.0 ~ 1.0)
     */
    void setProgressThreshold(double threshold);
    
    // === 이벤트 핸들러 등록 ===
    
    /**
     * @brief 진행률 업데이트 이벤트 핸들러 등록
     * @param handler 진행률 업데이트시 호출될 함수
     */
    void setProgressUpdateHandler(
        std::function<void(const domain::state::RouteProgress&)> handler
    );
    
    /**
     * @brief 에러 이벤트 핸들러 등록
     * @param handler 에러 발생시 호출될 함수
     */
    void setErrorHandler(
        std::function<void(const std::string&)> handler
    );
    
    // === 성능 및 진단 ===
    
    /**
     * @brief 현재 성능 통계 반환
     */
    struct PerformanceStats {
        size_t totalUpdates = 0;
        size_t successfulUpdates = 0;
        size_t skippedUpdates = 0;
        double averageUpdateTime = 0.0;  // 밀리초
        double lastUpdateTime = 0.0;     // 밀리초
        std::chrono::steady_clock::time_point startTime;
    };
    
    PerformanceStats getPerformanceStats() const;
    
    /**
     * @brief 성능 통계 리셋
     */
    void resetPerformanceStats();
    
    /**
     * @brief 현재 진행률 정보 반환
     * @return 최신 진행률 정보
     */
    std::optional<domain::state::RouteProgress> getCurrentProgress() const;

private:
    // === 내부 상태 ===
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> isTracking_{false};
    std::atomic<bool> shouldStop_{false};
    
    // 컴포넌트들
    std::weak_ptr<domain::state::HudStateStore> stateStore_;
    std::shared_ptr<VisualFeedbackManager> feedbackManager_;
    std::unique_ptr<domain::services::RouteProgressCalculator> calculator_;
    
    // 현재 경로 및 진행률
    Route currentRoute_;
    std::optional<domain::state::RouteProgress> lastProgress_;
    mutable std::mutex progressMutex_;
    
    // 스레드 관리
    std::unique_ptr<std::thread> updateThread_;
    std::mutex threadMutex_;
    std::condition_variable threadCondition_;
    
    // 업데이트 설정
    std::chrono::milliseconds updateInterval_{100};  // 기본 100ms
    bool adaptiveInterval_ = true;
    double progressThreshold_ = 0.001;  // 0.1% 변화
    
    // 이벤트 핸들러들
    std::function<void(const domain::state::RouteProgress&)> progressUpdateHandler_;
    std::function<void(const std::string&)> errorHandler_;
    
    // 성능 통계
    mutable std::mutex statsMutex_;
    PerformanceStats stats_;
    
    // === 내부 메서드들 ===
    
    /**
     * @brief 업데이트 스레드 메인 루프
     */
    void updateThreadLoop();
    
    /**
     * @brief 위치 업데이트 처리
     * @param location 새로운 위치 정보
     */
    void onLocationUpdate(const LocationState& location);
    
    /**
     * @brief 진행률 계산 및 업데이트
     * @param location 현재 위치
     * @return 계산 성공 여부
     */
    bool processProgressUpdate(const LocationState& location);
    
    /**
     * @brief 시각적 컴포넌트들에 알림
     * @param progress 진행률 정보
     */
    void notifyVisualComponents(const domain::state::RouteProgress& progress);
    
    /**
     * @brief 적응형 업데이트 간격 계산
     * @param currentSpeed 현재 속도 (m/s)
     * @return 조정된 업데이트 간격
     */
    std::chrono::milliseconds calculateAdaptiveInterval(double currentSpeed) const;
    
    /**
     * @brief 진행률 변화 유효성 검증
     * @param newProgress 새로운 진행률
     * @return 유효한 변화인지 여부
     */
    bool isValidProgressChange(const domain::state::RouteProgress& newProgress) const;
    
    /**
     * @brief 성능 통계 업데이트
     * @param updateDuration 업데이트 소요 시간
     * @param wasSuccessful 업데이트 성공 여부
     */
    void updatePerformanceStats(double updateDuration, bool wasSuccessful);
    
    /**
     * @brief 에러 핸들링
     * @param message 에러 메시지
     */
    void handleError(const std::string& message);
    
    /**
     * @brief HudStateStore 상태 변화 구독
     */
    void subscribeToStateChanges();
    
    /**
     * @brief HudStateStore 상태 변화 구독 해제
     */
    void unsubscribeFromStateChanges();
};

// === 구현부 ===

inline RouteProgressPipeline::RouteProgressPipeline() {
    calculator_ = std::make_unique<domain::services::RouteProgressCalculator>();
    stats_.startTime = std::chrono::steady_clock::now();
}

inline RouteProgressPipeline::~RouteProgressPipeline() {
    shutdown();
}

inline bool RouteProgressPipeline::initialize(
    std::weak_ptr<domain::state::HudStateStore> stateStore,
    std::shared_ptr<VisualFeedbackManager> feedbackManager
) {
    if (stateStore.expired() || !feedbackManager) {
        return false;
    }
    
    stateStore_ = stateStore;
    feedbackManager_ = feedbackManager;
    
    // HudStateStore 상태 변화 구독
    subscribeToStateChanges();
    
    isInitialized_ = true;
    return true;
}

inline void RouteProgressPipeline::shutdown() {
    if (!isInitialized_) return;
    
    // 추적 중지
    stopProgressTracking();
    
    // 상태 변화 구독 해제
    unsubscribeFromStateChanges();
    
    isInitialized_ = false;
}

inline void RouteProgressPipeline::startProgressTracking(const Route& route) {
    if (!isInitialized_ || route.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(threadMutex_);
    
    // 이전 추적 중지
    if (isTracking_) {
        stopProgressTracking();
    }
    
    currentRoute_ = route;
    lastProgress_.reset();
    shouldStop_ = false;
    
    // 업데이트 스레드 시작
    updateThread_ = std::make_unique<std::thread>(&RouteProgressPipeline::updateThreadLoop, this);
    
    isTracking_ = true;
}

inline void RouteProgressPipeline::stopProgressTracking() {
    if (!isTracking_) return;
    
    shouldStop_ = true;
    
    // 스레드 종료 대기
    if (updateThread_ && updateThread_->joinable()) {
        threadCondition_.notify_all();
        updateThread_->join();
        updateThread_.reset();
    }
    
    isTracking_ = false;
}

inline bool RouteProgressPipeline::isTracking() const {
    return isTracking_.load();
}

inline void RouteProgressPipeline::updateThreadLoop() {
    while (!shouldStop_) {
        try {
            auto start = std::chrono::steady_clock::now();
            
            // HudStateStore에서 현재 위치 가져오기
            auto stateStorePtr = stateStore_.lock();
            if (!stateStorePtr) {
                handleError("HudStateStore가 해제되었습니다");
                break;
            }
            
            auto currentLocation = stateStorePtr->GetLocationState();
            if (currentLocation.IsValid()) {
                bool success = processProgressUpdate(currentLocation);
                
                auto end = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double, std::milli>(end - start).count();
                updatePerformanceStats(duration, success);
            }
            
            // 고정 간격으로 대기 (속도 정보가 없으므로)
            auto interval = updateInterval_;
            
            std::unique_lock<std::mutex> lock(threadMutex_);
            threadCondition_.wait_for(lock, interval, [this] { return shouldStop_.load(); });
            
        } catch (const std::exception& e) {
            handleError("업데이트 루프 에러: " + std::string(e.what()));
        }
    }
}

inline std::optional<domain::state::RouteProgress> RouteProgressPipeline::getCurrentProgress() const {
    std::lock_guard<std::mutex> lock(progressMutex_);
    return lastProgress_;
}

} // namespace presentation::components