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

static const int ID_TOGGLE_ANIM = wxID_HIGHEST + 100; // custom event id

namespace {
    wxStaticText* s_demoLabel = nullptr;   // shows last action text
    wxGauge*      s_progress  = nullptr;   // simple banner/progress demo
    wxTimer*      s_timer     = nullptr;   // drives the progress animation
    int           s_progVal   = 0;
    std::chrono::steady_clock::time_point s_lastTick; // for jank metric
    int           s_expectedMs = 100;                  // 10Hz default
}

// JSON serialization for LonLat (ADL-enabled)
inline void to_json(nlohmann::json& j, const LonLat& p) {
    j = nlohmann::json{{"lon", p.lon}, {"lat", p.lat}};
}

MapPanel::MapPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    // WebView 초기화 코드 (생략)
    hud_ = new ui::MapOverlayHud(this);
    dpiScale_ = GetContentScaleFactor();

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
            if (s_timer && s_timer->IsRunning()) s_timer->Stop();
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
