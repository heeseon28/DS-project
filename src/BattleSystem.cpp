#include "BattleSystem.h" //
#include "BgmPlayer.h"
#include "PokemonFactory.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace
{
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

    bool isStrongBattlePokemon(const std::string& pokemonName)
    {
        return pokemonName == "썬더" ||
               pokemonName == "프리져" ||
               pokemonName == "프리저" ||
               pokemonName == "파이어" ||
               pokemonName == "뮤";
    }

    std::string getBattleBgmFileName(const std::string& pokemonName)
    {
        if (pokemonName == "오박사")
        {
            return "vs 오박사.mp3";
        }
        if (pokemonName == "뮤")
        {
            return "vs 뮤.mp3";
        }

        return isStrongBattlePokemon(pokemonName)
                   ? "battle_strong.mp3"
                   : "battle_bgm.mp3";
    }

    void startBattleBgm(const std::string& pokemonName)
    {
        BgmPlayer::playLoop(getBattleBgmFileName(pokemonName));
    }

    std::string readTextFile(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.good())
        {
            return "";
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string joinPath(const std::string& basePath, const std::string& relativePath)
    {
        if (basePath.empty() || basePath == ".")
        {
            return relativePath;
        }

        if (basePath[basePath.size() - 1] == '/')
        {
            return basePath + relativePath;
        }

        return basePath + "/" + relativePath;
    }

    std::string parentDirectory(const std::string& path)
    {
        if (path.empty() || path == "/")
        {
            return "";
        }

        std::string trimmed = path;
        while (trimmed.size() > 1 && trimmed[trimmed.size() - 1] == '/')
        {
            trimmed.erase(trimmed.size() - 1);
        }

        std::string::size_type slash = trimmed.find_last_of('/');
        if (slash == std::string::npos)
        {
            return "";
        }
        if (slash == 0)
        {
            return "/";
        }

        return trimmed.substr(0, slash);
    }

    std::string loadTextAssetFromDirectoryTree(
        const std::string& startDirectory,
        const std::string& relativePath)
    {
        std::string directory = startDirectory;
        for (int depth = 0; depth < 10 && !directory.empty(); ++depth)
        {
            std::string text = readTextFile(joinPath(directory, relativePath));
            if (!text.empty())
            {
                return text;
            }

            directory = parentDirectory(directory);
        }

        return "";
    }

    std::string loadTextAsset(const std::string& relativePath)
    {
        const std::string directText = readTextFile(relativePath);
        if (!directText.empty())
        {
            return directText;
        }

        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            std::string text = loadTextAssetFromDirectoryTree(cwd, relativePath);
            if (!text.empty())
            {
                return text;
            }
        }

#if defined(__APPLE__)
        uint32_t executablePathSize = PATH_MAX;
        std::vector<char> executablePath(executablePathSize, '\0');
        if (_NSGetExecutablePath(executablePath.data(), &executablePathSize) != 0)
        {
            executablePath.assign(executablePathSize + 1, '\0');
        }

        if (_NSGetExecutablePath(executablePath.data(), &executablePathSize) == 0)
        {
            char resolvedPath[PATH_MAX];
            const char *path = executablePath.data();
            if (realpath(executablePath.data(), resolvedPath) != nullptr)
            {
                path = resolvedPath;
            }

            std::string text = loadTextAssetFromDirectoryTree(
                parentDirectory(path),
                relativePath);
            if (!text.empty())
            {
                return text;
            }
        }
#endif

        return "";
    }

    void printPokemonSprite(const char *sprite)
    {
        if (sprite == nullptr || sprite[0] == '\0')
        {
            return;
        }

        std::cout << sprite << "\n";
    }

    void printProfessorSprite()
    {
        std::string sprite = loadTextAsset("data/oak_sprite.txt");
        if (!sprite.empty())
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

int PlayerBattle::getMonsterBallCount() const
{
    return monsterBalls;
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

int PlayerBattle::getFullHealCount() const
{
    return fullHeals;
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

BattleSystem::BattleSystem() : lastCatchSucceeded(false)
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

    std::cout << "몬스터볼을 던졌습니다!\n";

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

        std::cout << player.getName() << "의 공격! ";

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

        std::cout << enemy.getName() << "의 공격! ";

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
            lastCatchSucceeded = true;
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

bool BattleSystem::startBattle(PlayerBattle &player, EnemyBattle &enemy)
{
    lastCatchSucceeded = false;
    startBattleBgm(enemy.getName());

    const PokemonData *pokemonData = findPokemonData(enemy.getName());
    if (enemy.getName() == "오박사")
    {
        std::cout << "\n오박사가 승부를 걸어왔다!\n";
    }
    else
    {
        std::cout << "\n야생의 " << enemy.getName() << " 등장!\n";
    }

    while (player.isAlive() && enemy.isAlive())
    {
        if (pokemonData != nullptr)
        {
            printPokemonSprite(pokemonData->sprite);
        }
        else if (enemy.getName() == "오박사")
        {
            printProfessorSprite();
        }

        std::cout << "\n[전투] "
                  << player.getName() << " HP " << player.getHp() << "/" << player.getMaxHp()
                  << " | "
                  << enemy.getName() << " HP " << enemy.getHp() << "/" << enemy.getMaxHp()
                  << "\n";

        std::cout << "행동을 선택하세요:\n";
        std::cout << "1. 공격\n";
        std::cout << "2. 포획 (몬스터볼 " << player.getMonsterBallCount() << "개)\n";
        std::cout << "3. 회복 (풀회복약 " << player.getFullHealCount() << "개)\n";
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
            if (!player.hasMonsterBall())
            {
                std::cout << "몬스터볼이 없어 포획할 수 없습니다.\n";
                continue;
            }
            playerAction = BattleActionType::PlayerCatch;
        }
        else if (command == "heal" || command == "회복" || command == "3")
        {
            if (!player.hasFullHeal())
            {
                std::cout << "풀회복약이 없습니다.\n";
                continue;
            }
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

    BgmPlayer::stop();

    std::cout << "\n전투 종료.\n";

    if (player.isAlive() && !enemy.isAlive())
    {
        std::cout << enemy.getName() << "에게 승리했습니다!\n";
    }
    else if (!player.isAlive())
    {
        std::cout << "패배했습니다.\n";
    }

    return lastCatchSucceeded;
}

bool BattleSystem::startTestBattle()
{
    return startBattleByPokemonName(getDefaultPokemonData().name);
}

bool BattleSystem::startBattleByPokemonName(const std::string &pokemonName)
{
    const PokemonData *pokemonData = findPokemonData(pokemonName);
    if (pokemonData == nullptr)
    {
        std::cout << "'" << pokemonName << "' 포켓몬 데이터를 찾을 수 없어 기본 포켓몬으로 전투를 시작합니다.\n";
        pokemonData = &getDefaultPokemonData();
    }

    PlayerBattle player;
    EnemyBattle enemy = createEnemyBattle(*pokemonData);
    return startBattle(player, enemy);
}

bool BattleSystem::wasLastCatchSuccessful() const
{
    return lastCatchSucceeded;
}
