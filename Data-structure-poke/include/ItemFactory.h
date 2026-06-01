#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include "Item.h"
#include <string>

// PokemonFactory와 동일한 구조로 아이템 데이터 관리 -dc
struct ItemData {
    const char* name;
    const char* description;
    int value;
    const char* effect;
    const char* sprite;
};

const ItemData* findItemData(const std::string& name);
Item createItem(const std::string& name);
void printItemSprite(const std::string& name);

#endif
