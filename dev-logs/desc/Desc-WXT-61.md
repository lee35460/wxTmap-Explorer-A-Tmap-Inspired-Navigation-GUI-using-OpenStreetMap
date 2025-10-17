# WXT-61: HUD Real-time State Management Store

## 개요
네비게이션 HUD(Head-Up Display)의 실시간 상태 정보를 효율적으로 관리하기 위한 중앙화된 상태 관리 스토어를 구현합니다. WXT-55 MapOverlayHud와 WXT-59 LocationPuck 기반으로, 속도, 거리, ETA, 위치 정보 등의 실시간 데이터를 통합 관리하여 UI 컴포넌트 간 일관된 상태 동기화를 제공합니다.

**우선순위:** 높음 - HUD 컴포넌트들의 상태 일관성과 성능 최적화를 위한 핵심 기능

## 🔧 구현 및 주요 파일
app/include/domain/state/HudStateStore.h, app/src/domain/state/HudStateStore.cpp: 중앙화된 HUD 상태 관리 스토어
app/include/domain/state/NavigationState.h: 네비게이션 관련 상태 데이터 구조
app/include/domain/state/LocationState.h: 위치 관련 상태 데이터 확장
app/include/presentation/components/StateSubscriber.h: 상태 구독자 인터페이스
app/test/domain/state/HudStateStoreTest.cpp: 상태 관리 스토어 단위 테스트

## ✅ Acceptance Criteria (AC)
• 실시간 네비게이션 데이터(속도, 거리, ETA, 위치)를 중앙에서 관리한다
• Observer 패턴을 통해 UI 컴포넌트들이 상태 변화를 구독할 수 있다
• 상태 업데이트 시 필요한 컴포넌트들만 선택적으로 갱신된다
• 스레드 안전성을 보장하여 멀티스레드 환경에서 안정적으로 동작한다
• 메모리 효율성을 위해 불필요한 상태 복사를 최소화한다
• 상태 변경 히스토리 추적 및 디버깅 지원 기능을 제공한다
• UI 갱신 빈도 제어를 통해 60fps 렌더링 성능을 유지한다

## ☑️ 체크리스트
• HudStateStore 클래스 구현 (중앙화된 상태 관리)
• NavigationState 데이터 구조 정의 (속도, 거리, ETA, 방향)
• LocationState 확장 (정확도, 신호 강도, 움직임 감지)
• StateSubscriber 인터페이스 (Observer 패턴 구현)
• 스레드 안전성 보장 (mutex, atomic 연산 활용)
• 선택적 업데이트 메커니즘 (dirty flag, subscription filtering)
• 상태 변경 이벤트 시스템 (타입별 구독, 콜백 관리)
• 성능 최적화 (메모리 풀, 업데이트 배칭)

## 🧪 TEST
• StateStoreInitTest: 상태 스토어 초기화 및 기본값 설정
• StateUpdateTest: 개별 상태 업데이트 (속도, 위치, 네비게이션 데이터)
• SubscriptionManagementTest: 구독자 등록/해제 및 생명주기 관리
• SelectiveUpdateTest: 변경된 상태만 해당 구독자에게 알림
• ThreadSafetyTest: 멀티스레드 환경에서 상태 안전성 검증
• PerformanceTest: 대량 상태 업데이트 성능 (1000회/초 < 10ms)
• MemoryEfficiencyTest: 메모리 사용량 최적화 (불필요한 복사 방지)
• SubscriberLifecycleTest: 구독자 생명주기 안전성 (dangling pointer 방지)
• StateHistoryTest: 상태 변경 히스토리 추적 및 디버깅 지원
• EventFilteringTest: 상태 타입별 필터링 및 선택적 구독
• NavigationDataIntegrityTest: 네비게이션 데이터 일관성 검증
• LocationDataAccuracyTest: 위치 데이터 정확성 및 신뢰성 검증
• BulkUpdateStressTest: 대량 동시 업데이트 처리 (100개 구독자)
• StateSerializationTest: 상태 직렬화/역직렬화 (디버깅/로깅용)
• ComponentSyncTest: UI 컴포넌트 간 상태 동기화 검증

## 🚀 기술 스택 및 환경
기술스택: C++17, Observer Pattern, Thread-safe Collections, Smart Pointers
동시성: std::mutex, std::atomic, std::shared_ptr, std::weak_ptr
상태관리: Immutable State, Event Sourcing, Subscription Management
플랫폼: Cross-Platform (Windows 10+, macOS 10.15+, Ubuntu 20.04+)
빌드 시스템: CMake 3.16+, GoogleTest/GoogleMock
성능: Memory Pooling, Update Batching, Lazy Evaluation

## 🧩 참고/연관 이슈
• WXT-55: Map Overlay HUD (HUD 컴포넌트의 상태 소비자)
• WXT-59: Location Puck + Camera Follow (위치 상태 제공자)
• WXT-60: Camera Initialization Presets (카메라 상태 연동)
• WXT-56: Navigation Progress Bar (네비게이션 상태 소비자)