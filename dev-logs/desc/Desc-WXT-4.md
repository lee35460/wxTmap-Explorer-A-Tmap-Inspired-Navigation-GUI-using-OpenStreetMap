## 📋 개요
wxCallAfter 기반의 UI dispatcher(`UiDispatcher` 클래스)를 도입하여, 비동기 작업 결과를 UI 스레드에 안전하게 마샬링할 수 있도록 개선했습니다. 백프레셔와 메트릭 연동을 통해 대용량 이벤트 처리와 성능 계측도 지원합니다.

**우선순위:** Medium - UI 스레드 안정성 및 비동기 처리 핵심

## 🔧 구현 및 주요 파일
app/include/ui/UiDispatcher.h: UiDispatcher 클래스 정의 및 주요 메서드
app/test/ui/UiDispatcherTest.cpp: 단위 테스트
app/CMakeLists.txt: 빌드 연동

## ✅ Acceptance Criteria (AC)
• wxCallAfter 기반 UI dispatcher 구현
• 비동기 작업 결과의 UI 안전 마샬링
• 백프레셔 및 메트릭 연동
• 단위 테스트 및 성능 기준 충족

## ☑️ 체크리스트
• UiDispatcher 클래스로 분리 및 적용
• 기존 메서드 수정/확장
• 단위 테스트 및 코드 리뷰 통과
• 성능/안정성 검증

## 🧪 TEST
• UiDispatcherTest: 비동기 → UI 마샬링 정상 동작 (성공률: OK/FAIL, 메트릭 검증 및 post 성공)
• 메트릭 계측 및 백프레셔 테스트 (메트릭 수집 횟수: 1회 이상, 백프레셔 동작 확인)
• RenderPipelineTest: 렌더 파이프라인 정상 동작 (평균 FPS: 30fps 이상)
• RenderPipelineMetricsTest: 렌더 파이프라인 메트릭 검증 (CSV 출력 FPS: 30fps 이상)
• 커밋 6회, 코드 리뷰 완료 (수동 확인)

## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+
플랫폼: Cross-Platform (Windows/macOS/Ubuntu)
빌드 시스템: CMake, vcpkg/Conan