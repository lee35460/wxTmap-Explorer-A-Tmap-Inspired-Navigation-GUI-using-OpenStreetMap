#pragma once
#include <wx/wx.h>
#include <vector>
#include "render/RenderPipeline.h"  // LonLat 타입 사용을 위해

class MapPanel : public wxPanel {
public:
    explicit MapPanel(wxWindow* parent);
    void DrawPolyline(const std::vector<LonLat>& coords);
};