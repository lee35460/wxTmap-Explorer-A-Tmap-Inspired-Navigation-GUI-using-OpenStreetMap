#include <gtest/gtest.h>
#include "ui/MapOverlayHud.h"
#include <wx/dcmemory.h>
#include <wx/bitmap.h>
#include <chrono>

TEST(MapOverlayHudTest, FirstPaintUnder2s) {
    ui::HudState state;
    wxBitmap bmp(800, 600);
    wxMemoryDC memDC(bmp);
    wxBufferedDC dc(&memDC, bmp); // 버그 수정: wxBufferedPaintDC 대신 wxBufferedDC 사용
    auto t0 = std::chrono::steady_clock::now();
    ui::MapOverlayHud::DrawHudStatic(dc, state, 1);
    auto t1 = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(t1 - t0).count();
    EXPECT_LT(elapsed, 2.0); // 2초 이내에 그려져야 함
}