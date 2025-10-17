#pragma once
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace infrastructure {
namespace persistence {

/**
 * @brief WXT-33/34/35: 캐시 성능 모니터링 및 최적화 시스템
 * 
 * 타일 캐시의 성능을 모니터링하고 히트율을 최적화하는 종합 관리 시스템
 * 실시간 메트릭 수집 및 성능 분석 기능 제공
 */
class CacheManager {
public:
    struct CacheMetrics {
        size_t totalRequests;          // 총 요청 수
        size_t hitCount;               // 캐시 히트 수
        size_t missCount;              // 캐시 미스 수
        double hitRatio;               // 히트율 (0.0 - 1.0)
        size_t totalSize;              // 총 캐시 크기 (bytes)
        size_t usedSize;               // 사용 중인 크기 (bytes)
        double avgAccessTime;          // 평균 접근 시간 (ms)
        std::chrono::steady_clock::time_point lastUpdated;
    };

    struct CacheEntry {
        std::string key;
        std::vector<uint8_t> data;
        std::chrono::steady_clock::time_point createdAt;
        std::chrono::steady_clock::time_point lastAccessed;
        size_t accessCount;
        int priority;                  // 우선순위 (0-10)
    };

    enum class EvictionPolicy {
        LRU,                          // Least Recently Used
        LFU,                          // Least Frequently Used
        FIFO,                         // First In First Out
        ADAPTIVE                      // 적응형 정책
    };

public:
    CacheManager(size_t maxSize = 1024 * 1024 * 500); // 기본 500MB
    ~CacheManager();

    // WXT-33: 캐시 성능 모니터링 시스템
    CacheMetrics getCurrentMetrics() const;
    std::vector<CacheMetrics> getHistoricalMetrics(int days = 7) const;
    void startMonitoring(int intervalSeconds = 60);
    void stopMonitoring();

    // WXT-34: 캐시 히트율 최적화 로깅
    void logCacheAccess(const std::string& key, bool isHit, double accessTimeMs);
    void analyzeAccessPatterns();
    std::vector<std::string> getOptimizationRecommendations();
    void exportHitRatioReport(const std::string& filePath);

    // WXT-35: 타일 캐시 성능 메트릭 배출
    void exportMetricsToFile(const std::string& filePath);
    void exportMetricsToDatabase(const std::string& connectionString);
    std::string getMetricsAsJson() const;
    void configureMetricsExport(int intervalMinutes = 15);

    // 기본 캐시 연산
    bool get(const std::string& key, std::vector<uint8_t>& data);
    void put(const std::string& key, const std::vector<uint8_t>& data, int priority = 5);
    void remove(const std::string& key);
    void clear();

    // 캐시 정책 관리
    void setEvictionPolicy(EvictionPolicy policy);
    void setMaxSize(size_t maxSize);
    void setPrefetchThreshold(double threshold = 0.8);

    // 고급 기능
    void preloadCriticalData(const std::vector<std::string>& keys);
    void optimizeCache();
    bool validateCacheIntegrity();

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<CacheEntry>> cache_;
    
    // 설정
    size_t maxSize_;
    EvictionPolicy evictionPolicy_;
    double prefetchThreshold_;
    
    // 메트릭스
    CacheMetrics currentMetrics_;
    std::vector<CacheMetrics> historicalMetrics_;
    
    // 모니터링
    bool isMonitoring_;
    std::unique_ptr<std::thread> monitoringThread_;
    
    // 접근 패턴 분석
    std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> accessHistory_;
    
    // 내부 유틸리티
    void updateMetrics();
    void evictEntries();
    void evictLRU();
    void evictLFU();
    void evictFIFO();
    void evictAdaptive();
    
    double calculateHitRatio() const;
    size_t calculateUsedSize() const;
    std::vector<std::string> identifyHotKeys() const;
    std::vector<std::string> identifyColdKeys() const;
};

} // namespace persistence
} // namespace infrastructure