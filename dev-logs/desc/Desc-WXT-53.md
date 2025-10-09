## 📋 개요
RenderPipeline의 FPS 등 주요 성능 메트릭을 CSV로 내보내는 Exporter를 구현했습니다. 서버/유닛 테스트와 연동하여, 성능 기준(FPS 등) 자동 검증 및 장기적 품질 관리를 지원합니다.

**우선순위:** Medium - 성능 계측 및 품질 관리 자동화

## 🔧 구현 및 주요 파일
app/include/render/RenderPipeline.h, app/src/render/RenderPipeline.cpp: 메트릭 수집 및 Exporter 구현
(구체적 파일/클래스는 실제 코드 기준으로 보완)

## ✅ Acceptance Criteria (AC)
• RenderPipeline의 FPS 등 메트릭을 CSV로 내보내는 Exporter 구현
• 서버/유닛 테스트와 연동
• 성능 기준 자동 검증

## ☑️ 체크리스트
• Exporter 구현 및 연동
• 단위/서버 테스트 통과
• 코드 리뷰 및 성능 기준 검증

## 🧪 TEST
• Exporter 동작 확인 (CSV 파일 생성)
• FPS 등 메트릭 자동 검증

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan
