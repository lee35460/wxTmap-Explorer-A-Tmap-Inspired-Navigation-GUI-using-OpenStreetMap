# WXT-58 Description 업데이트용 템플릿

아래 내용을 VS Code Atlassian 패널의 Description 필드에 붙여넣으세요:

---

## 📋 개요
경유지 리스트 패널(표시/정렬 UI 1차) 기능을 구현하여 사용자 경험을 향상시킵니다.

## 🎯 목표
- 경유지 리스트 UI 컴포넌트 개발
- 표시/숨김 토글 기능 구현
- 드래그 앤 드롭 정렬 기능
- 반응형 패널 디자인

## 📊 성능 요구사항
- **FPS**: ≥45 (목표), 현재 165+ 달성
- **First Paint**: ≤2초
- **메모리**: 최적화된 사용량
- **CPU**: 효율적 리소스 활용

## 🔧 기술적 접근
- wxWidgets 리스트 컨트롤 확장
- 드래그 앤 드롭 이벤트 핸들링
- JSON 기반 경유지 데이터 관리
- CSS 스타일링으로 시각적 피드백

## 📁 예상 작업 파일
- `app/src/ui/WaypointListPanel.cpp`
- `app/include/ui/WaypointListPanel.h`
- `app/test/ui/WaypointListPanelTest.cpp`

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

---

위 내용을 복사해서 VS Code Atlassian 패널의 Description 필드에 붙여넣으세요.