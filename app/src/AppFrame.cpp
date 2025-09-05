#include "AppFrame.h"
#include "MapPanel.h"
#include <wx/wx.h>
#include <wx/sizer.h>

AppFrame::AppFrame()
    : wxFrame(nullptr, wxID_ANY, "wxTmap Explorer – A Tmap-Inspired Navigation GUI",
              wxDefaultPosition, wxSize(1200, 800)) {
    // 상위 프레임에 들어갈 메인 패널
    auto* panel = new MapPanel(this);

    // 레이아웃 매니저
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(panel, 1, wxEXPAND | wxALL, 0);

    SetSizerAndFit(sizer);
    Centre();
    // test
}