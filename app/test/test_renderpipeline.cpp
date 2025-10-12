#include <gtest/gtest.h>
#include "render/RenderPipeline.h"
#include "render/RenderMetricsExporter.h"
#include <fstream>
#include <cstdio>
#include <thread>
#include <memory>

// 복잡한 RenderPipeline 객체 관리를 위한 픽스처 (WXT-4 이슈용)
class WXT_4_RenderPipelineTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        renderPipeline_ = std::make_unique<RenderPipeline>();
    }
    
    void TearDown() override {
        // CSV 테스트 파일 정리 (시스템 리소스)
        std::remove("metrics_test.csv");
    }
    
protected:
    std::unique_ptr<RenderPipeline> renderPipeline_;
};

TEST_F(WXT_4_RenderPipelineTestFixture, AverageFpsAboveThreshold) {
    // 픽스처의 renderPipeline_ 사용
    for (int i = 0; i < 100; ++i) {
        renderPipeline_->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        renderPipeline_->endFrame();
    }
    double avg_fps = renderPipeline_->fpsAverage();
    // 결과 출력 (단위/의미 포함) - Desc 항목명과 일치
    std::cout << "test_output: RenderPipelineTest: 렌더 파이프라인 정상 동작 (평균 FPS: 30fps 이상): " 
              << avg_fps << std::endl;
    EXPECT_GE(avg_fps, 30.0);  // 30fps 이상이면 통과
}

TEST_F(WXT_4_RenderPipelineTestFixture, MetricsExporter_WritesCsvWithFps) {
    // 픽스처의 renderPipeline_ 사용
    for (int i = 0; i < 10; ++i) {
        renderPipeline_->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        renderPipeline_->endFrame();
    }
    const char* path = "metrics_test.csv";
    exportMetrics(*renderPipeline_, path);  // 픽스처 객체 참조로 전달

    std::ifstream in(path);
    ASSERT_TRUE(in.good()) << "metrics file not created";
    std::string line;
    std::getline(in, line);
    in.close();

    ASSERT_FALSE(line.empty());
    // Expect prefix "fps_avg," and a numeric value after
    // ASSERT_NE(line.find("fps_avg,"), std::string::npos) << "csv header missing";
    
    // CSV에서 fps 값 추출
    double fps_val = 0.0;
    if (line.find("fps_avg,") == 0) {
        std::string fps_str = line.substr(8);
        try {
            fps_val = std::stod(fps_str);
        } catch (...) { fps_val = 0.0; }
    }
    
    // 결과 출력 (단위/의미 포함) - Desc 항목명과 일치
    std::cout << "test_output: RenderPipelineMetricsTest: 렌더 파이프라인 메트릭 검증 (CSV 출력 FPS: 30fps 이상): " 
              << fps_val << std::endl;
    
    EXPECT_GE(fps_val, 30.0);  // CSV의 fps 값이 30 이상이면 통과
}