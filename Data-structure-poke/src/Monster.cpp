#include "Monster.h"
#include <iostream>

Monster::Monster() : name(""), health(0), attack(0), rewardScore(0) {}

Monster::Monster(const std::string& name, int health, int attack, int rewardScore)
    : name(name), health(health), attack(attack), rewardScore(rewardScore) {}

std::string Monster::getName() const {
    return name;
}

int Monster::getHealth() const {
    return health;
}

int Monster::getAttack() const {
    return attack;
}

int Monster::getRewardScore() const {
    return rewardScore;
}

bool Monster::isAlive() const {
    return health > 0;
}

void Monster::takeDamage(int amount) {
    health -= amount;
    if (health < 0) {
        health = 0;
    }
}

void Monster::print() const {
    std::cout << name << " (체력 " << health << ", 공격력 " << attack
              << ", 보상 점수 " << rewardScore << ")\n";
}
