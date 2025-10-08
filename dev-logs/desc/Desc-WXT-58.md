
## 📋 개요
경유지(waypoint) 리스트를 시각적으로 표시하고, 사용자가 경유지 순서를 직관적으로 확인 및 정렬할 수 있는 UI 패널을 구현합니다.
초기 버전에서는 경유지 추가/삭제/순서변경(드래그&드롭 또는 버튼) 기능과, 리스트의 반응형 레이아웃 및 접근성(키보드 내비게이션, 폰트 크기 등)을 지원합니다.

**우선순위:** 높음 - 경로 탐색/편집의 핵심 UX 기능

## 🔧 구현 및 주요 파일
app/include/ui/WaypointListPanel.h, app/src/ui/WaypointListPanel.cpp: 경유지 리스트 패널 UI 및 상태 관리
app/include/Types.h: Waypoint 데이터 구조 정의
app/src/ui/: 기존 UI와의 연동

## ✅ Acceptance Criteria (AC)
• 경유지 리스트가 패널에 시각적으로 표시된다
• 경유지 순서 변경(드래그&드롭 또는 버튼)이 가능하다
• 경유지 추가/삭제가 정상 동작한다
• 리스트가 반응형으로 동작하며, HiDPI/접근성(키보드, 폰트 크기) 지원
• 상태(State) 주입 및 외부 모의 데이터로 테스트 가능

## ☑️ 체크리스트
• WaypointListPanel 클래스로 분리
• 상태 바인딩(모의 데이터 주입 가능)
• 드래그&드롭 또는 순서변경 버튼 구현
• 접근성(키보드 내비, 폰트 대비/크기)
• 더블버퍼링/성능 최적화

## 🧪 TEST
• 경유지 추가/삭제/정렬 시 UI가 즉시 반영됨
• ctest: WaypointListPanelTest.AddRemoveReorder
• 접근성 테스트(키보드 내비, 폰트 크기 변경)
• 첫 렌더링 2s 이내

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, OpenStreetMap API
플랫폼: Cross-Platform (Windows 10+, macOS 10.15+, Ubuntu 20.04+)
빌드 시스템: CMake 3.16+, vcpkg/Conan
