#pragma once
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>

// wx 의존은 "기본 poster"를 쓸 때만 필요하도록 분리
// (단위 테스크에선 커스텀  poster 주입으로 wx 링크 없이도 테스크 가능)
struct UiDispatcherConfig
{
    /* data */
    size_t max_queue = 256;         // 큐 길이 상한
    bool   enable_drop = true;      // 초과 시 드롭
    bool   enable_coalesce = true;  // 동일 키 작업 합치기(옵션: 여기선 단순 드롭으로 시작)
};

class UiDispatcher {
public:
    // Poster : UI 스레드로 전달하는 함수 (기본은 wxCallAfter)
    using Poster = std::function<void(std::function<void()>)>;
    // MetricFn : 매트릭 기록 (key, value)
    using MetricFn = std::function<void(const std::string&, double)>;

    UiDispatcher(const UiDispatcherConfig& cfg, Poster poster, MetricFn metric = nullptr) 
    : cfg_(cfg), poster_(std::move(poster)), metric_(std::move(metric)), pending_(0), drop_cnt_(0) {}

    // UI 스레드로 콜백 전달. 성공 여부 반환.
    bool post(std::function<void()> fn) {
        // 백프레셔: 대기 수 체크
        {
            std::lock_guard<std::mutex> lk(mu_); // pending_ 증가
            if (cfg_.enable_drop && pending_ >= cfg_.max_queue) {
                ++drop_cnt_; // 드롭 카운트
                record("ui_drop_cnt", static_cast<double>(drop_cnt_.load()));
                record("ui_queue_len", static_cast<double>(pending_.load()));
                return false;
            }
            ++pending_;
            record("ui_queue_len", static_cast<double>(pending_.load()));
        }

        auto t0 = std::chrono::steady_clock::now();

        // UI 스레드로 전달
        poster_([this, t0, fn = std::move(fn)]() mutable {
            fn();
            auto t1 = std::chrono::steady_clock::now();
            double latency_ms =
                std::chrono::duration<double, std::milli>(t1 - t0).count();
            record("ui_callback_latency_ms", latency_ms);

            std::lock_guard<std::mutex> lk(mu_); // pending_ 감소
            if (pending_ > 0) --pending_; // 안전장치
            record("ui_queue_len", static_cast<double>(pending_.load()));
        });

        return true;
    }

    size_t pending() const { return pending_.load(); }
    size_t drops() const { return drop_cnt_.load(); }
    
private:
    void record(const std::string& key, double value) {
        // 매트릭 기록 헬퍼
        if (metric_) metric_(key, value);
    }

    UiDispatcherConfig cfg_;
    Poster poster_;
    MetricFn metric_;
    mutable std::mutex mu_;
    std::atomic<size_t> pending_;   // 대기 중 작업 수
    std::atomic<size_t> drop_cnt_;  // 드롭된 작업 수
    
};

// 기본 wxPoster (wxCallAfter 래퍼)
#ifdef __has_include
#    if __has_include(<wx/wx.h>)
#        define WXT_HAVE_WX 1
#    endif
#endif

#if WXT_HAVE_WX
#include <wx/wx.h>
static inline UiDispatcher::Poster MakeWxPoster() {
    // // wxWidgets: UI 스레드로 보장되는 CallAfter
    return [](std::function<void()> f) {
        wxTheApp->CallAfter([fn = std::move(f)]() { fn(); });
    };
}
#else
static inline UiDispatcher::Poster MakeWxPoster() {
    // wx 없음: 그냥 즉시 실행 (테스트용)
    return [](std::function<void()> f){ f(); };
}
#endif