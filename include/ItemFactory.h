#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include "Item.h"
#include "ds/HuffmanCodec.h"
#include <string>

struct ItemData {
    const char* name;
    const char* description;
    int value;
    const char* effect;
    const HuffmanEncodedSprite* sprite;
};

const ItemData* findItemData(const std::string& name);
Item createItem(const std::string& name);
void printItemSprite(const std::string& name);

#endif
