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
    
    // wxWidgets 컨트롤들은 unique_ptr로 관리하여 메모리 안전성 향상
    std::unique_ptr<MapPanel> map_;
    std::unique_ptr<wxTextCtrl> logBox_;
    
    // UI 컨트롤들의 원시 포인터 (sizer 관리용)
    MapPanel* mapPtr_{nullptr};
    wxTextCtrl* logBoxPtr_{nullptr};
};