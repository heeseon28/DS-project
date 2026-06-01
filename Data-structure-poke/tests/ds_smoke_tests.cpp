#include "Direction.h"
#include "Item.h"
#include "ds/DungeonGraph.h"
#include "ds/Inventory.h"
#include "ds/Queue.h"
#include "ds/ScoreTree.h"
#include "ds/Sorting.h"
#include "ds/Stack.h"
#include <cassert>
#include <iostream>

void testInventory() {
    Inventory inventory;
    assert(inventory.isEmpty());
    inventory.addItem(Item("key", "opens a door", 10));
    assert(inventory.size() == 1);
    assert(inventory.findItem("key") != nullptr);
    assert(inventory.removeItem("key"));
    assert(inventory.isEmpty());
}

void testStack() {
    Stack<int> stack;
    assert(stack.isEmpty());
    stack.push(3);
    stack.push(7);
    int value = 0;
    assert(stack.pop(value));
    assert(value == 7);
    assert(stack.pop(value));
    assert(value == 3);
    assert(!stack.pop(value));
}

void testQueue() {
    Queue<int> queue;
    assert(queue.isEmpty());
    queue.enqueue(1);
    queue.enqueue(2);
    int value = 0;
    assert(queue.dequeue(value));
    assert(value == 1);
    assert(queue.dequeue(value));
    assert(value == 2);
    assert(!queue.dequeue(value));
}

void testGraph() {
    DungeonGraph graph;
    int a = graph.addRoom("A", "first");
    int b = graph.addRoom("B", "second");
    assert(graph.connectRooms(a, Direction::East, b, true));
    assert(graph.getNeighbor(a, Direction::East) == b);
    assert(graph.getNeighbor(b, Direction::West) == a);
}

void testSorting() {
    Item items[3] = {
        Item("low", "", 1),
        Item("high", "", 10),
        Item("mid", "", 5)
    };
    sortItemsByValueDescending(items, 3);
    assert(items[0].getValue() == 10);
    assert(items[1].getValue() == 5);
    assert(items[2].getValue() == 1);
}

void testScoreTree() {
    ScoreTree tree;
    tree.insert(ScoreRecord("Ada", 80));
    tree.insert(ScoreRecord("Grace", 95));
    assert(tree.size() == 2);
    assert(tree.containsPlayer("Ada"));
    assert(!tree.containsPlayer("Unknown"));
}

int main() {
    std::cout << "Running smoke tests. Some tests should fail until TODO methods are implemented.\n";
    testInventory();
    testStack();
    testQueue();
    testGraph();
    testSorting();
    testScoreTree();
    std::cout << "All smoke tests passed.\n";
    return 0;
}
