#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <iostream>

// A small provided dynamic array used by the starter engine.
// Students may study it as a reference for array resizing, but the main required
// TODO work is in Inventory, Stack, Queue, DungeonGraph, ScoreTree, and Sorting.
template <typename T>
class DynamicArray {
private:
    T* data;
    int count;
    int capacity;

    void resize(int newCapacity) {
        if (newCapacity < 1) {
            newCapacity = 1;
        }

        T* newData = new T[newCapacity];
        for (int i = 0; i < count; ++i) {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    DynamicArray(int initialCapacity = 4)
        : data(nullptr), count(0), capacity(initialCapacity) {
        if (capacity < 1) {
            capacity = 1;
        }
        data = new T[capacity];
    }

    DynamicArray(const DynamicArray& other)
        : data(new T[other.capacity]), count(other.count), capacity(other.capacity) {
        for (int i = 0; i < count; ++i) {
            data[i] = other.data[i];
        }
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this == &other) {
            return *this;
        }

        T* newData = new T[other.capacity];
        for (int i = 0; i < other.count; ++i) {
            newData[i] = other.data[i];
        }

        delete[] data;
        data = newData;
        count = other.count;
        capacity = other.capacity;
        return *this;
    }

    ~DynamicArray() {
        delete[] data;
    }

    int size() const {
        return count;
    }

    bool isEmpty() const {
        return count == 0;
    }

    void pushBack(const T& value) {
        if (count >= capacity) {
            resize(capacity * 2);
        }
        data[count] = value;
        ++count;
    }

    bool removeAt(int index) {
        if (index < 0 || index >= count) {
            return false;
        }

        for (int i = index; i < count - 1; ++i) {
            data[i] = data[i + 1];
        }
        --count;
        return true;
    }

    void clear() {
        count = 0;
    }

    T& operator[](int index) {
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }
};

#endif
