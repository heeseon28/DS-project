#include "ds/Sorting.h"
#include <string>

namespace {

void qsortItemsByName(Item* items, int low, int high) {
    if (low >= high) return;

    std::string pivot = items[high].getName();
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (items[j].getName() <= pivot) {
            ++i;
            Item tmp = items[i]; items[i] = items[j]; items[j] = tmp;
        }
    }
    Item tmp = items[i + 1]; items[i + 1] = items[high]; items[high] = tmp;
    int pi = i + 1;

    qsortItemsByName(items, low, pi - 1);
    qsortItemsByName(items, pi + 1, high);
}

void qsortItemsByNamePtr(const Item** items, int low, int high) {
    if (low >= high) return;

    std::string pivot = items[high]->getName();
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (items[j]->getName() <= pivot) {
            ++i;
            const Item* tmp = items[i]; items[i] = items[j]; items[j] = tmp;
        }
    }
    const Item* tmp = items[i + 1]; items[i + 1] = items[high]; items[high] = tmp;
    int pi = i + 1;

    qsortItemsByNamePtr(items, low, pi - 1);
    qsortItemsByNamePtr(items, pi + 1, high);
}

void qsortPokedexByNumber(PokedexEntry* entries, int low, int high) {
    if (low >= high) return;

    int pivotNum = getPokemonNumber(entries[high].data->name);
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (getPokemonNumber(entries[j].data->name) <= pivotNum) {
            ++i;
            PokedexEntry tmp = entries[i]; entries[i] = entries[j]; entries[j] = tmp;
        }
    }
    PokedexEntry tmp = entries[i + 1]; entries[i + 1] = entries[high]; entries[high] = tmp;
    int pi = i + 1;

    qsortPokedexByNumber(entries, low, pi - 1);
    qsortPokedexByNumber(entries, pi + 1, high);
}

} // namespace

void sortItemsByNameAscending(Item* items, int count) {
    if (items == nullptr || count <= 1) return;
    qsortItemsByName(items, 0, count - 1);
}

void sortItemsByNameAscending(const Item** items, int count) {
    if (items == nullptr || count <= 1) return;
    qsortItemsByNamePtr(items, 0, count - 1);
}

void sortPokedexByNumber(PokedexEntry* entries, int count) {
    if (entries == nullptr || count <= 1) return;
    qsortPokedexByNumber(entries, 0, count - 1);
}
