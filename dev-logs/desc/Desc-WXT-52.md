## 📋 개요
MapPanel의 DrawPolyline 호출에 RenderPipeline을 연동하고, beginFrame/endFrame 주기로 ack 로깅을 추가했습니다. 이를 통해 렌더링 파이프라인의 성능 및 동작을 실시간으로 추적할 수 있습니다.

**우선순위:** Medium - 렌더링 연동 및 실시간 성능/동작 추적

## 🔧 구현 및 주요 파일
app/include/MapPanel.h, app/src/MapPanel.cpp: DrawPolyline 연동
app/include/render/RenderPipeline.h: RenderPipeline beginFrame/endFrame

## ✅ Acceptance Criteria (AC)
• MapPanel DrawPolyline에 RenderPipeline 연동
• beginFrame/endFrame 기반 ack 로깅 구현
• 실시간 성능/동작 추적 가능

## ☑️ 체크리스트
• DrawPolyline 연동 및 테스트
• ack 로깅 정상 동작
• 코드 리뷰 및 단위 테스트 통과

## 🧪 TEST
• DrawPolyline 호출 시 RenderPipeline 동작 확인
• ack 로깅 및 성능 계측 테스트

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
