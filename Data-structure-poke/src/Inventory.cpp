#include "ds/Inventory.h"
#include <iostream>

Inventory::Inventory() : head(nullptr), count(0) {}

Inventory::~Inventory() {
    clear();
}

void Inventory::addItem(const Item& item) {
    // 새 노드를 생성하여 리스트 맨 앞에 삽입 -dc
    Node* newNode = new Node(item);
    newNode->next = head;
    head = newNode;
    ++count;
}

bool Inventory::removeItem(const std::string& itemName) {
    // 이름이 일치하는 노드를 찾아 삭제, head 교체와 중간 삭제 모두 처리 -dc
    Node* current = head;
    Node* prev = nullptr;

    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            --count;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

Item* Inventory::findItem(const std::string& itemName) {
    // 이름이 일치하는 노드의 item 포인터 반환 -dc
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
    // const 버전: 읽기 전용 탐색 -dc
    const Node* current = head;
    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            return &current->item;
        }
        current = current->next;
    }
    return nullptr;
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
