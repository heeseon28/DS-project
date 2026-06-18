#ifndef POKEMON_FACTORY_H
#define POKEMON_FACTORY_H

#include "BattleSystem.h"
#include "ds/HuffmanCodec.h"
#include <string>

struct PokemonData {
    const char* name;
    int maxHp;
    int attack;
    int defense;
    int speed;
    int spawnRate; // 출현 확률 가중치 (1~100, 총 스탯이 높을수록 낮음) -dc
    ElementType element;
    const HuffmanEncodedSprite* sprite;
};

struct PokedexEntry {
    const PokemonData* data;
    int caughtOrder;
};

const PokemonData* findPokemonData(const std::string& name);
const PokemonData& getDefaultPokemonData();
const PokemonData& getRandomPokemonData();
const PokemonData& getRandomPokemonDataSafari();
int getPokemonNumber(const std::string& name);
EnemyBattle createEnemyBattle(const PokemonData& data);

#endif
