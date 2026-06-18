#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <iostream>

// DynamicArray는 실행 중 크기가 늘어날 수 있으면서도 인덱스로 바로 접근해야
// 하는 목록에 사용된다. 이 프로젝트에서는 Room이 아이템과 몬스터를 저장할 때
// 사용한다. 방마다 보유 개수는 다르지만, 출력/획득/복사 과정에서는
// Room::getItem(i), Room::getMonster(i)처럼 O(1) 인덱스 접근이 필요하다.
template <typename T>
class DynamicArray {
private:
    // data는 직접 할당한 배열의 시작 주소이고, count는 실제 저장된 원소 수,
    // capacity는 현재 배열이 담을 수 있는 최대 칸 수를 의미한다.
    T* data;
    int count;
    int capacity;

    void resize(int newCapacity) {
        if (newCapacity < 1) {
            newCapacity = 1;
        }

        // resize는 pushBack 중 가장 비용이 큰 경우이다. 더 큰 배열을 새로 만들고,
        // 기존 원소를 복사한 뒤 예전 배열을 해제한다. 한 번의 resize는 O(n)이지만
        // capacity를 보통 두 배로 늘리기 때문에 반복 삽입의 평균 비용은
        // amortized O(1)로 볼 수 있다.
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
        // 모든 방에 큰 고정 배열을 미리 잡지 않고 작은 배열에서 시작한다.
        // 아이템/몬스터가 적은 방에서 메모리를 낭비하지 않기 위해서이다.
        data = new T[capacity];
    }

    DynamicArray(const DynamicArray& other)
        : data(new T[other.capacity]), count(other.count), capacity(other.capacity) {
        // 깊은 복사를 수행한다. 두 Room 객체가 같은 data 배열을 공유하면 한쪽이
        // 해제될 때 다른 쪽의 아이템/몬스터 목록까지 깨질 수 있기 때문이다.
        for (int i = 0; i < count; ++i) {
            data[i] = other.data[i];
        }
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this == &other) {
            return *this;
        }

        // 새 배열을 먼저 준비한 뒤 기존 배열을 지운다. 대입 과정에서 자기 자신에게
        // 대입되는 경우를 처리하고, data가 해제된 주소를 가리키는 상황을 피한다.
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
        // 공간이 충분하면 count 위치에 값을 넣고 count만 증가시키면 된다.
        // 그래서 resize가 없는 일반 삽입은 O(1)에 끝난다.
        data[count] = value;
        ++count;
    }

    bool removeAt(int index) {
        if (index < 0 || index >= count) {
            return false;
        }

        // 중간 원소를 지운 뒤 뒤쪽 원소를 한 칸씩 당겨 인덱스를 빈틈없이 유지한다.
        // O(n)이지만 방 안의 아이템/몬스터 목록은 작기 때문에 부담이 크지 않다.
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
        // 범위 검사는 호출하는 쪽에서 책임진다. operator[]는 배열처럼 바로 접근하게
        // 두어 getItem(i), getMonster(i)가 O(1)로 동작하도록 한다.
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }
};

#endif
