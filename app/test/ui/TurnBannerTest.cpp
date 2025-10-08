#include <wx/wx.h>
#include <gtest/gtest.h>
#include "ui/TurnBanner.h"

using namespace ui;

TEST(TurnBannerTest, ProgressSmooth) {
    wxInitializer initializer;
    ASSERT_TRUE(initializer.IsOk());

    TurnBannerState s;
    s.visible = true;
    s.instruction = "좌회전";
    s.distance_m = 120.0;

    std::cout << "[DEBUG] TurnBannerState 생성: visible=" << s.visible << ", instruction=" << s.instruction << ", distance_m=" << s.distance_m << std::endl;

    // TurnBanner는 부모 wxWindow가 필요하다. 더미 프레임 생성
    wxFrame* dummyParent = new wxFrame(nullptr, wxID_ANY, "DummyParent");
    dummyParent->Show(); // 실제로 화면에 표시되도록 함
    // dummyParent 디버그 출력
    TurnBanner b(dummyParent);
    for (int i=0;i<=100;++i) {
        s.progress = i/100.0;
        b.UpdateState(s);
        ASSERT_GE(s.progress, 0.0);
        ASSERT_LE(s.progress, 1.0);
    }
}
