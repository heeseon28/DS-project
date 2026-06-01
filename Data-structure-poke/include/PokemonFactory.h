#ifndef POKEMON_FACTORY_H
#define POKEMON_FACTORY_H

#include "BattleSystem.h"
#include <string>

struct PokemonData {
    const char* name;
    int maxHp;
    int attack;
    int defense;
    int speed;
    int spawnRate; // 출현 확률 가중치 (1~100, 총 스탯이 높을수록 낮음) -dc
    ElementType element;
    const char* sprite;
};

const PokemonData* findPokemonData(const std::string& name);
const PokemonData& getDefaultPokemonData();
EnemyBattle createEnemyBattle(const PokemonData& data);

#endif
