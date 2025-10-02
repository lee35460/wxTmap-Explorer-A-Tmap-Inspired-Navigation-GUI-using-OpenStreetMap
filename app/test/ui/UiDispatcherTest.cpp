#include "ui/UiDispatcher.h"
#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <thread>
#include <iostream>

TEST(UiDispatcherTest, BackpressureDropWhenQueueFull) {
    // 동기 poster(바로 실행)로는 pending이 잘 안 쌓이므로
    // 일부러 느리게 실행되는 poster를 만들어 대기열을 쌓이게 한다.
    UiDispatcher::Poster slowPoster = [](std::function<void()> fn) {
        std::thread([fn = std::move(fn)]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            fn();
        }).detach();
    };

    std::atomic<size_t> metric_seen{0};
    UiDispatcher::MetricFn metric = [&](const std::string&, double){
        metric_seen++;
    };

    UiDispatcherConfig cfg;
    cfg.enable_drop = true;
    cfg.max_queue = 8; // 작게 설정

    UiDispatcher dispatcher(cfg, slowPoster, metric);
    size_t ok = 0, fail = 0;
    std::vector<std::thread> ths;
    for (int t = 0; t < 4; ++t) {
        ths.emplace_back([&](){
            for (int i = 0; i < 50; ++i) {
                if (dispatcher.post([]{})) ++ok;
                else ++fail;
            }
        });
    }

    for (auto& th : ths) th.join();

    // 결과 값 출력 (디버그)
    std::cout << "ok: " << ok << ", fail: " << fail << ", metric_seen: " << metric_seen.load() << std::endl;
    // 큐 상한이 작으므로 drop이 발생했을 것이다.
    EXPECT_GT(fail, 0);
    EXPECT_EQ(ok, 200 - fail);
    EXPECT_GE(metric_seen.load(), 1u);
    std::cout << "ok: " << ok << ", fail: " << fail << std::endl;
}

TEST(UiDispatcherTest, MetricsLatencyRecorded) {
    UiDispatcher::Poster testPoster = [](std::function<void()> fn) { fn(); };
    UiDispatcher d(UiDispatcherConfig{}, testPoster,
        [&](const std::string& key, double value){
            if (key == "ui_callback_latency_ms") {
                EXPECT_GE(value, 0.0);
            }
        });

    ASSERT_TRUE(d.post([]{}));
}