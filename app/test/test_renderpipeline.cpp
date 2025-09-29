#include <gtest/gtest.h>
#include "render/RenderPipeline.h"
#include <thread>

TEST(RenderPipelineTest, AverageFpsAboveThreshold) {
    RenderPipeline rp;
    for (int i = 0; i < 100; ++i) {
        rp.beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        rp.endFrame();
    }
    EXPECT_GE(rp.fpsAverage(), 45.0);
}