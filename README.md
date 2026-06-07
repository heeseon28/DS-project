# Dungeon Explorer

Data Structures PBL 프로젝트용 텍스트 기반 던전 탐험 게임입니다.

플레이어는 포켓몬 테마 지역을 탐험하고, 아이템을 줍고, 장비를 장착하고,
전투와 이벤트를 진행하며 점수를 기록합니다. 핵심 목표는 직접 구현한
자료구조를 실제 게임 기능에 연결하는 것입니다.

## 빌드 및 실행 방법

### 필요 환경

- C++17을 지원하는 컴파일러: `g++` 또는 `clang++`
- 기본 빌드용 `make`
- 선택 사항: CMake 빌드용 `cmake`
- 선택 사항(macOS): BGM 재생용 `afplay`

### Make로 빌드하고 실행하기

프로젝트 루트 디렉터리에서 아래 명령을 실행합니다.

```bash
make
./dungeon_explorer
```

배경음악 없이 실행하려면 아래처럼 실행합니다.

```bash
DS_DISABLE_BGM=1 ./dungeon_explorer
```

### 스모크 테스트 실행

```bash
make test
```

### 빌드 결과물 삭제

```bash
make clean
```

### CMake로 빌드하고 실행하기

CMake는 Makefile 대신 `CMakeLists.txt`를 읽어서 빌드 폴더를 만들고,
그 안에서 실행 파일과 테스트 실행 파일을 생성하는 빌드 도구입니다.
이 프로젝트에서는 `dungeon_explorer`와 `ds_smoke_tests` 두 타깃을 만듭니다.

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
./dungeon_explorer
```

CMake로 스모크 테스트를 실행하려면 아래 명령을 사용합니다.

```bash
cd build
ctest --output-on-failure
```

`cmake: command not found`가 나오면 CMake가 설치되지 않은 상태입니다.
macOS에서 Homebrew를 사용한다면 아래 명령으로 설치할 수 있습니다.

```bash
brew install cmake
```

CMake 설치가 어렵거나 시간이 없으면, 과제 요구사항의 다른 빌드 경로인
Make 방식(`make`, `make test`)을 사용하면 됩니다.

## 게임 조작키

- 방향키 또는 `WASD`: 한 칸 이동
- `L`: 현재 지역 설명 보기
- `M`: 지역 연결 그래프 보기
- `H`: 도움말
- `I`: 인벤토리 확인
- `G`: 아이템 장착 또는 해제
- `T`: 아이템 줍기
- `U`: 이동 되돌리기
- `P`: 플레이어 상태 보기
- `V`: 점수 기록 보기
- `R`: 현재 지역 아이템을 가치순으로 보기
- `E`: 대기 중인 이벤트 하나 처리
- `B`: 테스트 전투 시작
- `O`: 포켓몬 도감
- `C`: 동행 포켓몬 메뉴
- `Q`: 종료

## 자료구조 매핑

| Data structure | Implementation | Game feature |
| --- | --- | --- |
| Dynamic array | `include/ds/DynamicArray.h` | Room item and monster storage |
| Singly linked list | `include/ds/Inventory.h`, `src/Inventory.cpp` | Player inventory |
| Stack | `include/ds/Stack.h` | Movement history and undo |
| Queue | `include/ds/Queue.h` | Event and battle action processing |
| Binary search tree | `include/ds/ScoreTree.h`, `src/ScoreTree.cpp` | Score records and ranking display |
| Graph | `include/ds/DungeonGraph.h`, `src/DungeonGraph.cpp` | Room connectivity and gate transitions |
| Sorting algorithm | `include/ds/Sorting.h`, `src/Sorting.cpp` | Item and score ordering |
| Huffman coding | `include/ds/HuffmanCodec.h`, `src/HuffmanCodec.cpp` | Sprite text compression and decoding |

## 자료구조 실제 사용 위치

| 자료구조 | 구현 파일 | 실제 사용 위치 | 게임에서 보이는 기능 |
| --- | --- | --- | --- |
| Dynamic array | `include/ds/DynamicArray.h` | `include/Room.h`, `src/Room.cpp` | 각 지역의 아이템과 몬스터를 저장하고, `look` 또는 `T` 입력에서 아이템 목록/획득을 처리한다. |
| Singly linked list | `include/ds/Inventory.h`, `src/Inventory.cpp` | `include/Player.h`, `src/Player.cpp`, `src/Game.cpp` | 플레이어 가방을 관리한다. 아이템 추가, 제거, 검색, 개수 세기, 장착 표시가 linked list 기반으로 동작한다. |
| Stack | `include/ds/Stack.h` | `include/Player.h`, `src/Player.cpp`, `src/Game.cpp` | 이동 전 위치를 `moveHistory`에 push하고, `U` 입력 시 pop하여 이전 위치로 되돌아간다. |
| Queue | `include/ds/Queue.h` | `src/Game.cpp`, `include/BattleSystem.h`, `src/BattleSystem.cpp` | `E` 입력으로 이벤트를 FIFO 순서대로 처리하고, 전투에서는 플레이어/상대 행동 순서를 큐에 넣어 차례대로 실행한다. |
| Binary search tree | `include/ds/ScoreTree.h`, `src/ScoreTree.cpp` | `src/Game.cpp` | 초기 점수와 최종 점수를 삽입하고, `V` 입력 및 게임 종료 화면에서 점수를 내림차순으로 출력한다. |
| Graph | `include/ds/DungeonGraph.h`, `src/DungeonGraph.cpp` | `src/Game.cpp` | 태초마을, 1번 도로, 상록시티, 사파리존, 체육관 연결을 표현한다. 이동, 게이트 전환, `M` 입력의 연결 지도 출력에 사용된다. |
| Sorting algorithm | `include/ds/Sorting.h`, `src/Sorting.cpp` | `src/Game.cpp`, `tests/ds_smoke_tests.cpp` | `R` 입력 시 현재 지역 아이템을 가치 기준 내림차순으로 정렬해 보여준다. |
| Huffman coding | `include/ds/HuffmanCodec.h`, `src/HuffmanCodec.cpp` | `src/SpriteAssets.cpp`, `src/BattleSystem.cpp`, `src/Game.cpp`, `src/ItemFactory.cpp`, `tests/ds_smoke_tests.cpp` | 포켓몬, 오박사, 아이템 ASCII 스프라이트 원문을 압축 바이트와 빈도표로 저장하고, 출력 시 Huffman tree를 재구성해 원래 텍스트로 복원한다. |

## Additional Work: Huffman Coding Sprite Compression

스프라이트는 공백, `=`, `+`, `#`처럼 반복되는 문자가 매우 많다. 이 특성을 이용해 원본 ASCII art를 그대로 `PokemonFactory.cpp`에 저장하지 않고, Huffman coding으로 압축한 에셋으로 분리했다.

구현 흐름은 다음과 같다.

1. 원본 스프라이트 텍스트의 UTF-8 byte 빈도를 센다.
2. 빈도가 낮은 두 노드를 반복적으로 합쳐 Huffman tree를 만든다.
3. 자주 나오는 byte에는 짧은 bit code를, 드문 byte에는 긴 bit code를 부여한다.
4. 원본 스프라이트를 bitstream으로 압축하고, 8bit 단위로 packed byte 배열에 저장한다.
5. `src/SpriteAssets.cpp`에는 원본 텍스트 대신 compressed bytes, bit count, original byte count, frequency table만 남긴다.
6. 게임이 스프라이트를 출력할 때 `HuffmanCodec::decodeSprite()`가 frequency table로 tree를 다시 만들고, packed bits를 읽어 원래 ASCII sprite를 복원한다.

이 방식으로 `src/PokemonFactory.cpp`는 포켓몬 능력치와 compressed sprite pointer만 관리하고, 실제 스프라이트 데이터는 `src/SpriteAssets.cpp`에 압축 형태로 저장된다. 제출된 소스에는 이미 생성된 C++ 압축 에셋이 포함되어 있으므로, 게임 빌드와 실행에 별도 생성 도구가 필요하지 않다.

## 외부 자료 및 AI 도움 공개

- 포켓몬 ASCII 스프라이트는 외부 Pokedex 사이트의 포켓몬 정보를 참고해 프로젝트에 맞게 텍스트 형태로 정리했다.
- ASCII 스프라이트는 Huffman coding으로 압축된 `src/SpriteAssets.cpp`의 byte array와 frequency table에서 복원한다.
- BGM 파일과 스프라이트 파일은 게임 연출용 에셋이며, 자료구조 구현 자체를 대체하지 않는다.
- AI 도구는 코드 디버깅, 요구사항 점검, README 정리, 빌드 오류 분석 보조에 사용했다.
- 최종 코드는 팀이 직접 빌드와 테스트를 실행하며 검증했다. 검증 명령은 `make`, `make test`이다.
- 외부 자료를 보고서에 적을 때는 사용한 Pokedex 사이트 이름 또는 URL을 함께 기재한다.

## 프로젝트 구조

- `include/`: headers for game modules and data structures
- `include/ds/`: custom data-structure implementations
- `src/`: game logic and source files
- `tests/`: smoke tests for core data structures and mechanics
- `data/`: text assets
- `src/bgm/`: background music assets

## 제출 전 확인 명령

최종 제출 전에 아래 명령을 순서대로 실행해 확인합니다.

```bash
make clean
make
make test
```

최종 실행 파일 이름은 `dungeon_explorer`입니다.
