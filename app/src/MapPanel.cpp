#include "MapPanel.h"
#include <wx/webview.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

MapPanel::MapPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    // WebView 초기화 코드 (생략)
}

void MapPanel::DrawPolyline(const std::vector<LonLat>& coords) {
    nlohmann::json j = coords;
    wxCallAfter([this, s = j.dump()] {
        // JS 브리지 호출
        // CallJS("drawPolyline", s);
        spdlog::info("[ACK] JS drawPolyline called with {} points", s.size());
    });
}