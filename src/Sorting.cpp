#include "ds/Sorting.h"
#include <string>

namespace {

void qsortItemsByName(Item* items, int low, int high) {
    if (low >= high) {
        return;
    }

    // 마지막 원소의 이름을 pivot으로 삼아 pivot보다 이름이 작거나 같은 아이템을
    // 왼쪽으로 모은다. 평균 O(n log n)이지만 pivot 선택이 나쁘면 O(n^2)이 될 수 있다.
    std::string pivot = items[high].getName();
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (items[j].getName() <= pivot) {
            ++i;
            Item tmp = items[i];
            items[i] = items[j];
            items[j] = tmp;
        }
    }

    Item tmp = items[i + 1];
    items[i + 1] = items[high];
    items[high] = tmp;
    int pivotIndex = i + 1;

    qsortItemsByName(items, low, pivotIndex - 1);
    qsortItemsByName(items, pivotIndex + 1, high);
}

void qsortItemsByNamePtr(const Item** items, int low, int high) {
    if (low >= high) {
        return;
    }

    // Inventory는 linked list Node 순서를 바꾸지 않기 위해 Item 포인터 배열만
    // 임시로 만든다. 여기서는 포인터가 가리키는 Item 이름을 기준으로 정렬한다.
    std::string pivot = items[high]->getName();
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (items[j]->getName() <= pivot) {
            ++i;
            const Item* tmp = items[i];
            items[i] = items[j];
            items[j] = tmp;
        }
    }

    const Item* tmp = items[i + 1];
    items[i + 1] = items[high];
    items[high] = tmp;
    int pivotIndex = i + 1;

    qsortItemsByNamePtr(items, low, pivotIndex - 1);
    qsortItemsByNamePtr(items, pivotIndex + 1, high);
}

void qsortPokedexByNumber(PokedexEntry* entries, int low, int high) {
    if (low >= high) {
        return;
    }

    // 도감은 포획 순서를 원본 배열에 보존하고, 출력용 복사본만 포켓몬 번호순으로 정렬한다.
    int pivotNumber = getPokemonNumber(entries[high].data->name);
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (getPokemonNumber(entries[j].data->name) <= pivotNumber) {
            ++i;
            PokedexEntry tmp = entries[i];
            entries[i] = entries[j];
            entries[j] = tmp;
        }
    }

    PokedexEntry tmp = entries[i + 1];
    entries[i + 1] = entries[high];
    entries[high] = tmp;
    int pivotIndex = i + 1;

    qsortPokedexByNumber(entries, low, pivotIndex - 1);
    qsortPokedexByNumber(entries, pivotIndex + 1, high);
}

} // namespace

void sortItemsByNameAscending(Item* items, int count) {
    if (items == nullptr || count <= 1) {
        return;
    }
    qsortItemsByName(items, 0, count - 1);
}

void sortItemsByNameAscending(const Item** items, int count) {
    if (items == nullptr || count <= 1) {
        return;
    }
    qsortItemsByNamePtr(items, 0, count - 1);
}

void sortPokedexByNumber(PokedexEntry* entries, int count) {
    if (entries == nullptr || count <= 1) {
        return;
    }
    qsortPokedexByNumber(entries, 0, count - 1);
}
