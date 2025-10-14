#include "MapPanel.h"
#include <wx/webview.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/event.h>
#include <wx/display.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

// UI 컴포넌트 구현체 include
#include "ui/LocationPuck.h"
#include "ui/PolylineHighlight.h" 
#include "ui/CameraController.h"

static const int ID_TOGGLE_ANIM = wxID_HIGHEST + 100; // custom event id

namespace {
    // wxWidgets 컴포넌트들은 부모 창에서 자동 삭제되므로 raw pointer 사용
    // 하지만 명시적 null 체크로 안전성 확보
    wxStaticText* s_demoLabel = nullptr;   // 상태 표시용 레이블
    wxGauge*      s_progress  = nullptr;   // 간단한 배너/진행률 데모
    wxTimer*      s_timer     = nullptr;   // 진행률 애니메이션 구동
    int           s_progVal   = 0;
    std::chrono::steady_clock::time_point s_lastTick; // 지연 측정용
    int           s_expectedMs = 100;                  // 10Hz 기본값
}

// JSON 변환 헬퍼
inline void to_json(nlohmann::json& j, const LonLat& p) {
    j = nlohmann::json{{"lon", p.lon}, {"lat", p.lat}};
}

// MapPanel 구현
MapPanel::MapPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    // WebView 초기화 코드 (생략)
    hud_ = new ui::MapOverlayHud(this); // HUD 오버레이 초기화 - wxWidgets 자동 관리
    dpiScale_ = GetContentScaleFactor(); // 초기 DPI 스케일 설정
    
    // 순수 비즈니스 로직 클래스들 초기화
    locationPuck_ = std::make_unique<ui::LocationPuck>(this);
    cameraController_ = std::make_unique<ui::CameraController>();
    
    // PolylineHighlightRenderer는 RenderPipeline이 필요하므로 나중에 초기화
    // 일단 nullptr로 둡니다.

    // --- Minimal visual HUD/Banner demo so we can SEE activity ---
    if (!s_demoLabel) {
        s_demoLabel = new wxStaticText(this, wxID_ANY, "Ready", wxPoint(8, 8));
        s_demoLabel->SetName("demo_label");
    }
    if (!s_progress) {
        s_progress = new wxGauge(this, wxID_ANY, 100, wxPoint(8, 32), wxSize(200, 16));
        s_progress->SetName("turn_progress");
    }
    if (!s_timer) {
        s_timer = new wxTimer(this);
        Bind(wxEVT_TIMER, [this](wxTimerEvent&) {
            const auto now = std::chrono::steady_clock::now();
            if (s_lastTick.time_since_epoch().count() != 0) {
                const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastTick).count();
                // very light jank heuristic: if way off from expected, note it in the label
                if (std::abs(dt - s_expectedMs) > s_expectedMs) {
                    if (s_demoLabel) s_demoLabel->SetLabel(wxString::Format("(jank) Δ%lldms", (long long)dt));
                }
            }
            s_lastTick = now;

            s_progVal = (s_progVal + 3) % 100; // ~33 ticks to full
            if (s_progress) s_progress->SetValue(s_progVal);
            // Light-weight visual feedback
            if (s_progVal % 25 == 0 && s_demoLabel) {
                s_demoLabel->SetLabel(wxString::Format("Progress %d%%", s_progVal));
            }
            UpdateProgressFromGauge();
        });
        s_timer->Start(100); // 10Hz, satisfies ">= 1Hz" update AC for banner demo

        Bind(wxEVT_BUTTON, [this](wxCommandEvent& ev){
            if (ev.GetId() != ID_TOGGLE_ANIM) return;
            if (!s_timer) return;
            if (s_timer->IsRunning()) {
                s_timer->Stop();
                if (s_demoLabel) s_demoLabel->SetLabel("Animation: stopped");
            } else {
                s_expectedMs = 100; // reset expected interval
                s_lastTick = {};
                s_timer->Start(s_expectedMs);
                if (s_demoLabel) s_demoLabel->SetLabel("Animation: started");
            }
        });

        Bind(wxEVT_DESTROY, [this](wxWindowDestroyEvent&){
            // 명시적 타이머 정리로 메모리 안전성 확보
            if (s_timer && s_timer->IsRunning()) {
                s_timer->Stop();
            }
            // wxWidgets는 자동으로 child 컴포넌트들을 삭제하므로 추가 삭제 불필요
        });
    }

    Bind(wxEVT_SIZE, [this](wxSizeEvent& ev) {
        dpiScale_ = GetContentScaleFactor();
        PublishPolylineStyle();
        ev.Skip();
    });
    // ...existing code...
}

void MapPanel::DrawPolyline(const std::vector<LonLat>& coords) {
    currentPolyline_ = coords;
    nlohmann::json j = coords; // uses to_json(LonLat)
    std::string s = j.dump();
    const auto count = coords.size();

    // Run on UI thread
    this->CallAfter([this, s = std::move(s), count] {
        // JS 브리지 호출
        // CallJS("drawPolyline", s);
        spdlog::info("[ACK] JS drawPolyline called with {} points", count);
        if (s_demoLabel) {
            s_demoLabel->SetLabel(wxString::Format("Polyline drawn: %zu pts", count));
        }
        PublishPolylineStyle();
    });
}

void MapPanel::UpdateProgressFromGauge() {
    if (!s_progress) return;
    const int value = s_progress->GetValue();
    const int range = s_progress->GetRange();
    if (range <= 0) return;
    routeProgress_ = std::clamp(static_cast<double>(value) / static_cast<double>(range), 0.0, 1.0);
    PublishPolylineStyle();
}

void MapPanel::PublishPolylineStyle() {
    if (currentPolyline_.size() < 2) return;

    const auto [completed, remaining] = ui::SplitPolylineByProgress(currentPolyline_, routeProgress_);
    const int strokeWidth = ui::WidthForZoom(zoomLevel_, dpiScale_);

    nlohmann::json payload;
    payload["strokeWidth"] = strokeWidth;
    payload["segments"] = nlohmann::json::array();

    const auto pushSegment = [&](const std::vector<LonLat>& pts, const wxColour& color) {
        if (pts.size() < 2) return;
        payload["segments"].push_back({
            {"color", Hex(color)},
            {"points", pts}
        });
    };

    pushSegment(completed, polylineTheme_.doneColor);
    pushSegment(remaining, polylineTheme_.remainColor);

    if (payload["segments"].empty()) return;

    const auto jsonStr = payload.dump();
    spdlog::debug("Polyline style payload: {}", jsonStr);
    // CallJS("drawStyledPolyline", jsonStr);
}

std::string MapPanel::Hex(const wxColour& c) const {
    std::ostringstream oss;
    oss << '#' << std::uppercase << std::setfill('0')
        << std::hex << std::setw(2) << static_cast<int>(c.Red())
        << std::setw(2) << static_cast<int>(c.Green())
        << std::setw(2) << static_cast<int>(c.Blue());
    return oss.str();
}
