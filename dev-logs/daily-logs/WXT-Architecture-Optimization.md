# WXT 아키텍처 통합 최적화 계획

## 🚨 **현재 문제 분석**

### 중복 구현 현황 (WXT-4 ~ WXT-60)
| 기능 영역 | 중복 발생 이슈 | 중복 내용 | 통합 필요성 |
|----------|---------------|-----------|------------|
| **애니메이션 시스템** | WXT-59, WXT-60 | EaseInOutCubic, 타이밍, 진행률 계산 | 🚨 매우 높음 |
| **렌더링 파이프라인** | WXT-51, 52, 53, 57, 59, 60 | 성능 메트릭, 렌더링 호출, 좌표 변환 | 🚨 매우 높음 |
| **좌표 시스템** | WXT-51, 59, 60 | LonLat, 좌표 변환, 픽셀 매핑 | ⚠️ 높음 |
| **UI 스레드 안전성** | WXT-4, 59, 60 | wxCallAfter, 콜백 시스템 | ⚠️ 높음 |
| **성능 메트릭** | WXT-4, 53, 59, 60 | FPS 측정, CSV 출력, 메트릭 수집 | ⚠️ 중간 |

### 아키텍처 분산 문제
- **7개 이슈에 걸쳐 렌더링 로직 분산**
- **동일한 애니메이션 엔진을 2번 구현**
- **좌표 시스템이 3곳에 중복 정의**
- **성능 메트릭이 4곳에 산재**

## 💡 **통합 최적화 전략**

### **🎯 Phase 1: 핵심 공통 모듈 생성**

#### 1.1 통합 애니메이션 엔진
```cpp
// app/include/core/AnimationEngine.h
namespace core {
class AnimationEngine {
public:
    // WXT-59 + WXT-60 애니메이션 통합
    template<typename T>
    void StartAnimation(const T& from, const T& to, double durationMs);
    
    template<typename T>
    T GetCurrentValue() const;
    
    // 공통 Easing 함수들
    static double EaseInOutCubic(double t);
    static double EaseOutBounce(double t);
    static double Linear(double t);
    
private:
    std::chrono::steady_clock::time_point startTime_;
    double durationMs_{300.0};
    bool isActive_{false};
    
    // 템플릿 특화를 통한 타입별 보간
    template<typename T>
    T Interpolate(const T& from, const T& to, double progress) const;
};

// LocationState 특화
template<>
LocationState AnimationEngine::Interpolate<LocationState>(
    const LocationState& from, const LocationState& to, double progress) const;

// CameraViewport 특화  
template<>
CameraViewport AnimationEngine::Interpolate<CameraViewport>(
    const CameraViewport& from, const CameraViewport& to, double progress) const;
}
```

#### 1.2 통합 렌더링 시스템
```cpp
// app/include/core/RenderSystem.h
namespace core {
class RenderSystem {
public:
    // WXT-51, 52, 53, 57 통합
    void BeginFrame();
    void EndFrame();
    
    // 렌더링 객체 등록 시스템
    void RegisterRenderer(const std::string& name, std::unique_ptr<IRenderer> renderer);
    
    // 성능 메트릭 통합 (WXT-53)
    void ExportMetrics(const std::string& csvPath) const;
    double GetAverageFPS() const;
    
    // 좌표 변환 통합 (WXT-51, 59, 60)
    wxPoint LonLatToPixel(const LonLat& coord) const;
    LonLat PixelToLonLat(const wxPoint& pixel) const;
    
private:
    // WXT-4 UiDispatcher 통합
    std::unique_ptr<ui::UiDispatcher> uiDispatcher_;
    
    // 성능 메트릭
    std::vector<double> frameTimesMs_;
    std::chrono::steady_clock::time_point lastFrameStart_;
    
    // 등록된 렌더러들
    std::unordered_map<std::string, std::unique_ptr<IRenderer>> renderers_;
};

// 렌더링 인터페이스
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void Render(wxDC& dc, const RenderSystem& system) = 0;
    virtual void Update(double deltaTimeMs) = 0;
};
}
```

#### 1.3 통합 좌표 시스템
```cpp
// app/include/core/CoordinateSystem.h
namespace core {
class CoordinateSystem {
public:
    // WXT-51 LonLat + WXT-59 LocationState + WXT-60 CameraViewport 통합
    
    // 기본 좌표 변환
    static wxPoint LonLatToPixel(const LonLat& lonlat, const wxRect& viewport, double zoomLevel);
    static LonLat PixelToLonLat(const wxPoint& pixel, const wxRect& viewport, double zoomLevel);
    
    // 거리 계산 (WXT-59에서 사용)
    static double CalculateDistance(const LonLat& from, const LonLat& to);
    static double CalculateBearing(const LonLat& from, const LonLat& to);
    
    // 뷰포트 계산 (WXT-60에서 사용)
    static wxRect CalculateOptimalViewport(const std::vector<LonLat>& points, const wxSize& screenSize);
    static double CalculateOptimalZoom(const LonLat& center, const wxSize& screenSize);
    
private:
    static constexpr double EARTH_RADIUS_M = 6371000.0;
    static constexpr double DEG_TO_RAD = M_PI / 180.0;
};
}
```

### **🎯 Phase 2: 기존 클래스 리팩터링**

#### 2.1 LocationPuck 최적화
```cpp
// app/include/ui/LocationPuck.h (리팩터링)
namespace ui {
class LocationPuck : public core::IRenderer {
public:
    explicit LocationPuck(wxWindow* parent, core::RenderSystem& renderSystem);
    
    // 애니메이션을 core::AnimationEngine으로 위임
    void UpdateLocation(const LocationState& newLocation);
    
    // IRenderer 구현
    void Render(wxDC& dc, const core::RenderSystem& system) override;
    void Update(double deltaTimeMs) override;
    
private:
    wxWindow* parent_{nullptr};
    core::AnimationEngine animationEngine_;  // 통합 애니메이션 엔진 사용
    LocationState currentLocation_;
    LocationState targetLocation_;
    
    // 렌더링만 담당 (좌표 변환은 RenderSystem으로 위임)
    void DrawAccuracyCircle(wxDC& dc, const wxPoint& center, double radiusPixels);
    void DrawPuck(wxDC& dc, const wxPoint& center);
    void DrawBearingArrow(wxDC& dc, const wxPoint& center, double bearing);
};
}
```

#### 2.2 CameraController + TransitionAnimator 통합
```cpp
// app/include/ui/CameraSystem.h (WXT-59 + WXT-60 통합)
namespace ui {
class CameraSystem : public core::IRenderer {
public:
    explicit CameraSystem(wxWindow* parent, core::RenderSystem& renderSystem);
    
    // WXT-59 CameraController 기능
    void SetFollowMode(CameraFollowMode mode);
    void UpdateLocation(const LocationState& location);
    
    // WXT-60 CameraPreset + TransitionAnimator 기능  
    void LoadPreset(CameraPresetType preset);
    void TransitionToViewport(const CameraViewport& target);
    void SetUserLocationPreset(const LocationState& userLocation);
    
    // 통합 콜백
    void SetViewportChangeCallback(std::function<void(const CameraViewport&)> callback);
    
private:
    core::AnimationEngine animationEngine_;  // 공통 애니메이션 엔진
    
    // WXT-59 상태
    CameraFollowMode followMode_{CameraFollowMode::Off};
    LocationState lastLocation_;
    
    // WXT-60 상태  
    std::unordered_map<CameraPresetType, CameraViewport> presets_;
    CameraViewport currentViewport_;
    CameraViewport targetViewport_;
    
    // 통합 로직
    bool ShouldFollowLocation(const LocationState& newLocation) const;
    CameraViewport CalculateFollowViewport(const LocationState& location) const;
    CameraViewport CalculatePresetViewport(CameraPresetType preset) const;
};
}
```

#### 2.3 MapPanel 통합 최적화
```cpp
// app/include/MapPanel.h (전체 통합)
class MapPanel : public wxPanel {
public:
    MapPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    
private:
    // 통합된 핵심 시스템들
    std::unique_ptr<core::RenderSystem> renderSystem_;      // WXT-51,52,53,57 통합
    std::unique_ptr<ui::LocationPuck> locationPuck_;        // WXT-59 (최적화됨)
    std::unique_ptr<ui::CameraSystem> cameraSystem_;        // WXT-59+60 통합
    std::unique_ptr<ui::RouteRenderer> routeRenderer_;      // WXT-57 (새로 정리)
    
    // WXT-4 UiDispatcher는 RenderSystem에 통합됨
    
    // 이벤트 핸들러
    void OnPaint(wxPaintEvent& event);
    void OnTimer(wxTimerEvent& event);
    
    // 통합 렌더링
    void RenderFrame(wxDC& dc);
};
```

### **🎯 Phase 3: 성능 최적화 및 메트릭 통합**

#### 3.1 통합 성능 메트릭
```cpp
// app/include/core/PerformanceMetrics.h
namespace core {
class PerformanceMetrics {
public:
    // WXT-4, 53, 59, 60 메트릭 통합
    void RecordFrameTime(double frameTimeMs);
    void RecordAnimationUpdate(const std::string& animationType, double updateTimeMs);
    void RecordRenderOperation(const std::string& operation, double renderTimeMs);
    
    // CSV 출력 (WXT-53 확장)
    void ExportToCSV(const std::string& filePath) const;
    
    // 실시간 모니터링
    double GetAverageFPS(std::chrono::milliseconds window = std::chrono::seconds(1)) const;
    double GetMemoryUsageMB() const;
    
    // 임계값 기반 알림
    void SetFPSThreshold(double minFPS);
    void SetMemoryThreshold(double maxMemoryMB);
    
private:
    struct MetricEntry {
        std::chrono::steady_clock::time_point timestamp;
        std::string category;
        std::string operation;
        double valueMs;
    };
    
    std::deque<MetricEntry> metrics_;
    double fpsThreshold_{30.0};
    double memoryThreshold_{100.0};
};
}
```

### **🎯 Phase 4: 마이그레이션 계획**

#### 4.1 단계별 마이그레이션
| 단계 | 작업 내용 | 예상 소요 | 위험도 |
|------|-----------|-----------|--------|
| **1단계** | core 모듈 생성 (AnimationEngine, RenderSystem, CoordinateSystem) | 2-3일 | 낮음 |
| **2단계** | LocationPuck 리팩터링 (core 모듈 사용) | 1일 | 낮음 |
| **3단계** | CameraController + TransitionAnimator → CameraSystem 통합 | 2일 | 중간 |
| **4단계** | MapPanel 통합 및 테스트 | 1-2일 | 중간 |
| **5단계** | 성능 메트릭 통합 및 최적화 | 1일 | 낮음 |

#### 4.2 호환성 유지 전략
```cpp
// 기존 코드 호환성을 위한 어댑터 패턴
namespace ui {
// WXT-59 LocationPuck 인터페이스 유지
class LocationPuckAdapter {
public:
    LocationPuckAdapter(std::shared_ptr<ui::LocationPuck> impl) : impl_(impl) {}
    
    // 기존 메서드들을 새 구현으로 포워딩
    void UpdateLocation(const LocationState& newLocation) {
        impl_->UpdateLocation(newLocation);
    }
    
    void SetVisible(bool visible) {
        impl_->SetVisible(visible);
    }
    
private:
    std::shared_ptr<ui::LocationPuck> impl_;
};

// WXT-60 TransitionAnimator → CameraSystem 어댑터
class TransitionAnimatorAdapter {
public:
    TransitionAnimatorAdapter(std::shared_ptr<ui::CameraSystem> impl) : impl_(impl) {}
    
    void StartTransition(const CameraViewport& from, const CameraViewport& to) {
        impl_->TransitionToViewport(to);
    }
    
private:
    std::shared_ptr<ui::CameraSystem> impl_;
};
}
```

#### 4.3 테스트 전략
```cpp
// 통합 테스트 픽스처
class IntegratedWXTTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // wxWidgets 초기화
        if (!wxApp::GetInstance()) {
            wxApp::SetInstance(new wxApp());
            wxEntryStart(0, nullptr);
        }
        
        parent_ = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Test");
        
        // 통합된 시스템 초기화
        renderSystem_ = std::make_unique<core::RenderSystem>();
        locationPuck_ = std::make_unique<ui::LocationPuck>(parent_.get(), *renderSystem_);
        cameraSystem_ = std::make_unique<ui::CameraSystem>(parent_.get(), *renderSystem_);
    }
    
protected:
    std::unique_ptr<wxFrame> parent_;
    std::unique_ptr<core::RenderSystem> renderSystem_;
    std::unique_ptr<ui::LocationPuck> locationPuck_;
    std::unique_ptr<ui::CameraSystem> cameraSystem_;
};

// 통합 기능 테스트
TEST_F(IntegratedWXTTestFixture, LocationToCamera_Integration_Test) {
    // WXT-59 + WXT-60 통합 시나리오
    LocationState userLocation({127.1, 37.4}, 10.0, 45.0, true, true);
    
    // 1. LocationPuck 위치 업데이트 (WXT-59)
    locationPuck_->UpdateLocation(userLocation);
    
    // 2. CameraSystem Follow 모드 (WXT-59 + WXT-60)  
    cameraSystem_->SetFollowMode(CameraFollowMode::Position);
    cameraSystem_->UpdateLocation(userLocation);
    
    // 3. 통합 애니메이션 확인
    EXPECT_TRUE(cameraSystem_->IsAnimating());
    
    // 4. 성능 메트릭 검증
    auto metrics = renderSystem_->GetPerformanceMetrics();
    EXPECT_GE(metrics.GetAverageFPS(), 30.0);
    
    std::cout << "test_output: LocationToCamera_Integration_Test: WXT-59+60 통합 애니메이션: PASS" << std::endl;
}
```

## 📊 **통합 효과 예측**

### 코드 중복 제거
- **애니메이션**: 2개 클래스 → 1개 통합 엔진 (**50% 감소**)
- **렌더링**: 7개 분산 → 1개 통합 시스템 (**70% 감소**)  
- **좌표 시스템**: 3개 중복 → 1개 통합 (**66% 감소**)

### 성능 향상
- **메모리 사용량**: 중복 객체 제거로 **20-30% 감소** 예상
- **렌더링 성능**: 통합 파이프라인으로 **10-15% 향상** 예상
- **애니메이션 일관성**: 단일 엔진으로 **버그 감소**

### 유지보수성 향상  
- **단일 책임**: 각 모듈이 명확한 역할 담당
- **테스트 용이성**: 통합 테스트 픽스처로 시나리오 검증
- **확장성**: 새로운 애니메이션/렌더링 기능 추가 용이

## 🚀 **다음 단계 제안**

1. **즉시 시작**: core 모듈 스켈레톤 생성
2. **점진적 마이그레이션**: 기존 기능 유지하면서 단계별 통합  
3. **성능 검증**: 각 단계마다 메트릭 비교
4. **문서화**: 통합된 아키텍처 가이드 작성

**지금 시작하지 않으면 WXT-61, WXT-62...에서 더 큰 기술 부채가 쌓일 것입니다!** 💪