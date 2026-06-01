#include "ds/Inventory.h"
#include <iostream>

Inventory::Inventory() : head(nullptr), count(0) {}

Inventory::~Inventory() {
    clear();
}

void Inventory::addItem(const Item& item) {
    Node* newNode = new Node(item);
    newNode->next = head;
    head = newNode;
    ++count;
}

bool Inventory::removeItem(const std::string& itemName) {
    Node* current = head;
    Node* previous = nullptr;

    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            if (previous == nullptr) {
                head = current->next;
            } else {
                previous->next = current->next;
            }

            delete current;
            --count;
            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

Item* Inventory::findItem(const std::string& itemName) {
    Node* current = head;
    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            return &current->item;
        }
        current = current->next;
    }

    return nullptr;
}

const Item* Inventory::findItem(const std::string& itemName) const {
    Node* current = head;
    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            return &current->item;
        }
        current = current->next;
    }

    return nullptr;
}

int Inventory::countItem(const std::string& itemName) const {
    int matches = 0;
    Node* current = head;
    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            ++matches;
        }
        current = current->next;
    }

    return matches;
}

bool Inventory::isEmpty() const {
    return count == 0;
}

int Inventory::size() const {
    return count;
}

void Inventory::clear() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    count = 0;
}

void Inventory::print() const {
    if (head == nullptr) {
        std::cout << "가방이 비어 있습니다.\n";
        return;
    }

    std::cout << "가방 (" << count << "개):\n";
    Node* current = head;
    while (current != nullptr) {
        std::cout << "  - ";
        current->item.print();
        current = current->next;
    }
}
