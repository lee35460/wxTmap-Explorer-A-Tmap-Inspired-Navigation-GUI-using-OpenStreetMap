#include "ui/UiDispatcher.h"
#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <iostream>

TEST(WXT_4_UiDispatcherTest, BackpressureDropWhenQueueFull) {
    // 동기 poster(바로 실행)로는 pending이 잘 안 쌓이므로
    // 일부러 느리게 실행되는 poster를 만들어 대기열을 쌓이게 한다.
    UiDispatcher::Poster slowPoster = [](std::function<void()> fn) {
        std::thread([fn = std::move(fn)]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
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

    // 큐 상한이 작으므로 drop이 발생했을 것이다.
    EXPECT_GT(fail, 0);
    EXPECT_EQ(ok, 200 - fail);
    EXPECT_GE(metric_seen.load(), 1u);
    // 결과 출력 (단위/의미 포함) - Desc 항목명과 일치
    std::cout << "test_output: 메트릭 계측 및 백프레셔 테스트 (메트릭 수집 횟수: 1회 이상, 백프레셔 동작 확인): " << metric_seen.load() 
              << std::endl;
}

TEST(WXT_4_UiDispatcherTest, MetricsLatencyRecorded) {
    UiDispatcher::Poster testPoster = [](std::function<void()> fn) { fn(); };
    bool all_pass = true;
    UiDispatcher d(UiDispatcherConfig{}, testPoster,
        [&](const std::string& key, double value){
            if (key == "ui_callback_latency_ms") {
                if (!(value >= 0.0)) all_pass = false;
                EXPECT_GE(value, 0.0);
            }
            else if (key == "ui_queue_len") {
                if (!(value >= 0.0)) all_pass = false;
                EXPECT_GE(value, 0.0);
            }
            else if (key == "ui_drop_cnt") {
                if (!(value >= 0.0)) all_pass = false;
                EXPECT_GE(value, 0.0);
            }
            else {
                all_pass = false;
                FAIL() << "Unexpected metric key: " << key;
            }
        });

    bool post_ok = d.post([]{});
    all_pass = all_pass && post_ok;
    ASSERT_TRUE(post_ok);
    // 결과 출력 (GoogleTest 결과로 통과/실패 자동 판정) - Desc 항목명과 일치
    std::cout << "test_output: UiDispatcherTest: 비동기 → UI 마샬링 정상 동작 (성공률: OK/FAIL, 메트릭 검증 및 post 성공): " 
              << (all_pass ? "OK" : "FAIL") << std::endl;
}