#include "Game.h"
#include "BattleSystem.h"
#include "PokemonFactory.h"
#include "ds/Sorting.h"
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
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
    Pokedex,
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
            raw.c_lflag &= static_cast<unsigned long>(~(ICANON | ECHO));
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
    if (lower == 'p')
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
    if (lower == 'o')
    {
        return makeInput(InputAction::Pokedex);
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
      eventQueue(),
      scoreTree(),
      encounters(),
      itemSymbols(),
      pokedex(),
      encounterCount(0),
      itemSymbolCount(0),
      pokedexCount(0),
      running(true)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    buildSampleWorld();
    seedScores();
}

void Game::buildSampleWorld()
{
    int pallet = dungeon.addRoom("태초마을", "모험이 시작되는 평화로운 마을이다.", 0, -1);
    int route1 = dungeon.addRoom("1번 도로", "야생 포켓몬이 모습을 드러내는 풀숲 길이다.", 30, -1);
    int viridian = dungeon.addRoom("상록시티", "체육관으로 이어지는 조용한 도시다.", 0, 20);
    int safari = dungeon.addRoom("사파리존", "희귀한 포켓몬과 아이템이 숨어 있는 넓은 구역이다.", 50, -1);

    dungeon.connectRooms(pallet, Direction::North, route1, true);
    dungeon.connectRooms(route1, Direction::North, viridian, true);
    dungeon.connectRooms(route1, Direction::East, safari, true);

    dungeon.connectRoomsByGate(pallet, 39, 20, route1, 1, 20);
    dungeon.connectRoomsByGate(route1, 0, 20, pallet, 38, 20);
    dungeon.connectRoomsByGate(route1, 39, 30, viridian, 1, 20);
    dungeon.connectRoomsByGate(viridian, 0, 20, route1, 38, 30);
    dungeon.connectRoomsByGate(route1, 39, 10, safari, 1, 20);
    dungeon.connectRoomsByGate(safari, 0, 20, route1, 38, 10);

    addRoomItem(pallet, 5, 20, Item("몬스터볼", "포켓몬을 포획할 때 사용하는 기본 도구.", 0));
    addRoomItem(pallet, 8, 18, Item("풀회복약", "체력과 상태이상을 모두 회복시키는 약.", 100));
    addRoomItem(route1, 5, 28, Item("갑옷", "방어력을 올려 줄 수 있는 장비.", 40));
    addRoomItem(route1, 15, 25, Item("풀회복약", "체력과 상태이상을 모두 회복시키는 약.", 100));
    addRoomItem(safari, 28, 18, Item("라이플", "공격력을 올려 줄 수 있는 장비.", 10));

    addEncounterSymbol(route1, 8, 22, getRandomPokemonData().name, 'M');
    addEncounterSymbol(route1, 13, 18, getRandomPokemonData().name, 'M');
    addEncounterSymbol(route1, 22, 29, getRandomPokemonData().name, 'M');
    addEncounterSymbol(route1, 31, 12, getRandomPokemonData().name, 'M');
    addEncounterSymbol(safari, 9, 9, getRandomPokemonData().name, 'M');
    addEncounterSymbol(safari, 20, 20, getRandomPokemonData().name, 'M');
    addEncounterSymbol(safari, 32, 28, getRandomPokemonData().name, 'M');
    addEncounterSymbol(viridian, 18, 18, getRandomPokemonData().name, 'M');

    eventQueue.enqueue(GameEvent("상록시티 체육관 앞에서 낯선 기척이 느껴진다.", 10, 0));
    eventQueue.enqueue(GameEvent("길가의 표지판에서 포켓몬 배틀 팁을 발견했다.", 5, 0));
}

void Game::seedScores()
{
    scoreTree.insert(ScoreRecord("Red", 80));
    scoreTree.insert(ScoreRecord("Blue", 65));
    scoreTree.insert(ScoreRecord("Green", 95));
}

void Game::addRoomItem(int roomId, int x, int y, const Item& item)
{
    Room* room = dungeon.getRoom(roomId);
    if (room == nullptr)
    {
        return;
    }

    room->addItem(item);

    if (itemSymbolCount >= MAX_ITEM_SYMBOLS)
    {
        return;
    }

    itemSymbols[itemSymbolCount] = {roomId, x, y, item.getName(), true};
    ++itemSymbolCount;
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

Game::ItemSymbol* Game::findItemAt(int roomId, int x, int y)
{
    for (int i = 0; i < itemSymbolCount; ++i)
    {
        if (itemSymbols[i].active &&
            itemSymbols[i].roomId == roomId &&
            itemSymbols[i].x == x &&
            itemSymbols[i].y == y)
        {
            return &itemSymbols[i];
        }
    }

    return nullptr;
}

const Game::ItemSymbol* Game::findItemAt(int roomId, int x, int y) const
{
    for (int i = 0; i < itemSymbolCount; ++i)
    {
        if (itemSymbols[i].active &&
            itemSymbols[i].roomId == roomId &&
            itemSymbols[i].x == x &&
            itemSymbols[i].y == y)
        {
            return &itemSymbols[i];
        }
    }

    return nullptr;
}

Game::ItemSymbol* Game::findItemSymbol(int roomId, const std::string& itemName)
{
    for (int i = 0; i < itemSymbolCount; ++i)
    {
        if (itemSymbols[i].active &&
            itemSymbols[i].roomId == roomId &&
            itemSymbols[i].itemName == itemName)
        {
            return &itemSymbols[i];
        }
    }

    return nullptr;
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
        return 'P';
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

    const ItemSymbol* item = findItemAt(roomId, x, y);
    if (item != nullptr)
    {
        return 'I';
    }

    return ' ';
}

void Game::displayMap() const
{
    clearScreen();

    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    int width = room != nullptr ? room->getWidth() : 40;
    int height = room != nullptr ? room->getHeight() : 40;
    int viewHeight = height < 20 ? height : 20;
    int startY = player.getY() - viewHeight / 2;

    if (startY < 0)
    {
        startY = 0;
    }
    if (startY + viewHeight > height)
    {
        startY = height - viewHeight;
    }
    if (startY < 0)
    {
        startY = 0;
    }

    std::cout << "현재 위치: " << (room != nullptr ? room->getName() : "알 수 없음")
              << " (" << player.getX() << ", " << player.getY() << ")\n";
    std::cout << "P 플레이어 | M 몬스터 | I 아이템 | # 게이트\n";
    std::cout << "방향키/WASD 이동 | T 줍기 | I 가방 | O 도감 | H 도움말 | Q 종료\n";

    std::cout << '+';
    for (int x = 0; x < width; ++x)
    {
        std::cout << '-';
    }
    std::cout << "+\n";

    for (int y = startY + viewHeight - 1; y >= startY; --y)
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

    std::cout << "점수: " << player.getScore()
              << " | 걸음: " << player.getSteps()
              << " | 현재 방 ID: " << player.getCurrentRoomId() << "\n";
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
        bool caught = startPokemonBattle(getDefaultPokemonData().name);
        if (caught)
        {
            recordCaughtPokemon(&getDefaultPokemonData());
        }
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Pokedex)
    {
        showPokedex();
        return;
    }
    if (input.action == InputAction::Quit)
    {
        running = false;
        return;
    }
}

void Game::printHelp() const
{
    clearScreen();
    std::cout << "[도움말]\n";
    std::cout << "방향키 또는 WASD : 40x40 맵에서 한 칸 이동\n";
    std::cout << "C 심볼 접촉       : 해당 포켓몬과 배틀 시작\n";
    std::cout << "I                 : 인벤토리 확인\n";
    std::cout << "T                 : 현재 지역의 아이템 줍기\n";
    std::cout << "U                 : 이전 위치로 되돌리기\n";
    std::cout << "P                 : 플레이어 상태 확인\n";
    std::cout << "R                 : 현재 지역 아이템 가치순 보기\n";
    std::cout << "E                 : 대기 중인 이벤트 처리\n";
    std::cout << "B                 : 테스트 배틀 시작\n";
    std::cout << "O                 : 잡은 포켓몬 도감 보기 / 도감 번호 순 정렬\n";
    std::cout << "Q                 : 게임 종료\n";
    waitForEnter();
}

void Game::showPokedex() const
{
    clearScreen();
    std::cout << "[포켓몬 도감]\n";

    if (pokedexCount == 0)
    {
        std::cout << "아직 잡은 포켓몬이 없습니다. 전투에서 포획하면 이곳에 만난 순서대로 추가됩니다.\n";
        waitForEnter();
        return;
    }

    std::cout << "1. 잡은 순서로 보기\n";
    std::cout << "2. 도감 번호 순으로 정렬해서 보기\n";
    std::cout << "0. 닫기\n";
    std::cout << "선택: ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "0")
    {
        return;
    }

    if (choice == "2")
    {
        PokedexEntry sorted[MAX_POKEDEX];
        for (int i = 0; i < pokedexCount; ++i)
        {
            sorted[i] = pokedex[i];
        }

        for (int i = 0; i < pokedexCount - 1; ++i)
        {
            int minIndex = i;
            for (int j = i + 1; j < pokedexCount; ++j)
            {
                int currentNumber = getPokemonNumber(sorted[j].data->name);
                int minNumber = getPokemonNumber(sorted[minIndex].data->name);
                if (currentNumber < minNumber)
                {
                    minIndex = j;
                }
            }

            if (minIndex != i)
            {
                PokedexEntry temp = sorted[i];
                sorted[i] = sorted[minIndex];
                sorted[minIndex] = temp;
            }
        }

        std::cout << "\n[도감 번호 순]\n";
        printPokedexEntries(sorted, pokedexCount);
        waitForEnter();
        return;
    }

    std::cout << "\n[잡은 순서]\n";
    printPokedexEntries(pokedex, pokedexCount);
    waitForEnter();
}

void Game::printPokedexEntries(const PokedexEntry* entries, int count) const
{
    for (int i = 0; i < count; ++i)
    {
        const PokemonData* data = entries[i].data;
        if (data == nullptr)
        {
            continue;
        }

        std::cout << "----------------------------------------\n";
        std::cout << "#" << (i + 1)
                  << " | 포획 순서 " << entries[i].caughtOrder
                  << " | No." << getPokemonNumber(data->name)
                  << " " << data->name << "\n";
        std::cout << "타입: " << elementToKorean(data->element)
                  << " | HP: " << data->maxHp
                  << " | 공격: " << data->attack
                  << " | 방어: " << data->defense
                  << " | 속도: " << data->speed << "\n";

        if (data->sprite != nullptr && data->sprite[0] != '\0')
        {
            std::cout << data->sprite << "\n";
        }
    }
    std::cout << "----------------------------------------\n";
}

void Game::recordCaughtPokemon(const PokemonData* data)
{
    if (data == nullptr)
    {
        return;
    }

    if (pokedexCount >= MAX_POKEDEX)
    {
        std::cout << "도감 공간이 가득 차서 " << data->name << "을(를) 기록하지 못했습니다.\n";
        return;
    }

    pokedex[pokedexCount].data = data;
    pokedex[pokedexCount].caughtOrder = pokedexCount + 1;
    ++pokedexCount;

    std::cout << data->name << "이(가) 도감에 추가되었습니다. 현재 도감: "
              << pokedexCount << "마리\n";
}

void Game::look() const
{
    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 지역 정보를 찾을 수 없습니다.\n";
        return;
    }

    room->printDescription();
}

void Game::move(Direction direction)
{
    const Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 지역 정보를 찾을 수 없습니다.\n";
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
        startEncounterBattle(*encounter);
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

void Game::removeInventoryItems(const std::string& itemName, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (!player.getInventory().removeItem(itemName))
        {
            return;
        }
    }
}

void Game::takeItem(const std::string& itemName)
{
    if (itemName.empty())
    {
        std::cout << "아이템 이름이 비어 있습니다.\n";
        return;
    }

    Room* room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 지역 정보를 찾을 수 없습니다.\n";
        return;
    }

    Item item;
    if (!room->takeItem(itemName, item))
    {
        std::cout << "'" << itemName << "' 아이템을 찾을 수 없습니다.\n";
        return;
    }

    player.getInventory().addItem(item);
    ItemSymbol* itemSymbol = findItemSymbol(player.getCurrentRoomId(), itemName);
    if (itemSymbol != nullptr)
    {
        itemSymbol->active = false;
    }
    player.addScore(item.getValue());
    std::cout << "획득: ";
    item.print();
}

void Game::promptTakeItem()
{
    ItemSymbol* itemHere = findItemAt(
        player.getCurrentRoomId(),
        player.getX(),
        player.getY());

    if (itemHere != nullptr)
    {
        takeItem(itemHere->itemName);
        waitForEnter();
        return;
    }

    look();
    std::cout << "\n현재 칸에 아이템이 없습니다. 주울 아이템 이름을 입력하세요: ";

    std::string itemName;
    std::getline(std::cin, itemName);

    takeItem(itemName);
    waitForEnter();
}

bool Game::startPokemonBattle(const std::string& pokemonName)
{
    const PokemonData* pokemonData = findPokemonData(pokemonName);
    if (pokemonData == nullptr)
    {
        std::cout << "'" << pokemonName << "' 포켓몬 데이터를 찾을 수 없어 기본 포켓몬으로 전투를 시작합니다.\n";
        pokemonData = &getDefaultPokemonData();
    }

    int monsterBalls = player.getInventory().countItem("몬스터볼");
    int fullHeals = player.getInventory().countItem("풀회복약");

    PlayerBattle playerBattle(
        "플레이어",
        100,
        15,
        8,
        10,
        ElementType::Water,
        monsterBalls,
        fullHeals);
    EnemyBattle enemy = createEnemyBattle(*pokemonData);

    BattleSystem battleSystem;
    bool caught = battleSystem.startBattle(playerBattle, enemy);

    removeInventoryItems("몬스터볼", monsterBalls - playerBattle.getMonsterBallCount());
    removeInventoryItems("풀회복약", fullHeals - playerBattle.getFullHealCount());
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return caught;
}

void Game::startEncounterBattle(EncounterSymbol& encounter)
{
    clearScreen();
    std::cout << "야생의 " << encounter.pokemonName << " 심볼과 마주쳤습니다!\n";

    bool caught = startPokemonBattle(encounter.pokemonName);
    if (caught)
    {
        recordCaughtPokemon(findPokemonData(encounter.pokemonName));
    }

    encounter.active = false;
    std::cout << "\n심볼 인카운터가 처리되었습니다.\n";
    waitForEnter();
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
        std::cout << "현재 지역 정보를 찾을 수 없습니다.\n";
        return;
    }

    int count = room->itemCount();
    if (count == 0)
    {
        std::cout << "이 지역에는 정렬할 아이템이 없습니다.\n";
        return;
    }

    Item* items = new Item[count];
    for (int i = 0; i < count; ++i)
    {
        items[i] = room->getItem(i);
    }

    sortItemsByValueDescending(items, count);

    std::cout << "현재 지역 아이템 가치순:\n";
    for (int i = 0; i < count; ++i)
    {
        std::cout << "  - ";
        items[i].print();
    }

    delete[] items;
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
