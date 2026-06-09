#include "Direction.h"
#include "Item.h"
#include "ItemFactory.h"
#include "Player.h"
#include "SpriteAssets.h"
#include "ds/DungeonGraph.h"
#include "ds/HuffmanCodec.h"
#include "ds/Inventory.h"
#include "ds/Queue.h"
#include "ds/ScoreTree.h"
#include "ds/Sorting.h"
#include "ds/Stack.h"
#include <cassert>
#include <iostream>
#include <sstream>

void testInventory() {
    Inventory inventory;
    assert(inventory.isEmpty());
    inventory.addItem(Item("key", "opens a door", 10));
    assert(inventory.size() == 1);
    assert(inventory.findItem("key") != nullptr);
    assert(inventory.removeItem("key"));
    assert(inventory.isEmpty());
}

void testEquipment() {
    Player player("tester");
    player.getInventory().addItem(Item("rifle", "boosts attack", 10, 8, 0));
    player.getInventory().addItem(Item("armor", "boosts defense", 40, 0, 6));

    std::ostringstream outputSink;
    std::streambuf* originalOutput = std::cout.rdbuf(outputSink.rdbuf());

    assert(player.equipItem("rifle"));
    assert(player.getEquipmentAttackBonus() == 8);
    assert(player.getEquipmentDefenseBonus() == 0);

    assert(player.equipItem("armor"));
    assert(player.getEquipmentAttackBonus() == 8);
    assert(player.getEquipmentDefenseBonus() == 6);
    assert(!player.equipItem("key"));

    std::cout.rdbuf(originalOutput);
}

void testItemFactory() {
    Item rifle = createItem("라이플");
    Item armor = createItem("갑옷");

    assert(rifle.getAttackBonus() == 8);
    assert(rifle.getDefenseBonus() == 0);
    assert(armor.getAttackBonus() == 0);
    assert(armor.getDefenseBonus() == 6);
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
        Item("포션", "", 1),
        Item("몬스터볼", "", 10),
        Item("구슬", "", 5)
    };
    sortItemsByNameAscending(items, 3);
    assert(items[0].getName() == "구슬");
    assert(items[1].getName() == "몬스터볼");
    assert(items[2].getName() == "포션");
}

void testScoreTree() {
    ScoreTree tree;
    tree.insert(ScoreRecord("Ada", 80));
    tree.insert(ScoreRecord("Grace", 95));
    assert(tree.size() == 2);
    assert(tree.containsPlayer("Ada"));
    assert(!tree.containsPlayer("Unknown"));
}

void testHuffmanCodec() {
    std::string sprite = "..........\n##########\n....##....\n";
    HuffmanCodec::Result result = HuffmanCodec::compressAndDecode(sprite);
    assert(result.decodedText == sprite);
    assert(result.originalByteCount == static_cast<int>(sprite.size()));
    assert(result.uniqueSymbolCount >= 3);
    assert(result.compressedBitCount < result.originalByteCount * 8);
    assert(HuffmanCodec::decodeSprite(sprite.c_str()) == sprite);
    assert(!HuffmanCodec::decodeSprite(&SPRITE_03).empty());
    assert(!HuffmanCodec::decodeSprite(&OAK_SPRITE).empty());
}

int main() {
    std::cout << "Running smoke tests.\n";
    testInventory();
    testEquipment();
    testItemFactory();
    testStack();
    testQueue();
    testGraph();
    testSorting();
    testScoreTree();
    testHuffmanCodec();
    std::cout << "All smoke tests passed.\n";
    return 0;
}
