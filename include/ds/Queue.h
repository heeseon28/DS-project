#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>

// Linked-node FIFO queue.
// GameEvent와 BattleAction처럼 "먼저 준비된 작업을 먼저 처리"해야 하는 곳에 사용된다.
// Queue가 우선순위를 직접 계산하는 것은 아니며, enqueue 전에 이미 결정된 순서를
// 보존했다가 dequeue로 하나씩 실행해 순서 결정과 실제 처리를 분리한다.
template <typename T>
class Queue {
private:
    struct Node {
        T value;
        Node* next;
        Node(const T& value) : value(value), next(nullptr) {}
    };

    Node* frontNode;
    Node* rearNode;
    int count;

public:
    Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    ~Queue() {
        clear();
    }

    // 노드 소유권을 단순하게 유지하기 위해 복사는 막는다.
    Queue(const Queue& other) = delete;
    Queue& operator=(const Queue& other) = delete;

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void enqueue(const T& value) {
        // rearNode 뒤에 새 Node를 붙인다. 배열처럼 원소를 이동하거나 고정 용량을
        // 걱정하지 않아도 되므로 삽입은 O(1)이다.
        Node* newNode = new Node(value);

        // 첫 삽입이면 front와 rear가 같은 노드를 가리킨다.
        if (isEmpty()) {
            frontNode = newNode;
            rearNode = newNode;
        } else {
            // 이후 삽입은 rear 뒤에 붙이고 rear 포인터만 갱신한다.
            rearNode->next = newNode;
            rearNode = newNode;
        }

        ++count;
    }

    bool dequeue(T& output) {
        if (isEmpty()) {
            // 빈 큐는 꺼낼 값이 없으므로 호출자가 처리하도록 false를 반환한다.
            return false;
        }

        // frontNode가 가장 오래전에 들어온 값이다. 값을 output에 복사한 뒤
        // frontNode를 다음 노드로 옮기면 FIFO 순서가 유지된다.
        Node* oldFront = frontNode;
        output = oldFront->value;
        frontNode = frontNode->next;

        // 마지막 노드를 꺼낸 경우 rear도 nullptr로 맞춰 빈 큐 상태를 유지한다.
        if (frontNode == nullptr) {
            rearNode = nullptr;
        }

        delete oldFront;
        --count;
        return true;
    }

    bool peek(T& output) const {
        if (isEmpty()) {
            return false;
        }

        // peek은 값만 확인하고 노드는 제거하지 않는다.
        output = frontNode->value;
        return true;
    }

    void clear() {
        // 남은 노드를 앞에서부터 모두 삭제한다. rearNode도 nullptr로 맞춰
        // 완전히 빈 Queue 상태를 만든다.
        while (frontNode != nullptr) {
            Node* old = frontNode;
            frontNode = frontNode->next;
            delete old;
        }
        rearNode = nullptr;
        count = 0;
    }
};

#endif
