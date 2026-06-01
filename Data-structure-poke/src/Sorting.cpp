#include "ds/Sorting.h"

void sortItemsByValueDescending(Item* items, int count) {
    // 선택 정렬: 매 회차마다 최대값 위치를 찾아 앞으로 교환 -dc
    for (int i = 0; i < count - 1; ++i) {
        int maxIdx = i;
        for (int j = i + 1; j < count; ++j) {
            if (items[j].getValue() > items[maxIdx].getValue()) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            Item tmp = items[i];
            items[i] = items[maxIdx];
            items[maxIdx] = tmp;
        }
    }
}

void sortScoresDescending(ScoreRecord* records, int count) {
    // 삽입 정렬: 현재 원소를 앞쪽 정렬된 구간의 알맞은 위치에 삽입 -dc
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
