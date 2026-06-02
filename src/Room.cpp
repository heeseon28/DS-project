#include "Room.h"
#include <iostream>
#include <algorithm>

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
      eventTriggerStep(-1) {}

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
      eventTriggerStep(eventTriggerStep) {}

// Decoration grid helpers
void Room::setDecoration(int x, int y, char c)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    if (decorations.empty()) decorations.assign(width * height, '\0');
    decorations[y * width + x] = c;
}

char Room::getDecoration(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height) return '\0';
    if (decorations.empty()) return '\0';
    return decorations[y * width + x];
}

bool Room::isBlocked(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height) return true;
    if (decorations.empty()) return false;
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
    items.pushBack(item);
}

bool Room::takeItem(const std::string& itemName, Item& output) {
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
    return items[index];
}

void Room::addMonster(const Monster& monster) {
    monsters.pushBack(monster);
}

int Room::monsterCount() const {
    return monsters.size();
}

const Monster& Room::getMonster(int index) const {
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
