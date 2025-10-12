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
    
    // wxWidgets 컨트롤들은 부모에 의해 자동 관리됨 (double deletion 방지)
    MapPanel* map_{nullptr};
    wxTextCtrl* logBox_{nullptr};
};