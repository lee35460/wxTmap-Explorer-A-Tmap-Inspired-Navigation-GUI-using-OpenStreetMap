# Jira Cloud API 연동 설정 가이드

> 🌐 **VS Code Atlassian 패널과 완전 동기화되는 스마트 시스템**

## 🚀 새로운 기능

### ✨ **Jira Cloud API 직접 연동**
- VS Code Atlassian 패널에서 작업한 내용 실시간 반영
- CSV 파일 의존성 제거
- 항상 최신 정보 보장

### 🤖 **GitHub Copilot 통합 Description 제안**
- Description 누락 시 AI 기반 제안 생성
- 프로젝트 컨텍스트 반영한 맞춤형 템플릿
- 터미널에서 바로 확인 가능

## 🔧 설정 방법

### 1. **Jira Cloud API Token 생성**
1. https://id.atlassian.com/manage-profile/security/api-tokens 접속
2. "Create API token" 클릭
3. Token 이름 입력 (예: "wxTmap-vscode-integration")
4. 생성된 토큰 복사 (한 번만 표시됨!)

### 2. **환경변수 설정**
```bash
# ~/.bashrc 또는 ~/.zshrc에 추가
export JIRA_DOMAIN="your-domain.atlassian.net"     # 본인의 Jira 도메인
export JIRA_EMAIL="your-email@example.com"         # Jira 계정 이메일
export JIRA_API_TOKEN="your-generated-api-token"   # 위에서 생성한 토큰
export JIRA_PROJECT_KEY="WXT"                      # 프로젝트 키
```

### 3. **터미널 재시작**
```bash
source ~/.bashrc  # 또는 source ~/.zshrc
```

## 🎯 사용 방법

### **기본 사용법**
```bash
# Jira 이슈 상태 확인
./scripts/jira-cloud-api-logger.sh WXT-55 check

# Description 누락 시 AI 제안 받기
./scripts/jira-cloud-api-logger.sh WXT-55 suggest

# 완전한 로그 생성
./scripts/jira-cloud-api-logger.sh WXT-55 create

# 정보 업데이트 후 로그 재생성
./scripts/jira-cloud-api-logger.sh WXT-55 update
```

## 🔄 새로운 워크플로우

### **시나리오 1: Description이 완전한 경우**
```bash
$ ./scripts/jira-cloud-api-logger.sh WXT-55 check
✅ Jira Cloud API 연결 성공!
📋 WXT-55 정보:
   Summary: MapOverlay HUD (속도/제한속도/거리·ETA)
   Status: Backlog
   Assignee: kyung-min LEE
   Description: ✅ 있음

$ ./scripts/jira-cloud-api-logger.sh WXT-55 create
✅ Jira Cloud 기반 이슈 로그 생성 완료!
```

### **시나리오 2: Description이 누락된 경우 (핵심!)**
```bash
$ ./scripts/jira-cloud-api-logger.sh WXT-55 suggest
✅ Jira Cloud API 연결 성공!
📋 WXT-55 정보:
   Summary: MapOverlay HUD (속도/제한속도/거리·ETA)  
   Status: Backlog
   Assignee: kyung-min LEE
   Description: ❌ 누락

⚠️ Description이 누락되었습니다!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🤖 GitHub Copilot Description 제안 (AI 기반)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📋 이슈 정보
• 이슈 키: WXT-55
• Summary: MapOverlay HUD (속도/제한속도/거리·ETA)
• 프로젝트: wxTmap Explorer

🎯 제안된 Description 내용:

## 📋 개요
MapOverlay HUD (속도/제한속도/거리·ETA) 기능을 구현하여 사용자 경험을 향상시킵니다.

## 🎯 목표
- MapOverlay HUD 컴포넌트 구현
- 실시간 속도, 제한속도, 거리, ETA 표시
- 성능 최적화: FPS ≥45 유지
- 반응형 UI 디자인

## 📊 성능 요구사항
- **FPS**: ≥45 (목표), 현재 165+ 달성
- **First Paint**: ≤2초
- **메모리**: 최적화된 사용량
- **CPU**: 효율적 리소스 활용

## 🔧 기술적 접근
- wxWidgets Panel 기반 HUD 컴포넌트
- JavaScript 브리지를 통한 실시간 데이터 연동
- CSS3 스타일링 적용
- 이벤트 드리븐 업데이트 메커니즘

## 📁 예상 작업 파일
app/src/ui/MapOverlayHud.cpp, app/include/ui/MapOverlayHud.h

## ✅ Acceptance Criteria
- [ ] 기능 구현 완료 및 동작 확인
- [ ] 성능 요구사항 충족 (FPS ≥45)
- [ ] 단위 테스트 작성 및 통과
- [ ] 코드 리뷰 완료
- [ ] 문서 업데이트

## 🧪 테스트 계획
- 기능 테스트: 핵심 기능 동작 검증  
- 성능 테스트: FPS 및 메모리 사용량 측정
- UI 테스트: 다양한 화면 크기에서 테스트
- 통합 테스트: 다른 컴포넌트와의 연동 확인

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

💡 사용 방법:
1. 위 내용을 복사하여 Jira Cloud의 Description 필드에 붙여넣기
2. VS Code Atlassian 패널에서 해당 이슈 열어서 편집
3. 프로젝트 상황에 맞게 내용 수정 및 보완
4. 저장 후 다시 스크립트 실행: ./scripts/jira-cloud-api-logger.sh WXT-55 update

⚠️  알림: 이는 AI 제안사항입니다. 실제 요구사항에 맞게 수정해주세요.

🤖 AI 기반 Description 제안이 생성되었습니다!

Jira Cloud에 Description을 업데이트했나요? (y/N):
```

### **시나리오 3: API 인증 실패 시 (자동 백업)**
```bash
$ ./scripts/jira-cloud-api-logger.sh WXT-55 check
⚠️ Jira Cloud API 인증 실패

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔧 Jira Cloud API 설정 필요
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. Jira Cloud API Token 생성:
   https://id.atlassian.com/manage-profile/security/api-tokens

2. 환경변수 설정 (~/.bashrc 또는 ~/.zshrc):
   export JIRA_DOMAIN="your-domain.atlassian.net"
   export JIRA_EMAIL="your-email@example.com"
   export JIRA_API_TOKEN="your-api-token"
   export JIRA_PROJECT_KEY="WXT"

3. 터미널 재시작 후 다시 실행:
   ./scripts/jira-cloud-api-logger.sh WXT-55 check

⚠️  현재는 CSV 파일 기반 처리로 대체됩니다.

🔄 CSV 파일 기반 처리로 대체합니다...
# (기존 adaptive-issue-logger.sh 실행됨)
```

## 🎯 주요 장점

### ✅ **실시간 동기화**
- VS Code Atlassian 패널 작업 즉시 반영
- CSV 내보내기/동기화 불필요
- 항상 최신 Jira 정보 보장

### 🤖 **AI 기반 지원**  
- Description 누락 시 GitHub Copilot 제안
- 프로젝트 컨텍스트 반영한 맞춤형 템플릿
- 터미널에서 바로 확인 및 복사 가능

### 🔄 **자동 백업**
- API 인증 실패 시 기존 CSV 방식 자동 전환
- 설정 가이드 자동 표시
- 무중단 개발 환경 보장

### 📊 **향상된 정보**
- Jira Status, Assignee 등 추가 정보
- 실시간 업데이트 가능
- 더 정확한 프로젝트 추적

## 🚀 권장 사용법

1. **최초 설정**: API Token 생성 및 환경변수 설정
2. **일상 사용**: `./scripts/jira-cloud-api-logger.sh WXT-XX suggest`로 시작
3. **VS Code 연동**: Atlassian 패널에서 Description 업데이트
4. **로그 생성**: `./scripts/jira-cloud-api-logger.sh WXT-XX create`로 완성

**결과**: VS Code Atlassian 패널과 완전 동기화된 스마트한 개발 로그 시스템! 🎉