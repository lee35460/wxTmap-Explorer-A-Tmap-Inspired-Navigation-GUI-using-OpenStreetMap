# WXT-57: Route Polyline 스타일 + 진행 구간 하이라이트

## 🟣 개요
- MapPanel에서 경로(Polyline) 스타일을 개선하고, 진행 구간을 실시간으로 하이라이트하는 기능을 구현했습니다.
- 사용자 경험을 높이기 위해 경로의 시각적 구분과 진행 상태 표시를 강화했습니다.
- §3.1(MapPanel) 스펙을 충족하며, UI/렌더링 파이프라인과의 연동을 최적화했습니다.

## 🛠️ 구현 및 주요 파일
- **핵심 구현**: MapPanel이 경로 폴리라인을 스타일링하고, 현재 진행 구간을 하이라이트하여 실시간으로 갱신
- **주요 변경 파일**:
  - `app/include/MapPanel.h`, `app/src/MapPanel.cpp`: 폴리라인 스타일 및 하이라이트 로직 구현
  - `app/include/render/RenderPipeline.h`, `app/src/render/RenderPipeline.cpp`: 렌더 파이프라인 연동
  - `app/CMakeLists.txt`, `.github/workflows/ci.yml` 등 빌드/테스트 및 워크플로우 보강
- **신규 클래스**: MapPanel, RenderPipeline
- **주요 메서드**: MapPanel 내 폴리라인 렌더 및 진행 상태 갱신 함수

## ✅ AC (Acceptance Criteria)
- [x] Route polyline 기본 스타일 적용
- [x] 진행 구간 하이라이트 구현

## ☑️ 체크리스트
- [x] UI에서 경로 진행 상태가 실시간 반영되는지 확인
- [x] 스타일/색상 등 시각적 요소가 스펙과 일치하는지 검증
- [x] 렌더 파이프라인과의 연동 정상 동작
- [x] 단위 테스트 및 성능 기준 충족

## 🧪 TEST
- 단위 테스트: `app/test/test_renderpipeline.cpp`, `app/test/ui/MapOverlayHudTest.cpp` 등에서 폴리라인 렌더 및 하이라이트 동작 검증
- CI: GitHub Actions에서 xvfb 환경 포함, 자동 테스트 및 빌드 통과
- 성능: FPS 및 렌더 지연 기준 충족

## 🏗️ 기술스택 및 구조
- **C++ (wxWidgets, OpenStreetMap 데이터 활용)**
- **Mermaid 시퀀스/클래스 다이어그램**: MapPanel과 RenderPipeline의 상호작용 및 구조 명확화
- **자동화**: GitHub Actions, 커밋 메시지 훅 등

## 📈 메트릭
- 총 20개 파일 수정, 2개 신규 클래스, 2개 신규 메서드, 14회 커밋
- 빌드/테스트/코드리뷰 모두 완료

## 🔗 참고
- 상위 이슈: WXT-2 (MapPanel 초기화)
- 개발 가이드: wxTmap Explorer 개발 가이드 PDF §3.1
- 코드 위치: `app/include/`, `app/src/`
- 브랜치: feature/WXT-57-route-polyline
