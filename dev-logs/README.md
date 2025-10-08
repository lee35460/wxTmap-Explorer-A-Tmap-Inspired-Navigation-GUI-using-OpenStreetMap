# wxTmap Explorer 개발 로그 시스템

## 📋 개요
이 폴더는 wxTmap Explorer 프로젝트의 개발 과정과 결과를 체계적으로 기록하여, 향후 개발 가이드 PDF 문서 업데이트 시 참고자료로 활용하기 위한 시스템입니다.

## 📁 파일 구조
```
dev-logs/
├── README.md                    # 이 파일
├── development-guidelines.md    # 초기 지침사항 (PDF + Jira CSV 기반)
├── daily-logs/                  # 일별 개발 기록
├── issue-logs/                  # 이슈별 상세 기록 (WXT-xx)
├── technical-decisions/         # 기술적 결정 사항들
├── performance-metrics/         # 성능 측정 결과들
├── code-reviews/               # 코드 리뷰 기록
└── milestones/                 # 마일스톤별 정리
```

## 🎯 목적
1. **개발 과정 추적**: 모든 개발 활동의 상세 기록
2. **기술적 결정 문서화**: 왜 특정 기술이나 방법을 선택했는지 기록
3. **성능 및 품질 추적**: 메트릭과 테스트 결과 누적
4. **PDF 문서 업데이트 지원**: 구조화된 데이터로 문서 갱신 용이
5. **지식 보존**: 프로젝트 노하우와 경험 축적

## 📝 기록 규칙
- 모든 기록은 Markdown 형식으로 작성
- 날짜와 시간 정보 필수 포함
- Jira 이슈 키(WXT-xx) 연동
- SpecRef 참조 포함
- 성능 수치와 스크린샷 첨부

## 🔗 연동 시스템
- Jira 이슈 추적과 연동
- Git 커밋과 매핑
- 성능 메트릭 CSV와 연동
- CI/CD 결과 반영