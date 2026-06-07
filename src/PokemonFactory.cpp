#include "PokemonFactory.h"
#include "SpriteAssets.h"
#include <cstdlib>

namespace
{

    // Pokemon ASCII sprites are stored as compressed Huffman assets in src/SpriteAssets.cpp.
    // POKEMON_LIST keeps only pointers to those compressed assets.

    // spawnRate: 출현 확률 가중치 (총 스탯 합 기준, 스탯이 높을수록 낮음) -dc
    // <180→80, 180~220→60, 221~270→40, 271~320→20, 321~360→10, 361~390→5, >390→1
    const PokemonData POKEMON_LIST[] = {
        {"이상해씨", 45, 24, 24, 45, 60, ElementType::Grass, &SPRITE_01},     // 합=138
        {"이상해꽃", 80, 41, 41, 80, 10, ElementType::Grass, &SPRITE_02},     // 합=242
        {"파이리", 39, 26, 21, 65, 60, ElementType::Fire, &SPRITE_03},        // 합=151
        {"리자몽", 78, 42, 39, 100, 10, ElementType::Fire, &SPRITE_04},       // 합=259
        {"꼬부기", 44, 24, 32, 43, 60, ElementType::Water, &SPRITE_05},       // 합=143
        {"거북왕", 79, 41, 50, 78, 10, ElementType::Water, &SPRITE_06},       // 합=248
        {"버터플", 60, 22, 25, 70, 40, ElementType::Grass, &SPRITE_07},       // 합=177
        {"독침붕", 65, 45, 20, 75, 40, ElementType::Grass, &SPRITE_08},       // 합=205
        {"피죤투", 83, 40, 37, 101, 10, ElementType::Grass, &SPRITE_09},      // 합=261
        {"피카츄", 35, 27, 20, 90, 60, ElementType::Electric, &SPRITE_10},    // 합=172
        {"라이츄", 60, 45, 27, 110, 20, ElementType::Electric, &SPRITE_11},   // 합=242
        {"고지", 75, 50, 55, 65, 10, ElementType::Ground, &SPRITE_12},        // 합=245
        {"삐삐", 70, 22, 24, 35, 60, ElementType::Grass, &SPRITE_13},         // 합=151
        {"나인테일", 73, 38, 37, 100, 10, ElementType::Fire, &SPRITE_14},     // 합=248
        {"푸린", 115, 22, 10, 20, 60, ElementType::Grass, &SPRITE_15},        // 합=167
        {"주뱃", 40, 22, 17, 55, 80, ElementType::Grass, &SPRITE_16},         // 합=134
        {"뚜벅초", 45, 25, 27, 30, 80, ElementType::Grass, &SPRITE_17},       // 합=127
        {"라플레시아", 75, 40, 42, 50, 20, ElementType::Grass, &SPRITE_18},   // 합=207
        {"나옹", 40, 22, 17, 90, 60, ElementType::Fire, &SPRITE_19},          // 합=169
        {"고라파덕", 50, 26, 24, 55, 60, ElementType::Water, &SPRITE_20},     // 합=155
        {"윈디", 90, 55, 40, 95, 5, ElementType::Fire, &SPRITE_21},           // 합=280
        {"강챙이", 90, 47, 47, 70, 10, ElementType::Water, &SPRITE_22},       // 합=254
        {"후딘", 55, 25, 22, 120, 40, ElementType::Electric, &SPRITE_23},     // 합=222
        {"괴력몬", 90, 65, 40, 55, 10, ElementType::Ground, &SPRITE_24},      // 합=250
        {"모다피", 50, 37, 17, 40, 60, ElementType::Grass, &SPRITE_25},       // 합=144
        {"우츠보트", 80, 52, 32, 70, 20, ElementType::Grass, &SPRITE_26},     // 합=234
        {"딱구리", 80, 60, 65, 45, 5, ElementType::Ground, &SPRITE_27},       // 합=250
        {"야도란", 95, 37, 55, 30, 20, ElementType::Water, &SPRITE_28},       // 합=217
        {"코일", 25, 17, 35, 45, 80, ElementType::Electric, &SPRITE_29},      // 합=122
        {"레어코일", 50, 30, 47, 70, 20, ElementType::Electric, &SPRITE_30},  // 합=197
        {"파오리", 52, 45, 27, 60, 40, ElementType::Water, &SPRITE_31},       // 합=184
        {"팬텀", 60, 32, 30, 110, 20, ElementType::Electric, &SPRITE_32},     // 합=232
        {"롱스톤", 35, 22, 80, 70, 20, ElementType::Ground, &SPRITE_33},      // 합=207
        {"찌리리공", 40, 15, 25, 100, 60, ElementType::Electric, &SPRITE_34}, // 합=180
        {"붐볼", 60, 25, 35, 150, 10, ElementType::Electric, &SPRITE_35},     // 합=270
        {"나시", 95, 47, 42, 55, 10, ElementType::Grass, &SPRITE_36},         // 합=239
        {"탕구리", 50, 25, 42, 35, 60, ElementType::Ground, &SPRITE_37},      // 합=152
        {"텅구리", 60, 40, 55, 45, 20, ElementType::Ground, &SPRITE_38},      // 합=200
        {"시라소몬", 50, 60, 26, 87, 20, ElementType::Ground, &SPRITE_39},    // 합=223
        {"또도가스", 65, 45, 60, 60, 10, ElementType::Fire, &SPRITE_40},      // 합=230
        {"뿔카노", 80, 42, 47, 25, 20, ElementType::Ground, &SPRITE_41},      // 합=194
        {"코뿌리", 105, 65, 60, 40, 5, ElementType::Ground, &SPRITE_42},      // 합=270
        {"덩쿠리", 65, 27, 57, 60, 20, ElementType::Grass, &SPRITE_43},       // 합=209
        {"아쿠스타", 60, 37, 42, 115, 10, ElementType::Water, &SPRITE_44},    // 합=254
        {"마임맨", 40, 22, 32, 90, 40, ElementType::Electric, &SPRITE_45},    // 합=184
        {"스라크", 70, 55, 40, 105, 5, ElementType::Grass, &SPRITE_46},       // 합=270
        {"에레브", 65, 41, 28, 105, 20, ElementType::Electric, &SPRITE_47},   // 합=239
        {"마그마", 65, 47, 28, 93, 20, ElementType::Fire, &SPRITE_48},        // 합=233
        {"잉어킹", 20, 5, 27, 80, 80, ElementType::Water, &SPRITE_49},        // 합=132
        {"갸라도스", 95, 62, 39, 81, 5, ElementType::Water, &SPRITE_50},      // 합=277
        {"라프라스", 130, 42, 40, 60, 10, ElementType::Water, &SPRITE_51},    // 합=272
        {"샤미드", 130, 32, 30, 65, 20, ElementType::Water, &SPRITE_52},      // 합=257
        {"쥬피썬더", 65, 32, 30, 130, 20, ElementType::Electric, &SPRITE_53}, // 합=257
        {"부스터", 65, 65, 30, 65, 20, ElementType::Fire, &SPRITE_54},        // 합=225
        {"잠만보", 160, 55, 32, 30, 5, ElementType::Fire, &SPRITE_55},        // 합=277
        {"썬더", 90, 45, 42, 100, 5, ElementType::Electric, &SPRITE_56},      // 합=277
        {"파이어", 90, 50, 45, 90, 5, ElementType::Fire, &SPRITE_57},         // 합=275
        {"망나뇽", 91, 67, 47, 80, 1, ElementType::Ground, &SPRITE_58},       // 합=285
        {"뮤", 100, 100, 100, 100, 1, ElementType::Electric, &SPRITE_59},     // 합=300
    };

    const int POKEMON_COUNT = sizeof(POKEMON_LIST) / sizeof(POKEMON_LIST[0]);
}

const PokemonData *findPokemonData(const std::string &name)
{
    for (int i = 0; i < POKEMON_COUNT; ++i)
    {
        if (name == POKEMON_LIST[i].name)
        {
            return &POKEMON_LIST[i];
        }
    }

    return nullptr;
}

const PokemonData &getDefaultPokemonData()
{
    return POKEMON_LIST[2]; // 파이리 (No.03)
}

const PokemonData &getRandomPokemonData()
{
    int totalWeight = 0;
    for (int i = 0; i < POKEMON_COUNT; ++i)
    {
        totalWeight += POKEMON_LIST[i].spawnRate > 0 ? POKEMON_LIST[i].spawnRate : 1;
    }

    int roll = std::rand() % totalWeight;
    for (int i = 0; i < POKEMON_COUNT; ++i)
    {
        int weight = POKEMON_LIST[i].spawnRate > 0 ? POKEMON_LIST[i].spawnRate : 1;
        if (roll < weight)
        {
            return POKEMON_LIST[i];
        }
        roll -= weight;
    }

    return getDefaultPokemonData();
}

const PokemonData &getRandomPokemonDataSafari()
{
    int totalWeight = 0;
    for (int i = 0; i < POKEMON_COUNT; ++i)
    {
        int rate = POKEMON_LIST[i].spawnRate > 0 ? POKEMON_LIST[i].spawnRate : 1;
        if (rate < 50)
            rate *= 2;
        totalWeight += rate;
    }

    int roll = std::rand() % totalWeight;
    for (int i = 0; i < POKEMON_COUNT; ++i)
    {
        int rate = POKEMON_LIST[i].spawnRate > 0 ? POKEMON_LIST[i].spawnRate : 1;
        if (rate < 50)
            rate *= 2;
        if (roll < rate)
            return POKEMON_LIST[i];
        roll -= rate;
    }

    return getDefaultPokemonData();
}

int getPokemonNumber(const std::string &name)
{
    for (int i = 0; i < POKEMON_COUNT; ++i)
    {
        if (name == POKEMON_LIST[i].name)
        {
            return i + 1;
        }
    }

    return 0;
}

EnemyBattle createEnemyBattle(const PokemonData &data)
{
    return EnemyBattle(
        data.name,
        data.maxHp,
        data.attack,
        data.defense,
        data.speed,
        data.element);
}
