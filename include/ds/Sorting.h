#ifndef SORTING_H
#define SORTING_H

#include "Item.h"
#include "ds/ScoreTree.h"

// 출력용 배열을 사람이 읽기 좋은 순서로 재배치하는 정렬 함수들이다.
// 원본 자료구조를 STL sort로 대체하지 않고, 작은 게임 목록에 맞는 직접 구현
// 정렬을 사용한다. 방 아이템은 값이 높은 순서로, 점수 기록은 점수가 높은 순서로 보여준다.
void sortItemsByValueDescending(Item* items, int count);
void sortScoresDescending(ScoreRecord* records, int count);

#endif
