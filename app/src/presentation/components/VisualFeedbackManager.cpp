#include "ProjectMap.h"
#include <algorithm>  // std::clamp
#include PRESENTATION_VISUAL_FEEDBACK

namespace presentation::components {

// ============================================================================
// VisualFeedbackManager 구현
// ============================================================================

void VisualFeedbackManager::checkMilestones(const domain::state::RouteProgress& progress) {
    // VisualFeedbackManager에서 milestone 상태 관리
    for (size_t i = 0; i < milestonePercentages_.size(); ++i) {
        if (!milestoneReached_[i] && progress.completionPercentage >= milestonePercentages_[i]) {
            milestoneReached_[i] = true;
            if (milestoneReachedHandler_) {
                milestoneReachedHandler_(progress.completionPercentage);
            }
        }
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