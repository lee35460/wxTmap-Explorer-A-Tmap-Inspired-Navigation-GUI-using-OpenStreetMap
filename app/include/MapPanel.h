#pragma once
#include <wx/wx.h>
#include <vector>
#include "render/RenderPipeline.h"

class MapPanel : public wxPanel {
public:
    MapPanel(wxWindow* parent);
    void DrawPolyline(const std::vector<LonLat>& coords);
};