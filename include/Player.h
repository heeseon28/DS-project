#ifndef PLAYER_H
#define PLAYER_H

#include "Direction.h"
#include "Item.h"
#include "ds/Inventory.h"
#include "ds/Stack.h"
#include <string>

struct MoveRecord {
    int roomId;
    int x;
    int y;
};

class Player {
private:
    std::string name;
    int health;
    int score;
    int currentRoomId;
    int x;
    int y;
    int stepsInCurrentRoom;
    std::string equippedWeaponName;
    std::string equippedArmorName;
    int equipmentAttackBonus;
    int equipmentDefenseBonus;
    Inventory inventory;
    Stack<MoveRecord> moveHistory;

public:
    Player(const std::string& name = "Explorer");

    std::string getName() const;
    int getHealth() const;
    int getScore() const;
    int getCurrentRoomId() const;
    int getX() const;
    int getY() const;
    int getSteps() const;

    void setCurrentRoomId(int roomId);
    void setPosition(int newX, int newY);
    bool move(Direction direction, int roomWidth, int roomHeight);
    void saveMoveHistory(int roomId, int posX, int posY);
    bool undoMove(int& outRoomId, int& outX, int& outY);
    void resetSteps();

    void addScore(int amount);
    void changeHealth(int delta);
    bool isAlive() const;
    bool equipItem(const std::string& itemName);
    int getEquipmentAttackBonus() const;
    int getEquipmentDefenseBonus() const;
    std::string getEquippedWeaponName() const;
    std::string getEquippedArmorName() const;

    Inventory& getInventory();
    const Inventory& getInventory() const;

    void printStatus() const;
};

#endif
