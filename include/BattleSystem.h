#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include <string>
#include "ds/Queue.h"

enum class ElementType // 타입 정의
{
    Water,
    Fire,
    Grass,
    Electric,
    Ground
};

enum class StatusCondition
{
    None,
    Paralysis,
    Burn
};

enum class BattleActionType // 옵션 정의 : 플레이어 공격, 적의 공격, 잡는다, 치료한다
{
    PlayerAttack,
    EnemyAttack,
    PlayerCatch,
    PlayerHeal,
    PlayerRun
};

struct BattleAction
{
    BattleActionType type;
};

class BattleEntity // 부모 클래스 선언 : 적 개체 데이터 정의
{
protected:
    std::string name;
    int maxHp;
    int hp;
    int attack;
    int defense;
    int speed;
    ElementType element;
    StatusCondition status;

public:
    BattleEntity( // 생성자
        const std::string &name,
        int maxHp,
        int attack,
        int defense,
        int speed,
        ElementType element);

    virtual ~BattleEntity(); // 소멸자

    // 초기값 정의
    const std::string &getName() const;
    int getMaxHp() const;
    int getHp() const;
    int getAttack() const;
    int getDefense() const;
    int getSpeed() const;
    ElementType getElement() const;

    bool isAlive() const;
    void takeDamage(int damage);
    void heal(int amount);
    void fullHeal();

    void printStatus() const;

    // 상태이상 관련 추가 함수
    StatusCondition getStatus() const;
    void setStatus(StatusCondition newStatus);
    bool hasStatus() const;
    void clearStatus();

    int getEffectiveSpeed() const;
    int getEffectiveAttack() const;

    void applyEndTurnStatusDamage();
};

class PlayerBattle : public BattleEntity
{
private:
    int monsterBalls;
    int fullHeals;

public:
    PlayerBattle();
    PlayerBattle(
        const std::string &name,
        int maxHp,
        int attack,
        int defense,
        int speed,
        ElementType element,
        int initialMonsterBalls = 5,
        int initialFullHeals = 2);

    bool hasMonsterBall() const;
    bool useMonsterBall();
    int getMonsterBallCount() const;

    bool hasFullHeal() const;
    bool useFullHeal();
    int getFullHealCount() const;
};

class EnemyBattle : public BattleEntity
{
public:
    EnemyBattle(
        const std::string &name,
        int maxHp,
        int attack,
        int defense,
        int speed,
        ElementType element);
};

// 전투 시스템 클래스 정의
class BattleSystem
{
private:
    Queue<BattleAction> actionQueue;
    bool lastCatchSucceeded;

    double getTypeMultiplier(ElementType attacker, ElementType defender) const;
    int calculateDamage(const BattleEntity &attacker, const BattleEntity &defender) const;

    int calculateCatchChance(const EnemyBattle &enemy) const;
    int calculateRunChance(const PlayerBattle &player) const;

    bool tryCatch(PlayerBattle &player, EnemyBattle &enemy);
    bool tryRun(PlayerBattle &player);
    bool cannotMoveByParalysis(const BattleEntity &entity) const;
    void tryApplyStatusEffect(const BattleEntity &attacker, BattleEntity &defender) const;

    void enqueueTurnActions(PlayerBattle &player, EnemyBattle &enemy, BattleActionType playerAction);
    void processAction(BattleAction action, PlayerBattle &player, EnemyBattle &enemy, bool &battleEnded);

public:
    BattleSystem();

    bool startTestBattle();
    bool startBattleByPokemonName(const std::string &pokemonName);
    bool startBattle(PlayerBattle &player, EnemyBattle &enemy);
    bool wasLastCatchSuccessful() const;
};

#endif
