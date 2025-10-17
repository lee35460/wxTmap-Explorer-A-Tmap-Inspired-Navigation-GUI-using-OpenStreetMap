#pragma once
#include <chrono>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "../../Types.h"
#include "../IRouteCalculator.h"

namespace domain {
namespace services {

/**
 * @brief WXT-15: 경로 알고리즘 성능 벤치마킹
 * 
 * 다양한 경로 계산 알고리즘의 성능을 측정하고 비교하는 시스템
 * P95 ≤ 200ms 성능 목표 달성을 위한 벤치마킹 도구
 * 
 * 주요 기능:
 * - 실시간 성능 측정 및 통계 생성
 * - 다중 알고리즘 비교 분석
 * - 성능 회귀 감지 및 알림
 * - 자동화된 성능 리포트 생성
 */
class PerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string algorithmName;
        double averageMs;
        double p95Ms;
        double p99Ms;
        size_t totalTests;
        size_t failedTests;
        std::chrono::steady_clock::time_point timestamp;
    };

    struct TestCase {
        LonLat start;
        LonLat end;
        std::vector<LonLat> waypoints;
        std::string description;
    };

public:
    PerformanceBenchmark();
    ~PerformanceBenchmark();

    // 벤치마크 테스트 케이스 추가
    void addTestCase(const TestCase& testCase);
    void loadStandardTestCases();

    // 특정 알고리즘 벤치마크 실행
    BenchmarkResult runBenchmark(const std::string& algorithmName, 
                                size_t iterations = 100);

    // 모든 등록된 알고리즘 비교 벤치마크
    std::vector<BenchmarkResult> runComparativeBenchmark(size_t iterations = 100);

    // 성능 목표 검증 (P95 ≤ 200ms)
    bool validatePerformanceTarget(const BenchmarkResult& result, 
                                 double targetP95Ms = 200.0);

    // 결과 내보내기
    void exportResults(const std::vector<BenchmarkResult>& results, 
                      const std::string& filePath);

    // 히스토리컬 성능 트렌드 분석
    void analyzePerformanceTrend(const std::string& algorithmName, 
                               size_t days = 30);

private:
    std::vector<TestCase> testCases_;
    std::vector<std::string> registeredAlgorithms_;

    // 성능 측정 유틸리티
    template<typename Func>
    double measureExecutionTime(Func&& func);

    // 통계 계산
    double calculatePercentile(std::vector<double>& times, double percentile);
    double calculateAverage(const std::vector<double>& times);
};

} // namespace services
} // namespace domain