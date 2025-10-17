#include "ProjectMap.h"
#include PRESENTATION_ROUTE_PROGRESS_PIPELINE

namespace presentation::components {

// ============================================================================
// RouteProgressPipeline 핵심 구현
// ============================================================================

bool RouteProgressPipeline::processProgressUpdate(const LocationState& location) {
    if (!calculator_ || currentRoute_.empty()) {
        return false;
    }
    
    try {
        // 이전 진행률 가져오기
        std::optional<domain::state::RouteProgress> previousProgress;
        {
            std::lock_guard<std::mutex> lock(progressMutex_);
            previousProgress = lastProgress_;
        }
        
        // 새로운 진행률 계산
        auto newProgress = calculator_->calculateProgress(location, currentRoute_, previousProgress);
        
        // 유효성 검증
        if (!newProgress.isValid() || !isValidProgressChange(newProgress)) {
            std::lock_guard<std::mutex> lock(statsMutex_);
            stats_.skippedUpdates++;
            return false;
        }
        
        // 진행률 저장
        {
            std::lock_guard<std::mutex> lock(progressMutex_);
            lastProgress_ = newProgress;
        }
        
        // 시각적 컴포넌트들에 알림
        notifyVisualComponents(newProgress);
        
        // 사용자 이벤트 핸들러 호출
        if (progressUpdateHandler_) {
            progressUpdateHandler_(newProgress);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        handleError("진행률 업데이트 처리 중 오류: " + std::string(e.what()));
        return false;
    }
}

void RouteProgressPipeline::notifyVisualComponents(const domain::state::RouteProgress& progress) {
    if (feedbackManager_) {
        feedbackManager_->updateProgressFeedback(progress);
    }
}

std::chrono::milliseconds RouteProgressPipeline::calculateAdaptiveInterval(double currentSpeed) const {
    // 속도가 빠를수록 더 자주 업데이트
    if (currentSpeed < 1.0) {
        // 정지 상태: 1초 간격
        return std::chrono::milliseconds(1000);
    } else if (currentSpeed < 5.0) {
        // 저속 (18km/h 미만): 500ms
        return std::chrono::milliseconds(500);
    } else if (currentSpeed < 15.0) {
        // 중속 (54km/h 미만): 200ms
        return std::chrono::milliseconds(200);
    } else {
        // 고속: 100ms
        return std::chrono::milliseconds(100);
    }
}

bool RouteProgressPipeline::isValidProgressChange(const domain::state::RouteProgress& newProgress) const {
    std::lock_guard<std::mutex> lock(progressMutex_);
    
    if (!lastProgress_.has_value()) {
        return true; // 첫 번째 업데이트
    }
    
    const auto& last = lastProgress_.value();
    
    // 진행률이 역행하지 않는지 확인 (약간의 오차 허용)
    if (newProgress.completionPercentage < last.completionPercentage - 0.01) {
        return false;
    }
    
    // 급격한 변화 검증 (1초에 10% 이상 변화하면 의심스러움)
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
        newProgress.lastUpdateTime - last.lastUpdateTime
    ).count() / 1000.0; // 초 단위
    
    if (timeDiff > 0.0) {
        double progressDiff = std::abs(newProgress.completionPercentage - last.completionPercentage);
        double maxReasonableChange = timeDiff * 0.1; // 1초당 10%
        
        if (progressDiff > maxReasonableChange) {
            return false;
        }
    }
    
    return true;
}

void RouteProgressPipeline::updatePerformanceStats(double updateDuration, bool wasSuccessful) {
    std::lock_guard<std::mutex> lock(statsMutex_);
    
    stats_.totalUpdates++;
    stats_.lastUpdateTime = updateDuration;
    
    if (wasSuccessful) {
        stats_.successfulUpdates++;
        
        // 이동 평균으로 평균 업데이트 시간 계산
        if (stats_.averageUpdateTime == 0.0) {
            stats_.averageUpdateTime = updateDuration;
        } else {
            stats_.averageUpdateTime = (stats_.averageUpdateTime * 0.9) + (updateDuration * 0.1);
        }
    }
}

void RouteProgressPipeline::handleError(const std::string& message) {
    if (errorHandler_) {
        errorHandler_(message);
    }
    // 로깅도 여기서 수행할 수 있음
}

void RouteProgressPipeline::setUpdateInterval(std::chrono::milliseconds interval) {
    updateInterval_ = std::clamp(interval, 
                                std::chrono::milliseconds(50),   // 최소 50ms
                                std::chrono::milliseconds(2000)  // 최대 2초
    );
}

std::chrono::milliseconds RouteProgressPipeline::getUpdateInterval() const {
    return updateInterval_;
}

void RouteProgressPipeline::setAdaptiveInterval(bool enabled) {
    adaptiveInterval_ = enabled;
}

void RouteProgressPipeline::setProgressThreshold(double threshold) {
    progressThreshold_ = std::clamp(threshold, 0.0001, 0.1); // 0.01% ~ 10%
}

void RouteProgressPipeline::setProgressUpdateHandler(
    std::function<void(const domain::state::RouteProgress&)> handler
) {
    progressUpdateHandler_ = std::move(handler);
}

void RouteProgressPipeline::setErrorHandler(
    std::function<void(const std::string&)> handler
) {
    errorHandler_ = std::move(handler);
}

RouteProgressPipeline::PerformanceStats RouteProgressPipeline::getPerformanceStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

void RouteProgressPipeline::resetPerformanceStats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    stats_ = PerformanceStats{};
    stats_.startTime = std::chrono::steady_clock::now();
}

void RouteProgressPipeline::subscribeToStateChanges() {
    // HudStateStore 구독 로직 (실제 구현에서는 Observer 패턴 사용)
    // 현재는 updateThreadLoop에서 폴링 방식으로 구현
}

void RouteProgressPipeline::unsubscribeFromStateChanges() {
    // HudStateStore 구독 해제 로직
}

} // namespace presentation::components