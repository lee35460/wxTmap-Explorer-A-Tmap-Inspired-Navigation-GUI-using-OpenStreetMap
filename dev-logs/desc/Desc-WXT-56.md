## 📋 개요
경로 안내를 위한 Turn Banner UI와 진행 바를 구현했습니다. AppFrame, MapPanel, TurnBanner 클래스가 연동되며, 접근성 및 상태 관리, 단위 테스트가 강화되었습니다.

**우선순위:** Medium - 내비게이션 UX 및 상태 시각화 강화

## 🔧 구현 및 주요 파일
app/include/ui/TurnBanner.h, app/src/ui/TurnBanner.cpp: TurnBanner UI 및 진행 바 구현
app/include/AppFrame.h, app/src/AppFrame.cpp: AppFrame 연동
app/include/MapPanel.h, app/src/MapPanel.cpp: MapPanel 연동
app/test/ui/TurnBannerTest.cpp: 단위 테스트

## ✅ Acceptance Criteria (AC)
• Turn Banner UI 및 진행 바 구현
• AppFrame/MapPanel과 연동
• 접근성 및 상태 관리 강화
• 단위 테스트 및 성능 기준 충족

## ☑️ 체크리스트
• TurnBanner/진행 바 UI 구현
• 상태 바인딩 및 접근성 지원
• 단위 테스트 및 코드 리뷰 통과

## 🧪 TEST
• TurnBannerTest: UI/진행 바 동작 및 접근성 테스트
• 성능/상태 반영 확인

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
