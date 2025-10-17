#include "ProjectMap.h" 
#include PRESENTATION_LOCATION_PUCK
#include <wx/graphics.h> // for wxGraphicsContext
#include <spdlog/spdlog.h>
#include <cmath>
#include <iostream>

using namespace presentation::components;

namespace presentation::components {

/* 생성자 역할:
부모 윈도우 포인터를 저장 (wxWidgets parent-child 관계 설정)
기본 테마를 설정 (LocationPuckTheme의 기본 생성자 사용)
애니메이션 상태를 초기화 (false로 설정)
현재 위치를 유효하지 않은 상태로 초기화
*/
LocationPuck::LocationPuck(wxWindow* parent) 
    : parent_(parent)
    , isVisible_(true)
{
    // 기본 테마 설정
    theme_ = LocationPuckTheme();
    // 애니메이션 상태 초기화
    isAnimating_ = false;
    // 현재 위치를 유효하지 않은 상태로 초기화
    currentLocation_ = LocationState();
    animationStart_ = std::chrono::steady_clock::now();
}

/* 🎯 다음 구현 가이드: UpdateLocation 메서드
📋 UpdateLocation 메서드 요구사항
🔧 구현해야 할 핵심 기능:
위치 유효성 검사

newLocation의 위도/경도가 유효한 범위인지 확인
위도: -90.0 ~ 90.0, 경도: -180.0 ~ 180.0
애니메이션 트리거

현재 위치와 새 위치가 다를 경우 애니메이션 시작
isAnimating_을 true로 설정
animationStart_를 현재 시간으로 업데이트
위치 상태 업데이트

currentLocation_을 새로운 위치로 업데이트
부모 윈도우에게 리프레시 요청 (parent_->Refresh())
💡 구현 힌트:
위치 비교 시 부동소수점 정밀도 고려 (epsilon 값 사용)
std::chrono::system_clock::now()로 현재 시간 가져오기
parent_가 nullptr이 아닌지 확인 후 Refresh 호출
🎯 구현 목표:
LocationPuck.cpp 파일에서 생성자 다음에 UpdateLocation 메서드를 구현하세요. 위의 요구사항을 만족하는 코드를 작성해보세요.*/
void LocationPuck::UpdateLocation(const LocationState& newLocation) {
    // 위치 유효성 검사
    if (!newLocation.IsValid()) return;

    targetLocation_ = newLocation;

    // 첫 번째 위치는 애니매이션 없이 즉시 실행
    if (!currentLocation_.IsValid()) {
        currentLocation_ = newLocation;
        if (parent_) parent_->Refresh();
        return;
    }

    // 위치 변화가 있으면 애니매이션 시작
    double distance = CalculateDistance(currentLocation_.coordinates, newLocation.coordinates);
    if (distance > 0.1) { // 0.1m 이상 변화시에만 애니메이션
        StartAnimation(newLocation);
    }
}

void LocationPuck::StartAnimation(const LocationState& target) {
    targetLocation_ = target;
    animationStart_ = std::chrono::steady_clock::now();
    isAnimating_ = true;
    if (parent_) parent_->Refresh();
}

/*🔧 구현해야 할 핵심 기능:
가시성 체크

isVisible_이 false면 렌더링 하지 않음
위치 유효성 체크

currentLocation_.IsValid()가 false면 렌더링 하지 않음
좌표 변환

coordToPixel 함수로 GPS 좌표를 화면 픽셀로 변환
정확도 원 렌더링

반투명 원으로 위치 정확도 표시
색상: theme_.accuracyColor
위치 마커 렌더링

중심 원으로 현재 위치 표시
색상: theme_.puckColor, 반지름: theme_.puckRadius
방향 화살표 렌더링 (선택사항)

currentLocation_.hasBearing가 true일 때만
방향: currentLocation_.bearing 각도
*/

void LocationPuck::UpdateAnimation() {
    if (!isAnimating_) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - animationStart_).count();
    double progress = std::min(1.0, elapsed / static_cast<double>(ANIMATION_DURATION_MS));

    currentLocation_ = InterpolateLocation(EaseInOutCubic(progress));

    if (progress >= 1.0) {
        isAnimating_ = false;
        currentLocation_ = targetLocation_;
    }

    // 애니메이션이 진행 중이거나 완료되면 화면 갱신 요청
    if (parent_) {
        parent_->Refresh();
    }
}

void LocationPuck::Render(wxDC& dc, const std::function<wxPoint(const LonLat&)>& coordToPixel) {
    if (!isVisible_ || !currentLocation_.IsValid()) return;

    wxPoint center = coordToPixel(currentLocation_.coordinates);

    // 정확도 원 렌더링
    if (currentLocation_.accuracy > 0) {
        // 픽셀 단위 반지름 계산
        double radiusPixels = currentLocation_.accuracy * 0.5; // 단순 변환 (실제 투영법 고려 필요)
        DrawAccuracyCircle(dc, center, radiusPixels);
    }

    // 위치 마커 렌더링
    DrawPuck(dc, center);

    // 방향 화살표 렌더링
    if (currentLocation_.hasBearing) {
        // 방향 화살표 그리기 (선택사항)
        // 구현 생략 - 필요시 추가
        DrawBearingArrow(dc, center, currentLocation_.bearing);
    }
}

// 보간 함수
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

    // 방향 보간 (360도 회전 고려)
    if (currentLocation_.hasBearing && targetLocation_.hasBearing) {
        double angleDiff = targetLocation_.bearing - currentLocation_.bearing;
        // 최단 경로로 회전
        if (angleDiff > 180) angleDiff -= 360;
        if (angleDiff < -180) angleDiff += 360;

        interpolated.bearing = currentLocation_.bearing + angleDiff * progress;
        interpolated.hasBearing = true;
    } else {
        interpolated.bearing = targetLocation_.bearing;
        interpolated.hasBearing = targetLocation_.hasBearing;
    }
    
    return interpolated;
}

/**
 * @brief Cubic ease-in-out 보간 함수 (한글 설명)
 *
 * 이 함수는 애니메이션에서 자주 사용되는 cubic ease-in-out(삼차 완화) 보간 함수를 구현합니다.
 * t < 0.5 구간에서는 점점 가속(ease-in)되고, t >= 0.5 구간에서는 점점 감속(ease-out)되어
 * 자연스럽게 시작하고 부드럽게 멈추는 움직임을 만듭니다.
 *
 * 수식 유도 배경:
 *  - t < 0.5: 4 * t^3 (시작 시점에서 점점 빨라짐)
 *  - t >= 0.5: 1 + (2t - 2)^3 (끝 시점에서 점점 느려짐)
 * 이 수식은 t=0.5에서 가속과 감속이 부드럽게 연결되도록 설계되었습니다.
 * 위치 puck 등 애니메이션 이동에 적합합니다.
 *
 * @param t 0~1 사이의 정규화된 시간 값
 * @return 0~1 사이의 cubic ease-in-out 곡선을 따르는 보간 값
 */
double LocationPuck::EaseInOutCubic(double t) const {
    if (t < 0.5) {
        return 4 * t * t * t;
    } else {
        double p = 2 * t - 2;
        return 1 + p * p * p;
    }
}

// 정확도 원 그리기
void LocationPuck::DrawAccuracyCircle(wxDC& dc, const wxPoint& center, double radiusPixels) {
    wxGraphicsContext* gc =  dc.GetGraphicsContext(); // 고급 그래픽 컨텍스트 사용
    if (!gc) return; // 그래픽 컨텍스트가 없으면 종료

    gc->SetPen(wxPen(wxColor(theme_.accuracyColor.r, theme_.accuracyColor.g, theme_.accuracyColor.b, theme_.accuracyColor.a), 1)); // 얇은 테두리   
    gc->SetBrush(wxBrush(wxColor(theme_.accuracyColor.r, theme_.accuracyColor.g, theme_.accuracyColor.b, theme_.accuracyColor.a))); // 반투명 채우기

    gc->DrawEllipse(center.x - radiusPixels, center.y - radiusPixels, radiusPixels * 2, radiusPixels * 2); // 원 그리기

}

// 위치 마커 그리기
void LocationPuck::DrawPuck(wxDC& dc, const wxPoint& center) {
    wxGraphicsContext* gc =  dc.GetGraphicsContext(); // 고급 그래픽 컨텍스트 사용
    if (!gc) return; // 그래픽 컨텍스트가 없으면 종료

    // 외곽선
    gc->SetPen(wxPen(wxColor(255, 255, 255), theme_.strokeWidth));
    gc->SetBrush(wxBrush(wxColor(theme_.puckColor.r, theme_.puckColor.g, theme_.puckColor.b)));

    gc->DrawEllipse(center.x - theme_.puckRadius, center.y - theme_.puckRadius,
                   theme_.puckRadius * 2, theme_.puckRadius * 2);
}

// 방향 화살표 그리기 (선택사항)
void LocationPuck::DrawBearingArrow(wxDC& dc, const wxPoint& center, double bearing) {
    wxGraphicsContext* gc =  dc.GetGraphicsContext(); // 고급 그래픽 컨텍스트 사용
    if (!gc) return; // 그래픽 컨텍스트가 없으면 종료

    // 화살표 끝점 계산
    double radians = bearing * M_PI / 180.0;
    double arrowX = center.x + std::cos(radians) * theme_.arrowLength; // x축 방향)
    double arrowY = center.y - std::sin(radians) * theme_.arrowLength; // y축 반전

    // 화살표 끝 부분
    double arrowAngle = 30.0 * M_PI / 180.0; // 화살표 각도
    double backLength = theme_.arrowLength * 0.3; // 화살표

    double backX1 = arrowX - std::cos(radians + arrowAngle) * backLength;
    double backY1 = arrowY + std::sin(radians + arrowAngle) * backLength;
    double backX2 = arrowX - std::cos(radians - arrowAngle) * backLength;
    double backY2 = arrowY + std::sin(radians - arrowAngle) * backLength;

    // 화살표 그리기
    gc->StrokeLine(arrowX, arrowY, backX1, backY1);
    gc->StrokeLine(arrowX, arrowY, backX2, backY2);
}

void LocationPuck::SetVisible(bool visible) {
    isVisible_ = visible;
    if (parent_) {
        parent_->Refresh();
    }
}

LocationState LocationPuck::GetLocation() const {
    // 애니메이션 중이면 보간된 위치 반환
    if (isAnimating_) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - animationStart_);
        double progress = static_cast<double>(elapsed.count()) / ANIMATION_DURATION_MS;
        
        if (progress >= 1.0) {
            // 애니메이션 완료 - 타겟 위치 반환
            return targetLocation_;
        } else {
            // 애니메이션 진행 중 - 보간된 위치 반환
            return InterpolateLocation(EaseInOutCubic(progress));
        }
    }
    
    // 애니메이션 중이 아니면 현재 위치 반환
    return currentLocation_;
}

void LocationPuck::SetTheme(const LocationPuckTheme& theme) {
    theme_ = theme;
    if (parent_) {
        parent_->Refresh();
    }
}

} // namespace ui
