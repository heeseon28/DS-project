cmake_minimum_required(VERSION 3.10)
project(DungeonExplorerPBL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Main game executable sources.
set(PROJECT_SOURCES
    src/main.cpp
    src/Game.cpp
    src/BattleSystem.cpp
    src/BgmPlayer.cpp
    src/PokemonFactory.cpp
    src/ItemFactory.cpp
    src/Direction.cpp
    src/Player.cpp
    src/Room.cpp
    src/Item.cpp
    src/Monster.cpp
    src/Inventory.cpp
    src/DungeonGraph.cpp
    src/ScoreTree.cpp
    src/Sorting.cpp
)

add_executable(dungeon_explorer ${PROJECT_SOURCES})
target_include_directories(dungeon_explorer PRIVATE include)

# Smoke-test executable sources. This target excludes src/main.cpp because
# tests/ds_smoke_tests.cpp provides its own main function.
set(TEST_SOURCES
    tests/ds_smoke_tests.cpp
    src/Direction.cpp
    src/BattleSystem.cpp
    src/BgmPlayer.cpp
    src/PokemonFactory.cpp
    src/ItemFactory.cpp
    src/Player.cpp
    src/Room.cpp
    src/Item.cpp
    src/Monster.cpp
    src/Inventory.cpp
    src/DungeonGraph.cpp
    src/ScoreTree.cpp
    src/Sorting.cpp
)

add_executable(ds_smoke_tests ${TEST_SOURCES})
target_include_directories(ds_smoke_tests PRIVATE include)

enable_testing()
add_test(NAME ds_smoke_tests COMMAND ds_smoke_tests)
