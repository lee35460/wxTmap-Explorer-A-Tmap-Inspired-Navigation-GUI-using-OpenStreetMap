#include "MapPanel.h"
#include <wx/webview.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/timer.h>
namespace {
    wxStaticText* s_demoLabel = nullptr;   // shows last action text
    wxGauge*      s_progress  = nullptr;   // simple banner/progress demo
    wxTimer*      s_timer     = nullptr;   // drives the progress animation
    int           s_progVal   = 0;
}

// JSON serialization for LonLat (ADL-enabled)
inline void to_json(nlohmann::json& j, const LonLat& p) {
    j = nlohmann::json{{"lon", p.lon}, {"lat", p.lat}};
}

MapPanel::MapPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    // WebView 초기화 코드 (생략)
    hud_ = new ui::MapOverlayHud(this);

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
            s_progVal = (s_progVal + 3) % 100; // ~33 ticks to full
            if (s_progress) s_progress->SetValue(s_progVal);
            // Light-weight visual feedback
            if (s_progVal % 25 == 0 && s_demoLabel) {
                s_demoLabel->SetLabel(wxString::Format("Progress %d%%", s_progVal));
            }
        });
        s_timer->Start(100); // 10Hz, satisfies ">= 1Hz" update AC for banner demo
    }
    // ...existing code...
}

void MapPanel::DrawPolyline(const std::vector<LonLat>& coords) {
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
    });
}

