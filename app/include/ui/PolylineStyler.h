#pragma once
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "render/RenderPipeline.h"

namespace ui {

// 진행 퍼센트(0~1) → 완료 인덱스(split) 계산
inline std::size_t ProgressToSplit(std::size_t N, double progress01) {
    if (N == 0) return 0;
    double p = std::clamp(progress01, 0.0, 1.0);
    // N개의 점이면 N-1개 세그먼트. split은 "완료 마지막 점" 인덱스.
    return static_cast<std::size_t>(std::floor(p * (N - 1)));
}

// 줌 레벨/HiDPI → 선두께 픽셀
inline int WidthForZoom(int zoom, double dpiScale, int basePx = 4) {
    // 15레벨을 기준, 3레벨 당 2배 스케일 (적당한 감쇠)
    double zoomScale = std::pow(2.0, (zoom - 15) / 3.0);
    int w = static_cast<int>(std::round(basePx * zoomScale * std::max(1.0, dpiScale)));
    return std::max(1, w);
}

inline std::pair<std::vector<LonLat>, std::vector<LonLat>> SplitPolylineByProgress(
    const std::vector<LonLat>& pts, double progress01) {
    std::pair<std::vector<LonLat>, std::vector<LonLat>> result;
    if (pts.empty()) return result;

    const std::size_t split = std::min<std::size_t>(
        ProgressToSplit(pts.size(), progress01),
        pts.size() > 0 ? pts.size() - 1 : 0);

    if (split > 0 || progress01 >= 1.0) {
        result.first.insert(result.first.end(), pts.begin(), pts.begin() + split + 1);
    }

    if (split < pts.size() - 1) {
        auto startIt = pts.begin();
        startIt += result.first.empty() ? 0 : split;
        result.second.insert(result.second.end(), startIt, pts.end());

        if (!result.first.empty() && !result.second.empty()) {
            const LonLat& bridge = result.first.back();
            const LonLat& head = result.second.front();
            if (bridge.lon != head.lon || bridge.lat != head.lat) {
                result.second.insert(result.second.begin(), bridge);
            }
        }
    }

    return result;
}

} // namespace ui