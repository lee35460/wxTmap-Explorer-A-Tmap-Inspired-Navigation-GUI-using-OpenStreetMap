#include "AppFrame.h"
#include "MapPanel.h"
#include <wx/sizer.h>
#include <spdlog/spdlog.h>

AppFrame::AppFrame() : wxFrame(nullptr, wxID_ANY, "wxTmap Explorer") {
    render_ = std::make_unique<RenderPipeline>();
    map_ = new MapPanel(this);
}

void AppFrame::OnRouteReady(const std::vector<LonLat>& path) {
    render_->beginFrame();
    map_->DrawPolyline(path);
    render_->endFrame();
    spdlog::info("[ACK] DrawPolyline finished, FPS(avg)={}", render_->fpsAverage());
}