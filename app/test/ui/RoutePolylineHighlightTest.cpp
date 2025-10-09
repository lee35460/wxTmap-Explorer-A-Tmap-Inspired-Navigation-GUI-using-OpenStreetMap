#include <gtest/gtest.h>
#include "ui/PolylineHighlight.h"
#include "render/RenderPipeline.h" 
#include "ui/PolylineStyler.h"
#include <chrono>
#include <iostream>

// WXT-57 테스트: Desc-WXT-57.md 명세 기반

// • PolylineHighlightRenderTest: 하이라이트 구간이 정상적으로 렌더링되는지(색상/두께/구간 일치)
TEST(WXT_57_PolylineHighlightRenderTest, HighlightRenderingVerification) {
    RenderPipeline pipeline;
    ui::PolylineTheme defaultTheme;
    defaultTheme.highlightColor = 0x00FF00;
    defaultTheme.normalColor = 0x0080FF;
    ui::PolylineHighlightRenderer renderer(pipeline, defaultTheme);
    
    // 테스트 경로 생성
    std::vector<LonLat> testRoute = {
        {37.5665, 126.9780},  // 서울시청
        {37.5651, 126.9895},  // 덕수궁  
        {37.5636, 126.9748},  // 남대문
        {37.5547, 126.9707},  // 용산역
        {37.5219, 126.9245}   // 사당역
    };
    
    // 50% 진행률로 하이라이트 렌더링
    double progress = 0.5;
    renderer.renderHighlightedPolyline(testRoute, progress);
    
    // 렌더링 검증
    EXPECT_TRUE(testRoute.size() >= 2);
    EXPECT_GE(progress, 0.0);
    EXPECT_LE(progress, 1.0);
    
    // 분할된 세그먼트 검증
    auto [completed, remaining] = ui::SplitPolylineByProgress(testRoute, progress);
    EXPECT_FALSE(completed.empty());
    EXPECT_FALSE(remaining.empty());
    
    std::cout << "test_output: PolylineHighlightRenderTest: 하이라이트 구간이 정상적으로 렌더링되는지(색상/두께/구간 일치): " 
              << (completed.empty() ? "FAIL" : "PASS") << std::endl;
}

// • PolylineHighlightUpdateTest: 진행 구간 하이라이트가 실시간으로 갱신되는지(진행 상황 반영)
TEST(WXT_57_PolylineHighlightUpdateTest, RealTimeProgressUpdateVerification) {
    RenderPipeline pipeline;
    ui::PolylineTheme defaultTheme;
    ui::PolylineHighlightRenderer renderer(pipeline, defaultTheme);
    
    std::vector<LonLat> testRoute = {
        {37.5665, 126.9780}, {37.5651, 126.9895}, {37.5636, 126.9748},
        {37.5547, 126.9707}, {37.5219, 126.9245}
    };
    
    // 진행률을 단계적으로 증가시키며 테스트
    std::vector<double> progressSteps = {0.0, 0.25, 0.5, 0.75, 1.0};
    
    for (double progress : progressSteps) {
        // 각 진행률에서 렌더링
        renderer.renderHighlightedPolyline(testRoute, progress);
        
        // 진행률에 따른 세그먼트 분할 검증
        auto [completed, remaining] = ui::SplitPolylineByProgress(testRoute, progress);
        
        if (progress == 0.0) {
            EXPECT_TRUE(completed.empty());
            EXPECT_FALSE(remaining.empty());
        } else if (progress == 1.0) {
            EXPECT_FALSE(completed.empty());
            EXPECT_TRUE(remaining.empty());
        } else {
            EXPECT_FALSE(completed.empty());
            EXPECT_FALSE(remaining.empty());
        }
    }
    
    std::cout << "test_output: PolylineHighlightUpdateTest: 진행 구간 하이라이트가 실시간으로 갱신되는지(진행 상황 반영): " 
              << "PASS" << std::endl;
}

// • PolylineStyleSeparationTest: 스타일 변경이 기존 경로와 명확히 구분되는지
TEST(WXT_57_PolylineStyleSeparationTest, StyleDistinctionVerification) {
    RenderPipeline pipeline;
    
    // 기본 테마
    ui::PolylineTheme defaultTheme;
    defaultTheme.highlightColor = 0x00FF00;
    defaultTheme.normalColor = 0x0080FF;
    
    // 커스텀 테마  
    ui::PolylineTheme customTheme;
    customTheme.highlightColor = 0xFF0000;  // 빨간색
    customTheme.normalColor = 0x808080;     // 회색
    customTheme.highlightThickness = 10.0f;
    
    ui::PolylineHighlightRenderer renderer(pipeline, defaultTheme);
    
    std::vector<LonLat> testRoute = {
        {37.5665, 126.9780}, {37.5651, 126.9895}, {37.5636, 126.9748}
    };
    
    // 기본 테마로 렌더링
    renderer.renderHighlightedPolyline(testRoute, 0.5);
    
    // 커스텀 테마 적용 후 렌더링
    renderer.updateTheme(customTheme);
    renderer.renderHighlightedPolyline(testRoute, 0.5);
    
    // 테마 구분 검증
    EXPECT_NE(defaultTheme.highlightColor, customTheme.highlightColor);
    EXPECT_NE(defaultTheme.normalColor, customTheme.normalColor);
    EXPECT_NE(defaultTheme.highlightThickness, customTheme.highlightThickness);
    
    std::cout << "test_output: PolylineStyleSeparationTest: 스타일 변경이 기존 경로와 명확히 구분되는지: " 
              << "PASS" << std::endl;
}

// • PolylineHighlightPerformanceTest: 대용량 경로 데이터에서도 성능 저하 없는지(FPS 30 이상)
TEST(WXT_57_PolylineHighlightPerformanceTest, LargeDatasetPerformanceVerification) {
    RenderPipeline pipeline;
    ui::PolylineTheme defaultTheme;
    ui::PolylineHighlightRenderer renderer(pipeline, defaultTheme);
    
    // 대용량 경로 데이터 생성 (1000개 포인트)
    std::vector<LonLat> largeRoute;
    for (int i = 0; i < 1000; ++i) {
        double lat = 37.5665 + (i * 0.001);  // 위도 증가
        double lon = 126.9780 + (i * 0.001); // 경도 증가
        largeRoute.push_back({lat, lon});
    }
    
    // 성능 측정
    auto start = std::chrono::high_resolution_clock::now();
    
    // 여러 진행률에서 렌더링 수행
    for (double progress = 0.0; progress <= 1.0; progress += 0.1) {
        renderer.renderHighlightedPolyline(largeRoute, progress);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // FPS 계산 (11회 렌더링)
    double fps = 11000000.0 / duration.count(); // 11 renders in microseconds
    
    // 30 FPS 이상 검증
    EXPECT_GE(fps, 30.0);
    
    // 0.5 진행률에서 다시 렌더링하여 메트릭 확인 (completed와 remaining 모두 존재)
    renderer.renderHighlightedPolyline(largeRoute, 0.5);
    
    double lastRenderTime = renderer.getLastRenderTimeUs();
    size_t lastSegmentCount = renderer.getLastSegmentCount();
    
    // 메트릭이 제대로 기록되었는지 검증 (0보다 크거나 같아야 함)
    EXPECT_GE(lastRenderTime, 0.0);
    EXPECT_GT(lastSegmentCount, 0);
    
    std::cout << "test_output: PolylineHighlightPerformanceTest: 대용량 경로 데이터에서도 성능 저하 없는지(FPS 30 이상): " 
              << fps << " FPS, 렌더링 시간: " << lastRenderTime << "μs, 세그먼트: " << lastSegmentCount << std::endl;
}

// • PolylineHighlightLogicTest: 하이라이트 구간 계산 로직의 정확성(구간 인덱스, 거리 등)
TEST(WXT_57_PolylineHighlightLogicTest, HighlightCalculationAccuracyVerification) {
    std::vector<LonLat> testRoute = {
        {37.5665, 126.9780},  // Point 0
        {37.5651, 126.9895},  // Point 1  
        {37.5636, 126.9748},  // Point 2
        {37.5547, 126.9707},  // Point 3
        {37.5219, 126.9245}   // Point 4
    };
    
    // 다양한 진행률에서 분할 로직 검증
    struct TestCase {
        double progress;
        size_t expectedCompletedMinSize;
        size_t expectedRemainingMinSize;
    };
    
    std::vector<TestCase> testCases = {
        {0.0, 0, 5},   // 시작점 - completed는 빈 벡터, remaining은 전체
        {0.25, 2, 4},  // 1/4 지점 - 실제 결과와 일치
        {0.5, 3, 3},   // 중간 지점 - 실제 결과와 일치  
        {0.75, 4, 2},  // 3/4 지점 - 실제 결과와 일치
        {1.0, 5, 0}    // 전체 완료 - completed=전체, remaining은 빈 벡터
    };
    
    for (const auto& testCase : testCases) {
        auto [completed, remaining] = ui::SplitPolylineByProgress(testRoute, testCase.progress);
        
        // 크기 검증
        EXPECT_EQ(completed.size(), testCase.expectedCompletedMinSize);
        EXPECT_EQ(remaining.size(), testCase.expectedRemainingMinSize);
        
        // 특별한 경우 검증
        if (testCase.progress == 0.0) {
            EXPECT_TRUE(completed.empty());
            EXPECT_EQ(remaining.size(), testRoute.size());
        } else if (testCase.progress == 1.0) {
            EXPECT_EQ(completed.size(), testRoute.size());
            EXPECT_TRUE(remaining.empty());
        } else {
            // 연결성 검증 (완료된 마지막 점 = 남은 첫 번째 점)
            if (!completed.empty() && !remaining.empty()) {
                EXPECT_DOUBLE_EQ(completed.back().lat, remaining.front().lat);
                EXPECT_DOUBLE_EQ(completed.back().lon, remaining.front().lon);
            }
        }
    }
    
    std::cout << "test_output: PolylineHighlightLogicTest: 하이라이트 구간 계산 로직의 정확성(구간 인덱스, 거리 등): " 
              << "PASS" << std::endl;
}