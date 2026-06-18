#ifndef INVENTORY_H
#define INVENTORY_H

#include "Item.h"
#include <string>

// Inventory는 singly linked list로 구현한 플레이어 가방이다.
// 탐험 중 아이템 획득/사용/장비 변경으로 개수가 자주 바뀌므로 고정 배열보다
// 필요한 만큼 Node를 만들고 지우는 구조가 더 적합하다. 새 아이템은 head에
// 붙기 때문에 addItem은 O(1)이고, 이름 검색/삭제/count는 앞에서부터 순회하므로 O(n)이다.
class Inventory {
private:
    struct Node {
        Item item;
        Node* next;

        Node(const Item& item) : item(item), next(nullptr) {}
    };

    Node* head;
    int count;

    // Node를 직접 소유하므로 얕은 복사를 막는다. Inventory가 복사되면 두 객체가
    // 같은 Node를 delete하려고 할 수 있기 때문이다.
    Inventory(const Inventory& other) = delete;
    Inventory& operator=(const Inventory& other) = delete;

public:
    Inventory();
    ~Inventory();

    void addItem(const Item& item);
    bool removeItem(const std::string& itemName);
    Item* findItem(const std::string& itemName);
    const Item* findItem(const std::string& itemName) const;
    int countItem(const std::string& itemName) const;
    bool isEmpty() const;
    int size() const;
    void clear();
    void print(const std::string& equippedWeapon = "", const std::string& equippedArmor = "", bool sorted = false) const;
};

#endif
