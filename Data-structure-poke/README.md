# 포켓몬 게임

C++17 기반의 **포켓몬 스타일 텍스트 어드벤처 / 배틀 게임**입니다.  
본 프로젝트는 단순한 게임 구현뿐만 아니라, 수업에서 배운 자료구조를 직접 구현하고 실제 게임 시스템에 적용하는 것을 목표로 합니다.

## 프로젝트 개요

플레이어는 맵을 이동하며 포켓몬과 조우하고, 배틀을 진행하며, 아이템을 사용하고, 점수를 기록할 수 있습니다.  
각 기능은 C++의 기본 문법과 직접 구현한 자료구조를 기반으로 동작합니다.

## 주요 기능

- 플레이어 이동 및 맵 탐색
- 포켓몬 심볼 인카운터
- 포켓몬 배틀 시스템
- 타입 기반 전투 로직
- 아이템 획득 및 인벤토리 관리
- 이동 기록 저장 및 되돌리기
- 이벤트 처리 시스템
- 점수 저장 및 랭킹 관리
- 정렬 알고리즘을 이용한 데이터 정렬

## 사용 자료구조

이 프로젝트는 STL 컨테이너에만 의존하지 않고, 주요 자료구조를 직접 구현하는 것을 목표로 합니다.

| 자료구조 | 사용 목적 |
|---|---|
| Dynamic Array | 포켓몬, 아이템, 맵 데이터 등 가변 데이터 저장 |
| Singly Linked List | 인벤토리 아이템 관리 |
| Stack | 플레이어 이동 기록 저장 및 undo 기능 |
| Queue | 이벤트, 메시지, 전투 로그 순차 처리 |
| Binary Search Tree | 점수 및 랭킹 데이터 관리 |
| Graph | 맵의 지역 및 연결 관계 표현 |
| Sorting Algorithms | 점수, 포켓몬 정보, 아이템 목록 정렬 |

## 프로젝트 구조

```text
.
├── include/
│   └── ds/
│       ├── DynamicArray.h
│       ├── Stack.h
│       ├── Queue.h
│       └── ...
├── src/
│   ├── main.cpp
│   ├── Inventory.cpp
│   ├── DungeonGraph.cpp
│   ├── ScoreTree.cpp
│   ├── Sorting.cpp
│   └── ...
├── tests/
│   └── ...
├── Makefile
└── README.md
```

## 빌드 및 실행 방법

프로젝트 루트 폴더에서 아래 명령어를 실행합니다.

```bash
make
```

실행 파일이 생성되면 아래와 같이 실행합니다.

```bash
./dungeon_explorer
```

프로젝트 설정에 따라 실행 파일 이름이 다를 수 있습니다.  
Makefile에서 최종 실행 파일 이름을 확인하면 됩니다.

## 테스트 실행

테스트 코드가 제공되는 경우 아래 명령어로 실행할 수 있습니다.

```bash
make test
```

## GitHub 협업 방식

이 프로젝트는 GitHub를 사용하여 팀원들이 공통으로 코드를 관리합니다.

### 기본 작업 흐름

```bash
git pull
git checkout -b feature/작업이름
```

코드를 수정한 뒤:

```bash
git status
git add .
git commit -m "feat: 작업 내용 설명"
git push -u origin feature/작업이름
```

그 후 GitHub에서 Pull Request를 생성하고, 코드 리뷰 후 `main` 브랜치에 merge합니다.

## 브랜치 규칙

| 브랜치 이름 | 용도 |
|---|---|
| main | 최종 안정 버전 |
| feature/inventory | 인벤토리 기능 구현 |
| feature/battle | 배틀 시스템 구현 |
| feature/map | 맵 이동 및 그래프 구현 |
| feature/score | 점수 및 랭킹 기능 구현 |
| fix/bug-name | 버그 수정 |

## Commit 메시지 규칙

| 태그 | 의미 |
|---|---|
| feat | 새로운 기능 추가 |
| fix | 버그 수정 |
| docs | 문서 수정 |
| refactor | 코드 구조 개선 |
| test | 테스트 코드 추가 |
| chore | 기타 작업 |

예시:

```bash
git commit -m "feat: implement inventory linked list"
git commit -m "fix: prevent crash when queue is empty"
git commit -m "docs: update README"
```

## 역할 분담 예시

| 담당자 | 담당 기능 |
|---|---|
| 팀원 1 | 인벤토리 및 아이템 시스템 |
| 팀원 2 | 배틀 시스템 |
| 팀원 3 | 맵 이동 및 그래프 |
| 팀원 4 | 점수 저장 및 정렬 |

## 개발 목표

- C++17 문법을 활용한 게임 로직 구현
- 자료구조의 실제 활용 방식 이해
- GitHub 기반 협업 경험
- 기능별 branch 작업 및 Pull Request 관리
- 코드의 가독성과 유지보수성 향상

## 주의사항

- `main` 브랜치에 직접 push하지 않습니다.
- 작업 전에는 항상 `git pull`을 먼저 실행합니다.
- 실행 파일, 빌드 폴더, 개인 설정 파일은 commit하지 않습니다.
- 하나의 commit에는 하나의 작업 내용만 담는 것을 권장합니다.
- 충돌이 발생하면 팀원과 상의한 뒤 해결합니다.

## 라이선스

본 프로젝트는 수업 과제 및 학습 목적으로 제작되었습니다.
