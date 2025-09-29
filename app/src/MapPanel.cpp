#include "MapPanel.h"
#include <wx/webview.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

// JSON serialization for LonLat (ADL-enabled)
inline void to_json(nlohmann::json& j, const LonLat& p) {
    j = nlohmann::json{{"lon", p.lon}, {"lat", p.lat}};
}

MapPanel::MapPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    // WebView 초기화 코드 (생략)
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
    });
}