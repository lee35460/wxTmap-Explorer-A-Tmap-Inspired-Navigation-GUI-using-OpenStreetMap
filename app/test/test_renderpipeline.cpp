#include <gtest/gtest.h>
#include "render/RenderPipeline.h"
#include "render/RenderMetricsExporter.h"
#include <fstream>
#include <cstdio>
#include <thread>

TEST(WXT_4_RenderPipelineTest, AverageFpsAboveThreshold) {
    RenderPipeline rp;
    for (int i = 0; i < 100; ++i) {
        rp.beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rp.endFrame();
    }
    double avg_fps = rp.fpsAverage();
    // 결과 출력 (단위/의미 포함) - Desc 항목명과 일치
    std::cout << "test_output: RenderPipelineTest: 렌더 파이프라인 정상 동작 (평균 FPS: 30fps 이상): " 
              << avg_fps << std::endl;
    EXPECT_GE(avg_fps, 30.0);  // 30fps 이상이면 통과
}

TEST(WXT_4_RenderPipelineMetricsTest, MetricsExporter_WritesCsvWithFps) {
    RenderPipeline rp;
    for (int i = 0; i < 10; ++i) {
        rp.beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        rp.endFrame();
    }
    const char* path = "metrics_test.csv";
    std::remove(path); // ensure clean file
    exportMetrics(rp, path);

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