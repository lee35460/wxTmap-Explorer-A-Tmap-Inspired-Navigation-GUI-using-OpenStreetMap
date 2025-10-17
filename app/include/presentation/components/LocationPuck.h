#pragma once
#include <wx/wx.h>
#include <chrono>
#include <memory>
#include <functional>
#include "domain/Types.h"  // Clean direct access via include path
#include "presentation/components/BaseComponent.h"

namespace presentation::components {

// 위치 마커 렌더링 및 애니메이션 담당 (wxWidgets 불필요)
class LocationPuck : public AnimatedComponent<NoWxBase> {
public:
    explicit LocationPuck(wxWindow* parent = nullptr);
    
    // === AnimatedComponent 오버라이드 ===
    void Render(wxDC& dc, const CoordTransformFn& coordToPixel) override;
    void Update(double deltaTime) override;
    
    // === LocationPuck 특화 메서드들 ===
    void UpdateLocation(const LocationState& newLocation);
    void SetLocationTheme(const LocationPuckTheme& theme);
    LocationState GetLocation() const;


protected:
    // === AnimatedComponent 오버라이드 ===
    void OnAnimationComplete() override;

private:
    // LocationPuck 특화 상태
    LocationPuckTheme theme_;   // 테마 설정
    LocationState currentLocation_; // 현재 위치 상태
    wxWindow* parent_{nullptr}; // 화면 갱신용 부모 윈도우
    
    // 렌더링 메서드들
    void DrawAccuracyCircle(wxDC& dc, const wxPoint& center, double radiusPixels);
    void DrawPuck(wxDC& dc, const wxPoint& center);
    void DrawBearingArrow(wxDC& dc, const wxPoint& center, double bearing);
};

} // namespace presentation::components