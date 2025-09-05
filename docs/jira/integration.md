# Jira Cloud 연동 가이드 (GitHub/Bitbucket/GitLab)

본 문서는 Jira Cloud와 VCS를 연동하고 Smart Commit/자동화로 이슈 추적을 강화하는 절차를 정리합니다.

---

## 전제 조건
- Jira Cloud 사이트/프로젝트 보유(예: 키 `WXT`)
- 리포지토리: GitHub 또는 Bitbucket(권장), GitLab
- 프로젝트 권한: 프로젝트 관리자 + 앱 설치 권한

---

## 1) VCS 연동

### GitHub ↔ Jira Cloud
- 설치: GitHub Marketplace에서 `GitHub for Jira` 앱 설치(또는 Jira의 Apps → Find new apps)
- 연결: 조직/리포지토리 선택 후 연결 승인
- 확인: Jira 이슈 상세 화면의 Development 패널에 커밋/브랜치/PR이 표시되는지 확인

참고: https://support.atlassian.com/jira-cloud-administration/docs/integrate-with-github/ 

### Bitbucket ↔ Jira Cloud
- 동일 조직이라면 자동 연동 수준이 가장 높음
- Jira 프로젝트 Settings → Integrations → DVCS accounts에서 워크스페이스 연결

### GitLab ↔ Jira Cloud
- 앱: `GitLab for Jira Cloud` 설치 후 Workspace 연결
- 주의: 링크/트레이스는 원활하나 Smart Commit 일부 기능은 제한될 수 있음(플랜/버전에 따라 다름)

---

## 2) 이슈 키 규칙
- 형식: `<PROJECT-KEY>-<번호>` 예) `WXT-150`
- 브랜치: `feature/WXT-150-poi-client`
- 커밋: `WXT-150: implement Tmap POI client`
- PR 제목: `WXT-150 WXT-151: POI + debounce`

이슈 키가 포함되면 Jira가 자동으로 참조를 연결합니다.

---

## 3) Smart Commit (Bitbucket/GitHub)
Smart Commit은 커밋 메시지에서 Jira 이슈 갱신을 실행합니다.

지원 주요 명령(권장):
- `#comment <내용>`: 이슈에 댓글 추가
- `#time <기간>`: 작업 시간 기록(예: `1h 30m`)
- `#transition '<전이명>'`: 워크플로 전이 실행(예: `'In Progress'`, `'Done'`)

예시:
```
WXT-150: implement POI client #comment add retry/backoff #time 2h #transition 'In Progress'
```
복수 이슈:
```
WXT-120 WXT-300: build fixes #comment unify cmake flags
```

참고: https://support.atlassian.com/jira-software-cloud/docs/process-issues-with-smart-commits/ 

---

## 4) Jira Automation 레시피

### Rule A: PR 생성 시 In Progress 전이
- Trigger: Pull request created (Git Provider)
- Condition: Issue type in (Story, Task)
- Action: Transition issue to `In Progress`
- Action: Add comment `PR created: {{pullRequest.url}}`

### Rule B: PR 병합 시 Done 전이 + 릴리스 노트
- Trigger: Pull request merged
- Condition: Status not in (Done, Released)
- Action: Transition issue to `Done`
- Action: Add comment `Merged via {{pullRequest.url}}`
- (선택) Action: Update field Fix versions → `MVP-0.1.0`

### Rule C: 커밋 메시지 라벨 자동 부여
- Trigger: Commit created
- If: `{{commit.message}}` contains `feat:` → Add label `feat`
- Else-if: contains `fix:` → Add label `fix`

### Rule D: 정체 이슈 자동 핑
- Trigger: Scheduled (매일 09:00)
- JQL: `project = WXT AND status = 'In Progress' AND updated < -3d`
- Action: Comment `Stale: no updates for 3 days`

---

## 5) 개발자 워크플로 체크리스트
- [ ] 브랜치/커밋/PR에 이슈 키 포함
- [ ] PR 생성 시 이슈 자동 전이 확인
- [ ] 병합 후 이슈 Done/릴리스 버전 갱신 확인
- [ ] Smart Commit으로 시간/코멘트 기록

---

## 6) 문제 해결
- Development 패널이 비어있음: 앱 연결/권한, 이슈 키 포함 여부 확인
- Smart Commit 미동작: 공급자 지원 범위/권한, 전이 이름 정확성 확인
- GitLab 제한: 가능한 경우 GitHub/Bitbucket으로 전환 검토

---

## 7) 보안/공개 가이드
- 리포 공개 시 Jira URL/개인 이메일/내부 링크는 숨김 처리
- 스크린샷은 `docs/jira/` 폴더에 저장하고 캡션으로 맥락 설명
- JSON 템플릿: `docs/jira/automation/` 폴더의 예제 규칙을 가져오기하여 시작점으로 사용하세요.
