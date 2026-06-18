#include "ds/Sorting.h"

void sortItemsByValueDescending(Item* items, int count) {
    if (items == nullptr || count <= 1) {
        return;
    }

    // 선택 정렬 방식이다. 아직 정렬되지 않은 구간에서 가장 value가 큰 아이템을
    // 찾아 현재 위치와 바꾼다. O(n^2)이지만 방 안 아이템처럼 작은 목록을
    // 출력용으로 정렬하는 데 쓰이므로 구현이 단순하다는 장점이 있다.
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

    // 삽입 정렬 방식이다. 앞쪽은 이미 내림차순으로 정렬되어 있다고 보고,
    // 현재 key보다 점수가 낮은 기록을 오른쪽으로 밀어 key가 들어갈 자리를 만든다.
    // 작은 랭킹 배열에서는 코드가 짧고 안정적으로 동작한다.
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
