#include "Item.h"
#include <iostream>

Item::Item()
    : name(""),
      description(""),
      value(0),
      attackBonus(0),
      defenseBonus(0) {}

Item::Item(
    const std::string& name,
    const std::string& description,
    int value,
    int attackBonus,
    int defenseBonus)
    : name(name),
      description(description),
      value(value),
      attackBonus(attackBonus),
      defenseBonus(defenseBonus) {}

std::string Item::getName() const {
    return name;
}

std::string Item::getDescription() const {
    return description;
}

int Item::getValue() const {
    return value;
}

int Item::getAttackBonus() const {
    return attackBonus;
}

int Item::getDefenseBonus() const {
    return defenseBonus;
}

bool Item::isEquipment() const {
    return attackBonus > 0 || defenseBonus > 0;
}

void Item::print() const {
    std::cout << name << " (가치 " << value << "): " << description << "\n";
    if (isEquipment()) {
        std::cout << "    장비 효과:";
        if (attackBonus > 0) {
            std::cout << " 공격 +" << attackBonus;
        }
        if (defenseBonus > 0) {
            std::cout << " 방어 +" << defenseBonus;
        }
        std::cout << "\n";
    }
}
