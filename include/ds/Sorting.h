#ifndef SORTING_H
#define SORTING_H

#include "Item.h"
#include "PokemonFactory.h"

// 출력용 배열을 사람이 읽기 좋은 순서로 재배치하는 직접 구현 정렬 함수들이다.
// STL sort 대신 quicksort 기반 helper를 사용해 아이템 이름순, 인벤토리 포인터
// 이름순, 도감 번호순 정렬을 처리한다.
void sortItemsByNameAscending(Item* items, int count);
void sortItemsByNameAscending(const Item** items, int count);
void sortPokedexByNumber(PokedexEntry* entries, int count);

#endif
