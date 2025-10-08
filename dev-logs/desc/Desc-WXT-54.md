## 📋 개요
RenderPipeline에 대한 유닛 및 서버 테스트를 추가하여, FPS 45 이상, 첫 렌더링 2초 이내 등 성능 기준을 자동 검증합니다. GTest 기반 경량 하네스도 도입하여, 품질과 신뢰성을 강화했습니다.

**우선순위:** Medium - 성능 기준 자동 검증 및 테스트 신뢰성 강화

## 🔧 구현 및 주요 파일
app/include/render/RenderPipeline.h, app/src/render/RenderPipeline.cpp: RenderPipeline 테스트 대상
(구체적 테스트/하네스 파일은 실제 코드 기준으로 보완)

## ✅ Acceptance Criteria (AC)
• RenderPipeline 유닛/서버 테스트 구현
• FPS ≥ 45, First Paint ≤ 2s 자동 검증
• GTest 기반 경량 하네스 도입

## ☑️ 체크리스트
• 테스트 케이스 설계 및 구현
• 성능 기준 자동 검증
• 코드 리뷰 및 테스트 통과

## 🧪 TEST
• GTest 기반 유닛/서버 테스트 실행
• FPS/First Paint 기준 자동 검증

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, GTest
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
