## 📋 개요
MapOverlay HUD 구현을 통해 속도/제한속도/거리 정보를 실시간으로 표시하는 오버레이 시스템을 구축합니다. 투명한 HUD 패널과 동적 정보 업데이트를 통해 내비게이션 경험을 향상시킵니다.

**우선순위:** Medium - UI/UX 개선 및 실시간 정보 표시

## 🔧 구현 및 주요 파일
app/include/ui/MapOverlay.h: MapOverlay HUD 클래스 정의 및 메서드
app/src/ui/MapOverlay.cpp: HUD 렌더링 및 데이터 업데이트 로직
app/include/ui/HudComponents.h: 속도계, 거리계 등 HUD 컴포넌트
app/test/ui/MapOverlayTest.cpp: 단위 테스트
app/CMakeLists.txt: 빌드 연동

## ✅ Acceptance Criteria (AC)
• 투명한 HUD 오버레이 패널 구현
• 실시간 속도 표시 (현재속도/제한속도)
• 목적지까지 남은 거리 및 예상 도착 시간 표시
• 동적 정보 업데이트 (1초 간격)
• 사용자 설정 가능한 HUD 투명도/위치

## ☑️ 체크리스트
• MapOverlay 클래스 구현 및 HUD 컴포넌트 설계
• 속도계, 거리계, ETA 표시 기능 구현
• 투명도 및 위치 설정 기능 추가
• 실시간 데이터 업데이트 로직 구현
• 단위 테스트 및 성능 검증 완료

## 🧪 TEST
• MapOverlayTest: HUD 오버레이 렌더링 정상 동작 (렌더링 FPS: 30fps 이상)
• SpeedDisplayTest: 속도 표시 정확성 검증 (오차율: 5% 이하)
• DistanceCalculationTest: 거리 계산 및 ETA 정확성 (계산 오차: 1% 이하)
• HudTransparencyTest: 투명도 조절 기능 검증 (투명도 범위: 0.1-1.0)
• RealTimeUpdateTest: 실시간 데이터 업데이트 성능 (업데이트 주기: 1초 이하)
• 커밋 8회, 코드 리뷰 완료 (수동 확인)

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, OpenGL/DirectX
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
그래픽: 2D 오버레이 렌더링, 투명도 블렌딩