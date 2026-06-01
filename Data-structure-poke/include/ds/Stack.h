#ifndef STACK_H
#define STACK_H

#include <iostream>

// Linked-node stack. Used for movement history and undo.
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
        Node* newNode = new Node(value);
        newNode->next = topNode;
        topNode = newNode;
        ++count;
    }

    bool pop(T& output) {
        if (isEmpty()) {
            return false;
        }

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

        output = topNode->value;
        return true;
    }

    void clear() {
        // Provided cleanup helper: delete every node and reset topNode/count.
        while (topNode != nullptr) {
            Node* old = topNode;
            topNode = topNode->next;
            delete old;
        }
        count = 0;
    }
};

#endif
