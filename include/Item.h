#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
private:
    std::string name;
    std::string description;
    int value;
    int attackBonus;
    int defenseBonus;

public:
    Item();
    Item(
        const std::string& name,
        const std::string& description,
        int value,
        int attackBonus = 0,
        int defenseBonus = 0);

    std::string getName() const;
    std::string getDescription() const;
    int getValue() const;
    int getAttackBonus() const;
    int getDefenseBonus() const;
    bool isEquipment() const;

    void print() const;
};

#endif
