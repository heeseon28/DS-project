CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude

APP_SOURCES = src/main.cpp src/Game.cpp src/Direction.cpp src/Player.cpp src/BattleSystem.cpp src/PokemonFactory.cpp src/Room.cpp src/Item.cpp src/Monster.cpp src/Inventory.cpp src/DungeonGraph.cpp src/ScoreTree.cpp src/Sorting.cpp
TEST_SOURCES = tests/ds_smoke_tests.cpp src/Direction.cpp src/Player.cpp src/BattleSystem.cpp src/PokemonFactory.cpp src/Room.cpp src/Item.cpp src/Monster.cpp src/Inventory.cpp src/DungeonGraph.cpp src/ScoreTree.cpp src/Sorting.cpp

all: dungeon_explorer

dungeon_explorer: $(APP_SOURCES)
	$(CXX) $(CXXFLAGS) $(APP_SOURCES) -o dungeon_explorer

ds_smoke_tests: $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o ds_smoke_tests

test: ds_smoke_tests
	./ds_smoke_tests

clean:
	rm -f dungeon_explorer ds_smoke_tests
