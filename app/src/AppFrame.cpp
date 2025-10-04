#include "AppFrame.h"
#include "MapPanel.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <spdlog/spdlog.h>

AppFrame::AppFrame() : wxFrame(nullptr, wxID_ANY, "wxTmap Explorer") {
    render_ = std::make_unique<RenderPipeline>();
    map_ = new MapPanel(this);

    // 로그 출력용 텍스트 박스 추가
    logBox_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(600, 100),
                             wxTE_MULTILINE | wxTE_READONLY);

    auto* root = new wxBoxSizer(wxVERTICAL);

    // Top controls (demo buttons)
    auto* controls = new wxBoxSizer(wxHORIZONTAL);
    auto* btnSimRoute = new wxButton(this, wxID_ANY, "Simulate Route");
    controls->Add(btnSimRoute, 0, wxALL, 5);
    root->Add(controls, 0, wxEXPAND);

    // Map area and log box
    root->Add(map_, 1, wxEXPAND);
    root->Add(logBox_, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(root);

    // Wire: simulate a polyline so we can SEE activity in the GUI
    btnSimRoute->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        std::vector<LonLat> path;
        path.reserve(200);
        // Create a simple wavy path to visualize
        for (int i = 0; i < 200; ++i) {
            double lon = 127.000 + i * 0.001;
            double lat = 37.500 + std::sin(i / 10.0) * 0.001;
            path.push_back({lon, lat});
        }
        OnRouteReady(path);
    });
}

void AppFrame::OnRouteReady(const std::vector<LonLat>& path) {
    render_->beginFrame();
    map_->DrawPolyline(path);
    render_->endFrame();

    wxString msg = wxString::Format("DrawPolyline finished, FPS(avg)=%.2f\n", render_->fpsAverage());
    logBox_->AppendText(msg);
}