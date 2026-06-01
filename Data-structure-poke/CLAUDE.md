# Poké — Data Structure PBL 프로젝트

## 프로젝트 개요
- **과제명:** Dungeon Explorer: Guided Data Structures PBL
- **게임명:** Poké (포켓몬 테마 텍스트 어드벤처)
- **언어:** C++17
- **빌드:** `make` (프로젝트 루트 기준)
- **금지:** `std::vector`, `std::list`, `std::stack`, `std::queue`, `std::map`, `std::set` 사용 금지. 자료구조 직접 구현 필수.
- **허용:** `std::string`, iostream 계열

---

## 팀 구성 및 역할 분담

| 멤버 | 담당 | 구현 파일 |
|---|---|---|
| Heeseon Yoo | 배틀 시스템 | `include/ds/Queue.h`, `src/ScoreTree.cpp` (ScoreTree 미구현) |
| **Dongchan Shin** | 인벤토리 & 아이템 | `src/Inventory.cpp`, `src/Sorting.cpp`, `include/ItemFactory.h`, `src/ItemFactory.cpp`, `src/Game.cpp` |
| Gun Kim | 맵 & 이동 | `src/DungeonGraph.cpp`, `include/ds/Stack.h` (미구현) |

---

## ⚠️ 절대 수정 금지 파일 (Heeseon 구현)
- `include/ds/Queue.h` — enqueue/dequeue/peek 완전 구현됨
- `include/BattleSystem.h`
- `src/BattleSystem.cpp` — 전투 시스템 완전 구현됨
- `src/PokemonFactory.cpp` — 파이리 스프라이트 포함

---

## 자료구조 ↔ 게임 기능 매핑

| 자료구조 | 게임 기능 | 파일 |
|---|---|---|
| Dynamic array | Room 내 아이템/몬스터 저장 | `include/ds/DynamicArray.h` (완성) |
| Singly linked list | 플레이어 인벤토리 | `src/Inventory.cpp` ✅ |
| Stack | 이동 히스토리 & undo | `include/ds/Stack.h` ❌ 미구현 (Gun Kim) |
| Queue | 전투 액션 처리 | `include/ds/Queue.h` ✅ |
| BST | 점수 기록 | `src/ScoreTree.cpp` ❌ 미구현 (Heeseon) |
| Graph | 던전 방 연결 | `src/DungeonGraph.cpp` ❌ 미구현 (Gun Kim) |
| Sorting | 아이템/점수 정렬 | `src/Sorting.cpp` ✅ |

---

## 현재 구현 완료 (Dongchan)

### `src/Inventory.cpp`
- `addItem` — 링크드 리스트 front 삽입
- `removeItem` — 이름으로 노드 삭제
- `findItem` / `findItem const` — 이름으로 노드 탐색

### `src/Sorting.cpp`
- `sortItemsByValueDescending` — 선택 정렬, 내림차순
- `sortScoresDescending` — 삽입 정렬, 내림차순

### `include/ItemFactory.h` + `src/ItemFactory.cpp` (신규 추가)
- `PokemonFactory.cpp`와 동일한 구조로 작성
- 아이템별 ASCII 도트 스프라이트 포함
- `findItemData(name)` — 이름으로 ItemData 검색
- `createItem(name)` — Item 객체 생성
- `printItemSprite(name)` — 아이템 획득 시 스프라이트 출력

### `src/Game.cpp`
- `buildSampleWorld()` — 포켓몬 테마 맵으로 교체
- `takeItem()` — 아이템 획득 시 `printItemSprite()` 호출 추가
- `#if 0` 블록 제거하여 look/move/undo 등 전체 명령어 활성화
- `run()` — 정상 게임 모드로 복원

### `Makefile`
- `src/ItemFactory.cpp` 추가됨

---

## 맵 구조 (계획안 기반)

```
팔레트 타운 --[north]--> 1번 도로 --[north]--> 비리디안 시티
                              |
                           [east]
                              v
                          사파리 존
```

| Room | 아이템 | 몬스터 |
|---|---|---|
| 팔레트 타운 | 몬스터볼, 풀회복약 | — |
| 1번 도로 | 갑옷, 풀회복약 | — |
| 비리디안 시티 | — | 체육관 트레이너 |
| 사파리 존 | 라이플 | — |

---

## 아이템 목록 (계획안 기반)

| 아이템 | 위치 | Value | 효과 |
|---|---|---|---|
| 몬스터볼 | 팔레트 타운 | 0 | 포켓몬 포획 도구 |
| 갑옷 | 1번 도로 | 40 | 방어력 증가 |
| 라이플 | 사파리 존 | 10 | 공격력 증가 |
| 풀회복약 | 팔레트 타운, 1번 도로 | 100 | HP 전체 회복 |

---

## 게임 명령어

```
도움말(help)       명령어 목록
보기(look)         현재 방 설명
이동(move) <방향>  북/남/동/서 이동
줍기(take) <아이템> 아이템 획득
가방(inventory)    인벤토리 확인
되돌리기(undo)     이전 이동 취소 (Stack 구현 후 동작)
이벤트(event)      이벤트 처리
점수(scores)       점수 트리 (ScoreTree 구현 후 동작)
정렬(sortitems)    아이템 가치순 정렬
지도(map)          던전 지도 (DungeonGraph 구현 후 동작)
상태(status)       플레이어 상태
전투(battle) [이름] 포켓몬 전투
종료(quit)         게임 종료
```

---

## 코딩 규칙

- 새로 추가하거나 수정한 코드에는 **한글 주석 + `-dc`** 를 끝에 붙인다.
  ```cpp
  // 링크드 리스트의 맨 앞에 아이템을 삽입 -dc
  ```
- Heeseon 코드는 절대 수정하지 않는다.
- 새 파일 추가 시 `Makefile`의 `APP_SOURCES`와 `TEST_SOURCES` 양쪽에 추가한다.
- STL 컨테이너 사용 금지.

---

## 테스트

```bash
make test        # 스모크 테스트 실행
make             # 게임 빌드
./dungeon_explorer  # 게임 실행
```

현재 `testInventory`, `testSorting` 통과 확인됨.
`testStack`, `testGraph`, `testScoreTree`는 미구현 상태로 실패 예정.
