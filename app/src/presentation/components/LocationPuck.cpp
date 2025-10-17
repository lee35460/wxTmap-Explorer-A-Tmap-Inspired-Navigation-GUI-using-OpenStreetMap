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
    : AnimatedComponent<NoWxBase>(), parent_(parent)
{
    // 기본 테마 설정
    theme_ = LocationPuckTheme();
    // 현재 위치를 유효하지 않은 상태로 초기화
    currentLocation_ = LocationState();
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

    // 첫 번째 위치 설정인지 확인 (현재 위치가 유효하지 않은 경우)
    if (!currentLocation_.IsValid()) {
        currentLocation_ = newLocation;
        SetPosition(newLocation.coordinates);
        if (parent_) parent_->Refresh();
        return;
    }

    // 위치 변화가 있으면 애니메이션 시작
    double distance = CalculateDistance(currentLocation_.coordinates, newLocation.coordinates);
    if (distance > 0.1) { // 0.1m 이상 변화시에만 애니메이션
        currentLocation_ = newLocation;
        AnimateTo(newLocation.coordinates, 300.0); // BaseComponent의 AnimateTo 사용
    } else {
        // 작은 변화는 즉시 반영
        currentLocation_ = newLocation;
        SetPosition(newLocation.coordinates);
        if (parent_) parent_->Refresh();
    }
}



void LocationPuck::Update(double deltaTime) {
    // BaseComponent의 애니메이션 업데이트 호출
    AnimatedComponent::Update(deltaTime);

    // 화면 갱신 요청
    if (parent_) {
        parent_->Refresh();
    }
}

void LocationPuck::Render(wxDC& dc, const CoordTransformFn& coordToPixel) {
    if (!IsVisible() || !currentLocation_.IsValid()) return;

    wxPoint center = coordToPixel(GetPosition());

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

void LocationPuck::SetLocationTheme(const LocationPuckTheme& theme) {
    theme_ = theme;
    if (parent_) {
        parent_->Refresh();
    }
}

LocationState LocationPuck::GetLocation() const {
    LocationState result = currentLocation_;
    // 항상 BaseComponent의 현재 위치를 사용 (애니메이션 중이든 아니든)
    result.coordinates = GetPosition();
    return result;
}

void LocationPuck::OnAnimationComplete() {
    // 애니메이션 완료 시 호출되는 콜백
    if (parent_) {
        parent_->Refresh();
    }
}

} // namespace ui
