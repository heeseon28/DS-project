#include "ds/Sorting.h"

void sortItemsByValueDescending(Item* items, int count) {
    if (items == nullptr || count <= 1) {
        return;
    }

    for (int i = 0; i < count - 1; ++i) {
        int bestIndex = i;
        for (int j = i + 1; j < count; ++j) {
            if (items[j].getValue() > items[bestIndex].getValue()) {
                bestIndex = j;
            }
        }

        if (bestIndex != i) {
            Item temp = items[i];
            items[i] = items[bestIndex];
            items[bestIndex] = temp;
        }
    }
}

void sortScoresDescending(ScoreRecord* records, int count) {
    if (records == nullptr || count <= 1) {
        return;
    }

    for (int i = 1; i < count; ++i) {
        ScoreRecord key = records[i];
        int j = i - 1;

        while (j >= 0 && records[j].score < key.score) {
            records[j + 1] = records[j];
            --j;
        }

        records[j + 1] = key;
    }
}
