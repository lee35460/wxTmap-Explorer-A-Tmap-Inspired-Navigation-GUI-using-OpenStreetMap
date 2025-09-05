# Jira 케이스 스터디: wxTmap Explorer

본 문서는 Jira를 활용해 wxTmap Explorer 프로젝트를 계획·추적·리포팅한 과정을 정리한 케이스 스터디입니다. 포트폴리오에서 프로세스 역량(기획→실행→지표→회고)을 입증하는 것을 목표로 합니다.

---

## 1) 프로젝트 개요
- 목표: Tmap 유사 내비게이션 GUI의 MVP 구현 및 배포
- 범위: 검색(POI), 지도/마커, 경로 플래너(Dummy/Grid), 설정/빌드 파이프라인
- 성공지표(KPI):
  - MVP 기능 달성률 100%
  - 스프린트 목표 달성률 ≥ 80%
  - 스토리 사이클 타임 중앙값 ≤ 3일

---

## 2) Jira 프로젝트 설정
- 템플릿: Scrum (Company-managed)
- 이슈 타입: Epic, Story, Task, Bug
- 워크플로: To Do → In Progress → Code Review → Testing → Done
- 보드 컬럼: Backlog, Selected, In Progress, Review, Test, Done (WIP 제한 적용)
- 필드/라벨: Components(UI, Map, API, Routing, Build), Labels(osx, ubuntu, windows, docs)
- 추정: Story Points(Poker), Task는 시간 추적(Time Spent)
- 자동화:
  - PR 열리면 In Progress로 전환
  - PR 병합 시 Done으로 전환
- 통합: GitHub/Bitbucket 연결, Smart Commits(`WXT-123 #time 2h #comment ... #done`)

---

## 3) 백로그 구조(예시)
- Epics: UI/UX, Map/Leaflet, TmapClient, Routing, Build/CI, Release
- Stories(예):
  - WXT-101 검색창과 결과 리스트 UI
  - WXT-120 Leaflet 임베드 및 타일 스위치(OSM/VWorld)
  - WXT-150 Tmap POI 검색 API 연동
  - WXT-180 DummyPlanner 지오데식 경로
  - WXT-200 Grid A* 라우팅 베이스라인
  - WXT-250 설정 로더(appsettings.json)
  - WXT-300 macOS/Ubuntu/Windows 빌드 통합

Definition of Ready(DoR) 체크리스트:
- 수용 기준(AC) 명확, 외부 의존성/키/계정 준비, 테스트 기준 합의

Definition of Done(DoD) 체크리스트:
- 구현/리뷰/테스트 완료, 문서/샘플 업데이트, CI 통과, 배포 산출물 생성

백로그 캡처: `docs/jira/backlog.png`

---

## 4) 스프린트 계획
- 기간: 2주(10 영업일)
- 용량: 팀/개인 가능 시간 기반(휴가/회의 반영)
- 벨로시티 기준: 최근 2~3 스프린트 평균 스토리 포인트
- 스프린트 목표(예): MVP 경로 계산 end-to-end 데모 성공
- 스프린트 백로그 스냅샷: `docs/jira/board.png`

---

## 5) 실행 및 운영
- 데일리 스탠드업: 목표/진행/차단요인 공유(10분)
- WIP 제한: In Progress 3개 이하 유지 → 병목 방지
- 코드 리뷰: 최소 1명 승인 + CI 통과
- 브랜치/커밋 규칙:
  - 브랜치: `feature/WXT-123-brief-title`
  - 커밋: `WXT-123: summary` (Smart Commits 권장)

---

## 6) 지표와 리포트
- Sprint Burndown: `docs/jira/burndown.png` → 잔여 SP의 이상치 감지
- Cumulative Flow Diagram(CFD): `docs/jira/cfd.png` → 컬럼별 체류/병목 파악
- Velocity Chart: `docs/jira/velocity.png` → 예측 가능한 계획 수립
- Sprint Report(PDF): `docs/jira/sprint-report.pdf`
- Release Notes(PDF): `docs/jira/release-notes.pdf`

지표 해석 가이드(예시):
- Burndown이 평평하면 스코프 재평가 또는 리스크 처리 필요
- CFD에서 In Progress가 누적되면 WIP 초과 또는 리뷰 병목
- Velocity 변동이 크면 추정 보정/스코프 슬라이싱 재점검

---

## 7) 추적성(Traceability)
- 이슈 ↔ 브랜치/PR/커밋 매핑: Jira 키(WXT-###)를 모든 아티팩트에 포함
- 릴리스 태그: `v0.1.0` ↔ Jira Release/Version `MVP-0.1.0`
- 코드 레퍼런스: PR에 스크린샷/동영상 첨부(데모 링크)

---

## 8) 회고 템플릿
- 잘된 점: …
- 개선할 점: …
- 액션 아이템: 담당/기한/성공 기준
- 리스크 레지스터 업데이트 및 다음 스프린트 반영

회고 캡처: `docs/jira/retrospective.png`(선택)

---

## 9) 익명화 및 보안
- API 키/이메일/내부 링크 가림(블러), 고객/사내 고유 정보 제거
- 공개 링크 대신 PDF/이미지로 대체, 캡션에 맥락 설명

---

## 10) 캡처 체크리스트
- [ ] Backlog(필터/라벨 포함) → `backlog.png`
- [ ] Board(스프린트 중반) → `board.png`
- [ ] Sprint Burndown → `burndown.png`
- [ ] CFD → `cfd.png`
- [ ] Velocity → `velocity.png`
- [ ] Roadmap → `roadmap.png`
- [ ] Sprint Report(PDF) → `sprint-report.pdf`
- [ ] Release Notes(PDF) → `release-notes.pdf`

---

## 11) 포트폴리오 반영 가이드
- 1페이지 요약(문제→계획→실행→성과)과 아티팩트 링크를 함께 제시
- 각 지표 이미지 아래에 1~2줄 해석/의미 작성
- 코드/데모 영상과 연결하여 “문제 해결 서사”로 정리

---

부록) 샘플 Smart Commit
```
WXT-150 #time 3h #comment Implement POI client with retries #done
```

