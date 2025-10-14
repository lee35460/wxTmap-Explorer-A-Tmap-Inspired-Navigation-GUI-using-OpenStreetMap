# WXT-59: Location Puck + 카메라 Follow 기능

## 개요
지도 상에 사용자의 현재 위치를 시각적으로 표시하는 Location Puck(위치 마커) UI를 구현하고, 사용자의 위치 변화에 따라 카메라가 자동으로 따라가는(follow) 기능을 추가합니다. WXT-57의 경로 시각화와 WXT-58의 경유지 관리 기능을 기반으로, 실시간 위치 추적을 통한 완전한 내비게이션 경험을 제공합니다.

**우선순위:** 높음 - WXT-57/58 완료 후 내비게이션 완성을 위한 필수 기능

## 🔧 구현 및 주요 파일
app/include/ui/LocationPuck.h, app/src/ui/LocationPuck.cpp: 위치 마커 UI 및 애니메이션
app/include/ui/CameraController.h, app/src/ui/CameraController.cpp: 카메라 follow 로직 분리
app/include/MapPanel.h, app/src/MapPanel.cpp: LocationPuck 및 CameraController 통합
app/include/Types.h: LocationState, CameraFollowMode 데이터 구조 확장
app/test/ui/LocationPuckTest.cpp: Location Puck 단위 테스트

## ✅ Acceptance Criteria (AC)
• 사용자의 현재 위치가 지도 위에 Location Puck으로 시각적으로 표시된다
• 위치 변경 시 Puck이 부드러운 애니메이션으로 이동한다 (보간 애니메이션)
• 카메라 Follow 모드에서 위치 변화에 따라 지도 중심이 자동으로 이동한다
• Follow 모드 on/off 토글 버튼 및 상태 표시 제공
• 방향/베어링 정보가 있을 때 Puck 회전 표시 지원
• 정확도 반경(accuracy circle) 시각적 표시
• HiDPI/접근성(색상 대비, 크기) 지원

## ☑️ 체크리스트
• LocationPuck 클래스로 분리 (위치 마커 + 애니메이션)
• CameraController 클래스로 분리 (follow 로직 + 카메라 제어)
• 상태 바인딩 (실제/모의 위치 데이터 지원)
• 부드러운 애니메이션 (위치 보간, 회전 애니메이션)
• Follow 모드 UI 컨트롤 (토글 버튼, 상태 표시)
• 접근성 (색상 대비, 크기 조절, 키보드 제어)
• 성능 최적화 (60fps 애니메이션, 메모리 효율성)

## 🧪 TEST
• LocationPuckRenderTest: 위치 마커가 정확한 좌표에 렌더링되는지
• LocationPuckAnimationTest: 위치 변화 시 부드러운 애니메이션 duration
• ctest: LocationPuckTest.FollowMode
• CameraFollowTest: Follow 모드에서 카메라 중심 이동 검증
• LocationAccuracyTest: 정확도 반영 및 계산 정확성
• FollowToggleTest: Follow 모드 on/off 토글 기능 및 상태 관리
• VisibilityTest: 위치 마커 표시/숨김 토글 기능
• 성능 테스트: 애니메이션 렌더링 60fps 유지, 메모리 사용량 최적화

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, OpenStreetMap API
플랫폼: Cross-Platform (Windows 10+, macOS 10.15+, Ubuntu 20.04+)
빌드 시스템: CMake 3.16+, GoogleTest/GoogleMock
위치 시뮬레이션: Mock GPS 데이터 + 실시간 애니메이션

## 🧩 참고/연관 이슈
• WXT-57: Route Polyline (경로 시각화와 Location Puck 연동)
• WXT-58: Waypoint List Panel (경유지와 현재 위치 관계 표시)
• WXT-56: Turn Banner + Progress Bar (내비게이션 UI와 위치 동기화)
