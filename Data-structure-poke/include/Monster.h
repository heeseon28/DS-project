#ifndef MONSTER_H
#define MONSTER_H

#include <string>

class Monster {
private:
    std::string name;
    int health;
    int attack;
    int rewardScore;

public:
    Monster();
    Monster(const std::string& name, int health, int attack, int rewardScore);

    std::string getName() const;
    int getHealth() const;
    int getAttack() const;
    int getRewardScore() const;

    bool isAlive() const;
    void takeDamage(int amount);
    void print() const;
};

#endif
