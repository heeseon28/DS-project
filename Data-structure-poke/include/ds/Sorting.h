#ifndef SORTING_H
#define SORTING_H

#include "Item.h"
#include "ds/ScoreTree.h"

// TODO(Student): Implement at least one comparison-based sorting algorithm.
// You may use selection sort, insertion sort, or another algorithm approved by your instructor.

void sortItemsByValueDescending(Item* items, int count);
void sortScoresDescending(ScoreRecord* records, int count);

#endif
