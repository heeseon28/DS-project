#ifndef ROOM_H
#define ROOM_H

#include "Item.h"
#include "Monster.h"
#include "ds/DynamicArray.h"
#include <string>

class Room {
private:
    int id;
    std::string name;
    std::string description;
    DynamicArray<Item> items;
    DynamicArray<Monster> monsters;
    bool visited;
    int width;
    int height;
    int encounterRate;
    int eventTriggerStep;

public:
    Room();
    Room(int id, const std::string& name, const std::string& description);
    Room(
        int id,
        const std::string& name,
        const std::string& description,
        int encounterRate,
        int eventTriggerStep);

    int getId() const;
    std::string getName() const;
    std::string getDescription() const;

    void setVisited(bool value);
    bool hasBeenVisited() const;

    void addItem(const Item& item);
    bool takeItem(const std::string& itemName, Item& output);
    int itemCount() const;
    const Item& getItem(int index) const;

    void addMonster(const Monster& monster);
    int monsterCount() const;
    const Monster& getMonster(int index) const;

    int getWidth() const;
    int getHeight() const;
    int getEncounterRate() const;
    int getEventTriggerStep() const;

    void printDescription() const;
};

#endif
