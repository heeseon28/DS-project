#include "ds/Inventory.h"
#include <iostream>

Inventory::Inventory() : head(nullptr), count(0) {}

Inventory::~Inventory() {
    clear();
}

void Inventory::addItem(const Item& item) {
    // 새 Node를 head 앞에 붙인다. 기존 원소들을 한 칸씩 옮기지 않아도 되므로
    // 아이템 획득은 항상 O(1)에 처리된다.
    Node* newNode = new Node(item);
    newNode->next = head;
    head = newNode;
    ++count;
}

bool Inventory::removeItem(const std::string& itemName) {
    // singly linked list는 이전 노드 주소를 모르면 current를 목록에서 뺄 수 없다.
    // current로 삭제 대상을 찾고 previous로 연결을 다시 이어 준다.
    Node* current = head;
    Node* previous = nullptr;

    while (current != nullptr) {
        if (current->item.getName() == itemName) {
            if (previous == nullptr) {
                // 지울 대상이 첫 노드이면 head 자체를 다음 노드로 옮긴다.
                head = current->next;
            } else {
                // 중간/마지막 노드이면 previous가 current 다음 노드를 건너뛰게 한다.
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
    // linked list는 임의 위치로 바로 갈 수 없기 때문에 이름 검색은 head부터
    // 하나씩 비교한다. 최악의 경우 모든 노드를 확인하므로 O(n)이다.
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
    // const 버전은 장비 확인처럼 Inventory 내용을 바꾸면 안 되는 곳에서 사용한다.
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
    // 전투 시작 전에 몬스터볼/회복약 개수를 세는 데 사용된다. 같은 이름의 아이템이
    // 여러 Node에 나뉘어 저장될 수 있으므로 끝까지 순회하며 개수를 누적한다.
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
    // 동적으로 만든 모든 Node를 직접 delete한다. 소멸자에서도 호출되므로
    // 게임 종료 시 가방에 남은 아이템 Node가 메모리에 남지 않는다.
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    count = 0;
}

void Inventory::print(const std::string& equippedWeapon, const std::string& equippedArmor, bool sorted) const {
    if (head == nullptr) {
        std::cout << "가방이 비어 있습니다.\n";
        return;
    }

    std::cout << "가방 (" << count << "개):\n";

    // 출력 순서만 바꾸기 위해 노드 자체는 건드리지 않고 포인터 배열만 임시로 만든다.
    // linked list 자체는 저장 순서대로만 순회할 수 있다. 출력 순서만 바꾸기 위해
    // Node를 재배치하지 않고 Item 포인터 배열을 임시로 만든다.
    const Item** items = new const Item*[count];
    int index = 0;
    for (Node* cur = head; cur != nullptr; cur = cur->next) {
        items[index] = &cur->item;
        ++index;
    }

    if (sorted) {
        // 출력 전용 정렬이다. 실제 linked list의 Node 순서는 그대로 두기 때문에
        // 장비 상태나 삭제 로직에는 영향을 주지 않는다.
        // STL 정렬 대신 직접 선택 정렬을 사용한다.
        for (int i = 0; i < count - 1; ++i) {
            int bestIndex = i;
            for (int j = i + 1; j < count; ++j) {
                if (items[j]->getName() < items[bestIndex]->getName()) {
                    bestIndex = j;
                }
            }
            if (bestIndex != i) {
                const Item* temp = items[i];
                items[i] = items[bestIndex];
                items[bestIndex] = temp;
            }
        }
    }

    for (int i = 0; i < count; ++i) {
        const Item* item = items[i];
        const std::string& n = item->getName();
        bool equipped = (!equippedWeapon.empty() && n == equippedWeapon) ||
                        (!equippedArmor.empty()  && n == equippedArmor);
        std::cout << "  - ";
        if (equipped) std::cout << "[장착] ";
        item->print();
    }

    delete[] items;
}
