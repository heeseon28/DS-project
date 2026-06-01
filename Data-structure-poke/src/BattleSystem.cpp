#include "BattleSystem.h" //
#include "PokemonFactory.h"
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
#if defined(__APPLE__)
pid_t battleBgmLoopPid = -1;
#endif

bool fileExists(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    return file.good();
}

std::string findBattleBgmPath()
{
    const char* candidates[] = {
        "battle_bgm.mp3",
        "./battle_bgm.mp3",
        "../battle_bgm.mp3",
        "Data-structure-poke/battle_bgm.mp3",
        "student_starter/battle_bgm.mp3",
        "student_starter/Data-structure-poke/battle_bgm.mp3"
    };

    for (const char* path : candidates)
    {
        if (fileExists(path))
        {
            return path;
        }
    }

    char cwdBuffer[4096];
    if (getcwd(cwdBuffer, sizeof(cwdBuffer)) != nullptr)
    {
        std::string current = cwdBuffer;
        for (int depth = 0; depth < 8 && !current.empty(); ++depth)
        {
            std::string candidate = current + "/battle_bgm.mp3";
            if (fileExists(candidate))
            {
                return candidate;
            }

            std::size_t slash = current.find_last_of('/');
            if (slash == std::string::npos || slash == 0)
            {
                break;
            }
            current = current.substr(0, slash);
        }
    }

    return "";
}

std::string statusToKorean(StatusCondition status)
{
    if (status == StatusCondition::Paralysis)
    {
        return "마비";
    }

    if (status == StatusCondition::Burn)
    {
        return "화상";
    }

    return "정상";
}

void startBattleBgm()
{
#if defined(__APPLE__)
    if (battleBgmLoopPid > 0)
    {
        return;
    }

    std::string bgmPath = findBattleBgmPath();
    if (bgmPath.empty())
    {
        std::cout << "battle_bgm.mp3 파일을 찾을 수 없어 BGM을 재생하지 않습니다.\n";
        return;
    }

    if (access("/usr/bin/afplay", X_OK) != 0)
    {
        std::cout << "macOS afplay를 찾을 수 없어 BGM을 재생하지 않습니다.\n";
        return;
    }

    pid_t loopPid = fork();
    if (loopPid < 0)
    {
        std::cout << "BGM 재생 프로세스를 시작하지 못했습니다.\n";
        return;
    }

    if (loopPid == 0)
    {
        setpgid(0, 0);

        while (true)
        {
            pid_t playerPid = fork();
            if (playerPid == 0)
            {
                execl("/usr/bin/afplay", "afplay", bgmPath.c_str(), static_cast<char*>(nullptr));
                execlp("afplay", "afplay", bgmPath.c_str(), static_cast<char*>(nullptr));
                _exit(127);
            }

            if (playerPid < 0)
            {
                _exit(1);
            }

            int status = 0;
            pid_t waited = 0;
            do
            {
                waited = waitpid(playerPid, &status, 0);
            } while (waited == -1 && errno == EINTR);

            if (waited == -1)
            {
                _exit(1);
            }

            if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
            {
                _exit(127);
            }
        }
    }

    setpgid(loopPid, loopPid);
    battleBgmLoopPid = loopPid;
    std::cout << "BGM 재생 시작: " << bgmPath << "\n";
    std::cout.flush();
#else
    (void)findBattleBgmPath;
#endif
}

void stopBattleBgm()
{
#if defined(__APPLE__)
    if (battleBgmLoopPid <= 0)
    {
        return;
    }

    kill(-battleBgmLoopPid, SIGTERM);

    int status = 0;
    while (waitpid(battleBgmLoopPid, &status, 0) == -1 && errno == EINTR)
    {
    }

    battleBgmLoopPid = -1;
#endif
}

void printPokemonSprite(const char* sprite)
{
    if (sprite != nullptr && sprite[0] != 0)
    {
        std::cout << sprite << "\n";
    }
}
}

BattleEntity::BattleEntity( // 적 개체 최초 선언
    const std::string &name,
    int maxHp,
    int attack,
    int defense,
    int speed,
    ElementType element)
    : name(name),
      maxHp(maxHp),
      hp(maxHp),
      attack(attack),
      defense(defense),
      speed(speed),
      element(element),
      status(StatusCondition::None)
{
}

BattleEntity::~BattleEntity()
{
}

const std::string &BattleEntity::getName() const
{
    return name;
}

int BattleEntity::getMaxHp() const
{
    return maxHp;
}

int BattleEntity::getHp() const
{
    return hp;
}

int BattleEntity::getAttack() const
{
    return attack;
}

int BattleEntity::getDefense() const
{
    return defense;
}

int BattleEntity::getSpeed() const
{
    return speed;
}

ElementType BattleEntity::getElement() const
{
    return element;
}

bool BattleEntity::isAlive() const
{
    return hp > 0;
}

void BattleEntity::takeDamage(int damage)
{
    if (damage < 0)
    {
        damage = 0;
    }

    hp -= damage;

    if (hp < 0)
    {
        hp = 0;
    }
}

void BattleEntity::heal(int amount)
{
    if (amount < 0)
    {
        return;
    }

    hp += amount;

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

void BattleEntity::fullHeal()
{
    hp = maxHp;
}

StatusCondition BattleEntity::getStatus() const
{
    return status;
}

void BattleEntity::setStatus(StatusCondition newStatus)
{
    if (status == StatusCondition::None)
    {
        status = newStatus;
    }
}

bool BattleEntity::hasStatus() const
{
    return status != StatusCondition::None;
}

void BattleEntity::clearStatus()
{
    status = StatusCondition::None;
}

int BattleEntity::getEffectiveSpeed() const
{
    if (status == StatusCondition::Paralysis)
    {
        return speed / 2;
    }
    return speed;
}

int BattleEntity::getEffectiveAttack() const
{
    if (status == StatusCondition::Burn)
    {
        return attack / 2;
    }
    return attack;
}

void BattleEntity::printStatus() const
{
    std::cout << name << " 체력: " << hp << " / " << maxHp;

    if (hasStatus())
    {
        std::cout << " | 상태: " << statusToKorean(status);
    }

    std::cout << "\n";
}

PlayerBattle::PlayerBattle()
    : PlayerBattle("건이", 100, 15, 8, 10, ElementType::Water)
{
}

PlayerBattle::PlayerBattle(
    const std::string &name,
    int maxHp,
    int attack,
    int defense,
    int speed,
    ElementType element,
    int initialMonsterBalls,
    int initialFullHeals)
    : BattleEntity(name, maxHp, attack, defense, speed, element),
      monsterBalls(initialMonsterBalls),
      fullHeals(initialFullHeals)
{
}

void BattleEntity::applyEndTurnStatusDamage()
{
    if (status == StatusCondition::Burn)
    {
        int burnDamage = maxHp / 10;

        if (burnDamage < 1)
        {
            burnDamage = 1;
        }

        takeDamage(burnDamage);

        std::cout << name << "은(는) 화상으로 " << burnDamage << "의 피해를 입었습니다!\n";
    }
}

bool PlayerBattle::hasMonsterBall() const
{
    return monsterBalls > 0;
}

bool PlayerBattle::useMonsterBall()
{
    if (monsterBalls <= 0)
    {
        return false;
    }

    --monsterBalls;
    return true;
}

bool PlayerBattle::hasFullHeal() const
{
    return fullHeals > 0;
}

bool PlayerBattle::useFullHeal()
{
    if (fullHeals <= 0)
    {
        return false;
    }

    --fullHeals;
    fullHeal();
    clearStatus();
    return true;
}

EnemyBattle::EnemyBattle(
    const std::string &name,
    int maxHp,
    int attack,
    int defense,
    int speed,
    ElementType element)
    : BattleEntity(name, maxHp, attack, defense, speed, element)
{
}

BattleSystem::BattleSystem()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

double BattleSystem::getTypeMultiplier(ElementType attacker, ElementType defender) const
{
    if (attacker == ElementType::Water && defender == ElementType::Fire)
    {
        return 2.0;
    }

    if (attacker == ElementType::Water && defender == ElementType::Ground)
    {
        return 2.0;
    }

    if (attacker == ElementType::Fire && defender == ElementType::Grass)
    {
        return 2.0;
    }

    if (attacker == ElementType::Grass && defender == ElementType::Water)
    {
        return 2.0;
    }

    if (attacker == ElementType::Grass && defender == ElementType::Ground)
    {
        return 2.0;
    }

    if (attacker == ElementType::Electric && defender == ElementType::Water)
    {
        return 2.0;
    }

    if (attacker == ElementType::Ground && defender == ElementType::Electric)
    {
        return 2.0;
    }

    if (attacker == ElementType::Ground && defender == ElementType::Fire)
    {
        return 2.0;
    }

    if (attacker == ElementType::Fire && defender == ElementType::Water)
    {
        return 0.5;
    }

    if (attacker == ElementType::Fire && defender == ElementType::Ground)
    {
        return 0.5;
    }

    if (attacker == ElementType::Water && defender == ElementType::Grass)
    {
        return 0.5;
    }

    if (attacker == ElementType::Grass && defender == ElementType::Fire)
    {
        return 0.5;
    }

    if (attacker == ElementType::Electric && defender == ElementType::Ground)
    {
        return 0.0;
    }

    return 1.0;
}

int BattleSystem::calculateDamage(const BattleEntity &attacker, const BattleEntity &defender) const
{
    int baseDamage = attacker.getEffectiveAttack() - defender.getDefense() / 2;

    if (baseDamage < 1)
    {
        baseDamage = 1;
    }

    double multiplier = getTypeMultiplier(attacker.getElement(), defender.getElement());

    int finalDamage = static_cast<int>(baseDamage * multiplier);

    if (finalDamage < 0)
    {
        finalDamage = 0;
    }

    if (multiplier > 1.0)
    {
        std::cout << "효과가 굉장했다!\n";
    }
    else if (multiplier > 0.0 && multiplier < 1.0)
    {
        std::cout << "효과가 별로였다...\n";
    }
    else if (multiplier == 0.0)
    {
        std::cout << "효과가 없다!\n";
    }

    return finalDamage;
}

int BattleSystem::calculateCatchChance(const EnemyBattle &enemy) const
{
    int missingHp = enemy.getMaxHp() - enemy.getHp();

    int chance = 20 + (missingHp * 60 / enemy.getMaxHp());

    if (chance < 20)
    {
        chance = 20;
    }

    if (chance > 80)
    {
        chance = 80;
    }

    return chance;
}

int BattleSystem::calculateRunChance(const PlayerBattle &player) const
{
    int chance = 20 + (player.getHp() * 50 / player.getMaxHp());

    if (chance < 20)
    {
        chance = 20;
    }

    if (chance > 70)
    {
        chance = 70;
    }

    return chance;
}

bool BattleSystem::tryCatch(PlayerBattle &player, EnemyBattle &enemy)
{
    if (!player.useMonsterBall())
    {
        std::cout << "남은 몬스터볼이 없습니다!\n";
        return false;
    }

    int chance = calculateCatchChance(enemy);
    int roll = std::rand() % 100 + 1;

    std::cout << "포획 확률: " << chance << "%\n";
    std::cout << "판정값: " << roll << "\n";

    if (roll <= chance)
    {
        std::cout << enemy.getName() << " 포획 성공!\n";

        /*
            나중에 여기서 Monster -> Item 변환.
            예시:

            Item caughtPokemon = createPokemonItemFromEnemy(enemy);
            player.getInventory().addItem(caughtPokemon);
            pokedex.insert(caughtPokemon);
        */

        return true;
    }

    std::cout << enemy.getName() << " 몬스터볼에서 빠져나왔습니다!\n";
    return false;
}

bool BattleSystem::tryRun(PlayerBattle &player)
{
    int chance = calculateRunChance(player);
    int roll = std::rand() % 100 + 1;

    std::cout << "도망 확률: " << chance << "%\n";
    std::cout << "판정값: " << roll << "\n";

    if (roll <= chance)
    {
        std::cout << "도망에 성공했습니다!\n";
        return true;
    }

    std::cout << "도망에 실패했습니다!\n";
    return false;
}

void BattleSystem::enqueueTurnActions(PlayerBattle &player, EnemyBattle &enemy, BattleActionType playerAction)
{
    BattleAction playerBattleAction;
    playerBattleAction.type = playerAction;

    BattleAction enemyBattleAction;
    enemyBattleAction.type = BattleActionType::EnemyAttack;

    /*
        힐은 우선권이 있으므로 speed 비교 없이 무조건 먼저 들어간다.
    */
    if (playerAction == BattleActionType::PlayerHeal)
    {
        actionQueue.enqueue(playerBattleAction);

        if (enemy.isAlive())
        {
            actionQueue.enqueue(enemyBattleAction);
        }

        return;
    }

    /*
        도망도 일단 플레이어 행동이므로 먼저 시도하게 둘 수 있다.
        만약 도망 실패 시 적 공격이 이어진다.
    */
    if (playerAction == BattleActionType::PlayerRun)
    {
        actionQueue.enqueue(playerBattleAction);

        if (enemy.isAlive())
        {
            actionQueue.enqueue(enemyBattleAction);
        }

        return;
    }

    /*
        포획도 플레이어가 던지는 행동이므로 먼저 시도하게 둔다.
        포획 실패 시 적 공격이 이어진다.
    */
    if (playerAction == BattleActionType::PlayerCatch)
    {
        actionQueue.enqueue(playerBattleAction);

        if (enemy.isAlive())
        {
            actionQueue.enqueue(enemyBattleAction);
        }

        return;
    }

    /*
        일반 공격은 speed 기준으로 순서 결정.
    */
    if (player.getEffectiveSpeed() >= enemy.getEffectiveSpeed())
    {
        actionQueue.enqueue(playerBattleAction);
        actionQueue.enqueue(enemyBattleAction);
    }
    else
    {
        actionQueue.enqueue(enemyBattleAction);
        actionQueue.enqueue(playerBattleAction);
    }
}

bool BattleSystem::cannotMoveByParalysis(const BattleEntity &entity) const
{
    if (entity.getStatus() != StatusCondition::Paralysis)
    {
        return false;
    }

    int roll = std::rand() % 100 + 1;

    return roll <= 25;
}

void BattleSystem::tryApplyStatusEffect(const BattleEntity &attacker, BattleEntity &defender) const
{
    if (!defender.isAlive() || defender.hasStatus())
    {
        return;
    }

    int roll = std::rand() % 100 + 1;

    if (roll > 30)
    {
        return;
    }

    if (attacker.getElement() == ElementType::Electric)
    {
        defender.setStatus(StatusCondition::Paralysis);
        std::cout << defender.getName() << "은(는) 마비되었습니다!\n";
        return;
    }

    if (attacker.getElement() == ElementType::Fire)
    {
        defender.setStatus(StatusCondition::Burn);
        std::cout << defender.getName() << "은(는) 화상을 입었습니다!\n";
        return;
    }
}

void BattleSystem::processAction(
    BattleAction action,
    PlayerBattle &player,
    EnemyBattle &enemy,
    bool &battleEnded)
{
    if (battleEnded)
    {
        return;
    }

    if (!player.isAlive() || !enemy.isAlive())
    {
        battleEnded = true;
        return;
    }

    if (action.type == BattleActionType::PlayerAttack)
    {
        if (cannotMoveByParalysis(player))
        {
            std::cout << player.getName() << "는 마비되어 움직일 수 없다!\n";
            return;
        }

        if (!player.isAlive())
        {
            return;
        }

        std::cout << player.getName() << "의 공격!\n";

        int damage = calculateDamage(player, enemy);
        enemy.takeDamage(damage);

        std::cout << enemy.getName() << "에게 " << damage << "의 피해!\n";

        if (!enemy.isAlive())
        {
            std::cout << enemy.getName() << " 쓰러졌습니다!\n";
            battleEnded = true;
        }
        else
        {
            tryApplyStatusEffect(player, enemy);
        }

        return;
    }

    if (action.type == BattleActionType::EnemyAttack)
    {
        if (cannotMoveByParalysis(enemy))
        {
            std::cout << enemy.getName() << "는 마비되어 움직일 수 없다!\n";
            return;
        }
        if (!enemy.isAlive())
        {
            return;
        }

        std::cout << enemy.getName() << "의 공격!\n";

        int damage = calculateDamage(enemy, player);
        player.takeDamage(damage);

        std::cout << player.getName() << "에게 " << damage << "의 피해!\n";

        if (!player.isAlive())
        {
            std::cout << "플레이어가 쓰러졌습니다!\n";
            battleEnded = true;
        }
        else
        {
            tryApplyStatusEffect(enemy, player);
        }

        return;
    }

    if (action.type == BattleActionType::PlayerCatch)
    {
        bool caught = tryCatch(player, enemy);

        if (caught)
        {
            battleEnded = true;
        }

        return;
    }

    if (action.type == BattleActionType::PlayerHeal)
    {
        if (player.useFullHeal())
        {
            std::cout << "풀회복약을 사용했습니다!\n";
            std::cout << "체력과 상태이상이 모두 회복되었습니다.\n";
        }
        else
        {
            std::cout << "남은 풀회복약이 없습니다!\n";
        }

        return;
    }

    if (action.type == BattleActionType::PlayerRun)
    {
        bool escaped = tryRun(player);

        if (escaped)
        {
            battleEnded = true;
        }

        return;
    }
}

void BattleSystem::startBattle(PlayerBattle &player, EnemyBattle &enemy)
{
    startBattleBgm();

    const PokemonData* pokemonData = findPokemonData(enemy.getName());
    if (pokemonData != nullptr)
    {
        printPokemonSprite(pokemonData->sprite);
    }

    std::cout << "\n야생의 " << enemy.getName() << " 등장!\n";

    while (player.isAlive() && enemy.isAlive())
    {
        std::cout << "\n====================\n";
        player.printStatus();
        enemy.printStatus();
        std::cout << "====================\n";

        std::cout << "행동을 선택하세요:\n";
        std::cout << "1. 공격\n";
        std::cout << "2. 포획\n";
        std::cout << "3. 회복\n";
        std::cout << "4. 도망\n";
        std::cout << "> ";

        std::string command;
        if (!(std::cin >> command))
        {
            std::cout << "입력이 종료되어 전투를 중단합니다.\n";
            break;
        }

        BattleActionType playerAction;

        if (command == "attack" || command == "공격" || command == "1")
        {
            playerAction = BattleActionType::PlayerAttack;
        }
        else if (command == "catch" || command == "포획" || command == "잡기" || command == "2")
        {
            playerAction = BattleActionType::PlayerCatch;
        }
        else if (command == "heal" || command == "회복" || command == "3")
        {
            playerAction = BattleActionType::PlayerHeal;
        }
        else if (command == "run" || command == "도망" || command == "4")
        {
            playerAction = BattleActionType::PlayerRun;
        }
        else
        {
            std::cout << "올바르지 않은 명령입니다.\n";
            continue;
        }

        enqueueTurnActions(player, enemy, playerAction);

        bool battleEnded = false;
        BattleAction currentAction;

        while (actionQueue.dequeue(currentAction))
        {
            processAction(currentAction, player, enemy, battleEnded);

            if (battleEnded)
            {
                break;
            }
        }

        if (battleEnded)
        {
            break;
        }

        player.applyEndTurnStatusDamage();
        enemy.applyEndTurnStatusDamage();

        if (!player.isAlive() || !enemy.isAlive())
        {
            break;
        }
    }

    stopBattleBgm();

    std::cout << "\n전투 종료.\n";

    if (player.isAlive() && !enemy.isAlive())
    {
        std::cout << enemy.getName() << "에게 승리했습니다!\n";
    }
    else if (!player.isAlive())
    {
        std::cout << "패배했습니다.\n";
    }
}

void BattleSystem::startTestBattle()
{
    startBattleByPokemonName(getDefaultPokemonData().name);
}

void BattleSystem::startBattleByPokemonName(const std::string &pokemonName)
{
    const PokemonData* pokemonData = findPokemonData(pokemonName);
    if (pokemonData == nullptr)
    {
        std::cout << "'" << pokemonName << "' 포켓몬 데이터를 찾을 수 없어 기본 포켓몬으로 전투를 시작합니다.\n";
        pokemonData = &getDefaultPokemonData();
    }

    PlayerBattle player;
    EnemyBattle enemy = createEnemyBattle(*pokemonData);
    startBattle(player, enemy);
}
