#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>

// Linked-node queue. Used for game event processing.
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

    Queue(const Queue& other) = delete;
    Queue& operator=(const Queue& other) = delete;

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void enqueue(const T& value) {
        Node* newNode = new Node(value);

        if (isEmpty()) {
            frontNode = newNode;
            rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }

        ++count;
    }

    bool dequeue(T& output) {
        if (isEmpty()) {
            return false;
        }

        Node* oldFront = frontNode;
        output = oldFront->value;
        frontNode = frontNode->next;

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

        output = frontNode->value;
        return true;
    }

    void clear() {
        // Provided cleanup helper: delete every node and reset frontNode/rearNode/count.
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
