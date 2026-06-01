#ifndef POKEMON_PARTY_H
#define POKEMON_PARTY_H

#include "BattleSystem.h"
#include <string>

// 포획된 포켓몬 데이터 -dc
struct CaughtPokemon {
    std::string name;
    ElementType element;
    int attack;
    int defense;
    int speed;         // 전기 타입 버프에 사용 -dc
    int pokedexNumber; // 포켓몬 도감 번호 -dc
};

// 도감 정렬 기준 -dc
enum class PokedexSortBy {
    StatTotal,  // 개체값 (공격+방어)
    Attack,     // 공격력
    Defense,    // 방어력
    Number      // 포켓몬 번호 (오름차순)
};

// 포켓몬 이름 → 도감 번호 매핑 (PokemonFactory 수정 불가로 별도 관리) -dc
int getPokemonNumber(const std::string& name);

// 포획한 포켓몬을 보관하는 링크드 리스트 -dc
class PokemonParty {
private:
    struct Node {
        CaughtPokemon pokemon;
        Node* next;
        Node(const CaughtPokemon& p) : pokemon(p), next(nullptr) {}
    };

    Node* head;
    int count;
    std::string selectedName; // 현재 동행 중인 포켓몬 이름 -dc

    PokemonParty(const PokemonParty&) = delete;
    PokemonParty& operator=(const PokemonParty&) = delete;

public:
    PokemonParty();
    ~PokemonParty();

    void add(const CaughtPokemon& pokemon);
    bool has(const std::string& name) const;
    const CaughtPokemon* find(const std::string& name) const;
    int size() const;
    bool isEmpty() const;
    void print() const;
    void printBuffs() const;     // 동행 포켓몬 버프만 출력 -dc
    void clear();

    // 정렬 기준을 받아 out 배열에 내림차순(번호는 오름차순) 정렬 후 저장 -dc
    void getSorted(PokedexSortBy sortBy, CaughtPokemon* out) const;
    void setSelected(const std::string& name);
    const std::string& getSelected() const;
    bool hasSelected() const;
};

#endif
