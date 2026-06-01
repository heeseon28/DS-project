cmake_minimum_required(VERSION 3.10)
project(DungeonExplorerPBL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(PROJECT_SOURCES
    src/main.cpp
    src/Game.cpp
    src/BattleSystem.cpp
    src/PokemonFactory.cpp
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

set(TEST_SOURCES
    tests/ds_smoke_tests.cpp
    src/Direction.cpp
    src/BattleSystem.cpp
    src/PokemonFactory.cpp
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
