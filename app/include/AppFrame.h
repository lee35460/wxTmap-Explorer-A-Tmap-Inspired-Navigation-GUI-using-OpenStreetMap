#pragma once
#include <wx/wx.h>
#include <memory>
#include "render/RenderPipeline.h"

class MapPanel;

class AppFrame : public wxFrame {
public:
    AppFrame();
    void OnRouteReady(const std::vector<LonLat>& path);

private:
    std::unique_ptr<RenderPipeline> render_;
    MapPanel* map_{nullptr};
};