# WXT-60 개발 참조 코드

## 🚧 실시간 개발 진행 상황  
**🔄 아키텍처 통합 진행중!** - WXT-59+60 중복 제거 및 Core 모듈 통합  
**현재 단계:** 통합 아키텍처 설계 완료, Core 모듈 구현 완료  
**다음 단계:** ui::CameraSystem 통합 구현 → 기존 코드 마이그레이션  
**완료된 작업:** ✅ core::AnimationEngine, ✅ core::RenderSystem, ✅ 통합 아키텍처 설계  
**진행률:** 65% (통합 아키텍처 구현, 마이그레이션 준비)  
**현재 작업:** WXT-59+60 통합 CameraSystem 구현

### 📊 WXT-60 통합 아키텍처 구현 상황
| 통합 모듈 | 파일 | 상태 | 구현된 기능 |
|----------|------|------|-------------|
| 통합 애니메이션 | core/AnimationEngine.h | ✅ 완료 | LocationState/CameraViewport 애니메이션, EaseInOutCubic, 60fps |
| 통합 렌더링 | core/RenderSystem.h/.cpp | ✅ 완료 | 성능 메트릭, 좌표 변환, UI 디스패처, 우선순위 렌더링 |
| 통합 UI 컴포넌트 | ui/IntegratedComponents.h | ✅ 완료 | LocationRenderer, CameraSystem, RouteRenderer |
| 기존 호환 레이어 | ui/LocationPuck.h (어댑터) | ⏳ 대기 | WXT-59 호환성 유지 어댑터 |
| 빌드 설정 | CMakeLists.txt | ⏳ 대기 | Core 모듈 포함, 테스트 설정 |

### 📋 통합 아키텍처 마이그레이션 계획
| 마이그레이션 단계 | 우선순위 | 상태 | 의존성 |
|------------------|----------|------|--------|
| Core 모듈 구현 | 최고 | ✅ 완료 | AnimationEngine, RenderSystem, 좌표 변환 |
| 통합 UI 컴포넌트 | 높음 | ✅ 완료 | CameraSystem (WXT-59+60), LocationRenderer, RouteRenderer |
| 호환성 어댑터 | 높음 | ⏳ 진행 | 기존 WXT-59 코드 호환성 유지 |
| MapPanel 리팩터링 | 중간 | ⏳ 대기 | 통합 컴포넌트 사용으로 변경 |
| 통합 테스트 | 중간 | ⏳ 대기 | 전체 시스템 검증 |

---

> **📋 Desc-WXT-60.md 요구사항 참조**  
> 본 개발 참조 코드는 `#file:Desc-WXT-60.md`에 명시된 요구사항을 기반으로 작성되었습니다.

## 🎯 Desc-WXT-60.md 요구사항 요약

### 핵심 기능 (Acceptance Criteria)
- **카메라 초기화**: 지도 첫 로딩 시 기본 위치(서울/사용자 위치)로 부드럽게 진입
- **다양한 프리셋**: 도시 전체, 근거리, 원거리 뷰 지원 및 전환
- **사용자 위치 전환**: 위치 감지 시 현재 뷰에서 자연스럽게 이동
- **부드러운 애니메이션**: 줌/팬/회전 동시 보간, easing 함수 적용
- **인터랙션 처리**: 애니메이션 중 사용자 입력 시 자연스러운 중단/재시작
- **다중 해상도**: 모바일, 데스크톱, HiDPI 일관된 동작
- **성능 최적화**: 60fps 애니메이션, GPU 가속, 메모리 효율성

### 테스트 요구사항
- CameraPresetRenderTest: 프리셋 좌표 정확성 검증
- TransitionAnimationTest: 애니메이션 품질 및 지속시간 측정
- UserLocationTransitionTest: 사용자 위치 전환 자연스러움
- 성능 테스트: 60fps 유지, 메모리 효율성

## 🏗️ 프로젝트 아키텍처 패턴 (WXT-59 기반 확장)

### WXT-59 기반 설계 원칙 확장
```cpp
// WXT-59 LocationPuck/CameraController 패턴 기반
// 메모리 관리: wxWidgets UI는 raw pointer, 비즈니스 로직은 unique_ptr

// MapPanel.h 확장 예정
class MapPanel : public wxPanel {
private:
    // WXT-59 기존 컴포넌트들
    std::unique_ptr<ui::LocationPuck> locationPuck_;
    std::unique_ptr<ui::CameraController> cameraController_;
    
    // WXT-60 새 컴포넌트들
    std::unique_ptr<ui::CameraPreset> cameraPreset_;      // 프리셋 관리
    std::unique_ptr<ui::TransitionAnimator> animator_;     // 애니메이션 엔진
    
    // UI 상태
    bool isInitialAnimationComplete_{false};
    AnimationState currentAnimation_;
};
```

### 테스트 작성 패턴 (WXT-59 패턴 확장)
```cpp
// WXT-59 테스트 패턴을 WXT-60에 적용
class WXT60TestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // WXT-59 패턴과 동일한 wxWidgets 초기화
        if (!wxApp::GetInstance()) {
            wxApp::SetInstance(new wxApp());
            wxEntryStart(0, nullptr);
        }
        
        parent_ = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Test");
        
        // WXT-60 컴포넌트 초기화
        cameraPreset_ = std::make_unique<CameraPreset>();
        transitionAnimator_ = std::make_unique<TransitionAnimator>(parent_.get());
    }
    
    void TearDown() override {
        // RAII 자동 정리
        transitionAnimator_.reset();
        cameraPreset_.reset();
        parent_.reset();
    }
    
protected:
    std::unique_ptr<wxFrame> parent_;
    std::unique_ptr<CameraPreset> cameraPreset_;
    std::unique_ptr<TransitionAnimator> transitionAnimator_;
};

// Desc-WXT-60.md TEST 항목과 일치하는 함수명
TEST_F(WXT60TestFixture, CameraPresetRenderTest) {
    // 테스트 로직...
    std::cout << "test_output: CameraPresetRenderTest: 프리셋 위치가 정확한 좌표로 설정되는지: PASS" << std::endl;
}

TEST_F(WXT60TestFixture, TransitionAnimationTest) {
    auto start = std::chrono::steady_clock::now();
    // 애니메이션 테스트...
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "test_output: TransitionAnimationTest: 부드러운 전환 애니메이션 duration="
              << duration.count() << "ms: PASS" << std::endl;
}

// ctest 항목 (Desc 파일 "ctest: CameraPresetTest.InitialPosition")
TEST_F(WXT60TestFixture, CameraPresetTest_InitialPosition) {
    // 테스트 로직...
    std::cout << "test_output: ctest: CameraPresetTest.InitialPosition: PASS" << std::endl;
}
```

### 상수 및 설정 패턴 (WXT-59 확장)
```cpp
// WXT-59 LocationPuck 패턴 확장
namespace ui {
    class CameraPreset {
    private:
        // WXT-59와 일관된 상수 정의 스타일
        static constexpr double ANIMATION_DURATION_MS = 1500.0;  // 초기 진입은 더 길게
        static constexpr double ZOOM_TRANSITION_MS = 800.0;      // 줌 전환은 중간
        static constexpr double PAN_TRANSITION_MS = 1000.0;      // 팬 전환
        
        // 서울 기본 좌표 (WXT-59 LocationState 패턴 활용)
        static constexpr double DEFAULT_SEOUL_LON = 126.9784;
        static constexpr double DEFAULT_SEOUL_LAT = 37.5665;
        static constexpr double DEFAULT_ZOOM_LEVEL = 12.0;
    };
    
    class TransitionAnimator {
    private:
        // WXT-59 LocationPuck 애니메이션 패턴 확장
        static constexpr double EASING_TENSION = 0.3;           // 부드러운 easing
        static constexpr int TARGET_FPS = 60;                   // WXT-59와 동일
        static constexpr double FRAME_TIME_MS = 1000.0 / 60.0;  // 16.67ms per frame
    };
}
```

## 📋 구체적 구현 계획

### 1. Types.h 확장 (WXT-59 LocationState 기반)
```cpp
// WXT-59 기존 구조 기반으로 확장
// LocationState, CameraFollowMode 등은 그대로 유지

// 카메라 프리셋 정의
enum class CameraPresetType {
    Default,        // 기본 (서울 중심)
    UserLocation,   // 사용자 위치
    CityOverview,   // 도시 전체 뷰
    CloseUp,        // 근거리 뷰  
    Satellite       // 위성 뷰
};

struct CameraViewport {
    LonLat center{126.9784, 37.5665};  // 서울 기본
    double zoomLevel{12.0};             // OpenStreetMap 줌 레벨
    double bearing{0.0};                // 회전 (0-360도)
    double tilt{0.0};                   // 기울기 (향후 3D 지원용)
    
    CameraViewport() = default;
    CameraViewport(const LonLat& centerPos, double zoom, double bear = 0.0)
        : center(centerPos), zoomLevel(zoom), bearing(bear) {}
        
    bool operator==(const CameraViewport& other) const {
        return center == other.center && 
               std::abs(zoomLevel - other.zoomLevel) < 0.01 &&
               std::abs(bearing - other.bearing) < 0.1;
    }
};

struct AnimationState {
    CameraViewport startView;          // 시작 카메라 위치
    CameraViewport targetView;         // 목표 카메라 위치
    CameraViewport currentView;        // 현재 카메라 위치 (보간 중)
    
    std::chrono::steady_clock::time_point startTime;
    double durationMs{1000.0};         // 애니메이션 지속 시간
    bool isActive{false};              // 애니메이션 진행 중
    bool canInterrupt{true};           // 사용자 입력으로 중단 가능
    
    // 진행률 계산 (0.0 ~ 1.0)
    double GetProgress() const {
        if (!isActive) return 1.0;
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        return std::min(1.0, elapsed.count() / durationMs);
    }
};

// 전환 설정
struct TransitionConfig {
    double panDurationMs{1000.0};      // 위치 이동 시간
    double zoomDurationMs{800.0};      // 줌 변경 시간
    double bearingDurationMs{600.0};   // 회전 시간
    
    // Easing 함수 타입
    enum class EasingType {
        Linear,
        EaseInOut,
        EaseInOutCubic,    // WXT-59에서 사용한 것과 동일
        EaseOutBounce
    } easingType{EasingType::EaseInOutCubic};
};
```

### 2. CameraPreset.h (WXT-59 LocationPuck 패턴 기반)
```cpp
#pragma once
#include <wx/wx.h>
#include <memory>
#include <unordered_map>
#include "Types.h"

namespace ui {

// 카메라 프리셋 관리 클래스 (WXT-59 LocationPuck 패턴 확장)
class CameraPreset {
public:
    CameraPreset();
    
    // 프리셋 관리 (WXT-59 LocationPuck SetTheme 패턴)
    void LoadPreset(CameraPresetType type);
    void SaveCustomPreset(const std::string& name, const CameraViewport& viewport);
    
    // 뷰포트 계산 (WXT-59 LocationState 패턴 활용)
    CameraViewport GetPresetViewport(CameraPresetType type) const;
    CameraViewport AdaptToScreenSize(const CameraViewport& viewport, const wxSize& screenSize) const;
    
    // 사용자 위치 기반 프리셋 (WXT-59 LocationState 연동)
    void SetUserLocationPreset(const LocationState& userLocation);
    CameraViewport GetUserLocationViewport() const;
    
    // 설정 (WXT-59 LocationPuck SetVisible 패턴)
    void SetDefaultZoomLevel(double zoom);
    void SetAnimationEnabled(bool enabled);
    
    // 상태 조회 (WXT-59 LocationPuck GetLocation 패턴)
    bool HasUserLocation() const { return hasUserLocation_; }
    CameraPresetType GetCurrentPreset() const { return currentPreset_; }
    
private:
    // 프리셋 저장소
    std::unordered_map<CameraPresetType, CameraViewport> presets_;
    std::unordered_map<std::string, CameraViewport> customPresets_;
    
    // 현재 상태 (WXT-59 LocationPuck 패턴)
    CameraPresetType currentPreset_{CameraPresetType::Default};
    LocationState userLocation_;
    bool hasUserLocation_{false};
    bool animationEnabled_{true};
    
    // 설정값
    double defaultZoomLevel_{12.0};
    
    // 내부 메서드 (WXT-59 LocationPuck 패턴)
    void InitializeDefaultPresets();
    CameraViewport CalculateOptimalZoom(const LonLat& center, const wxSize& screenSize) const;
    
    // 상수 (WXT-59 LocationPuck 패턴)
    static constexpr double MIN_ZOOM_LEVEL = 3.0;   // 세계 전체
    static constexpr double MAX_ZOOM_LEVEL = 18.0;  // 건물 레벨
    static constexpr double DEFAULT_SEOUL_LON = 126.9784;
    static constexpr double DEFAULT_SEOUL_LAT = 37.5665;
};

} // namespace ui
```

### 3. TransitionAnimator.h (WXT-59 LocationPuck 애니메이션 확장)
```cpp
#pragma once
#include <wx/wx.h>
#include <functional>
#include <chrono>
#include "Types.h"

namespace ui {

// 카메라 전환 애니메이션 엔진 (WXT-59 LocationPuck 애니메이션 확장)
class TransitionAnimator {
public:
    explicit TransitionAnimator(wxWindow* parent);
    
    // 애니메이션 시작 (WXT-59 LocationPuck StartAnimation 패턴)
    void StartTransition(const CameraViewport& from, const CameraViewport& to, 
                        const TransitionConfig& config = {});
    
    // 애니메이션 업데이트 (WXT-59 LocationPuck UpdateAnimation 패턴)
    void UpdateAnimation();
    bool IsAnimating() const { return animationState_.isActive; }
    
    // 사용자 인터랙션 처리
    void InterruptAnimation(); // 사용자 입력 시 중단
    void ResumeAnimation();    // 재시작
    
    // 콜백 설정 (WXT-59 CameraController 패턴)
    void SetViewportChangeCallback(std::function<void(const CameraViewport&)> callback);
    void SetAnimationCompleteCallback(std::function<void()> callback);
    
    // 현재 상태 (WXT-59 LocationPuck GetLocation 패턴)
    CameraViewport GetCurrentViewport() const { return animationState_.currentView; }
    double GetProgress() const { return animationState_.GetProgress(); }
    
private:
    wxWindow* parent_{nullptr}; // WXT-59 LocationPuck 패턴
    AnimationState animationState_;
    TransitionConfig config_;
    
    // 콜백 (WXT-59 CameraController 패턴)
    std::function<void(const CameraViewport&)> viewportCallback_;
    std::function<void()> completeCallback_;
    
    // 타이머 (wxWidgets 이벤트 기반)
    wxTimer* animationTimer_{nullptr};
    
    // 보간 메서드 (WXT-59 LocationPuck 패턴 확장)
    CameraViewport InterpolateViewport(double progress) const;
    double ApplyEasing(double t, TransitionConfig::EasingType type) const;
    
    // WXT-59 LocationPuck EaseInOutCubic 확장
    double EaseInOutCubic(double t) const;
    double EaseOutBounce(double t) const;
    
    // 이벤트 핸들러
    void OnAnimationTimer(wxTimerEvent& event);
    
    // 상수 (WXT-59 LocationPuck 패턴)
    static constexpr int ANIMATION_TIMER_MS = 16; // 약 60fps
    static constexpr double MIN_PROGRESS_STEP = 0.01; // 최소 진행 단위
};

} // namespace ui
```

### 4. 테스트 구현 예시 (WXT-59 패턴 확장)
```cpp
#include <gtest/gtest.h>
#include "ui/CameraPreset.h"
#include "ui/TransitionAnimator.h"
#include "Types.h"
#include <wx/wx.h>
#include <chrono>
#include <thread>

// WXT-60 테스트 Fixture (WXT-59 WXT59TestFixture 패턴 확장)
class WXT60TestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // WXT-59와 동일한 wxWidgets 초기화
        if (!wxApp::GetInstance()) {
            wxApp::SetInstance(new wxApp());
            wxEntryStart(0, nullptr);
        }
        
        parent_ = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Test");
        cameraPreset_ = std::make_unique<CameraPreset>();
        transitionAnimator_ = std::make_unique<TransitionAnimator>(parent_.get());
    }
    
    void TearDown() override {
        // WXT-59 패턴과 동일한 RAII 정리
        transitionAnimator_.reset();
        cameraPreset_.reset();
        parent_.reset();
    }
    
protected:
    std::unique_ptr<wxFrame> parent_;
    std::unique_ptr<CameraPreset> cameraPreset_;
    std::unique_ptr<TransitionAnimator> transitionAnimator_;
};

// Desc-WXT-60.md TEST 항목과 일치하는 테스트들
TEST_F(WXT60TestFixture, CameraPresetRenderTest) {
    CameraViewport defaultView = cameraPreset_->GetPresetViewport(CameraPresetType::Default);
    
    EXPECT_DOUBLE_EQ(defaultView.center.lon, 126.9784);  // 서울 경도
    EXPECT_DOUBLE_EQ(defaultView.center.lat, 37.5665);   // 서울 위도
    EXPECT_DOUBLE_EQ(defaultView.zoomLevel, 12.0);       // 기본 줌
    
    std::cout << "test_output: CameraPresetRenderTest: 프리셋 위치가 정확한 좌표로 설정되는지: PASS" << std::endl;
}

TEST_F(WXT60TestFixture, TransitionAnimationTest) {
    CameraViewport start({127.0, 37.0}, 10.0);
    CameraViewport end({126.9784, 37.5665}, 12.0);
    
    auto animStart = std::chrono::steady_clock::now();
    
    transitionAnimator_->StartTransition(start, end);
    EXPECT_TRUE(transitionAnimator_->IsAnimating());
    
    // 애니메이션 중간 진행 확인
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    double progress = transitionAnimator_->GetProgress();
    EXPECT_GT(progress, 0.0);
    EXPECT_LT(progress, 1.0);
    
    auto animEnd = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(animEnd - animStart);
    
    std::cout << "test_output: TransitionAnimationTest: 부드러운 전환 애니메이션 duration="
              << duration.count() << "ms: PASS" << std::endl;
}

// WXT-59 ctest 패턴 적용
TEST_F(WXT60TestFixture, CameraPresetTest_InitialPosition) {
    cameraPreset_->LoadPreset(CameraPresetType::Default);
    CameraViewport initialView = cameraPreset_->GetPresetViewport(CameraPresetType::Default);
    
    EXPECT_TRUE(initialView.center.IsValid());
    EXPECT_GT(initialView.zoomLevel, 0.0);
    
    std::cout << "test_output: ctest: CameraPresetTest.InitialPosition: PASS" << std::endl;
}

TEST_F(WXT60TestFixture, UserLocationTransitionTest) {
    LocationState userLoc({127.1, 37.4}, 10.0, 45.0, true, true);
    cameraPreset_->SetUserLocationPreset(userLoc);
    
    EXPECT_TRUE(cameraPreset_->HasUserLocation());
    
    CameraViewport userView = cameraPreset_->GetUserLocationViewport();
    EXPECT_EQ(userView.center.lon, userLoc.coordinates.lon);
    EXPECT_EQ(userView.center.lat, userLoc.coordinates.lat);
    
    std::cout << "test_output: UserLocationTransitionTest: 사용자 위치 감지 시 자연스러운 전환: PASS" << std::endl;
}

TEST_F(WXT60TestFixture, InteractionHandlingTest) {
    CameraViewport start({127.0, 37.0}, 10.0);
    CameraViewport end({126.9784, 37.5665}, 12.0);
    
    transitionAnimator_->StartTransition(start, end);
    EXPECT_TRUE(transitionAnimator_->IsAnimating());
    
    // 사용자 인터랙션으로 중단
    transitionAnimator_->InterruptAnimation();
    EXPECT_FALSE(transitionAnimator_->IsAnimating());
    
    std::cout << "test_output: InteractionHandlingTest: 애니메이션 중 사용자 입력 처리: PASS" << std::endl;
}
```

### 5. MapPanel 통합 (WXT-59 CameraController 패턴 확장)
```cpp
// MapPanel.h에 추가
private:
    // WXT-60 컴포넌트들 (WXT-59와 함께 사용)
    std::unique_ptr<ui::CameraPreset> cameraPreset_;
    std::unique_ptr<ui::TransitionAnimator> transitionAnimator_;
    
    // 초기화 상태
    bool isMapInitialized_{false};
    bool isInitialAnimationComplete_{false};

public:
    // WXT-60 메서드들
    void InitializeCamera();                               // 지도 첫 로딩 시 호출
    void TransitionToPreset(CameraPresetType preset);      // 프리셋으로 전환
    void TransitionToUserLocation();                       // 사용자 위치로 전환
    
private:
    // WXT-59 CameraController와 연동
    void OnCameraViewportChanged(const CameraViewport& viewport);
    void OnInitialAnimationComplete();
```

## 🎯 구현 순서 가이드 (WXT-59 TDD 패턴 확장)

### Phase 1: 데이터 구조 확장 (WXT-59 Types.h 기반)
1. **Types.h에 WXT-60 구조체 추가** (CameraViewport, AnimationState 등)
2. **기본 테스트 Fixture 생성** (WXT60TestFixture)
3. **컴파일 확인**

### Phase 2: CameraPreset 구현 + 테스트 (WXT-59 LocationPuck 패턴)
1. **CameraPresetRenderTest 작성** (TDD)
2. **CameraPreset 기본 구현**
3. **테스트 통과 확인**

### Phase 3: TransitionAnimator 구현 + 테스트 (WXT-59 애니메이션 확장)
1. **TransitionAnimationTest 작성** (TDD)
2. **TransitionAnimator 애니메이션 엔진 구현**
3. **성능 테스트 (60fps 확인)**

### Phase 4: 추가 테스트 케이스 (WXT-59 패턴 확장)
1. **UserLocationTransitionTest, InteractionHandlingTest 등**
2. **WXT-59와의 통합 테스트**

### Phase 5: MapPanel 통합 (WXT-59 CameraController 기반)
1. **MapPanel에 WXT-60 컴포넌트 통합**
2. **WXT-59 LocationPuck/CameraController와 연동**
3. **전체 시스템 테스트**

### 최종 검증
```bash
# WXT-59 패턴과 동일한 테스트 실행
bash scripts/summarize_wxt_test.sh WXT-60

# 결과 확인
cat dev-logs/test-log/Test-WXT-60.md
```

**각 Phase 완료 기준 (WXT-59와 동일):**
- 해당 Phase 테스트 모두 PASS
- `test_output:` 형식 출력 생성
- 60fps 성능 유지
- WXT-59와의 호환성 확인

## 📝 WXT-59와의 연동 고려사항

### 기존 컴포넌트 활용
- **LocationPuck**: WXT-60의 사용자 위치 전환 시 활용
- **CameraController**: Follow 모드와 Transition 애니메이션 조화
- **LocationState**: 사용자 위치 기반 프리셋 생성

### 애니메이션 통합
- WXT-59의 300ms 애니메이션과 WXT-60의 1000ms+ 전환 조화
- 동시 애니메이션 시 우선순위 관리
- 성능 최적화: 동일한 60fps 타겟 유지

### 사용자 경험
- 자연스러운 전환: 초기 로딩 → 사용자 위치 → Follow 모드
- 일관된 애니메이션 스타일
- 적절한 애니메이션 지속시간 (너무 빠르지도 느리지도 않게)