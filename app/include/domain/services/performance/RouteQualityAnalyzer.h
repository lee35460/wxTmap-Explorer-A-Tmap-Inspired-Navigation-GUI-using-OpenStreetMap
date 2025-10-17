#pragma once
#include <vector>
#include <memory>
#include <string>
#include "../../Types.h"
#include "../../entities/Route.h"

namespace domain {
namespace services {

/**
 * @brief WXT-16: 경로 품질 저하 이슈 해결
 * 
 * 경로 품질을 분석하고 개선점을 찾는 시스템
 * 경로의 다양한 품질 지표를 측정하고 최적화 제안을 제공
 */
class RouteQualityAnalyzer {
public:
    struct QualityMetrics {
        double totalDistance;           // 총 거리 (km)
        double estimatedTime;          // 예상 시간 (분)
        double efficiency;             // 효율성 (직선거리 대비)
        double smoothness;             // 부드러움 (급회전 횟수)
        double trafficScore;           // 교통 상황 점수
        double safetyScore;            // 안전성 점수
        double comfortScore;           // 편의성 점수
        double overallQuality;         // 종합 품질 점수 (0-100)
    };

    struct QualityIssue {
        enum Type {
            EXCESSIVE_DETOUR,          // 과도한 우회
            SHARP_TURNS,               // 급회전
            HIGH_TRAFFIC_AREA,         // 고교통량 구간
            UNSAFE_INTERSECTION,       // 위험한 교차로
            POOR_ROAD_CONDITION        // 도로 상태 불량
        };

        Type type;
        LonLat location;
        std::string description;
        int severity;                  // 1-10 심각도
        std::string suggestion;        // 개선 제안
    };

    struct AnalysisReport {
        QualityMetrics metrics;
        std::vector<QualityIssue> issues;
        std::vector<std::string> recommendations;
        double confidenceScore;        // 분석 신뢰도
        std::chrono::steady_clock::time_point timestamp;
    };

public:
    RouteQualityAnalyzer();
    ~RouteQualityAnalyzer();

    // 경로 품질 분석
    AnalysisReport analyzeRoute(const Route& route);
    
    // 배치 분석 (여러 경로 동시 분석)
    std::vector<AnalysisReport> analyzeBatch(const std::vector<Route>& routes);

    // 품질 기준 설정
    void setQualityThresholds(double minEfficiency = 0.8, 
                            double maxDetourRatio = 1.3,
                            int maxSharpTurns = 5);

    // 경로 비교 분석
    AnalysisReport compareRoutes(const Route& route1, const Route& route2);

    // 품질 개선 제안
    std::vector<Route> suggestAlternativeRoutes(const Route& originalRoute, 
                                              int maxAlternatives = 3);

    // 품질 트렌드 분석
    void analyzeQualityTrend(const std::vector<AnalysisReport>& historicalData);

    // 리포트 내보내기
    void exportReport(const AnalysisReport& report, const std::string& filePath);

private:
    // 품질 기준값들
    double minEfficiency_;
    double maxDetourRatio_;
    int maxSharpTurns_;

    // 분석 알고리즘들
    double calculateEfficiency(const Route& route);
    double calculateSmoothness(const Route& route);
    double calculateTrafficScore(const Route& route);
    double calculateSafetyScore(const Route& route);
    double calculateComfortScore(const Route& route);
    
    // 이슈 감지 알고리즘들
    std::vector<QualityIssue> detectDetourIssues(const Route& route);
    std::vector<QualityIssue> detectSharpTurns(const Route& route);
    std::vector<QualityIssue> detectTrafficIssues(const Route& route);
    std::vector<QualityIssue> detectSafetyIssues(const Route& route);

    // 유틸리티 함수들
    double calculateDirectDistance(const LonLat& start, const LonLat& end);
    double calculateTurnAngle(const LonLat& p1, const LonLat& p2, const LonLat& p3);
};

} // namespace services
} // namespace domain