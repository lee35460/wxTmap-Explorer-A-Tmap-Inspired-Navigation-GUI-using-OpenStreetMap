# WXT-60: Camera Initialization Presets & Animated Transition Entry

## 개요
지도 초기 로딩 시 사용자에게 부드러운 시각적 경험을 제공하기 위해 카메라 초기화 프리셋과 애니메이션 진입 효과를 구현합니다. WXT-59의 LocationPuck과 CameraController 기반으로, 지도 첫 로딩부터 사용자 위치 추적까지의 완전한 애니메이션 전환을 제공합니다.

**우선순위:** 높음 - WXT-59 완료 후 완전한 사용자 경험 제공을 위한 필수 기능

## 🔧 구현 및 주요 파일
app/include/ui/CameraPreset.h, app/src/ui/CameraPreset.cpp: 카메라 초기 설정 및 프리셋 관리
app/include/ui/TransitionAnimator.h, app/src/ui/TransitionAnimator.cpp: 부드러운 전환 애니메이션 엔진
app/include/MapPanel.h, app/src/MapPanel.cpp: CameraPreset 및 TransitionAnimator 통합
app/include/Types.h: CameraPreset, AnimationState 데이터 구조 확장
app/test/ui/CameraPresetTest.cpp: 카메라 프리셋 및 애니메이션 단위 테스트

## ✅ Acceptance Criteria (AC)
• 지도 초기 로딩 시 기본 카메라 위치(서울/사용자 위치)로 부드럽게 진입한다
• 다양한 카메라 프리셋(도시 전체, 근거리, 원거리) 지원 및 전환 가능하다
• 사용자 위치 감지 시 현재 뷰에서 사용자 위치로 부드럽게 애니메이션 전환한다
• 줌/팬 애니메이션이 자연스럽고 일정한 속도로 진행된다 (easing 적용)
• 애니메이션 중 사용자 인터랙션 시 자연스럽게 중단/재시작 가능하다
• 다양한 화면 크기/해상도에서 일관된 카메라 동작 보장한다
• 성능 최적화: 60fps 애니메이션 및 메모리 효율성 유지

## ☑️ 체크리스트
• CameraPreset 클래스로 분리 (프리셋 관리 + 좌표 계산)
• TransitionAnimator 클래스로 분리 (애니메이션 엔진 + 인터폴레이션)
• 기본 카메라 프리셋 정의 (서울 중심, 사용자 위치, 확대/축소 레벨)
• 부드러운 전환 애니메이션 (위치, 줌, 회전 동시 보간)
• 사용자 인터랙션 처리 (애니메이션 중단/재시작)
• 다중 해상도 지원 (모바일, 데스크톱, HiDPI)
• 성능 최적화 (60fps 애니메이션, GPU 가속 활용)

## 🧪 TEST
• CameraViewportTest: 카메라 뷰포트 생성 및 유효성 검증
• AnimationStateTest: 애니메이션 상태 관리 (시작/진행/완료)
• EasingFunctionTest: 이징 함수 정확성 (Linear/EaseIn/EaseOut)
• DefaultPresetTest: 기본 프리셋 생성 (서울 중심, 적절한 줌 레벨)
• UserLocationPresetTest: 사용자 위치 프리셋 정확성 (오차 0.001도 이내)
• RoutePresetTest: 경로 프리셋 계산 (모든 포인트 포함, 적절한 줌)
• CustomPresetTest: 커스텀 프리셋 관리 (저장/로드/삭제)
• BasicTransitionTest: 기본 전환 애니메이션 (시작 성공, 진행률 유효)
• EasingFunctionTest: 다양한 이징 함수 지원 (Linear, EaseInOut, EaseOut)
• CompletionDetectionTest: 애니메이션 완료 감지 (시작/정지 상태 확인)
• ControllerInitTest: 카메라 컨트롤러 초기화 (프리셋 적용, 유효한 뷰포트)
• AnimationStateTest: 애니메이션 상태 추적 (시작/진행 상태, 진행률)
• PresetPerformanceTest: 프리셋 생성 성능 (1000회 < 10ms)
• AnimationPerformanceTest: 애니메이션 계산 성능 (1000회 < 50ms)
• BulkCreationStressTest: 대량 프리셋 생성 (100개 < 1초)


## 🚀 기술 스택 및 환경
기술스택: C++17, wxWidgets 3.2+, OpenStreetMap API, OpenGL/WebGL
플랫폼: Cross-Platform (Windows 10+, macOS 10.15+, Ubuntu 20.04+)
빌드 시스템: CMake 3.16+, GoogleTest/GoogleMock
애니메이션: Easing Functions, GPU 가속 렌더링 지원

## 🧩 참고/연관 이슈
• WXT-59: Location Puck + Camera Follow (CameraController 기반 확장)
• WXT-57: Route Polyline (경로 표시와 카메라 프리셋 연동)
• WXT-58: Waypoint List Panel (경유지와 카메라 뷰 동기화)