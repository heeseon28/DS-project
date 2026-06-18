#ifndef STACK_H
#define STACK_H

#include <iostream>

// Linked-node Stack. Player의 이동 기록을 저장해 undo 기능을 만든다.
// 이동을 되돌릴 때는 가장 최근 위치부터 복원해야 하므로 LIFO
// (Last-In, First-Out) 구조가 기능의 의미와 정확히 맞는다.
template <typename T>
class Stack {
private:
    struct Node {
        T value;
        Node* next;
        Node(const T& value) : value(value), next(nullptr) {}
    };

    Node* topNode;
    int count;

public:
    Stack() : topNode(nullptr), count(0) {}

    ~Stack() {
        clear();
    }

    Stack(const Stack& other) = delete;
    Stack& operator=(const Stack& other) = delete;

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void push(const T& value) {
        // 새 기록을 topNode 앞에 붙인다. 이전 기록 전체를 옮기지 않고 포인터만
        // 바꾸므로 이동 기록 저장은 O(1)에 끝난다.
        Node* newNode = new Node(value);
        newNode->next = topNode;
        topNode = newNode;
        ++count;
    }

    bool pop(T& output) {
        if (isEmpty()) {
            // 빈 stack에서 undo를 요청하면 꺼낼 값이 없으므로 false를 반환해
            // 호출자가 안내 메시지를 출력하도록 한다.
            return false;
        }

        // topNode가 가장 최근 이동 기록이다. 값을 output에 복사한 뒤 topNode를
        // 다음 노드로 내리면 바로 이전 위치가 새로운 top이 된다.
        Node* oldTop = topNode;
        output = oldTop->value;
        topNode = topNode->next;
        delete oldTop;
        --count;
        return true;
    }

    bool peek(T& output) const {
        if (isEmpty()) {
            return false;
        }

        // peek는 undo 후보를 확인만 하고 실제 기록은 제거하지 않는다.
        output = topNode->value;
        return true;
    }

    void clear() {
        // 모든 이동 기록 Node를 해제한다. Player가 소멸될 때 남은 이동 기록이
        // 메모리에 남지 않도록 소멸자에서 호출된다.
        while (topNode != nullptr) {
            Node* old = topNode;
            topNode = topNode->next;
            delete old;
        }
        count = 0;
    }
};

#endif
