# Dungeon Explorer Student Starter

This is the C++ starter project for the guided data structures PBL assignment.

The program compiles, but key data-structure operations are intentionally incomplete. Your team must replace TODO stubs with correct implementations and then customize the game content.

## Build with Make

```bash
make
./dungeon_explorer
```

## Build with CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
./dungeon_explorer
```

## Run Smoke Tests

```bash
make test
```

Some tests are expected to fail before your implementations are complete.

## Important Rule

Do not use STL containers such as `std::vector`, `std::list`, `std::stack`, `std::queue`, `std::map`, `std::set`, or `std::unordered_map` for the required data structures.

`std::string` and standard input/output headers are allowed.

## Main TODO Files

- `src/Inventory.cpp`
- `include/ds/Stack.h`
- `include/ds/Queue.h`
- `src/DungeonGraph.cpp`
- `src/ScoreTree.cpp`
- `src/Sorting.cpp`

## Suggested Implementation Order

1. Inventory linked list
2. Stack
3. Queue
4. Dungeon graph movement
5. Score tree
6. Sorting
7. Game customization and report

## Useful Commands Inside the Game

```text
help
look
move north
move east
take key
inventory
undo
event
scores
map
quit
```
