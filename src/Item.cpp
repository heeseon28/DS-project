#include "Item.h"
#include <iostream>

Item::Item() : name(""), description(""), value(0) {}

Item::Item(const std::string& name, const std::string& description, int value)
    : name(name), description(description), value(value) {}

std::string Item::getName() const {
    return name;
}

std::string Item::getDescription() const {
    return description;
}

int Item::getValue() const {
    return value;
}

void Item::print() const {
    std::cout << name << " (가치 " << value << "): " << description << "\n";
}
