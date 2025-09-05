# Jira Automation JSON 템플릿 모음

이 디렉터리는 Jira Cloud Automation 규칙을 빠르게 생성하기 위한 템플릿입니다. Atlassian의 내보내기 JSON 스키마는 워크스페이스/버전에 따라 달라질 수 있으므로, 아래 템플릿은 "가져오기 후 매핑/수정"이 필요합니다. 규칙의 핵심 로직(JQL/전이/코멘트/라벨)은 그대로 활용 가능합니다.

사용 방법(권장)
- Jira → Project settings → Automation → Three dots → Import rule → JSON 업로드
- 필드 매핑/전이 이름/프로젝트 스코프를 확인 후 저장
- Git 공급자(Webhook) 연결이 필요한 규칙은 연동(Apps) 설정을 완료한 뒤 매핑

파일 목록
- `pr-created-in-progress.json` — PR 생성 시 이슈를 In Progress 전이
- `pr-merged-done.json` — PR 병합 시 Done 전이 및 코멘트
- `commit-labeling.json` — 커밋 메시지에 따라 feat/fix 라벨 부여
- `stale-issues-ping.json` — 3일 이상 업데이트 없는 In Progress 이슈 핑
- `fixversion-on-merge.json` — PR 병합 시 Fix versions 업데이트

전제
- 프로젝트 키 예시는 `WXT`입니다. 필요 시 바꾸세요.
- 전이 이름은 워크플로에 맞게 수정하세요(예: `In Progress`, `Done`).
- Smart Commit/개발자 패널 동작을 위해 VCS 연동이 선행되어야 합니다.

