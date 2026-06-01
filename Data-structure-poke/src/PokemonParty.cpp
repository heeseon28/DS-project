#include "PokemonParty.h"
#include <iostream>

namespace
{

// 포켓몬 스탯 기반 버프 수치 계산 (타입 고정, 값은 포켓몬마다 다름) -dc
int buffValue(const CaughtPokemon& p)
{
    int val = 0;
    switch (p.element)
    {
        case ElementType::Fire:     val = p.attack  / 3; break; // 공격력 기반
        case ElementType::Water:    val = p.defense / 3; break; // 방어력 기반
        case ElementType::Grass:    val = (p.attack + p.defense) / 4; break; // 균형 기반
        case ElementType::Electric: val = p.speed   / 3; break; // 속도 기반
        case ElementType::Ground:   val = (p.attack + p.defense) / 3; break; // 합산 기반
        default: break;
    }
    return val < 1 ? 1 : val;
}

// 버프 설명 문자열 생성: 타입별 버프 종류 + 포켓몬별 수치 -dc
std::string buffDescription(const CaughtPokemon& p)
{
    int val = buffValue(p);
    switch (p.element)
    {
        case ElementType::Fire:     return "공격력 +" + std::to_string(val) + " (불꽃)";
        case ElementType::Water:    return "방어력 +" + std::to_string(val) + " (물)";
        case ElementType::Grass:    return "최대HP +" + std::to_string(val) + " (풀)";
        case ElementType::Electric: return "속도 +"   + std::to_string(val) + " (전기)";
        case ElementType::Ground:   return "방어력 +" + std::to_string(val) + " (땅)";
        default:                    return "효과 없음";
    }
}

// 정렬 기준에 따른 키값 반환 -dc
int getSortKey(const CaughtPokemon& p, PokedexSortBy sortBy)
{
    switch (sortBy)
    {
        case PokedexSortBy::StatTotal: return p.attack + p.defense;
        case PokedexSortBy::Attack:    return p.attack;
        case PokedexSortBy::Defense:   return p.defense;
        case PokedexSortBy::Number:    return p.pokedexNumber;
        default:                       return 0;
    }
}

} // namespace

// 포켓몬 이름 → 도감 번호 매핑 -dc
int getPokemonNumber(const std::string& name)
{
    if (name == "이상해씨")   return  1;
    if (name == "이상해꽃")   return  2;
    if (name == "파이리")     return  3;
    if (name == "리자몽")     return  4;
    if (name == "꼬부기")     return  5;
    if (name == "거북왕")     return  6;
    if (name == "버터플")     return  7;
    if (name == "독침붕")     return  8;
    if (name == "피죤투")     return  9;
    if (name == "피카츄")     return 10;
    if (name == "라이츄")     return 11;
    if (name == "고지")       return 12;
    if (name == "삐삐")       return 13;
    if (name == "나인테일")   return 14;
    if (name == "푸린")       return 15;
    if (name == "주뱃")       return 16;
    if (name == "뚜벅초")     return 17;
    if (name == "라플레시아") return 18;
    if (name == "나옹")       return 19;
    if (name == "고라파덕")   return 20;
    if (name == "윈디")       return 21;
    if (name == "강챙이")     return 22;
    if (name == "후딘")       return 23;
    if (name == "괴력몬")     return 24;
    if (name == "모다피")     return 25;
    if (name == "우츠보트")   return 26;
    if (name == "딱구리")     return 27;
    if (name == "야도란")     return 28;
    if (name == "코일")       return 29;
    if (name == "레어코일")   return 30;
    if (name == "파오리")     return 31;
    if (name == "팬텀")       return 32;
    if (name == "롱스톤")     return 33;
    if (name == "찌리리공")   return 34;
    if (name == "붐볼")       return 35;
    if (name == "나시")       return 36;
    if (name == "탕구리")     return 37;
    if (name == "텅구리")     return 38;
    if (name == "시라소몬")   return 39;
    if (name == "또도가스")   return 40;
    if (name == "뿔카노")     return 41;
    if (name == "코뿌리")     return 42;
    if (name == "덩쿠리")     return 43;
    if (name == "아쿠스타")   return 44;
    if (name == "마임맨")     return 45;
    if (name == "스라크")     return 46;
    if (name == "에레브")     return 47;
    if (name == "마그마")     return 48;
    if (name == "잉어킹")     return 49;
    if (name == "갸라도스")   return 50;
    if (name == "라프라스")   return 51;
    if (name == "샤미드")     return 52;
    if (name == "쥬피썬더")   return 53;
    if (name == "부스터")     return 54;
    if (name == "잠만보")     return 55;
    if (name == "썬더")       return 56;
    if (name == "파이어")     return 57;
    if (name == "망나뇽")     return 58;
    if (name == "뮤")         return 59;
    return 0;
}

PokemonParty::PokemonParty() : head(nullptr), count(0), selectedName("") {}

PokemonParty::~PokemonParty()
{
    clear();
}

// 파티 맨 앞에 포켓몬 추가 -dc
void PokemonParty::add(const CaughtPokemon& pokemon)
{
    Node* newNode = new Node(pokemon);
    newNode->next = head;
    head = newNode;
    ++count;
}

bool PokemonParty::has(const std::string& name) const
{
    return find(name) != nullptr;
}

const CaughtPokemon* PokemonParty::find(const std::string& name) const
{
    const Node* current = head;
    while (current != nullptr)
    {
        if (current->pokemon.name == name)
        {
            return &current->pokemon;
        }
        current = current->next;
    }
    return nullptr;
}

int PokemonParty::size() const
{
    return count;
}

bool PokemonParty::isEmpty() const
{
    return count == 0;
}

void PokemonParty::print() const
{
    if (head == nullptr)
    {
        std::cout << "파티에 포켓몬이 없습니다.\n";
        return;
    }

    std::cout << "파티 (" << count << "마리):\n";
    const Node* current = head;
    while (current != nullptr)
    {
        const CaughtPokemon& p = current->pokemon;
        std::cout << "  No." << p.pokedexNumber
                  << " " << p.name
                  << "  공격:" << p.attack
                  << "  방어:" << p.defense
                  << "  | " << buffDescription(p);
        if (p.name == selectedName)
        {
            std::cout << "  ★ 동행 중";
        }
        std::cout << "\n";
        current = current->next;
    }
}

// 동행 포켓몬 1마리의 버프만 출력 -dc
void PokemonParty::printBuffs() const
{
    if (selectedName.empty())
    {
        return;
    }

    const Node* current = head;
    while (current != nullptr)
    {
        if (current->pokemon.name == selectedName)
        {
            std::cout << "\n[동행 포켓몬 버프] " << selectedName
                      << " → " << buffDescription(current->pokemon) << "\n\n";
            return;
        }
        current = current->next;
    }
}

// 정렬 기준으로 선택 정렬 후 out 배열에 저장 -dc
void PokemonParty::getSorted(PokedexSortBy sortBy, CaughtPokemon* out) const
{
    // 링크드 리스트를 배열로 복사 -dc
    int i = 0;
    const Node* current = head;
    while (current != nullptr)
    {
        out[i++] = current->pokemon;
        current = current->next;
    }

    // 선택 정렬: 번호는 오름차순, 나머지는 내림차순 -dc
    for (int a = 0; a < count - 1; ++a)
    {
        int targetIdx = a;
        for (int b = a + 1; b < count; ++b)
        {
            int keyB      = getSortKey(out[b], sortBy);
            int keyTarget = getSortKey(out[targetIdx], sortBy);

            bool better = (sortBy == PokedexSortBy::Number)
                          ? (keyB < keyTarget)   // 번호: 오름차순
                          : (keyB > keyTarget);  // 나머지: 내림차순

            if (better)
            {
                targetIdx = b;
            }
        }
        if (targetIdx != a)
        {
            CaughtPokemon tmp  = out[a];
            out[a]             = out[targetIdx];
            out[targetIdx]     = tmp;
        }
    }
}

void PokemonParty::setSelected(const std::string& name)
{
    selectedName = name;
}

const std::string& PokemonParty::getSelected() const
{
    return selectedName;
}

bool PokemonParty::hasSelected() const
{
    return !selectedName.empty();
}

void PokemonParty::clear()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    count = 0;
    selectedName = "";
}
