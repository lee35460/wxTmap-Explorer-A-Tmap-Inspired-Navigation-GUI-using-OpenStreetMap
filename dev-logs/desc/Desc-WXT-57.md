
## 📋 개요
지도 상의 경로(Route Polyline) 스타일을 개선하고, 진행 구간을 시각적으로 하이라이트 처리하는 기능을 구현합니다. 사용자 경험(UX) 향상 및 경로 안내의 명확성을 목표로 합니다.

**우선순위:** Medium - 경로 안내의 시각적 명확성 및 UI/UX 개선

## 🔧 구현 및 주요 파일
app/include/MapPanel.h: Polyline 렌더링 및 하이라이트 로직
app/src/MapPanel.cpp: Polyline 렌더링 및 하이라이트 구현
app/include/render/: 렌더링 파이프라인 연동
app/src/render/: 렌더링 파이프라인 연동
app/include/ui/: UI 요소 및 상호작용
app/src/ui/: UI 요소 및 상호작용

## ✅ Acceptance Criteria (AC)
• Polyline 스타일링 로직 개선 및 적용
• 진행 구간 하이라이트 기능 구현
• 하이라이트 구간 기준 및 시각적 요소 명확화
• 테스트 케이스 및 UI/UX 피드백 반영

## ☑️ 체크리스트
• Polyline 스타일링 개선 및 하이라이트 적용
• 기존 메서드 수정/확장 및 코드 리뷰 통과
• 단위 테스트 및 성능/안정성 검증
• UI/UX 피드백 반영

## 🧪 TEST
• PolylineHighlightRenderTest: 하이라이트 구간이 정상적으로 렌더링되는지(색상/두께/구간 일치)
• PolylineHighlightUpdateTest: 진행 구간 하이라이트가 실시간으로 갱신되는지(진행 상황 반영)
• PolylineStyleSeparationTest: 스타일 변경이 기존 경로와 명확히 구분되는지
• PolylineHighlightPerformanceTest: 대용량 경로 데이터에서도 성능 저하 없는지(FPS 30 이상)
• PolylineHighlightLogicTest: 하이라이트 구간 계산 로직의 정확성(구간 인덱스, 거리 등)

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, OpenStreetMap
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan

## 🧩 참고/연관 이슈
WXT-56 Turn Banner + 진행 바
WXT-52 MapPanel DrawPolyline 연동
