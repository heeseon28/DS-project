#ifndef GAME_H
#define GAME_H

#include "GameEvent.h"
#include "Player.h"
#include "PokemonParty.h"
#include "ds/DungeonGraph.h"
#include "ds/Queue.h"
#include "ds/ScoreTree.h"
#include "ds/Stack.h"
#include <string>

class Game {
private:
    struct EncounterSymbol {
        int roomId;
        int x;
        int y;
        const char* pokemonName;
        char symbol;
        bool active;
    };

    static const int MAX_ENCOUNTERS = 8;

    DungeonGraph dungeon;
    Player player;
    PokemonParty pokemonParty; // 포획한 포켓몬 파티 -dc
    Queue<GameEvent> eventQueue;
    ScoreTree scoreTree;
    EncounterSymbol encounters[MAX_ENCOUNTERS];
    int encounterCount;
    bool running;

    void buildSampleWorld();
    void seedScores();
    void addEncounterSymbol(int roomId, int x, int y, const char* pokemonName, char symbol);
    EncounterSymbol* findEncounterAt(int roomId, int x, int y);
    const EncounterSymbol* findEncounterAt(int roomId, int x, int y) const;
    char tileAt(int roomId, int x, int y) const;

    void displayMap() const;
    void handleInput();
    void promptCommand();
    void promptTakeItem();
    void startBattleByPokemonName(const std::string& pokemonName);
    void showPokedex();
    void printHelp() const;
    void look() const;
    void move(Direction direction);
    void undoMove();
    void takeItem(const std::string& itemName);
    void processOneEvent();
    void showScores() const;
    void showSortedRoomItems() const;
    void processCommand(const std::string& line);

public:
    Game();
    void run();
};

#endif
