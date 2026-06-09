#ifndef SORTING_H
#define SORTING_H

#include "Item.h"
#include "PokemonFactory.h"

void sortItemsByNameAscending(Item* items, int count);
void sortItemsByNameAscending(const Item** items, int count);
void sortPokedexByNumber(PokedexEntry* entries, int count);

#endif
