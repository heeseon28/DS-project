#include "Player.h"
#include <iostream>

Player::Player(const std::string& name)
    : name(name),
      health(100),
      score(0),
      currentRoomId(0),
      x(1),
      y(20),
      stepsInCurrentRoom(0),
      equippedWeaponName(""),
      equippedArmorName(""),
      equipmentAttackBonus(0),
      equipmentDefenseBonus(0),
      inventory(),
      moveHistory() {}

std::string Player::getName() const {
    return name;
}

int Player::getHealth() const {
    return health;
}

int Player::getScore() const {
    return score;
}

int Player::getCurrentRoomId() const {
    return currentRoomId;
}

int Player::getX() const {
    return x;
}

int Player::getY() const {
    return y;
}

int Player::getSteps() const {
    return stepsInCurrentRoom;
}

void Player::setCurrentRoomId(int roomId) {
    currentRoomId = roomId;
}

void Player::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

bool Player::move(Direction direction, int roomWidth, int roomHeight) {
    if (direction == Direction::Invalid) {
        return false;
    }

    int nextX = x + getDeltaX(direction);
    int nextY = y + getDeltaY(direction);

    if (nextX < 0 || nextX >= roomWidth || nextY < 0 || nextY >= roomHeight) {
        std::cout << "벽에 부딪혔습니다. 더 이상 이동할 수 없습니다.\n";
        return false;
    }

    x = nextX;
    y = nextY;
    ++stepsInCurrentRoom;
    return true;
}

void Player::saveMoveHistory(int roomId, int posX, int posY) {
    // 실제 이동을 적용하기 전에 이전 방/좌표를 저장한다. Stack의 top에는 항상
    // 가장 최근 위치가 있으므로 undo 시 바로 직전 위치부터 복원된다.
    MoveRecord record = {roomId, posX, posY};
    moveHistory.push(record);
}

bool Player::undoMove(int& outRoomId, int& outX, int& outY) {
    MoveRecord previous;
    if (!moveHistory.pop(previous)) {
        // 저장된 이동 기록이 없으면 pop이 실패한다. 이 edge case를 false로 알려
        // Game::undoMove가 화면 메시지와 흐름을 처리할 수 있게 한다.
        std::cout << "되돌릴 이동 기록이 없습니다.\n";
        return false;
    }

    // pop으로 얻은 MoveRecord는 "되돌아갈 위치"이다. 현재 위치와 방 id를 복원하고,
    // 새 방에서 다시 걸음 수를 세도록 stepsInCurrentRoom을 초기화한다.
    currentRoomId = previous.roomId;
    x = previous.x;
    y = previous.y;
    stepsInCurrentRoom = 0;

    outRoomId = currentRoomId;
    outX = x;
    outY = y;
    return true;
}

void Player::resetSteps() {
    stepsInCurrentRoom = 0;
}

void Player::addScore(int amount) {
    score += amount;
}

void Player::changeHealth(int delta) {
    health += delta;
    if (health > 100) {
        health = 100;
    }
    if (health < 0) {
        health = 0;
    }
}

bool Player::isAlive() const {
    return health > 0;
}

bool Player::equipItem(const std::string& itemName) {
    const Item* item = inventory.findItem(itemName);
    if (item == nullptr) {
        std::cout << "'" << itemName << "' 아이템이 가방에 없습니다.\n";
        return false;
    }

    if (!item->isEquipment()) {
        std::cout << "'" << itemName << "'은(는) 장착할 수 없는 아이템입니다.\n";
        return false;
    }

    if (item->getAttackBonus() > 0) {
        equippedWeaponName = item->getName();
        equipmentAttackBonus = item->getAttackBonus();
    }

    if (item->getDefenseBonus() > 0) {
        equippedArmorName = item->getName();
        equipmentDefenseBonus = item->getDefenseBonus();
    }

    std::cout << item->getName() << " 장착 완료!";
    if (item->getAttackBonus() > 0) {
        std::cout << " 공격 +" << item->getAttackBonus();
    }
    if (item->getDefenseBonus() > 0) {
        std::cout << " 방어 +" << item->getDefenseBonus();
    }
    std::cout << "\n";
    return true;
}

bool Player::unequipItem(const std::string& itemName) {
    if (equippedWeaponName == itemName) {
        equippedWeaponName = "";
        equipmentAttackBonus = 0;
        std::cout << itemName << " 장착 해제!\n";
        return true;
    }
    if (equippedArmorName == itemName) {
        equippedArmorName = "";
        equipmentDefenseBonus = 0;
        std::cout << itemName << " 장착 해제!\n";
        return true;
    }
    return false;
}

bool Player::isItemEquipped(const std::string& itemName) const {
    return equippedWeaponName == itemName || equippedArmorName == itemName;
}

int Player::getEquipmentAttackBonus() const {
    return equipmentAttackBonus;
}

int Player::getEquipmentDefenseBonus() const {
    return equipmentDefenseBonus;
}

std::string Player::getEquippedWeaponName() const {
    return equippedWeaponName;
}

std::string Player::getEquippedArmorName() const {
    return equippedArmorName;
}

Inventory& Player::getInventory() {
    return inventory;
}

const Inventory& Player::getInventory() const {
    return inventory;
}

void Player::printStatus() const {
    std::cout << "점수: " << score
              << " | 체력: " << health
              << " | 위치: Room " << currentRoomId
              << " (" << x << ", " << y << ")"
              << " | 걸음: " << stepsInCurrentRoom << "\n";
    std::cout << "장비: 무기 "
              << (equippedWeaponName.empty() ? "없음" : equippedWeaponName)
              << " (공격 +" << equipmentAttackBonus << ")"
              << " | 방어구 "
              << (equippedArmorName.empty() ? "없음" : equippedArmorName)
              << " (방어 +" << equipmentDefenseBonus << ")\n";
}
