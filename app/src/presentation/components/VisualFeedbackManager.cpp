#include "ProjectMap.h"
#include PRESENTATION_VISUAL_FEEDBACK

namespace presentation::components {

// ============================================================================
// VisualFeedbackManager 구현
// ============================================================================

void VisualFeedbackManager::checkMilestones(const domain::state::RouteProgress& progress) {
    // shouldNotifyMilestone은 내부 상태를 변경하므로 const_cast 사용
    auto& mutableProgress = const_cast<domain::state::RouteProgress&>(progress);
    if (milestoneReachedHandler_ && mutableProgress.milestones.shouldNotifyMilestone(progress.completionPercentage)) {
        milestoneReachedHandler_(progress.completionPercentage);
    }
}

void VisualFeedbackManager::checkRouteCompletion(const domain::state::RouteProgress& progress) {
    if (routeCompletedHandler_ && progress.isCompleted()) {
        routeCompletedHandler_(progress);
    }
}

void VisualFeedbackManager::resetAllFeedbacks() {
    for (auto& [type, feedback] : feedbacks_) {
        if (feedback) {
            feedback->resetFeedback();
        }
    }
    isInitialized_ = false;
    lastProgress_ = domain::state::RouteProgress{};
}

void VisualFeedbackManager::setFeedbackEnabled(FeedbackType type, bool enabled) {
    auto it = feedbacks_.find(type);
    if (it != feedbacks_.end() && it->second) {
        it->second->setFeedbackEnabled(enabled);
    }
}

void VisualFeedbackManager::setAllFeedbacksEnabled(bool enabled) {
    for (auto& [type, feedback] : feedbacks_) {
        if (feedback) {
            feedback->setFeedbackEnabled(enabled);
        }
    }
}

bool VisualFeedbackManager::isFeedbackEnabled(FeedbackType type) const {
    auto it = feedbacks_.find(type);
    if (it != feedbacks_.end() && it->second) {
        return it->second->isFeedbackEnabled();
    }
    return false;
}

void VisualFeedbackManager::setMilestoneReachedHandler(
    std::function<void(double percentage)> handler
) {
    milestoneReachedHandler_ = std::move(handler);
}

void VisualFeedbackManager::setRouteCompletedHandler(
    std::function<void(const domain::state::RouteProgress&)> handler
) {
    routeCompletedHandler_ = std::move(handler);
}

void VisualFeedbackManager::setUpdateThreshold(double threshold) {
    updateThreshold_ = std::clamp(threshold, 0.0001, 0.1); // 0.01% ~ 10% 범위
}

double VisualFeedbackManager::getUpdateThreshold() const {
    return updateThreshold_;
}

} // namespace presentation::components