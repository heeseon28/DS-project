#include "Game.h"
#include "BattleSystem.h"
#include "ItemFactory.h"
#include "PokemonFactory.h"
#include "ds/Sorting.h"
#include <cctype>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <termios.h>
#include <unistd.h>

namespace
{
enum class InputAction
{
    None,
    Move,
    Help,
    Inventory,
    Take,
    Undo,
    Status,
    Scores,
    SortItems,
    Event,
    Battle,
    Party,
    Pokedex,
    Command,
    Quit
};

struct GameInput
{
    InputAction action;
    Direction direction;
};

class RawTerminalGuard
{
private:
    termios original;
    bool active;

public:
    RawTerminalGuard() : original(), active(false)
    {
        if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &original) == 0)
        {
            termios raw = original;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN] = 1;
            raw.c_cc[VTIME] = 0;

            if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0)
            {
                active = true;
            }
        }
    }

    ~RawTerminalGuard()
    {
        if (active)
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &original);
        }
    }
};

void clearScreen()
{
    std::cout << "\033[2J\033[H";
}

GameInput makeInput(InputAction action, Direction direction = Direction::Invalid)
{
    GameInput input = {action, direction};
    return input;
}

const char* elementToKorean(ElementType element)
{
    switch (element)
    {
        case ElementType::Water:    return "물";
        case ElementType::Fire:     return "불";
        case ElementType::Grass:    return "풀";
        case ElementType::Electric: return "전기";
        case ElementType::Ground:   return "땅";
        default:                    return "알 수 없음";
    }
}

GameInput decodeCharacter(char key)
{
    char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(key)));

    if (lower == 'w')
    {
        return makeInput(InputAction::Move, Direction::North);
    }
    if (lower == 's')
    {
        return makeInput(InputAction::Move, Direction::South);
    }
    if (lower == 'd')
    {
        return makeInput(InputAction::Move, Direction::East);
    }
    if (lower == 'a')
    {
        return makeInput(InputAction::Move, Direction::West);
    }
    if (lower == 'h')
    {
        return makeInput(InputAction::Help);
    }
    if (lower == 'i')
    {
        return makeInput(InputAction::Inventory);
    }
    if (lower == 't')
    {
        return makeInput(InputAction::Take);
    }
    if (lower == 'u')
    {
        return makeInput(InputAction::Undo);
    }
    if (lower == 'z')
    {
        return makeInput(InputAction::Status);
    }
    if (lower == 'r')
    {
        return makeInput(InputAction::SortItems);
    }
    if (lower == 'e')
    {
        return makeInput(InputAction::Event);
    }
    if (lower == 'b')
    {
        return makeInput(InputAction::Battle);
    }
    if (lower == 'p')
    {
        return makeInput(InputAction::Party);
    }
    if (lower == 'o')
    {
        return makeInput(InputAction::Pokedex);
    }
    if (lower == 'c')
    {
        return makeInput(InputAction::Command);
    }
    if (lower == 'q')
    {
        return makeInput(InputAction::Quit);
    }
    if (key == '\n' || key == '\r')
    {
        return makeInput(InputAction::None);
    }

    return makeInput(InputAction::None);
}

GameInput readGameInput()
{
    char key = 0;

    if (!isatty(STDIN_FILENO))
    {
        if (!std::cin.get(key))
        {
            return makeInput(InputAction::Quit);
        }
        return decodeCharacter(key);
    }

    RawTerminalGuard guard;
    if (read(STDIN_FILENO, &key, 1) != 1)
    {
        return makeInput(InputAction::None);
    }

    if (key == '\033')
    {
        char sequence[2] = {0, 0};
        if (read(STDIN_FILENO, &sequence[0], 1) != 1 ||
            read(STDIN_FILENO, &sequence[1], 1) != 1)
        {
            return makeInput(InputAction::None);
        }

        if (sequence[0] == '[')
        {
            if (sequence[1] == 'A')
            {
                return makeInput(InputAction::Move, Direction::North);
            }
            if (sequence[1] == 'B')
            {
                return makeInput(InputAction::Move, Direction::South);
            }
            if (sequence[1] == 'C')
            {
                return makeInput(InputAction::Move, Direction::East);
            }
            if (sequence[1] == 'D')
            {
                return makeInput(InputAction::Move, Direction::West);
            }
        }

        return makeInput(InputAction::None);
    }

    return decodeCharacter(key);
}

void waitForEnter()
{
    if (!isatty(STDIN_FILENO))
    {
        return;
    }

    std::cout << "\n계속하려면 Enter를 누르세요...";
    std::cout.flush();
    std::string unused;
    std::getline(std::cin, unused);
}
}

Game::Game()
    : dungeon(),
      player("탐험가"),
      pokemonParty(),
      eventQueue(),
      scoreTree(),
      encounters(),
      encounterCount(0),
      running(true)
{
    buildSampleWorld();
    seedScores();
}

void Game::buildSampleWorld()
{
    int palletTown = dungeon.addRoom("팔레트 타운", "오박사의 연구소가 있는 조용한 마을이다. 모험의 시작점.", 0, -1);
    int route1 = dungeon.addRoom("1번 도로", "팔레트 타운과 비리디안 시티를 잇는 풀밭 길이다.", 30, -1);
    int viridianCity = dungeon.addRoom("비리디안 시티", "큰 체육관이 버티고 있는 도시다. 트레이너의 기운이 느껴진다.", 0, 20);
    int safariZone = dungeon.addRoom("사파리 존", "희귀한 포켓몬이 사는 특별 보호구역이다.", 50, -1);

    dungeon.connectRooms(palletTown, Direction::North, route1, true);
    dungeon.connectRooms(route1, Direction::North, viridianCity, true);
    dungeon.connectRooms(route1, Direction::East, safariZone, true);

    dungeon.connectRoomsByGate(palletTown, 39, 20, route1, 1, 20);
    dungeon.connectRoomsByGate(route1, 0, 20, palletTown, 38, 20);
    dungeon.connectRoomsByGate(route1, 39, 30, viridianCity, 1, 20);
    dungeon.connectRoomsByGate(viridianCity, 0, 20, route1, 38, 30);
    dungeon.connectRoomsByGate(route1, 39, 10, safariZone, 1, 20);
    dungeon.connectRoomsByGate(safariZone, 0, 20, route1, 38, 10);

    Room* pallet = dungeon.getRoom(palletTown);
    if (pallet != nullptr)
    {
        pallet->addItem(createItem("몬스터볼"));
        pallet->addItem(createItem("풀회복약"));
    }

    Room* route = dungeon.getRoom(route1);
    if (route != nullptr)
    {
        route->addItem(createItem("갑옷"));
        route->addItem(createItem("풀회복약"));
    }

    Room* safari = dungeon.getRoom(safariZone);
    if (safari != nullptr)
    {
        safari->addItem(createItem("라이플"));
    }

    Room* viridian = dungeon.getRoom(viridianCity);
    if (viridian != nullptr)
    {
        viridian->addMonster(Monster("체육관 트레이너", 30, 8, 20));
    }

    addEncounterSymbol(route1, 12, 20, "파이리", 'C');
    addEncounterSymbol(route1, 25, 31, "파이리", 'C');
    addEncounterSymbol(safariZone, 20, 20, "파이리", 'C');

    eventQueue.enqueue(GameEvent("오박사: '세상은 포켓몬으로 가득하다, 얘야!'", 0, 0));
    eventQueue.enqueue(GameEvent("라이벌이 나타났다! 긴장감이 흐른다.", 0, -5));
    eventQueue.enqueue(GameEvent("포켓몬의 울음소리가 들렸다. 사기가 올랐다!", 10, 0));

    player.setCurrentRoomId(palletTown);
    player.setPosition(1, 20);
}

void Game::seedScores()
{
    scoreTree.insert(ScoreRecord("오박사", 95));
    scoreTree.insert(ScoreRecord("지우", 80));
    scoreTree.insert(ScoreRecord("로켓단", 60));
}

void Game::addEncounterSymbol(int roomId, int x, int y, const char* pokemonName, char symbol)
{
    if (encounterCount >= MAX_ENCOUNTERS)
    {
        return;
    }

    encounters[encounterCount] = {roomId, x, y, pokemonName, symbol, true};
    ++encounterCount;
}

Game::EncounterSymbol* Game::findEncounterAt(int roomId, int x, int y)
{
    for (int i = 0; i < encounterCount; ++i)
    {
        if (encounters[i].active &&
            encounters[i].roomId == roomId &&
            encounters[i].x == x &&
            encounters[i].y == y)
        {
            return &encounters[i];
        }
    }

    return nullptr;
}

const Game::EncounterSymbol* Game::findEncounterAt(int roomId, int x, int y) const
{
    for (int i = 0; i < encounterCount; ++i)
    {
        if (encounters[i].active &&
            encounters[i].roomId == roomId &&
            encounters[i].x == x &&
            encounters[i].y == y)
        {
            return &encounters[i];
        }
    }

    return nullptr;
}

char Game::tileAt(int roomId, int x, int y) const
{
    if (player.getCurrentRoomId() == roomId && player.getX() == x && player.getY() == y)
    {
        return '@';
    }

    const EncounterSymbol* encounter = findEncounterAt(roomId, x, y);
    if (encounter != nullptr)
    {
        return encounter->symbol;
    }

    int targetRoomId = -1;
    int targetX = -1;
    int targetY = -1;
    if (dungeon.checkGate(roomId, x, y, targetRoomId, targetX, targetY))
    {
        return '#';
    }

    return ' ';
}

void Game::displayMap() const
{
    clearScreen();

    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    int width = room != nullptr ? room->getWidth() : 40;
    int height = room != nullptr ? room->getHeight() : 40;

    std::cout << "==========================================\n";
    std::cout << "  LOCATION: " << (room != nullptr ? room->getName() : "알 수 없음") << "\n";
    std::cout << "  방향키/WASD: 이동 | I: 가방 | T: 줍기 | C키: 명령 | H: 도움말 | Q: 종료\n";
    std::cout << "  @ 플레이어 | 지도 C: 포켓몬 심볼 | # 지역 이동 게이트\n";
    std::cout << "==========================================\n";

    std::cout << '+';
    for (int x = 0; x < width; ++x)
    {
        std::cout << '-';
    }
    std::cout << "+\n";

    for (int y = height - 1; y >= 0; --y)
    {
        std::cout << '|';
        for (int x = 0; x < width; ++x)
        {
            std::cout << tileAt(player.getCurrentRoomId(), x, y);
        }
        std::cout << "|\n";
    }

    std::cout << '+';
    for (int x = 0; x < width; ++x)
    {
        std::cout << '-';
    }
    std::cout << "+\n";

    player.printStatus();
}

void Game::handleInput()
{
    GameInput input = readGameInput();

    if (input.action == InputAction::Move)
    {
        move(input.direction);
        return;
    }
    if (input.action == InputAction::Help)
    {
        printHelp();
        return;
    }
    if (input.action == InputAction::Inventory)
    {
        player.getInventory().print();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Take)
    {
        promptTakeItem();
        return;
    }
    if (input.action == InputAction::Undo)
    {
        undoMove();
        return;
    }
    if (input.action == InputAction::Status)
    {
        player.printStatus();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Scores)
    {
        showScores();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::SortItems)
    {
        showSortedRoomItems();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Event)
    {
        processOneEvent();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Battle)
    {
        startBattleByPokemonName("");
        return;
    }
    if (input.action == InputAction::Party)
    {
        pokemonParty.print();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Pokedex)
    {
        showPokedex();
        return;
    }
    if (input.action == InputAction::Command)
    {
        promptCommand();
        return;
    }
    if (input.action == InputAction::Quit)
    {
        running = false;
        return;
    }
}

void Game::promptCommand()
{
    std::cout << "\n명령어 입력 > ";

    std::string line;
    std::getline(std::cin, line);
    processCommand(line);
    waitForEnter();
}

void Game::promptTakeItem()
{
    look();
    std::cout << "\n주울 아이템 이름을 입력하세요: ";

    std::string itemName;
    std::getline(std::cin, itemName);
    takeItem(itemName);
    waitForEnter();
}

void Game::startBattleByPokemonName(const std::string& pokemonName)
{
    pokemonParty.printBuffs();

    const PokemonData* data = pokemonName.empty()
        ? &getDefaultPokemonData()
        : findPokemonData(pokemonName);

    if (data == nullptr)
    {
        std::cout << "'" << pokemonName << "' 포켓몬 데이터를 찾을 수 없습니다.\n";
        waitForEnter();
        return;
    }

    PlayerBattle playerBattle;
    EnemyBattle enemy = createEnemyBattle(*data);

    BattleSystem battleSystem;
    battleSystem.startBattle(playerBattle, enemy);

    if (playerBattle.isAlive() && enemy.isAlive())
    {
        if (!pokemonParty.has(enemy.getName()))
        {
            CaughtPokemon caught;
            caught.name = enemy.getName();
            caught.element = enemy.getElement();
            caught.attack = enemy.getAttack();
            caught.defense = enemy.getDefense();
            caught.speed = enemy.getSpeed();
            caught.pokedexNumber = getPokemonNumber(enemy.getName());
            pokemonParty.add(caught);
            std::cout << enemy.getName() << "이(가) 파티에 추가되었습니다!\n";
        }
        else
        {
            std::cout << enemy.getName() << "은(는) 이미 파티에 있습니다.\n";
        }
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    waitForEnter();
}

void Game::showPokedex()
{
    if (pokemonParty.isEmpty())
    {
        std::cout << "보유한 포켓몬이 없습니다. 전투에서 포획하세요.\n";
        waitForEnter();
        return;
    }

    std::cout << "\n[포켓몬 도감]\n";
    std::cout << "  1. 목록 보기 / 정렬 / 동행 선택\n";
    std::cout << "  2. 이름으로 검색\n";
    std::cout << "  0. 닫기\n";
    std::cout << "선택 (기본 1): ";

    std::string menuStr;
    std::getline(std::cin, menuStr);
    std::istringstream menuSS(menuStr);
    int menuChoice = 1;
    menuSS >> menuChoice;

    if (menuChoice == 0)
    {
        return;
    }

    if (menuChoice == 2)
    {
        std::cout << "\n검색할 포켓몬 이름: ";
        std::string searchName;
        std::getline(std::cin, searchName);

        const CaughtPokemon* found = pokemonParty.find(searchName);
        if (found != nullptr)
        {
            std::cout << "\n[검색 결과]\n";
            std::cout << "No." << found->pokedexNumber << " " << found->name << "\n";
            std::cout << "속성: " << elementToKorean(found->element) << "\n";
            std::cout << "공격: " << found->attack << "\n";
            std::cout << "방어: " << found->defense << "\n";
            std::cout << "속도: " << found->speed << "\n";
            std::cout << "개체값: " << (found->attack + found->defense) << "\n";
            if (found->name == pokemonParty.getSelected())
            {
                std::cout << "상태: 동행 중\n";
            }
            else
            {
                std::cout << "상태: 보유 중\n";
            }
        }
        else
        {
            int pokedexNumber = getPokemonNumber(searchName);
            if (pokedexNumber == 0)
            {
                std::cout << "'" << searchName << "'은(는) 도감 데이터에 없습니다.\n";
            }
            else
            {
                std::cout << "No." << pokedexNumber << " " << searchName
                          << "은(는) 도감 데이터에는 있지만 아직 포획하지 않았습니다.\n";
            }
        }

        waitForEnter();
        return;
    }

    std::cout << "\n정렬 기준:\n";
    std::cout << "  1. 개체값 (공격+방어)\n";
    std::cout << "  2. 공격력\n";
    std::cout << "  3. 방어력\n";
    std::cout << "  4. 포켓몬 번호\n";
    std::cout << "선택 (기본 1): ";

    std::string sortStr;
    std::getline(std::cin, sortStr);
    std::istringstream sortSS(sortStr);
    int sortChoice = 1;
    sortSS >> sortChoice;

    PokedexSortBy sortBy;
    const char* sortLabel;
    switch (sortChoice)
    {
        case 2:  sortBy = PokedexSortBy::Attack;    sortLabel = "공격력 순"; break;
        case 3:  sortBy = PokedexSortBy::Defense;   sortLabel = "방어력 순"; break;
        case 4:  sortBy = PokedexSortBy::Number;    sortLabel = "포켓몬 번호 순"; break;
        default: sortBy = PokedexSortBy::StatTotal; sortLabel = "개체값 순"; break;
    }

    int n = pokemonParty.size();
    CaughtPokemon* arr = new CaughtPokemon[n];
    pokemonParty.getSorted(sortBy, arr);

    std::cout << "\n[포켓몬 도감] " << sortLabel << "\n";
    std::cout << "----------------------------------------------\n";
    for (int i = 0; i < n; ++i)
    {
        const CaughtPokemon& p = arr[i];
        std::cout << "  " << (i + 1) << ". "
                  << "No." << p.pokedexNumber << " "
                  << p.name
                  << "  개체값:" << (p.attack + p.defense)
                  << "  공격:" << p.attack
                  << "  방어:" << p.defense;
        if (p.name == pokemonParty.getSelected())
        {
            std::cout << "  ★ 동행 중";
        }
        std::cout << "\n";
    }
    std::cout << "----------------------------------------------\n";
    std::cout << "동행할 포켓몬 번호 입력 (0: 취소): ";

    std::string choiceStr;
    std::getline(std::cin, choiceStr);
    std::istringstream ss(choiceStr);
    int choice = 0;
    ss >> choice;

    if (choice >= 1 && choice <= n)
    {
        pokemonParty.setSelected(arr[choice - 1].name);
        std::cout << arr[choice - 1].name << "을(를) 동행 포켓몬으로 선택했습니다!\n";
    }
    else if (choice != 0)
    {
        std::cout << "올바르지 않은 번호입니다.\n";
    }

    delete[] arr;
    waitForEnter();
}

void Game::printHelp() const
{
    clearScreen();
    std::cout << "[도움말]\n";
    std::cout << "도움말 출력 조건:\n";
    std::cout << "  - 맵 화면에서 H 키를 누르면 즉시 출력됩니다.\n";
    std::cout << "  - C 키로 명령 모드에 들어간 뒤 help 또는 도움말을 입력해도 출력됩니다.\n";
    std::cout << "  - 알 수 없는 텍스트 명령어를 입력하면 H 키로 확인하라는 안내가 출력됩니다.\n\n";
    std::cout << "기본 조작:\n";
    std::cout << "방향키/WASD : 40x40 맵에서 한 칸 이동\n";
    std::cout << "지도 C 접촉 : 해당 포켓몬과 배틀 시작\n";
    std::cout << "I           : 인벤토리 확인\n";
    std::cout << "T           : 현재 지역의 아이템 줍기\n";
    std::cout << "U           : 이전 위치로 되돌리기\n";
    std::cout << "Z           : 플레이어 상태 확인\n";
    std::cout << "R           : 현재 지역 아이템 가치순 보기\n";
    std::cout << "E           : 이벤트 처리\n";
    std::cout << "B           : 테스트 배틀 시작\n";
    std::cout << "P           : 포켓몬 파티 보기\n";
    std::cout << "O           : 포켓몬 도감 보기 / 이름 검색\n";
    std::cout << "C           : 기존 텍스트 명령어 입력\n";
    std::cout << "Q           : 게임 종료\n";
    std::cout << "\n주의: 지도 위의 C는 포켓몬 심볼이고, 키보드 C는 명령 모드입니다.\n";
    waitForEnter();
}

void Game::look() const
{
    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 방 정보가 올바르지 않습니다.\n";
        return;
    }

    room->printDescription();
}

void Game::move(Direction direction)
{
    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 방 정보가 올바르지 않습니다.\n";
        waitForEnter();
        return;
    }

    int oldRoomId = player.getCurrentRoomId();
    int oldX = player.getX();
    int oldY = player.getY();

    if (!player.move(direction, room->getWidth(), room->getHeight()))
    {
        waitForEnter();
        return;
    }

    player.saveMoveHistory(oldRoomId, oldX, oldY);
    player.addScore(1);

    int nextRoomId = -1;
    int nextX = -1;
    int nextY = -1;
    if (dungeon.checkGate(player.getCurrentRoomId(), player.getX(), player.getY(), nextRoomId, nextX, nextY))
    {
        player.setCurrentRoomId(nextRoomId);
        player.setPosition(nextX, nextY);
        player.resetSteps();
        return;
    }

    EncounterSymbol* encounter = findEncounterAt(player.getCurrentRoomId(), player.getX(), player.getY());
    if (encounter != nullptr)
    {
        clearScreen();
        std::cout << "야생의 " << encounter->pokemonName << " 심볼과 마주쳤습니다!\n";
        startBattleByPokemonName(encounter->pokemonName);
        encounter->active = false;
        return;
    }

    const Room* currentRoom = dungeon.getRoom(player.getCurrentRoomId());
    if (currentRoom != nullptr &&
        currentRoom->getEventTriggerStep() != -1 &&
        player.getSteps() >= currentRoom->getEventTriggerStep())
    {
        std::cout << "\n[이벤트] 이 지역에서 충분히 이동했습니다. E 키로 이벤트를 확인할 수 있습니다.\n";
        waitForEnter();
    }
}

void Game::undoMove()
{
    int roomId = -1;
    int x = -1;
    int y = -1;
    if (player.undoMove(roomId, x, y))
    {
        std::cout << "이전 위치로 돌아왔습니다: Room " << roomId << " (" << x << ", " << y << ")\n";
    }
    waitForEnter();
}

void Game::takeItem(const std::string& itemName)
{
    if (itemName.empty())
    {
        std::cout << "무엇을 주울까요? 예: 몬스터볼\n";
        return;
    }

    Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 방 정보가 올바르지 않습니다.\n";
        return;
    }

    Item item;
    if (!room->takeItem(itemName, item))
    {
        std::cout << "여기에는 '" << itemName << "' 아이템이 없습니다.\n";
        return;
    }

    printItemSprite(item.getName());
    player.getInventory().addItem(item);
    player.addScore(item.getValue());
    std::cout << "획득: " << item.getName() << "\n";
}

void Game::processOneEvent()
{
    GameEvent event;
    if (!eventQueue.dequeue(event))
    {
        std::cout << "대기 중인 이벤트가 없습니다.\n";
        return;
    }

    std::cout << "이벤트: " << event.description << "\n";
    player.addScore(event.scoreDelta);
    player.changeHealth(event.healthDelta);
    player.printStatus();
}

void Game::showScores() const
{
    std::cout << "점수 기록:\n";
    scoreTree.printDescending();
}

void Game::showSortedRoomItems() const
{
    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 방 정보가 올바르지 않습니다.\n";
        return;
    }

    int count = room->itemCount();
    if (count == 0)
    {
        std::cout << "이 방에는 정렬할 아이템이 없습니다.\n";
        return;
    }

    Item* copy = new Item[count];
    for (int i = 0; i < count; ++i)
    {
        copy[i] = room->getItem(i);
    }

    sortItemsByValueDescending(copy, count);

    std::cout << "가치가 높은 순서의 아이템:\n";
    for (int i = 0; i < count; ++i)
    {
        std::cout << "  - ";
        copy[i].print();
    }

    delete[] copy;
}

void Game::processCommand(const std::string& line)
{
    std::istringstream input(line);
    std::string command;
    input >> command;

    if (command == "help" || command == "도움말")
    {
        printHelp();
    }
    else if (command == "look" || command == "보기")
    {
        look();
    }
    else if (command == "move" || command == "이동")
    {
        std::string directionText;
        input >> directionText;
        move(parseDirection(directionText));
    }
    else if (command == "take" || command == "줍기")
    {
        std::string itemName;
        std::getline(input >> std::ws, itemName);
        takeItem(itemName);
    }
    else if (command == "inventory" || command == "가방")
    {
        player.getInventory().print();
    }
    else if (command == "undo" || command == "되돌리기")
    {
        undoMove();
    }
    else if (command == "event" || command == "이벤트")
    {
        processOneEvent();
    }
    else if (command == "scores" || command == "점수")
    {
        showScores();
    }
    else if (command == "sortitems" || command == "정렬")
    {
        showSortedRoomItems();
    }
    else if (command == "map" || command == "지도")
    {
        dungeon.printMap();
    }
    else if (command == "status" || command == "상태")
    {
        player.printStatus();
    }
    else if (command == "quit" || command == "종료")
    {
        running = false;
    }
    else if (command == "battle" || command == "전투")
    {
        std::string pokemonName;
        input >> pokemonName;
        startBattleByPokemonName(pokemonName);
    }
    else if (command == "party" || command == "파티")
    {
        pokemonParty.print();
    }
    else if (command == "pokedex" || command == "도감")
    {
        showPokedex();
    }
    else if (!command.empty())
    {
        std::cout << "알 수 없는 명령어입니다. H 키로 도움말을 확인하세요.\n";
    }
}

void Game::run()
{
    while (running && player.isAlive())
    {
        displayMap();
        handleInput();
    }

    scoreTree.insert(ScoreRecord(player.getName(), player.getScore()));
    clearScreen();
    std::cout << "최종 상태:\n";
    player.printStatus();
    std::cout << "\n최종 점수 기록:\n";
    scoreTree.printDescending();
    std::cout << "게임을 종료합니다.\n";
}
