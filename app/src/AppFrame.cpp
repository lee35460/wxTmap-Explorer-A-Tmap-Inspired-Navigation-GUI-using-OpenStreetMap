#include "AppFrame.h"
#include "MapPanel.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <spdlog/spdlog.h>

static const int ID_TOGGLE_ANIM = wxID_HIGHEST + 100; // must match MapPanel


// unique_ptr를 사용한 메모리 관리 자동화로 안전성과 현대적 C++ 스타일 적용
AppFrame::AppFrame() : wxFrame(nullptr, wxID_ANY, "wxTmap Explorer") {
    render_ = std::make_unique<RenderPipeline>();
    
    // unique_ptr로 생성하여 메모리 안전성 보장
    map_ = std::make_unique<MapPanel>(this);
    mapPtr_ = map_.get(); // sizer와 이벤트 처리를 위한 원시 포인터
    
    // 로그 출력용 텍스트 박스도 unique_ptr로 관리
    logBox_ = std::make_unique<wxTextCtrl>(this, wxID_ANY, "", wxDefaultPosition, wxSize(600, 100),
                                          wxTE_MULTILINE | wxTE_READONLY);
    logBoxPtr_ = logBox_.get();

    auto* root = new wxBoxSizer(wxVERTICAL);

    // Top controls (demo buttons)
    auto* controls = new wxBoxSizer(wxHORIZONTAL);
    auto* btnSimRoute = new wxButton(this, wxID_ANY, "Simulate Route");
    controls->Add(btnSimRoute, 0, wxALL, 5);
    auto* btnToggleAnim = new wxButton(this, wxID_ANY, "Toggle Banner Anim");
    controls->Add(btnToggleAnim, 0, wxALL, 5);
    auto* btnResetView = new wxButton(this, wxID_ANY, "Reset View");
    controls->Add(btnResetView, 0, wxALL, 5);
    root->Add(controls, 0, wxEXPAND);

    // Map area and log box (원시 포인터 사용 - sizer는 원시 포인터 필요)
    root->Add(mapPtr_, 1, wxEXPAND);
    root->Add(logBoxPtr_, 0, wxEXPAND | wxALL, 5);
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

    btnResetView->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        // 맵 뷰 리셋 기능 - 현재는 간단한 새로고침으로 대체
        mapPtr_->Refresh();
    });
}

void AppFrame::OnRouteReady(const std::vector<LonLat>& path) {
    render_->resetDrawStats();
    render_->beginFrame();
    render_->submitPolyline(path);
    map_->DrawPolyline(path);
    render_->endFrame();

    wxString msg = wxString::Format(
        "DrawPolyline finished, FPS(avg)=%.2f, polyline_draw_ms=%.2f, draw_calls=%zu\n",
        render_->fpsAverage(),
        render_->lastPolylineDrawMs(),
        render_->drawCalls());
    logBox_->AppendText(msg);
}