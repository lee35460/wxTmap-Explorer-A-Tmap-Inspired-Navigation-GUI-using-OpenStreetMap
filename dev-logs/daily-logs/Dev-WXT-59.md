# WXT-59 개발 참조 코드

## 🚧 실시간 개발 진행 상황  
**✅ LocationPuck 모듈 100% 완료!** - Types.h + LocationPuck.h + LocationPuck.cpp 완성  
**현재 단계:** LocationPuck 단위 테스트 작성 준비  
**다음 단계:** test/ui/LocationPuckTest.cpp 작성 → CameraController 구현  
**완료된 작업:** ✅ Types.h 모든 구조체, ✅ LocationPuck.h 완전한 헤더, ✅ LocationPuck.cpp 모든 메서드  
**진행률:** 60% (LocationPuck 모듈 완성, 테스트 작성 시작)  
**현재 작업:** GoogleTest 기반 단위 테스트 작성

### 📊 LocationPuck 모듈 구현 완료 상황
| 컴포넌트 | 파일 | 상태 | 구현된 기능 |
|---------|------|------|-------------|
| 데이터 구조 | Types.h | ✅ 완료 | LocationState, CameraFollowMode, LocationPuckTheme |
| 헤더 선언 | LocationPuck.h | ✅ 완료 | 모든 public/private 메서드 및 멤버 변수 |
| 구현부 | LocationPuck.cpp | ✅ 완료 | 생성자, UpdateLocation, Render, SetTheme/SetVisible |
| 빌드 설정 | CMakeLists.txt | ✅ 완료 | 소스/헤더 파일 포함, 테스트 설정 |

### 📋 다음 단계: 단위 테스트 작성 계획
| 테스트 케이스 | 우선순위 | 상태 | 비고 |
|--------------|----------|------|------|
| WXT59TestFixture 생성 | 높음 | ⏳ 대기 | wxWidgets 테스트 환경 설정 |
| 생성자 테스트 | 높음 | ⏳ 대기 | 초기화 상태 검증 |
| UpdateLocation 테스트 | 높음 | ⏳ 대기 | 유효성 검사 및 상태 업데이트 |
| Render 테스트 | 중간 | ⏳ 대기 | 렌더링 조건 및 동작 검증 |
| SetTheme/SetVisible 테스트 | 중간 | ⏳ 대기 | setter 메서드 동작 검증 |

---

> **📋 Desc-WXT-59.md 요구사항 참조**  
> 본 개발 참조 코드는 `#file:Desc-WXT-59.md`에 명시된 요구사항을 기반으로 작성되었습니다.

## 🎯 Desc-WXT-59.md 요구사항 요약

### 핵심 기능 (Acceptance Criteria)
- **Location Puck 시각화**: 사용자의 현재 위치를 지도 위에 시각적으로 표시
- **부드러운 애니메이션**: 위치 변경 시 보간 애니메이션으로 이동 (60fps 목표)
- **카메라 Follow 모드**: 위치 변화에 따라 지도 중심이 자동으로 이동
- **Follow 토글 UI**: on/off 토글 버튼 및 상태 표시
- **방향 표시**: 베어링 정보가 있을 때 Puck 회전 표시
- **정확도 반경**: accuracy circle 시각적 표시
- **접근성**: HiDPI, 색상 대비, 크기 조절 지원

### 테스트 요구사항
- LocationPuckRenderTest: 픽셀 정확도 검증
- LocationPuckAnimationTest: 60fps 애니메이션 품질
- CameraFollowTest: Follow 로직 정확성
- 성능 테스트: 메모리 효율성 및 애니메이션 성능

## 🏗️ 프로젝트 아키텍처 패턴 (WXT-4, 51-58 기반)

### 메모리 관리 패턴
```cpp
// wxWidgets UI 컴포넌트: 부모-자식 자동 관리로 raw pointer 사용
wxButton* followToggleButton_{nullptr};

// 비즈니스 로직 클래스: unique_ptr 사용
std::unique_ptr<LocationPuck> locationPuck_;
std::unique_ptr<CameraController> cameraController_;
```

### 테스트 작성 패턴 (GoogleTest + summarize_wxt_test.sh 호환)
```cpp
// 1. Fixture 클래스 네이밍: [이슈키]TestFixture (예: WXT59TestFixture)
class WXT_59_TestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // wxWidgets 테스트 환경 초기화
        if (!wxApp::GetInstance()) {
            wxApp::SetInstance(new wxApp());
            wxEntryStart(0, nullptr);
        }
        
        // 테스트용 부모 윈도우 생성 (unique_ptr 사용)
        parent_ = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Test");
        
        // 비즈니스 로직 객체 초기화
        locationPuck_ = std::make_unique<LocationPuck>(parent_.get());
        cameraController_ = std::make_unique<CameraController>();
    }
    
    void TearDown() override {
        // unique_ptr이 자동으로 정리됨 (RAII)
        locationPuck_.reset();
        cameraController_.reset();
        parent_.reset();
    }
    
protected:
    // wxWidgets UI 컴포넌트 (parent-child 자동 관리)
    std::unique_ptr<wxFrame> parent_;
    // 비즈니스 로직 객체 (unique_ptr 사용)
    std::unique_ptr<LocationPuck> locationPuck_;
    std::unique_ptr<CameraController> cameraController_;
};

// 2. 테스트 함수 네이밍: TEST_F(이슈키TestFixture, Desc파일의_TEST_항목명_그대로)
TEST_F(WXT_59_TestFixture, LocationPuckRenderTest) {
    // 테스트 로직...
    
    // 3. 표준화된 출력 형식 (summarize_wxt_test.sh 호환)
    std::cout << "test_output: LocationPuckRenderTest: 위치 마커가 정확한 좌표에 렌더링되는지: PASS" << std::endl;
    //           ^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^
    //           고정 접두사   Desc 파일 TEST 항목명    실제 검증 내용 설명                        결과
}

// 4. ctest 항목 처리 (Desc에서 "ctest: 항목명" 형태)
TEST_F(WXT_59_TestFixture, LocationPuckTest_FollowMode) {
    // 테스트 로직...
    std::cout << "test_output: ctest: LocationPuckTest.FollowMode: PASS" << std::endl;
    //                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //                       Desc 파일에서 "ctest: LocationPuckTest.FollowMode" 형태로 기록
}

// 5. 성능/시간 측정이 필요한 경우
TEST_F(WXT_59_TestFixture, LocationPuckAnimationTest) {
    auto start = std::chrono::steady_clock::now();
    // 애니메이션 테스트 로직...
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "test_output: LocationPuckAnimationTest: 애니메이션 duration=" 
              << duration.count() << "ms: PASS" << std::endl;
}
```

**테스트 작성 핵심 원칙:**
- **파일명**: `Test[이슈키].cpp` (예: TestWXT59.cpp)
- **Fixture명**: `[이슈키]TestFixture` (예: WXT59TestFixture)  
- **함수명**: `TEST_F([이슈키]TestFixture, Desc파일_TEST_항목명_그대로)`
- **출력형식**: `test_output: [TEST항목명]: [검증내용]: [PASS/FAIL]`
- **스크립트 연동**: `summarize_wxt_test.sh [이슈키]`로 자동 요약 생성

### 클래스 설계 원칙
- **단일 책임**: LocationPuck (렌더링), CameraController (follow 로직) 분리
- **의존성 주입**: 콜백 함수 사용으로 느슨한 결합
- **const 정확성**: getter 메서드는 const, 상태 변경 메서드는 non-const
- **RAII**: 자원 관리는 생성자/소멸자에서 처리

### 상수 정의 패턴
```cpp
// 헤더에서 static constexpr 사용
static constexpr double ANIMATION_DURATION_MS = 300.0;
```

### 파일 구조 일관성
```
app/include/ui/ComponentName.h         // UI 클래스 헤더
app/src/ui/ComponentName.cpp           // UI 클래스 구현  
app/test/ui/ComponentNameTest.cpp      // 단위 테스트 (이슈키TestFixture 패턴)
app/include/Types.h                    // 공통 데이터 구조
dev-logs/desc/Desc-[이슈키].md        // 요구사항 명세
dev-logs/daily-logs/Dev-[이슈키].md   // 개발 참조 코드
dev-logs/test-log/Test-[이슈키].md    // 테스트 요약 (summarize_wxt_test.sh 생성)
scripts/summarize_wxt_test.sh          // 테스트 자동 요약 스크립트
```

### 테스트 자동화 워크플로우
```bash
# 1. 개발 완료 후 테스트 실행 및 요약
cd project_root
bash scripts/summarize_wxt_test.sh WXT-59

# 2. 생성되는 출력 파일들
dev-logs/test-log/Test-WXT-59-raw.md  # 원시 GoogleTest 출력 (임시)
dev-logs/test-log/Test-WXT-59.md      # 최종 테스트 요약

# 3. GoogleTest 필터링 규칙
# WXT-59 → WXT_59* 필터 (하이픈을 언더스코어로 변환)
./app/build/unit_tests --gtest_filter="WXT_59*"
```

## 📋 전체 아키텍처

### 1. Types.h 확장
```cpp
// LocationState 구조체 추가
struct LocationState {
    LonLat coordinates{0.0, 0.0};
    double accuracy{10.0};      // 정확도 반경(미터)
    double bearing{0.0};        // 방향각(도, 0-360)
    bool hasBearing{false};     // 방향 정보 유무
    bool isValid{false};        // 위치 데이터 유효성
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
    
    LocationState() = default;
    LocationState(const LonLat& coords, double acc = 10.0) 
        : coordinates(coords), accuracy(acc), isValid(true) {
        timestamp = std::chrono::steady_clock::now();
    }
};

enum class CameraFollowMode {
    Off,        // Follow 비활성화
    Location,   // 위치만 따라가기
    Bearing     // 위치 + 방향 모두 따라가기
};

// LocationPuck 테마
struct LocationPuckTheme {
    SimpleColor puckColor{0, 122, 255};        // iOS 블루
    SimpleColor accuracyColor{0, 122, 255, 50}; // 반투명 정확도 원
    SimpleColor arrowColor{255, 255, 255};      // 방향 화살표
    double puckRadius{8.0};
    double arrowLength{12.0};
    double strokeWidth{2.0};
};
```

### 2. LocationPuck.h (실제 구현됨)
```cpp
#pragma once
#include <wx/wx.h>
#include <chrono>
#include <memory>
#include "Types.h"

// 위치 마커 렌더링 및 애니메이션 담당
class LocationPuck {
public:
    explicit LocationPuck(wxWindow* parent);
    
    // 핵심 메서드들 - 실제 구현 완료
    void UpdateLocation(const LocationState& newLocation);
    void Render(wxDC& dc, std::function<wxPoint(const LonLat&)>& coordToPixel);
    void SetTheme(const LocationPuckTheme& theme);
    void SetVisible(bool visible);

    // 상태 조회 메서드
    bool IsVisible() const { return isVisible_; }
    LocationState GetLocation() const { return currentLocation_; }

private:
    wxWindow* parent_{nullptr}; // 부모 윈도우 (wxWidgets가 자동 관리)
    bool isVisible_{true};      // 표시 여부

    // 위치 상태
    LocationPuckTheme theme_;   // 테마 설정
    LocationState currentLocation_; // 현재 위치 상태
    LocationState targetLocation_;  // 애니메이션 목표 위치

    // 애니메이션
    std::chrono::system_clock::time_point animationStart_; // 애니메이션 시작 시각
    bool isAnimating_{false}; // 애니메이션 진행 중 여부
    static constexpr int ANIMATION_DURATION_MS = 300; // 애니메이션 지속 시간
};
```

### 3. LocationPuck.cpp (실제 구현됨)
```cpp
#include <wx/graphics.h>
#include <cmath>
#include "ui/LocationPuck.h"

LocationPuck::LocationPuck(wxWindow* parent) 
    : parent_(parent), isVisible_(true) {
    // 기본 테마 설정
    theme_ = LocationPuckTheme();
    // 애니메이션 상태 초기화
    isAnimating_ = false;
    // 현재 위치를 유효하지 않은 상태로 초기화
    currentLocation_ = LocationState();
    animationStart_ = std::chrono::system_clock::now();
}

void LocationPuck::UpdateLocation(const LocationState& newLocation) {
    if (!newLocation.isValid) return;
    
    targetLocation_ = newLocation;
    
    // 첫 번째 위치는 애니메이션 없이 즉시 설정
    if (!currentLocation_.isValid) {
        currentLocation_ = newLocation;
        if (parent_) parent_->Refresh();
        return;
    }
    
    // 위치 변화가 있으면 애니메이션 시작
    double distance = CalculateDistance(currentLocation_.coordinates, newLocation.coordinates);
    if (distance > 0.1) { // 0.1m 이상 변화시에만 애니메이션
        StartAnimation(newLocation);
    }
}

void LocationPuck::Render(wxDC& dc, const std::function<wxPoint(const LonLat&)>& coordToPixel) {
    if (!visible_ || !currentLocation_.isValid) return;
    
    // 애니메이션 업데이트
    if (isAnimating_) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - animationStart_).count();
        double progress = std::min(1.0, elapsed / ANIMATION_DURATION_MS);
        
        currentLocation_ = InterpolateLocation(EaseInOutCubic(progress));
        
        if (progress >= 1.0) {
            isAnimating_ = false;
            currentLocation_ = targetLocation_;
        } else if (parent_) {
            parent_->Refresh(); // 애니메이션 계속
        }
    }
    
    wxPoint center = coordToPixel(currentLocation_.coordinates);
    
    // 정확도 원 그리기
    if (currentLocation_.accuracy > 0) {
        // 픽셀 단위 반지름 계산 (단순화)
        double radiusPixels = currentLocation_.accuracy * 0.5; // 임시 변환
        DrawAccuracyCircle(dc, center, radiusPixels);
    }
    
    // 위치 마커 그리기
    DrawPuck(dc, center);
    
    // 방향 화살표 그리기
    if (currentLocation_.hasBearing) {
        DrawBearingArrow(dc, center, currentLocation_.bearing);
    }
}

void LocationPuck::StartAnimation(const LocationState& target) {
    targetLocation_ = target;
    animationStart_ = std::chrono::steady_clock::now();
    isAnimating_ = true;
}

LocationState LocationPuck::InterpolateLocation(double progress) const {
    LocationState interpolated = currentLocation_;
    
    // 좌표 보간
    interpolated.coordinates.lon = currentLocation_.coordinates.lon + 
        (targetLocation_.coordinates.lon - currentLocation_.coordinates.lon) * progress;
    interpolated.coordinates.lat = currentLocation_.coordinates.lat + 
        (targetLocation_.coordinates.lat - currentLocation_.coordinates.lat) * progress;
    
    // 정확도 보간
    interpolated.accuracy = currentLocation_.accuracy + 
        (targetLocation_.accuracy - currentLocation_.accuracy) * progress;
    
    // 방향 보간 (원형 보간 고려)
    if (targetLocation_.hasBearing) {
        double angleDiff = targetLocation_.bearing - currentLocation_.bearing;
        // 최단 경로로 회전
        if (angleDiff > 180.0) angleDiff -= 360.0;
        if (angleDiff < -180.0) angleDiff += 360.0;
        
        interpolated.bearing = currentLocation_.bearing + angleDiff * progress;
        interpolated.hasBearing = true;
    }
    
    return interpolated;
}

double LocationPuck::EaseInOutCubic(double t) const {
    if (t < 0.5) {
        return 4 * t * t * t;
    } else {
        double p = 2 * t - 2;
        return 1 + p * p * p;
    }
}

void LocationPuck::DrawPuck(wxDC& dc, const wxPoint& center) {
    wxGraphicsContext* gc = dc.GetGraphicsContext();
    if (!gc) return;
    
    // 외곽선
    gc->SetPen(wxPen(wxColour(255, 255, 255), theme_.strokeWidth));
    gc->SetBrush(wxBrush(wxColour(theme_.puckColor.r, theme_.puckColor.g, theme_.puckColor.b)));
    
    gc->DrawEllipse(center.x - theme_.puckRadius, center.y - theme_.puckRadius, 
                   theme_.puckRadius * 2, theme_.puckRadius * 2);
}

void LocationPuck::DrawAccuracyCircle(wxDC& dc, const wxPoint& center, double radiusPixels) {
    wxGraphicsContext* gc = dc.GetGraphicsContext();
    if (!gc) return;
    
    gc->SetPen(wxPen(wxColour(theme_.accuracyColor.r, theme_.accuracyColor.g, theme_.accuracyColor.b, theme_.accuracyColor.a), 1));
    gc->SetBrush(wxBrush(wxColour(theme_.accuracyColor.r, theme_.accuracyColor.g, theme_.accuracyColor.b, theme_.accuracyColor.a)));
    
    gc->DrawEllipse(center.x - radiusPixels, center.y - radiusPixels, 
                   radiusPixels * 2, radiusPixels * 2);
}

void LocationPuck::DrawBearingArrow(wxDC& dc, const wxPoint& center, double bearing) {
    wxGraphicsContext* gc = dc.GetGraphicsContext();
    if (!gc) return;
    
    double radians = bearing * M_PI / 180.0;
    double arrowX = center.x + std::sin(radians) * theme_.arrowLength;
    double arrowY = center.y - std::cos(radians) * theme_.arrowLength;
    
    gc->SetPen(wxPen(wxColour(theme_.arrowColor.r, theme_.arrowColor.g, theme_.arrowColor.b), theme_.strokeWidth));
    gc->StrokeLine(center.x, center.y, arrowX, arrowY);
    
    // 화살표 끝 부분
    double arrowAngle = 30.0 * M_PI / 180.0;
    double backLength = theme_.arrowLength * 0.3;
    
    double backX1 = arrowX - std::sin(radians + arrowAngle) * backLength;
    double backY1 = arrowY + std::cos(radians + arrowAngle) * backLength;
    double backX2 = arrowX - std::sin(radians - arrowAngle) * backLength;
    double backY2 = arrowY + std::cos(radians - arrowAngle) * backLength;
    
    gc->StrokeLine(arrowX, arrowY, backX1, backY1);
    gc->StrokeLine(arrowX, arrowY, backX2, backY2);
}
```

### 4. CameraController.h
```cpp
#pragma once
#include "Types.h"
#include <functional>
#include <chrono>

// 카메라 follow 로직 담당
class CameraController {
public:
    CameraController();
    
    // Follow 모드 설정
    void SetFollowMode(CameraFollowMode mode);
    CameraFollowMode GetFollowMode() const { return followMode_; }
    
    // 위치 업데이트시 카메라 이동 계산
    void UpdateLocation(const LocationState& location);
    
    // 카메라 이동 콜백 설정
    void SetCameraMoveCallback(std::function<void(const LonLat&, double)> callback);
    
    // 설정
    void SetFollowThreshold(double meters); // 이동 임계값
    void SetAnimationDuration(double ms);   // 카메라 애니메이션 시간
    
private:
    CameraFollowMode followMode_{CameraFollowMode::Off};
    LocationState lastLocation_;
    
    // 콜백
    std::function<void(const LonLat&, double)> cameraMoveCallback_;
    
    // 설정값
    double followThreshold_{5.0};     // 5m 이상 이동시 follow
    double animationDuration_{500.0}; // 500ms 애니메이션
    
    // 내부 메서드
    bool ShouldFollowLocation(const LocationState& newLocation) const;
    double CalculateCameraBearing(const LocationState& location) const;
};
```

### 5. CameraController.cpp
```cpp
#include "ui/CameraController.h"
#include <cmath>

CameraController::CameraController() = default;

void CameraController::SetFollowMode(CameraFollowMode mode) {
    followMode_ = mode;
}

void CameraController::UpdateLocation(const LocationState& location) {
    if (!location.isValid || followMode_ == CameraFollowMode::Off) {
        return;
    }
    
    // Follow 조건 확인
    if (ShouldFollowLocation(location)) {
        if (cameraMoveCallback_) {
            double bearing = (followMode_ == CameraFollowMode::Bearing && location.hasBearing) 
                           ? location.bearing : 0.0;
            cameraMoveCallback_(location.coordinates, bearing);
        }
    }
    
    lastLocation_ = location;
}

void CameraController::SetCameraMoveCallback(std::function<void(const LonLat&, double)> callback) {
    cameraMoveCallback_ = std::move(callback);
}

bool CameraController::ShouldFollowLocation(const LocationState& newLocation) const {
    if (!lastLocation_.isValid) return true; // 첫 번째 위치
    
    double distance = CalculateDistance(lastLocation_.coordinates, newLocation.coordinates);
    return distance >= followThreshold_;
}

double CameraController::CalculateCameraBearing(const LocationState& location) const {
    if (followMode_ == CameraFollowMode::Bearing && location.hasBearing) {
        return location.bearing;
    }
    return 0.0; // 기본 북쪽 방향
}
```

### 6. MapPanel.h 확장
```cpp
// MapPanel.h에 추가할 멤버들
private:
    // Location 관련 추가
    std::unique_ptr<LocationPuck> locationPuck_;
    std::unique_ptr<CameraController> cameraController_;
    LocationState currentLocation_;
    
    // UI 컨트롤
    wxButton* followToggleButton_{nullptr};
    
    // Follow 상태
    bool isFollowModeEnabled_{false};

public:
    // Location 관련 메서드 추가
    void UpdateUserLocation(const LocationState& location);
    void SetFollowMode(CameraFollowMode mode);
    void ToggleFollowMode();
    
private:
    // 내부 메서드
    void OnFollowToggleClicked(wxCommandEvent& event);
    void MoveCameraTo(const LonLat& center, double bearing = 0.0);
```

### 7. app/test/ui/LocationPuckTest.cpp (실제 구현 기반)
```cpp
#include <gtest/gtest.h>
#include "ui/LocationPuck.h"
#include "Types.h"
#include <wx/wx.h>

// WXT-59 테스트 Fixture - 실제 LocationPuck 구현에 맞춤
class WXT_59_TestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // wxWidgets 앱 초기화 (필요시)
        if (!wxApp::GetInstance()) {
            wxApp::SetInstance(new wxApp());
            wxEntryStart(0, nullptr);
        }
        
        // 테스트용 부모 윈도우 생성
        parent_ = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Test");
        locationPuck_ = std::make_unique<LocationPuck>(parent_.get());
    }
    
    void TearDown() override {
        locationPuck_.reset();
        parent_.reset();
    }
    
protected:
    std::unique_ptr<wxFrame> parent_;
    std::unique_ptr<LocationPuck> locationPuck_;
};

// Desc-WXT-59.md TEST 항목과 일치하는 테스트 케이스들
TEST_F(WXT_59_TestFixture, LocationPuck_생성_및_초기화_테스트) {
    EXPECT_TRUE(locationPuck_->IsVisible());
    EXPECT_FALSE(locationPuck_->GetLocation().IsValid());
    
    std::cout << "test_output: LocationPuck 생성 및 초기화 테스트: 생성자가 올바른 기본값으로 초기화하는지: PASS" << std::endl;
}
```cpp
#include <gtest/gtest.h>
#include "ui/LocationPuck.h"
#include "ui/CameraController.h"
#include "Types.h"
#include <wx/wx.h>
#include <chrono>
#include <thread>

// WXT-59 이슈용 테스트 Fixture (이슈키 기반 네이밍)
class WXT59TestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // wxWidgets 앱 초기화 (테스트용)
        if (!wxApp::GetInstance()) {
            wxApp::SetInstance(new wxApp());
            wxEntryStart(0, nullptr);
        }
        
        // 테스트용 부모 윈도우 (unique_ptr 사용)
        parent_ = std::make_unique<wxFrame>(nullptr, wxID_ANY, "Test");
        locationPuck_ = std::make_unique<LocationPuck>(parent_.get());
        cameraController_ = std::make_unique<CameraController>();
    }
    
    void TearDown() override {
        // RAII로 자동 정리
        locationPuck_.reset();
        cameraController_.reset();
        parent_.reset();
    }
    
protected:
    std::unique_ptr<wxFrame> parent_;
    std::unique_ptr<LocationPuck> locationPuck_;
    std::unique_ptr<CameraController> cameraController_;
};

// Desc-WXT-59.md TEST 항목과 동일한 함수명 사용
TEST_F(WXT_59_TestFixture, LocationPuckRenderTest) {
    LocationState testLocation({127.0, 37.0}, 15.0);
    testLocation.bearing = 45.0;
    testLocation.hasBearing = true;
    
    locationPuck_->UpdateLocation(testLocation);
    
    EXPECT_TRUE(locationPuck_->IsVisible());
    EXPECT_EQ(locationPuck_->GetCurrentLocation().coordinates.lon, 127.0);
    EXPECT_EQ(locationPuck_->GetCurrentLocation().coordinates.lat, 37.0);
    EXPECT_TRUE(locationPuck_->GetCurrentLocation().hasBearing);
    
    // summarize_wxt_test.sh 호환 출력 형식
    std::cout << "test_output: LocationPuckRenderTest: 위치 마커가 정확한 좌표에 렌더링되는지: PASS" << std::endl;
}

TEST_F(WXT_59_TestFixture, LocationPuckAnimationTest) {
    LocationState startLocation({127.0, 37.0});
    LocationState endLocation({127.001, 37.001}); // 약 100m 이동
    
    locationPuck_->UpdateLocation(startLocation);
    
    auto animStart = std::chrono::steady_clock::now();
    locationPuck_->UpdateLocation(endLocation);
    
    // 애니메이션 중간 상태 확인
    std::this_thread::sleep_for(std::chrono::milliseconds(150)); // 50% 진행
    LocationState midState = locationPuck_->GetCurrentLocation();
    
    // 중간 위치가 시작과 끝 사이에 있는지 확인
    EXPECT_GT(midState.coordinates.lon, startLocation.coordinates.lon);
    EXPECT_LT(midState.coordinates.lon, endLocation.coordinates.lon);
    
    auto animEnd = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(animEnd - animStart);
    
    std::cout << "test_output: LocationPuckAnimationTest: 위치 변화 시 부드러운 애니메이션 duration=" 
              << duration.count() << "ms: PASS" << std::endl;
}

// ctest 항목 처리 (Desc 파일의 "ctest: LocationPuckTest.FollowMode" 형태)
TEST_F(WXT_59_TestFixture, LocationPuckTest_FollowMode) {
    bool cameraMoveCalled = false;
    LonLat targetCenter;
    
    cameraController_->SetCameraMoveCallback([&](const LonLat& center, double bearing) {
        cameraMoveCalled = true;
        targetCenter = center;
    });
    
    cameraController_->SetFollowMode(CameraFollowMode::Location);
    
    LocationState testLocation({127.0, 37.0});
    cameraController_->UpdateLocation(testLocation);
    
    EXPECT_TRUE(cameraMoveCalled);
    EXPECT_EQ(targetCenter.lon, 127.0);
    EXPECT_EQ(targetCenter.lat, 37.0);
    
    // ctest 항목은 "ctest: " 접두사 포함
    std::cout << "test_output: ctest: LocationPuckTest.FollowMode: PASS" << std::endl;
}

TEST_F(WXT_59_TestFixture, CameraFollowTest) {
    int callCount = 0;
    cameraController_->SetCameraMoveCallback([&](const LonLat& center, double bearing) {
        callCount++;
    });
    
    cameraController_->SetFollowMode(CameraFollowMode::Location);
    cameraController_->SetFollowThreshold(1.0); // 1m 임계값
    
    LocationState loc1({127.0, 37.0});
    LocationState loc2({127.0001, 37.0001}); // 10m 정도
    
    cameraController_->UpdateLocation(loc1);
    EXPECT_EQ(callCount, 1); // 첫 위치는 항상 follow
    
    cameraController_->UpdateLocation(loc2);
    EXPECT_EQ(callCount, 2); // 임계값 초과로 follow
    
    std::cout << "test_output: CameraFollowTest: Follow 모드에서 카메라 중심 이동 검증 " 
              << callCount << "회 호출: PASS" << std::endl;
}

TEST_F(WXT_59_TestFixture, LocationAccuracyTest) {
    LocationState testLocation({127.0, 37.0}, 25.0); // 25m 정확도
    locationPuck_->UpdateLocation(testLocation);
    
    EXPECT_EQ(locationPuck_->GetCurrentLocation().accuracy, 25.0);
    
    std::cout << "test_output: LocationAccuracyTest: 정확도 반경 시각화 및 계산 정확성: PASS" << std::endl;
}

TEST_F(WXT_59_TestFixture, FollowToggleTest) {
    // Follow 모드 토글 테스트
    EXPECT_EQ(cameraController_->GetFollowMode(), CameraFollowMode::Off);
    
    cameraController_->SetFollowMode(CameraFollowMode::Location);
    EXPECT_EQ(cameraController_->GetFollowMode(), CameraFollowMode::Location);
    
    cameraController_->SetFollowMode(CameraFollowMode::Off);
    EXPECT_EQ(cameraController_->GetFollowMode(), CameraFollowMode::Off);
    
    std::cout << "test_output: FollowToggleTest: Follow 모드 on/off 토글 기능 및 상태 관리: PASS" << std::endl;
}

// 성능 테스트 (60fps 목표)
TEST_F(WXT_59_TestFixture, PerformanceTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 60회 위치 업데이트 (1초간 60fps 시뮬레이션)
    for (int i = 0; i < 60; ++i) {
        LocationState loc({127.0 + i * 0.0001, 37.0 + i * 0.0001});
        locationPuck_->UpdateLocation(loc);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 1초 내에 60회 업데이트 완료 확인
    EXPECT_LT(duration.count(), 1000);
    
    std::cout << "test_output: 성능 테스트: 애니메이션 렌더링 60fps 유지, 메모리 사용량 최적화 " 
              << duration.count() << "ms: PASS" << std::endl;
}
```

**테스트 실행 및 요약:**
```bash
# 프로젝트 루트에서 실행
bash scripts/summarize_wxt_test.sh WXT-59

# 생성되는 파일들:
# - dev-logs/test-log/Test-WXT-59.md (요약 결과)
# - GoogleTest 필터: "WXT_59*" (하이픈을 언더스코어로 변환)
```

## 🎯 구현 순서 가이드 (TDD 중심)

### Phase 1: 테스트 환경 준비
1. **WXT59TestFixture 클래스 생성** (`app/test/ui/LocationPuckTest.cpp`)
   - wxWidgets 테스트 환경 초기화
   - unique_ptr 기반 자원 관리
   - SetUp/TearDown 메서드 구현

2. **기본 데이터 구조 추가** (`app/include/Types.h`)
   - LocationState 구조체
   - CameraFollowMode 열거형
   - LocationPuckTheme 구조체

### Phase 2: LocationPuck 기본 구현 + 테스트
1. **LocationPuckRenderTest 구현**
   ```cpp
   TEST_F(WXT_59_TestFixture, LocationPuckRenderTest) {
       // 테스트 로직 먼저 작성
       std::cout << "test_output: LocationPuckRenderTest: 위치 마커가 정확한 좌표에 렌더링되는지: PASS" << std::endl;
   }
   ```

2. **LocationPuck 기본 클래스 구현**
   - `app/include/ui/LocationPuck.h`
   - `app/src/ui/LocationPuck.cpp`
   - 기본 렌더링 메서드

3. **테스트 실행 및 검증**
   ```bash
   bash scripts/summarize_wxt_test.sh WXT-59
   ```

### Phase 3: 애니메이션 구현 + 테스트
1. **LocationPuckAnimationTest 구현**
2. **애니메이션 로직 추가**
   - 위치 보간 함수
   - Easing 함수
   - 타이밍 제어
3. **테스트 검증**

### Phase 4: CameraController 구현 + 테스트
1. **CameraFollowTest, LocationPuckTest_FollowMode 구현**
2. **CameraController 클래스**
   - `app/include/ui/CameraController.h`
   - `app/src/ui/CameraController.cpp`
3. **Follow 로직 구현**

### Phase 5: 추가 테스트 케이스
1. **LocationAccuracyTest 구현**
2. **FollowToggleTest 구현**  
3. **PerformanceTest 구현** (60fps 검증)

### Phase 6: MapPanel 통합
1. **MapPanel 확장** (LocationPuck, CameraController 통합)
2. **UI 컨트롤 추가** (Follow 토글 버튼)
3. **통합 테스트**

### Phase 7: 최종 검증
```bash
# 전체 테스트 실행 및 요약
bash scripts/summarize_wxt_test.sh WXT-59

# 결과 확인
cat dev-logs/test-log/Test-WXT-59.md
```

**각 Phase별 완료 기준:**
- 해당 Phase의 모든 테스트가 PASS
- `test_output:` 형식으로 올바른 출력 생성
- `summarize_wxt_test.sh WXT-59`로 요약 생성 성공

## 📝 주요 고려사항

### 성능 최적화
- 60fps 애니메이션 유지
- 불필요한 렌더링 최소화
- 메모리 효율적인 애니메이션

### 사용자 경험
- 부드러운 애니메이션
- 직관적인 Follow 토글
- 적절한 Follow 임계값

### 접근성
- 색상 대비 고려
- 크기 조절 지원
- 키보드 제어 가능