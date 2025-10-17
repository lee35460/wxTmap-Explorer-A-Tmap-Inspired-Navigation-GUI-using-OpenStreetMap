#pragma once

#include <chrono>
#include <vector>
#include <algorithm>  // std::clamp, std::fill
#include <cmath>      // std::abs
#include <string>     // std::string, std::to_string
#include "ProjectMap.h"
#include DOMAIN_TYPES
#include DOMAIN_STATE_NAVIGATION

namespace domain::state {

// Forward declarations
using RoutePoint = LocationState;  // RoutePoint는 LocationState와 동일한 구조로 가정

/**
 * @brief 경로 진행률 관련 상태를 관리하는 구조체
 * 
 * 현재 위치에서 목적지까지의 진행률, 남은 거리, 예상 소요 시간 등
 * 경로 진행과 관련된 모든 데이터를 포함합니다.
 */
struct RouteProgress {
    // === 핵심 진행률 데이터 ===
    double completionPercentage = 0.0;        // 전체 경로 완료율 (0.0 ~ 1.0)
    double remainingDistance = 0.0;           // 남은 거리 (미터)
    double estimatedTimeRemaining = 0.0;      // 예상 남은 시간 (초)
    double currentSegmentProgress = 0.0;      // 현재 구간 진행률 (0.0 ~ 1.0)
    
    // === 타이밍 정보 ===
    std::chrono::system_clock::time_point lastUpdateTime;
    std::chrono::milliseconds updateInterval{100}; // 업데이트 간격
    
    // === 구간 정보 ===
    struct ProgressSegment {
        size_t segmentIndex = 0;              // 현재 구간 인덱스
        double segmentLength = 0.0;           // 현재 구간 길이 (미터)
        double distanceIntoSegment = 0.0;     // 구간 내 진행 거리
        RoutePoint segmentStart;              // 구간 시작점
        RoutePoint segmentEnd;                // 구간 끝점
        
        bool isValid() const {
            return segmentLength > 0.0 && 
                   distanceIntoSegment >= 0.0 && 
                   distanceIntoSegment <= segmentLength;
        }
    } currentSegment;
    
    // === 추세 분석 ===
    struct ProgressTrends {
        double averageSpeed = 0.0;            // 평균 속도 (m/s)
        double instantaneousSpeed = 0.0;      // 순간 속도 (m/s)
        double speedTrend = 0.0;              // 속도 변화 추세 (-1.0 ~ 1.0)
        double etaAccuracy = 0.95;            // ETA 정확도 (0.0 ~ 1.0)
        
        // 최근 N개의 속도 데이터 (이동 평균 계산용)
        std::vector<double> recentSpeeds;
        static constexpr size_t MAX_SPEED_HISTORY = 10;
        
        void addSpeedSample(double speed) {
            recentSpeeds.push_back(speed);
            if (recentSpeeds.size() > MAX_SPEED_HISTORY) {
                recentSpeeds.erase(recentSpeeds.begin());
            }
            updateAverageSpeed();
        }
        
    private:
        void updateAverageSpeed() {
            if (recentSpeeds.empty()) return;
            double sum = 0.0;
            for (double speed : recentSpeeds) {
                sum += speed;
            }
            averageSpeed = sum / recentSpeeds.size();
        }
    } trends;
    
    // === 이정표 및 알림 ===
    struct ProgressMilestones {
        std::vector<double> milestonePercentages{0.25, 0.5, 0.75}; // 25%, 50%, 75%
        std::vector<bool> milestoneReached;  // 각 이정표 도달 여부
        double lastNotifiedPercentage = 0.0; // 마지막 알림 전송 진행률
        
        ProgressMilestones() {
            milestoneReached.resize(milestonePercentages.size(), false);
        }
        
        bool shouldNotifyMilestone(double currentPercentage) {
            for (size_t i = 0; i < milestonePercentages.size(); ++i) {
                if (!milestoneReached[i] && currentPercentage >= milestonePercentages[i]) {
                    milestoneReached[i] = true;
                    return true;
                }
            }
            return false;
        }
        
        void reset() {
            std::fill(milestoneReached.begin(), milestoneReached.end(), false);
            lastNotifiedPercentage = 0.0;
        }
    } milestones;
    
    // === 생성자 ===
    RouteProgress() {
        lastUpdateTime = std::chrono::system_clock::now();
        milestones.reset();
    }
    
    // === 유효성 검증 ===
    bool isValid() const {
        // 기본 진행률 필드 검증만 수행 (더 관대한 검증)
        return completionPercentage >= 0.0 && completionPercentage <= 1.0 &&
               remainingDistance >= 0.0 &&
               estimatedTimeRemaining >= 0.0 &&
               currentSegmentProgress >= 0.0 && currentSegmentProgress <= 1.0;
    }
    
    // === 진행률 업데이트 ===
    void updateProgress(double newCompletion, double newRemaining, double newETA) {
        completionPercentage = std::clamp(newCompletion, 0.0, 1.0);
        remainingDistance = std::max(0.0, newRemaining);
        estimatedTimeRemaining = std::max(0.0, newETA);
        lastUpdateTime = std::chrono::system_clock::now();
        
        // 이정표 확인
        milestones.shouldNotifyMilestone(completionPercentage);
    }
    
    // === 속도 업데이트 ===
    void updateSpeed(double instantSpeed) {
        trends.instantaneousSpeed = std::max(0.0, instantSpeed);
        trends.addSpeedSample(instantSpeed);
        
        // 속도 추세 계산 (단순화된 버전)
        if (trends.recentSpeeds.size() >= 3) {
            double recent = trends.recentSpeeds.back();
            double older = trends.recentSpeeds[trends.recentSpeeds.size() - 3];
            trends.speedTrend = std::clamp((recent - older) / std::max(1.0, older), -1.0, 1.0);
        }
    }
    
    // === 구간 업데이트 ===
    void updateCurrentSegment(const ProgressSegment& segment) {
        currentSegment = segment;
        if (currentSegment.segmentLength > 0.0) {
            currentSegmentProgress = std::clamp(
                currentSegment.distanceIntoSegment / currentSegment.segmentLength, 
                0.0, 1.0
            );
        }
    }
    
    // === 진행률 완료 여부 ===
    bool isCompleted() const {
        return completionPercentage >= 1.0 || remainingDistance <= 1.0; // 1미터 이하면 완료
    }
    
    // === 디버깅용 정보 ===
    std::string getDebugString() const {
        return "RouteProgress{completion=" + std::to_string(completionPercentage * 100) + "%, " +
               "remaining=" + std::to_string(remainingDistance) + "m, " +
               "eta=" + std::to_string(estimatedTimeRemaining) + "s, " +
               "segment=" + std::to_string(currentSegment.segmentIndex) + "/" +
               std::to_string(currentSegmentProgress * 100) + "%, " +
               "speed=" + std::to_string(trends.instantaneousSpeed) + "m/s}";
    }
};

/**
 * @brief 확장된 내비게이션 상태 (RouteProgress 포함)
 * 
 * 기존 NavigationState에 RouteProgress를 추가하여
 * 경로 진행률 기반 시각적 피드백을 지원합니다.
 */
struct ExtendedNavigationStateV2 : public NavigationState {
    RouteProgress routeProgress;           // 경로 진행률 정보
    
    // 시각적 피드백 관련 설정
    struct VisualFeedbackSettings {
        bool enableProgressAnimation = true;
        bool enableMilestoneNotifications = true;
        bool enableETAUpdates = true;
        double animationSpeed = 1.0;      // 애니메이션 속도 배율
        double feedbackSensitivity = 0.01; // 피드백 민감도 (1% 변화시 업데이트)
    } visualSettings;
    
    // === 유효성 검증 ===
    bool isValid() const {
        return routeProgress.isValid();
    }
    
    // === 진행률 기반 상태 업데이트 ===
    void updateWithProgress(const RouteProgress& progress) {
        routeProgress = progress;
        
        // NavigationState의 관련 필드들도 업데이트
        if (progress.isValid()) {
            // ETA를 NavigationState에도 반영
            // estimatedArrivalTime 등의 필드가 있다면 업데이트
        }
    }
    
    // === 진행률 변화 감지 ===
    bool hasSignificantProgressChange(const RouteProgress& newProgress) const {
        double percentageDiff = std::abs(newProgress.completionPercentage - routeProgress.completionPercentage);
        return percentageDiff >= visualSettings.feedbackSensitivity;
    }
};

} // namespace domain::state