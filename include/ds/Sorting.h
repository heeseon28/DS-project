#ifndef SORTING_H
#define SORTING_H

#include "Item.h"
#include "ds/ScoreTree.h"

void sortItemsByValueDescending(Item* items, int count);
void sortScoresDescending(ScoreRecord* records, int count);

#endif
