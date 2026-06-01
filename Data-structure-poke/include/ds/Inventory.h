#ifndef INVENTORY_H
#define INVENTORY_H

#include "Item.h"
#include <string>

class Inventory {
private:
    struct Node {
        Item item;
        Node* next;

        Node(const Item& item) : item(item), next(nullptr) {}
    };

    Node* head;
    int count;

    // Copying is disabled for this beginner project to avoid accidental shallow copies.
    Inventory(const Inventory& other) = delete;
    Inventory& operator=(const Inventory& other) = delete;

public:
    Inventory();
    ~Inventory();

    void addItem(const Item& item);
    bool removeItem(const std::string& itemName);
    Item* findItem(const std::string& itemName);
    const Item* findItem(const std::string& itemName) const;
    bool isEmpty() const;
    int size() const;
    void clear();
    void print() const;
};

#endif
