#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
private:
    std::string name;
    std::string description;
    int value;

public:
    Item();
    Item(const std::string& name, const std::string& description, int value);

    std::string getName() const;
    std::string getDescription() const;
    int getValue() const;

    void print() const;
};

#endif
