## 📋 개요
지도 렌더링 파이프라인의 구조적 확장을 위해 RenderPipeline 클래스의 스켈레톤을 도입했습니다. LonLat 구조체와 FPS 버퍼를 포함하여, 향후 성능 계측 및 타일 렌더링 최적화의 기반을 마련했습니다.

**우선순위:** Medium - 렌더링 구조 확장 및 성능 계측 기반 마련

## 🔧 구현 및 주요 파일
app/include/render/RenderPipeline.h: RenderPipeline 클래스, LonLat 구조체, FPS 버퍼
(구체적 파일/클래스는 실제 코드 기준으로 보완)

## ✅ Acceptance Criteria (AC)
• RenderPipeline 스켈레톤 구현
• LonLat 구조체 및 FPS 버퍼 포함
• 향후 성능 계측/최적화 기반 마련

## ☑️ 체크리스트
• RenderPipeline 클래스 도입
• 구조체/버퍼 설계
• 코드 리뷰 및 단위 테스트 통과

## 🧪 TEST
• RenderPipeline 생성/초기화 테스트
• FPS 버퍼 동작 확인

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
