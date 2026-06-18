#include "Room.h"
#include <iostream>

Room::Room()
    : id(-1),
      name(""),
      description(""),
      items(),
      monsters(),
      visited(false),
      width(40),
      height(40),
      encounterRate(0),
      eventTriggerStep(-1),
      decorations(nullptr) {}

Room::Room(int id, const std::string& name, const std::string& description)
    : Room(id, name, description, 0, -1) {}

Room::Room(
    int id,
    const std::string& name,
    const std::string& description,
    int encounterRate,
    int eventTriggerStep)
    : id(id),
      name(name),
      description(description),
      items(),
      monsters(),
      visited(false),
      width(40),
      height(40),
      encounterRate(encounterRate),
      eventTriggerStep(eventTriggerStep),
      decorations(nullptr) {}

Room::Room(const Room& other)
    : id(other.id),
      name(other.name),
      description(other.description),
      items(other.items),
      monsters(other.monsters),
      visited(other.visited),
      width(other.width),
      height(other.height),
      encounterRate(other.encounterRate),
      eventTriggerStep(other.eventTriggerStep),
      decorations(nullptr) {
    copyDecorationsFrom(other);
}

Room& Room::operator=(const Room& other) {
    if (this == &other) {
        return *this;
    }

    id = other.id;
    name = other.name;
    description = other.description;
    items = other.items;
    monsters = other.monsters;
    visited = other.visited;
    width = other.width;
    height = other.height;
    encounterRate = other.encounterRate;
    eventTriggerStep = other.eventTriggerStep;

    delete[] decorations;
    decorations = nullptr;
    copyDecorationsFrom(other);
    return *this;
}

Room::~Room() {
    delete[] decorations;
}

void Room::allocateDecorations() {
    if (decorations != nullptr) {
        return;
    }

    decorations = new char[width * height];
    for (int i = 0; i < width * height; ++i) {
        decorations[i] = '\0';
    }
}

void Room::copyDecorationsFrom(const Room& other) {
    if (other.decorations == nullptr) {
        return;
    }

    allocateDecorations();
    for (int i = 0; i < width * height; ++i) {
        decorations[i] = other.decorations[i];
    }
}

// Decoration grid helpers
void Room::setDecoration(int x, int y, char c)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    allocateDecorations();
    decorations[y * width + x] = c;
}

char Room::getDecoration(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height) return '\0';
    if (decorations == nullptr) return '\0';
    return decorations[y * width + x];
}

bool Room::isBlocked(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height) return true;
    if (decorations == nullptr) return false;
    return decorations[y * width + x] != '\0';
}

int Room::getId() const {
    return id;
}

std::string Room::getName() const {
    return name;
}

std::string Room::getDescription() const {
    return description;
}

void Room::setVisited(bool value) {
    visited = value;
}

bool Room::hasBeenVisited() const {
    return visited;
}

void Room::addItem(const Item& item) {
    // 방에 아이템이 추가될 때 DynamicArray 뒤에 붙인다. 방마다 아이템 수가 달라도
    // capacity가 부족할 때만 내부 배열이 확장된다.
    items.pushBack(item);
}

bool Room::takeItem(const std::string& itemName, Item& output) {
    // 아이템 이름을 찾기 위해 현재 저장된 원소를 인덱스로 순회한다.
    // 찾은 뒤 removeAt으로 빈 칸을 메워 방 목록을 compact하게 유지한다.
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].getName() == itemName) {
            output = items[i];
            items.removeAt(i);
            return true;
        }
    }
    return false;
}

int Room::itemCount() const {
    return items.size();
}

const Item& Room::getItem(int index) const {
    // 외부 출력/정렬 코드가 i번째 아이템을 바로 가져갈 수 있게 한다.
    // DynamicArray의 operator[]를 사용하므로 접근 자체는 O(1)이다.
    return items[index];
}

void Room::addMonster(const Monster& monster) {
    // 몬스터도 아이템과 같은 이유로 DynamicArray에 저장한다. 방마다 등장 몬스터 수가
    // 다르지만, 출력과 전투 진입 시에는 번호 기반 접근이 편하다.
    monsters.pushBack(monster);
}

int Room::monsterCount() const {
    return monsters.size();
}

const Monster& Room::getMonster(int index) const {
    // i번째 몬스터를 직접 조회한다. linked list였다면 여기서 O(n) 순회가 필요했을 것이다.
    return monsters[index];
}

int Room::getWidth() const {
    return width;
}

int Room::getHeight() const {
    return height;
}

int Room::getEncounterRate() const {
    return encounterRate;
}

int Room::getEventTriggerStep() const {
    return eventTriggerStep;
}

void Room::printDescription() const {
    std::cout << "\n== " << name << " ==\n";
    std::cout << "맵 크기: " << width << " x " << height << "\n";
    std::cout << description << "\n";

    if (items.isEmpty()) {
        std::cout << "아이템: 없음\n";
    } else {
        std::cout << "아이템:\n";
        for (int i = 0; i < items.size(); ++i) {
            std::cout << "  - ";
            items[i].print();
        }
    }

    if (!monsters.isEmpty()) {
        std::cout << "몬스터:\n";
        for (int i = 0; i < monsters.size(); ++i) {
            std::cout << "  - ";
            monsters[i].print();
        }
    }
}
