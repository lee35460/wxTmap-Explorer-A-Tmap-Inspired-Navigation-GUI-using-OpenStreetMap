# WXT-59: Location Puck + 카메라 Follow 기능

## 📋 개요
지도 상에 사용자의 현재 위치를 시각적으로 표시하는 Location Puck(위치 마커) UI를 구현하고, 사용자의 위치 변화에 따라 카메라가 자동으로 따라가는(follow) 기능을 추가합니다. 이 기능은 내비게이션 및 실시간 위치 기반 서비스의 핵심 UX로, 지도 상에서 사용자의 이동 경로를 직관적으로 확인할 수 있도록 지원합니다.

**우선순위:** 높음 - 실시간 위치 기반 내비게이션의 필수 기능

## 🔧 구현 및 주요 파일
app/include/ui/LocationPuck.h, app/src/ui/LocationPuck.cpp: 위치 마커 UI 및 상태 관리
app/include/MapPanel.h, app/src/MapPanel.cpp: 카메라 이동 및 follow 로직
app/include/Types.h: 위치/좌표 데이터 구조 정의

## ✅ Acceptance Criteria (AC)
• 사용자의 현재 위치가 지도 위에 puck(마커)로 표시된다
• 위치가 변경되면 puck이 부드럽게 이동한다
• 카메라 follow 모드에서 사용자의 위치 변화에 따라 지도가 자동으로 이동/중심을 맞춘다
• follow 모드 on/off 토글 지원
• HiDPI/접근성(색상 대비, 크기) 지원

## ☑️ 체크리스트
• LocationPuck 클래스로 분리
• 상태 바인딩(실제/모의 위치 데이터)
• 카메라 follow 로직 구현
• 접근성(색상 대비, 크기 조절)
• 애니메이션/성능 최적화

## 🧪 TEST
• 위치 변화 시 puck 및 카메라가 즉시 반영됨
• ctest: LocationPuckTest.FollowMode
• 접근성 테스트(색상 대비, 크기 변경)
• 첫 렌더링/이동 2s 이내

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, OpenStreetMap API, GPS/위치 모듈
플랫폼: Cross-Platform (Windows 10+, macOS 10.15+, Ubuntu 20.04+)
빌드 시스템: CMake 3.16+, vcpkg/Conan
