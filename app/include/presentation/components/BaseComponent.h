#pragma once
#include <wx/wx.h>
#include <functional>
#include <string>
#include <memory>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <map>
#include "domain/Types.h"

namespace presentation::components {

// ============================================================================
// 기본 컴포넌트 타입 정의
// ============================================================================

/**
 * @brief 빈 기본 클래스 (wxWidgets 비의존적 컴포넌트용)
 */
class NoWxBase {
public:
    NoWxBase() = default;
    virtual ~NoWxBase() = default;
};

// ============================================================================
// 템플릿 기반 BaseComponent
// ============================================================================

/**
 * @brief 모든 컴포넌트의 기본 클래스 (템플릿으로 wxWidgets 선택적 지원)
 * 
 * @tparam WxBase wxWidgets 기본 클래스 (wxPanel, wxWindow, NoWxBase 등)
 * 
 * 사용 예:
 * - class LocationPuck : public BaseComponent<NoWxBase>      // wx 불필요
 * - class MapOverlayHud : public BaseComponent<wxPanel>     // wxPanel 필요
 * - class CustomControl : public BaseComponent<wxWindow>    // wxWindow 필요
 */
template<typename WxBase = NoWxBase>
class BaseComponent : public WxBase {
public:
    // === 생성자 - wxWidgets 의존성에 따라 다르게 처리 ===
    template<typename... Args>
    explicit BaseComponent(Args&&... args) : WxBase(std::forward<Args>(args)...) {
        Initialize();
    }
    
    virtual ~BaseComponent() = default;

    // === 기본 상태 관리 ===
    virtual void SetVisible(bool visible) {
        if (isVisible_ != visible) {
            isVisible_ = visible;
            if constexpr (std::is_base_of_v<wxWindow, WxBase>) {
                WxBase::Show(visible);
            }
            NotifyStateChange("visible", visible);
        }
    }
    
    virtual bool IsVisible() const { 
        if constexpr (std::is_base_of_v<wxWindow, WxBase>) {
            return WxBase::IsShown() && isVisible_;
        }
        return isVisible_; 
    }
    
    virtual void SetEnabled(bool enabled) {
        if (isEnabled_ != enabled) {
            isEnabled_ = enabled;
            if constexpr (std::is_base_of_v<wxWindow, WxBase>) {
                WxBase::Enable(enabled);
            }
            NotifyStateChange("enabled", enabled);
        }
    }
    
    virtual bool IsEnabled() const { 
        if constexpr (std::is_base_of_v<wxWindow, WxBase>) {
            return WxBase::IsEnabled() && isEnabled_;
        }
        return isEnabled_; 
    }

    // === 렌더링 인터페이스 ===
    virtual void Render(wxDC& dc) = 0;  // 순수 가상 함수
    virtual void Update(double deltaTime) {}  // 애니메이션/상태 업데이트
    
    // === 테마 지원 ===
    virtual void SetTheme(const std::string& themeName) {
        if (currentTheme_ != themeName) {
            currentTheme_ = themeName;
            ApplyTheme();
            NotifyStateChange("theme", !themeName.empty());
        }
    }
    virtual std::string GetTheme() const { return currentTheme_; }

    // === 이벤트/콜백 시스템 ===
    using StateChangeCallback = std::function<void(const std::string&, bool)>;
    void SetStateChangeCallback(StateChangeCallback callback) {
        stateChangeCallback_ = callback;
    }

    // === 애니메이션 기본 지원 ===
    virtual void StartAnimation() {
        if (!isAnimating_) {
            isAnimating_ = true;
            animationStart_ = std::chrono::steady_clock::now();
            NotifyStateChange("animating", true);
        }
    }
    
    virtual void StopAnimation() {
        if (isAnimating_) {
            isAnimating_ = false;
            NotifyStateChange("animating", false);
        }
    }
    
    virtual bool IsAnimating() const { return isAnimating_; }

    // === 디버깅/메트릭 지원 ===
    virtual std::string GetDebugInfo() const {
        std::ostringstream oss;
        oss << "BaseComponent<" << typeid(WxBase).name() << "> {"
            << " visible=" << isVisible_
            << ", enabled=" << isEnabled_
            << ", theme=" << currentTheme_
            << ", animating=" << isAnimating_
            << ", renderCount=" << renderCount_
            << " }";
        return oss.str();
    }
    
    virtual void ResetMetrics() {
        renderCount_ = 0;
        lastRenderTime_ = std::chrono::steady_clock::now();
    }

protected:
    // === 보호된 멤버들 - 상속 클래스에서 접근 가능 ===
    bool isVisible_{true};
    bool isEnabled_{true};
    std::string currentTheme_{"default"};
    
    // 애니메이션 상태
    bool isAnimating_{false};
    std::chrono::steady_clock::time_point animationStart_;
    
    // 콜백
    StateChangeCallback stateChangeCallback_;
    
    // 메트릭/디버깅
    mutable std::chrono::steady_clock::time_point lastRenderTime_;
    mutable size_t renderCount_{0};
    
    // === 보호된 헬퍼 메서드들 ===
    void Initialize() {
        animationStart_ = std::chrono::steady_clock::now();
        lastRenderTime_ = std::chrono::steady_clock::now();
        
        // wxWidgets 기반 컴포넌트인 경우 기본 설정
        if constexpr (std::is_base_of_v<wxPanel, WxBase>) {
            WxBase::SetBackgroundStyle(wxBG_STYLE_PAINT);  // 더블 버퍼링
        }
    }
    
    void NotifyStateChange(const std::string& property, bool newValue) {
        if (stateChangeCallback_) {
            stateChangeCallback_(property, newValue);
        }
    }
    
    double GetAnimationProgress(double durationMs) const {
        if (!isAnimating_) return 1.0;
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - animationStart_).count();
        
        double progress = static_cast<double>(elapsed) / durationMs;
        return std::clamp(progress, 0.0, 1.0);
    }
    
    double EaseInOutCubic(double t) const {
        if (t < 0.5) {
            return 4 * t * t * t;
        } else {
            double f = 2 * t - 2;
            return 1 + f * f * f / 2;
        }
    }
    
    // 테마 관련 헬퍼
    virtual void ApplyTheme() {}  // 상속 클래스에서 오버라이드
    
    // 렌더링 헬퍼
    int ScaleForDPI(int logicalPixels) const {
        return logicalPixels;  // 기본값
    }
    
    wxColour GetThemeColor(const std::string& colorName) const {
        // 테마별 색상 매핑
        static std::map<std::string, std::map<std::string, wxColour>> themeColors = {
            {"default", {
                {"primary", wxColour(0, 122, 255)},
                {"secondary", wxColour(128, 128, 128)},
                {"background", wxColour(255, 255, 255)},
                {"text", wxColour(0, 0, 0)}
            }},
            {"dark", {
                {"primary", wxColour(10, 132, 255)},
                {"secondary", wxColour(152, 152, 157)},
                {"background", wxColour(28, 28, 30)},
                {"text", wxColour(255, 255, 255)}
            }}
        };
        
        auto themeIt = themeColors.find(currentTheme_);
        if (themeIt != themeColors.end()) {
            auto colorIt = themeIt->second.find(colorName);
            if (colorIt != themeIt->second.end()) {
                return colorIt->second;
            }
        }
        return wxColour(0, 0, 0);
    }
};

// ============================================================================
// 특화된 컴포넌트 클래스들
// ============================================================================

/**
 * @brief 렌더링 가능한 컴포넌트의 기본 클래스
 * 
 * BaseComponent + 좌표 변환, 지도 렌더링 특화 기능
 */
template<typename WxBase = NoWxBase>
class RenderableComponent : public BaseComponent<WxBase> {
public:
    template<typename... Args>
    explicit RenderableComponent(Args&&... args) 
        : BaseComponent<WxBase>(std::forward<Args>(args)...) {}
    
    virtual ~RenderableComponent() = default;

    // === 좌표 변환 지원 ===
    using CoordTransformFn = std::function<wxPoint(const LonLat&)>;
    void SetCoordTransform(CoordTransformFn transform) {
        coordTransform_ = transform;
    }
    
    // === 렌더링 오버로드 - 좌표 변환 함수 포함 ===
    virtual void Render(wxDC& dc) override {
        if (!this->IsVisible()) return;
        
        // 좌표 변환 함수가 설정되어 있으면 호출
        if (coordTransform_) {
            Render(dc, coordTransform_);
        }
        
        // 렌더링 메트릭 업데이트
        this->renderCount_++;
        this->lastRenderTime_ = std::chrono::steady_clock::now();
    }
    
    virtual void Render(wxDC& dc, const CoordTransformFn& coordToPixel) = 0;

    // === 위치 기반 렌더링 ===
    virtual void SetPosition(const LonLat& position) { position_ = position; }
    virtual LonLat GetPosition() const { return position_; }

protected:
    LonLat position_{0.0, 0.0};  // 기본 위치
    CoordTransformFn coordTransform_;
    
    // 렌더링 헬퍼
    wxPoint TransformCoordinate(const LonLat& coord) const {
        if (coordTransform_) {
            return coordTransform_(coord);
        }
        return wxPoint(0, 0);
    }
    
    bool IsInViewport(const wxPoint& point, const wxSize& viewportSize) const {
        return point.x >= 0 && point.y >= 0 && 
               point.x < viewportSize.GetWidth() && point.y < viewportSize.GetHeight();
    }
};

/**
 * @brief 애니메이션 가능한 컴포넌트의 기본 클래스
 * 
 * RenderableComponent + 위치 애니메이션, 상태 전환 애니메이션
 */
template<typename WxBase = NoWxBase>
class AnimatedComponent : public RenderableComponent<WxBase> {
public:
    template<typename... Args>
    explicit AnimatedComponent(Args&&... args) 
        : RenderableComponent<WxBase>(std::forward<Args>(args)...) {}
    
    virtual ~AnimatedComponent() = default;

    // === 애니메이션 제어 ===
    virtual void AnimateTo(const LonLat& targetPosition, double durationMs = 300.0) {
        startPosition_ = this->position_;
        targetPosition_ = targetPosition;
        animationDurationMs_ = durationMs;
        this->StartAnimation();
    }
    
    virtual void Update(double deltaTime) override {
        if (this->IsAnimating()) {
            double progress = this->GetAnimationProgress(animationDurationMs_);
            double easedProgress = this->EaseInOutCubic(progress);
            
            this->position_ = InterpolatePosition(easedProgress);
            
            if (progress >= 1.0) {
                this->position_ = targetPosition_;
                this->StopAnimation();
                OnAnimationComplete();
            }
        }
    }

    // === 애니메이션 설정 ===
    void SetAnimationDuration(double ms) { animationDurationMs_ = ms; }
    double GetAnimationDuration() const { return animationDurationMs_; }

protected:
    // 애니메이션 상태
    LonLat startPosition_{0.0, 0.0};
    LonLat targetPosition_{0.0, 0.0};
    double animationDurationMs_{300.0};
    
    // 애니메이션 보간
    virtual LonLat InterpolatePosition(double progress) const {
        double lon = startPosition_.lon + (targetPosition_.lon - startPosition_.lon) * progress;
        double lat = startPosition_.lat + (targetPosition_.lat - startPosition_.lat) * progress;
        return LonLat{lon, lat};
    }
    
    virtual void OnAnimationComplete() {}  // 애니메이션 완료 콜백
};

// ============================================================================
// 타입 별칭 및 사용 예제
// ============================================================================

/*
사용 예제:

1. wxWidgets 불필요한 순수 컴포넌트:
   class LocationPuck : public AnimatedComponent<NoWxBase> { ... }
   class CameraController : public BaseComponent<NoWxBase> { ... }

2. wxPanel 기반 UI 컴포넌트:
   class MapOverlayHud : public BaseComponent<wxPanel> { 
       MapOverlayHud(wxWindow* parent) : BaseComponent(parent, wxID_ANY) {}
       ...
   }

3. wxWindow 기반 커스텀 컨트롤:
   class CustomMap : public RenderableComponent<wxWindow> {
       CustomMap(wxWindow* parent) : RenderableComponent(parent, wxID_ANY) {}
       ...
   }

4. 이벤트 테이블이 필요한 경우:
   class InteractivePanel : public BaseComponent<wxPanel> {
       wxDECLAR_EVENT_TABLE();
   public:
       void OnPaint(wxPaintEvent& event);
       void OnMouse(wxMouseEvent& event);
   };
   
   wxBEGIN_EVENT_TABLE(InteractivePanel, wxPanel)
       EVT_PAINT(InteractivePanel::OnPaint)
       EVT_LEFT_DOWN(InteractivePanel::OnMouse)
   wxEND_EVENT_TABLE()
*/

// 편의를 위한 타입 별칭
using PureComponent = BaseComponent<NoWxBase>;
using PanelComponent = BaseComponent<wxPanel>;
using WindowComponent = BaseComponent<wxWindow>;
using RenderablePanelComponent = RenderableComponent<wxPanel>;
using AnimatedPanelComponent = AnimatedComponent<wxPanel>;

} // namespace presentation::components