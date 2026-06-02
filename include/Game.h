#ifndef GAME_H
#define GAME_H

#include "GameEvent.h"
#include "Item.h"
#include "Player.h"
#include "ds/DungeonGraph.h"
#include "ds/Queue.h"
#include "ds/ScoreTree.h"
#include "ds/Stack.h"
#include <string>
#include <vector>

struct PokemonData;

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

    struct ItemSymbol {
        int roomId;
        int x;
        int y;
        std::string itemName;
        bool active;
    };

    static const int MAX_ENCOUNTERS = 8;
    static const int MAX_ITEM_SYMBOLS = 16;
    static const int MAX_POKEDEX = 128;

    struct PokedexEntry {
        const PokemonData* data;
        int caughtOrder;
    };

    struct GateRecord {
        int roomId;
        int x;
        int y;
        int targetRoomId;
        int targetX;
        int targetY;
    };

    DungeonGraph dungeon;
    Player player;
    Queue<GameEvent> eventQueue;
    ScoreTree scoreTree;
    EncounterSymbol encounters[MAX_ENCOUNTERS];
    ItemSymbol itemSymbols[MAX_ITEM_SYMBOLS];
    PokedexEntry pokedex[MAX_POKEDEX];
    std::vector<GateRecord> gateRecords;
    int encounterCount;
    int itemSymbolCount;
    int pokedexCount;
    int companionIndex;
    bool running;

    void buildSampleWorld();
    void seedScores();
    void addRoomItem(int roomId, int x, int y, const Item& item);
    void addEncounterSymbol(int roomId, int x, int y, const char* pokemonName, char symbol);
    ItemSymbol* findItemAt(int roomId, int x, int y);
    const ItemSymbol* findItemAt(int roomId, int x, int y) const;
    ItemSymbol* findItemSymbol(int roomId, const std::string& itemName);
    EncounterSymbol* findEncounterAt(int roomId, int x, int y);
    const EncounterSymbol* findEncounterAt(int roomId, int x, int y) const;
    char tileAt(int roomId, int x, int y) const;

    void displayMap() const;
    void handleInput();
    void printHelp() const;
    void showPokedex() const;
    void showCompanionMenu();
    void printPokedexEntries(const PokedexEntry* entries, int count) const;
    void recordCaughtPokemon(const PokemonData* data);
    const PokemonData* getCompanionPokemon() const;
    void look() const;
    void move(Direction direction);
    void undoMove();
    void removeInventoryItems(const std::string& itemName, int count);
    void takeItem(const std::string& itemName);
    void promptTakeItem();
    void promptEquipItem();
    bool startPokemonBattle(const std::string& pokemonName);
    void startEncounterBattle(EncounterSymbol& encounter);
    void processOneEvent();
    void showScores() const;
    void showSortedRoomItems() const;

public:
    Game();
    void run();
};

#endif
