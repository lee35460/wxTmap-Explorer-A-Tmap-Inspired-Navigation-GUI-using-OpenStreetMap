#pragma once

#include <memory>
#include <map>
#include <functional>
#include "ProjectMap.h"
#include DOMAIN_STATE_ROUTE_PROGRESS

namespace presentation::components {

/**
 * @brief 시각적 피드백 유형 열거형
 */
enum class FeedbackType {
    PROGRESS_BAR,        // 진행률 바
    ROUTE_POLYLINE,      // 경로 폴리라인
    MAP_OVERLAY,         // 맵 오버레이
    TURN_BANNER,         // 턴 배너
    LOCATION_PUCK,       // 위치 표시
    NOTIFICATION         // 알림
};

/**
 * @brief 시각적 피드백 인터페이스
 * 
 * 경로 진행률 기반으로 다양한 UI 컴포넌트에서 시각적 피드백을 제공하기 위한
 * 공통 인터페이스입니다.
 */
class IVisualFeedback {
public:
    virtual ~IVisualFeedback() = default;
    
    /**
     * @brief 진행률 기반 시각적 피드백 적용
     * @param progress 현재 경로 진행률 정보
     */
    virtual void applyProgressFeedback(const domain::state::RouteProgress& progress) = 0;
    
    /**
     * @brief 시각적 피드백 초기화/리셋
     */
    virtual void resetFeedback() = 0;
    
    /**
     * @brief 피드백 활성화/비활성화
     * @param enabled 활성화 여부
     */
    virtual void setFeedbackEnabled(bool enabled) = 0;
    
    /**
     * @brief 피드백 활성화 상태 확인
     * @return 활성화 여부
     */
    virtual bool isFeedbackEnabled() const = 0;
    
    /**
     * @brief 피드백 유형 반환
     * @return 피드백 유형
     */
    virtual FeedbackType getFeedbackType() const = 0;
};

/**
 * @brief 시각적 피드백 매니저
 * 
 * 여러 UI 컴포넌트의 시각적 피드백을 중앙에서 관리하고 조정합니다.
 * Observer 패턴을 사용하여 HudStateStore의 진행률 변화를 감지하고
 * 등록된 모든 피드백 컴포넌트에 업데이트를 전파합니다.
 */
class VisualFeedbackManager {
public:
    // === 생성자/소멸자 ===
    VisualFeedbackManager();
    virtual ~VisualFeedbackManager();
    
    // === 피드백 컴포넌트 관리 ===
    
    /**
     * @brief 시각적 피드백 컴포넌트 등록
     * @param type 피드백 유형
     * @param feedback 피드백 컴포넌트
     */
    void registerFeedbackComponent(FeedbackType type, 
                                 std::shared_ptr<IVisualFeedback> feedback);
    
    /**
     * @brief 시각적 피드백 컴포넌트 해제
     * @param type 피드백 유형
     */
    void unregisterFeedbackComponent(FeedbackType type);
    
    /**
     * @brief 등록된 모든 피드백 컴포넌트 해제
     */
    void clearAllFeedbacks();
    
    // === 진행률 업데이트 ===
    
    /**
     * @brief 진행률 기반 시각적 피드백 업데이트
     * @param progress 현재 경로 진행률 정보
     */
    void updateProgressFeedback(const domain::state::RouteProgress& progress);
    
    /**
     * @brief 모든 피드백 초기화
     */
    void resetAllFeedbacks();
    
    // === 피드백 제어 ===
    
    /**
     * @brief 특정 유형의 피드백 활성화/비활성화
     * @param type 피드백 유형
     * @param enabled 활성화 여부
     */
    void setFeedbackEnabled(FeedbackType type, bool enabled);
    
    /**
     * @brief 모든 피드백 활성화/비활성화
     * @param enabled 활성화 여부
     */
    void setAllFeedbacksEnabled(bool enabled);
    
    /**
     * @brief 특정 유형 피드백의 활성화 상태 확인
     * @param type 피드백 유형
     * @return 활성화 여부
     */
    bool isFeedbackEnabled(FeedbackType type) const;
    
    // === 피드백 이벤트 핸들링 ===
    
    /**
     * @brief 이정표 도달 이벤트 핸들러 등록
     * @param handler 이정표 도달시 호출될 함수
     */
    void setMilestoneReachedHandler(
        std::function<void(double percentage)> handler
    );
    
    /**
     * @brief 경로 완료 이벤트 핸들러 등록
     * @param handler 경로 완료시 호출될 함수
     */
    void setRouteCompletedHandler(
        std::function<void(const domain::state::RouteProgress&)> handler
    );
    
    // === 성능 및 설정 ===
    
    /**
     * @brief 업데이트 임계값 설정 (성능 최적화)
     * @param threshold 변화 임계값 (0.0 ~ 1.0)
     */
    void setUpdateThreshold(double threshold);
    
    /**
     * @brief 현재 업데이트 임계값 반환
     * @return 업데이트 임계값
     */
    double getUpdateThreshold() const;
    
    /**
     * @brief 등록된 피드백 컴포넌트 수 반환
     * @return 컴포넌트 수
     */
    size_t getFeedbackComponentCount() const;

private:
    // === 내부 데이터 ===
    std::map<FeedbackType, std::shared_ptr<IVisualFeedback>> feedbacks_;
    
    // 이벤트 핸들러들
    std::function<void(double)> milestoneReachedHandler_;
    std::function<void(const domain::state::RouteProgress&)> routeCompletedHandler_;
    
    // 성능 최적화를 위한 설정
    double updateThreshold_ = 0.001;  // 0.1% 변화시 업데이트
    domain::state::RouteProgress lastProgress_;
    bool isInitialized_ = false;
    
    // === 내부 헬퍼 메서드들 ===
    
    /**
     * @brief 진행률 변화가 임계값을 초과하는지 확인
     * @param newProgress 새로운 진행률
     * @return 업데이트 필요 여부
     */
    bool shouldUpdateFeedback(const domain::state::RouteProgress& newProgress) const;
    
    /**
     * @brief 이정표 도달 확인 및 이벤트 발생
     * @param progress 현재 진행률
     */
    void checkMilestones(const domain::state::RouteProgress& progress);
    
    /**
     * @brief 경로 완료 확인 및 이벤트 발생
     * @param progress 현재 진행률
     */
    void checkRouteCompletion(const domain::state::RouteProgress& progress);
    
    /**
     * @brief 에러 핸들링 - 안전한 피드백 업데이트
     * @param type 피드백 유형
     * @param feedback 피드백 컴포넌트
     * @param progress 진행률 정보
     */
    void safeUpdateFeedback(FeedbackType type, 
                           std::shared_ptr<IVisualFeedback> feedback,
                           const domain::state::RouteProgress& progress);
};

// === 구현부 (인라인) ===

inline VisualFeedbackManager::VisualFeedbackManager() {
    // 기본 설정 초기화
    lastProgress_ = domain::state::RouteProgress{};
}

inline VisualFeedbackManager::~VisualFeedbackManager() {
    clearAllFeedbacks();
}

inline void VisualFeedbackManager::registerFeedbackComponent(
    FeedbackType type, 
    std::shared_ptr<IVisualFeedback> feedback
) {
    if (feedback && feedback->getFeedbackType() == type) {
        feedbacks_[type] = feedback;
    }
}

inline void VisualFeedbackManager::unregisterFeedbackComponent(FeedbackType type) {
    auto it = feedbacks_.find(type);
    if (it != feedbacks_.end()) {
        it->second->resetFeedback();
        feedbacks_.erase(it);
    }
}

inline void VisualFeedbackManager::clearAllFeedbacks() {
    for (auto& [type, feedback] : feedbacks_) {
        if (feedback) {
            feedback->resetFeedback();
        }
    }
    feedbacks_.clear();
    isInitialized_ = false;
}

inline void VisualFeedbackManager::updateProgressFeedback(
    const domain::state::RouteProgress& progress
) {
    // 진행률이 유효하지 않으면 업데이트하지 않음
    if (!progress.isValid()) {
        return;
    }
    
    // 초기화 확인 또는 임계값 검사
    if (!isInitialized_ || shouldUpdateFeedback(progress)) {
        // 모든 등록된 피드백 컴포넌트에 업데이트 전파
        for (auto& [type, feedback] : feedbacks_) {
            if (feedback && feedback->isFeedbackEnabled()) {
                safeUpdateFeedback(type, feedback, progress);
            }
        }
        
        // 이정표 및 완료 상태 확인
        checkMilestones(progress);
        checkRouteCompletion(progress);
        
        // 마지막 진행률 저장
        lastProgress_ = progress;
        isInitialized_ = true;
    }
}

inline bool VisualFeedbackManager::shouldUpdateFeedback(
    const domain::state::RouteProgress& newProgress
) const {
    if (!isInitialized_) return true;
    
    double percentageDiff = std::abs(
        newProgress.completionPercentage - lastProgress_.completionPercentage
    );
    
    return percentageDiff >= updateThreshold_;
}

inline void VisualFeedbackManager::safeUpdateFeedback(
    FeedbackType type, 
    std::shared_ptr<IVisualFeedback> feedback,
    const domain::state::RouteProgress& progress
) {
    try {
        feedback->applyProgressFeedback(progress);
    } catch (const std::exception& e) {
        // 로깅 또는 에러 처리
        // 개별 피드백 컴포넌트의 오류가 전체 시스템에 영향을 주지 않도록 함
    }
}

inline size_t VisualFeedbackManager::getFeedbackComponentCount() const {
    return feedbacks_.size();
}

} // namespace presentation::components