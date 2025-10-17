#pragma once
#include <wx/wx.h>
#include <chrono>
#include <memory>
#include <functional>
#include "domain/Types.h"  // Clean direct access via include path

namespace presentation::components {

// 위치 마커 렌더링 및 애니메이션 담당
class LocationPuck {
public:
    explicit LocationPuck(wxWindow* parent);
    
    // 핵심 메서드들 - 실제 구현 완료
    void UpdateLocation(const LocationState& newLocation);
    void StartAnimation(const LocationState& target);
    void UpdateAnimation(); // 애니메이션 상태 업데이트 (타이머에서 호출)
    void Render(wxDC& dc, const std::function<wxPoint(const LonLat&)>& coordToPixel);
    void SetTheme(const LocationPuckTheme& theme);
    void SetVisible(bool visible);

    // 상태 조회 메서드
    bool IsVisible() const { return isVisible_; }
    LocationState GetLocation() const;


private:
    wxWindow* parent_{nullptr}; // 부모 윈도우 (wxWidgets가 자동 관리)
    bool isVisible_{true};      // 표시 여부

    // 위치 상태
    LocationPuckTheme theme_;   // 테마 설정
    LocationState currentLocation_; // 현재 위치 상태
    LocationState targetLocation_;  // 애니메이션 목표 위치

    // 애니메이션
    std::chrono::steady_clock::time_point animationStart_; // 애니메이션 시작 시각
    bool isAnimating_{false}; // 애니메이션 진행 중 여부
    static constexpr int ANIMATION_DURATION_MS = 300; // 애니메이션 지속 시간

    // 내부 메서드들
    LocationState InterpolateLocation(double progress) const;
    double EaseInOutCubic(double t) const;
    void DrawAccuracyCircle(wxDC& dc, const wxPoint& center, double radiusPixels);
    void DrawPuck(wxDC& dc, const wxPoint& center);
    void DrawBearingArrow(wxDC& dc, const wxPoint& center, double bearing);
};

} // namespace presentation::components