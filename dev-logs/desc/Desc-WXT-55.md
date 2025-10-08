## 📋 개요
지도 상단에 속도, 제한속도, 남은 거리·ETA를 표시하는 MapOverlay HUD를 구현했습니다. 접근성, HiDPI, 테마 지원과 함께, 메트릭 계측 및 단위 테스트도 포함되어 있습니다.

**우선순위:** Medium - 실시간 정보 제공 및 접근성 강화

## 🔧 구현 및 주요 파일
app/include/ui/MapOverlayHud.h, app/src/ui/MapOverlayHud.cpp: MapOverlayHud 클래스 및 UI 구현
app/include/MapPanel.h, app/src/MapPanel.cpp: MapPanel 연동
app/test/ui/MapOverlayHudTest.cpp: 단위 테스트

## ✅ Acceptance Criteria (AC)
• 속도/제한속도/거리·ETA 정보 표시
• 접근성(HiDPI, 테마, 키보드 내비) 지원
• 메트릭 계측 및 단위 테스트 통과

## ☑️ 체크리스트
• MapOverlayHud 클래스 구현 및 연동
• 접근성/테마/HiDPI 지원
• 단위 테스트 및 코드 리뷰 통과

## 🧪 TEST
• MapOverlayHudTest: 정보 표시 및 접근성 테스트
• 성능/메트릭 계측 확인

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
