#include <gtest/gtest.h>
#include <vector>
#include "ui/PolylineStyler.h"

TEST(PolylineStyleTest, HighlightSegments) {
    using ui::SplitPolylineByProgress;
    using ui::ProgressToSplit;

    std::vector<LonLat> pts;
    for (int i = 0; i < 6; ++i) {
        pts.push_back({static_cast<double>(i), static_cast<double>(i)});
    }

    auto pairMid = SplitPolylineByProgress(pts, 0.5);
    EXPECT_EQ(ProgressToSplit(pts.size(), 0.5), 2u);
    EXPECT_EQ(pairMid.first.size(), 3u);
    EXPECT_EQ(pairMid.second.size(), 4u); // includes bridge point
    ASSERT_FALSE(pairMid.first.empty());
    ASSERT_FALSE(pairMid.second.empty());
    EXPECT_DOUBLE_EQ(pairMid.first.front().lon, 0.0);
    EXPECT_DOUBLE_EQ(pairMid.first.back().lon, 2.0);
    EXPECT_DOUBLE_EQ(pairMid.second.front().lon, pairMid.first.back().lon);

    // clamp behaviour
    auto pairZero = SplitPolylineByProgress(pts, 0.0);
    EXPECT_TRUE(pairZero.first.empty());
    EXPECT_EQ(pairZero.second.size(), pts.size());

    auto pairFull = SplitPolylineByProgress(pts, 1.0);
    EXPECT_EQ(pairFull.first.size(), pts.size());
    EXPECT_TRUE(pairFull.second.empty());

    // still expose split API for backwards compatibility
    EXPECT_EQ(ProgressToSplit(11, -0.1), 0u);
    EXPECT_EQ(ProgressToSplit(11, 0.49), 4u);
}

TEST(PolylineStyleTest, WidthScaleWithZoomAndHiDpi) {
    using ui::WidthForZoom;
    int w15 = WidthForZoom(15, 1.0, 4);
    int w18 = WidthForZoom(18, 1.0, 4);
    int w12 = WidthForZoom(12, 1.0, 4);
    EXPECT_GE(w18, w15);
    EXPECT_LE(w12, w15);
    // HiDPI면 더 두꺼워야 함
    EXPECT_GT(WidthForZoom(15, 2.0, 4), w15);
}