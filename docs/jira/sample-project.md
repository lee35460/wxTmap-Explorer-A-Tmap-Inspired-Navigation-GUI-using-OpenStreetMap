# 샘플 Jira 프로젝트 구조 (WXT)

## 프로젝트 세팅
- 키/이름: `WXT` / wxTmap Explorer
- 템플릿: Scrum (Company-managed)
- 이슈 타입: Epic, Story, Task, Bug
- 컴포넌트: UI, Map, API, Routing, Build, Docs, Release
- 라벨: osx, ubuntu, windows, ci, tech-debt, spike
- 워크플로: To Do → In Progress → Code Review → Testing → Done
- 보드 컬럼: Backlog, Selected, In Progress, Review, Test, Done (WIP 제한)
- 스토리 포인트: Fibonacci(1,2,3,5,8,13)

## 에픽(예시)
- WXT-EPIC-UI: 검색/리스트/상호작용 UX
- WXT-EPIC-MAP: Leaflet 임베드/타일/마커/폴리라인
- WXT-EPIC-API: Tmap POI 클라이언트
- WXT-EPIC-ROUTE: 플래너(Direct/Grid)/통합
- WXT-EPIC-BUILD: CMake/멀티 OS/Docker/CI
- WXT-EPIC-RELEASE: 패키징/릴리스 노트

## 스토리(샘플)
- WXT-101 검색창과 결과 리스트(UI, 3)
- WXT-120 Leaflet 임베드 및 타일 스위치(Map, 5)
- WXT-150 Tmap POI API 연동(API, 5)
- WXT-180 DummyPlanner 지오데식 경로(Routing, 3)
- WXT-200 Grid A* 라우팅 베이스라인(Routing, 8)
- WXT-230 C++↔JS 브릿지(지도 반영)(Map, 5)
- WXT-250 설정 로더(appsettings.json)(Build, 3)
- WXT-300 macOS/Ubuntu/Windows 빌드 통합(Build, 5)
- WXT-350 Dockerfile/CI 파이프라인(Build, 5)
- WXT-400 릴리스/체인지로그(Release, 3)

## 작업 분해 규칙
- Story → 세부 Task 2~5개, 각 0.5~1일
- PR 단위는 가능한 Story당 1~2개
- DoR/DoD 체크리스트 필수

## 브랜치/커밋/PR 규칙
- 브랜치: `feature/WXT-123-title`, `bugfix/WXT-456-issue`
- 커밋: `WXT-123: summary #comment ... #time 1h`
- PR 제목: `WXT-123: concise title`
- PR 본문: 수용 기준/테스트 포인트/스크린샷 첨부

## 리포팅
- Sprint Burndown, CFD, Velocity 캡처 → `docs/jira/`
- Sprint/Release Report PDF 내보내기 → `docs/jira/`

