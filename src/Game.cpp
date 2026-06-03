#include "Game.h"
#include "BattleSystem.h"
#include "ItemFactory.h"
#include "PokemonFactory.h"
#include "ds/Sorting.h"
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
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
    Companion,
    Equip,
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
    if (lower == 'c')
    {
        return makeInput(InputAction::Companion);
    }
    if (lower == 'g')
    {
        return makeInput(InputAction::Equip);
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
      companionIndex(-1),
      running(true),
      roomIdRoute1(-1),
      roomIdSafari(-1),
      playerElement(ElementType::Water)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    choosePlayerElement();
    buildSampleWorld();
    for (int i = 0; i < 10; ++i)
    {
        player.getInventory().addItem(createItem("몬스터볼"));
    }
    for (int i = 0; i < 2; ++i)
    {
        player.getInventory().addItem(createItem("풀회복약"));
    }
    seedScores();
}

void Game::computeElementBuffs(int& outAtk, int& outDef, int& outSpd) const
{
    outAtk = 0; outDef = 0; outSpd = 0;

    int pAtk = 0, pDef = 0, pSpd = 0;
    switch (playerElement)
    {
        case ElementType::Water:    pDef = 3; break;
        case ElementType::Fire:     pAtk = 3; break;
        case ElementType::Grass:    pAtk = 2; pDef = 2; break;
        case ElementType::Electric: pSpd = 3; break;
        case ElementType::Ground:   pDef = 4; break;
    }

    const PokemonData* companion = getCompanionPokemon();
    int cAtk = 0, cDef = 0, cSpd = 0;
    if (companion != nullptr)
    {
        switch (companion->element)
        {
            case ElementType::Water:    cDef = companion->defense / 10; break;
            case ElementType::Fire:     cAtk = companion->attack  / 10; break;
            case ElementType::Grass:    cAtk = companion->attack  / 10;
                                        cDef = companion->defense / 10; break;
            case ElementType::Electric: cSpd = companion->speed   / 10; break;
            case ElementType::Ground:   cDef = companion->defense / 10; break;
        }

        if (playerElement == companion->element)
        {
            auto doublelarge = [](int& a, int& b) {
                if (a >= b) { a *= 2; b = 0; }
                else        { b *= 2; a = 0; }
            };
            if (pAtk > 0 || cAtk > 0) doublelarge(pAtk, cAtk);
            if (pDef > 0 || cDef > 0) doublelarge(pDef, cDef);
            if (pSpd > 0 || cSpd > 0) doublelarge(pSpd, cSpd);
        }
    }

    outAtk = pAtk + cAtk;
    outDef = pDef + cDef;
    outSpd = pSpd + cSpd;
}

void Game::choosePlayerElement()
{
    std::cout << "=== 속성 선택 ===\n";
    std::cout << "1. 물    → 방어 +3\n";
    std::cout << "2. 불    → 공격 +3\n";
    std::cout << "3. 풀    → 공격 +2, 방어 +2\n";
    std::cout << "4. 전기  → 속도 +3\n";
    std::cout << "5. 땅    → 방어 +4\n";
    std::cout << "> ";

    std::string input;
    while (true)
    {
        if (!(std::cin >> input)) break;
        if      (input == "1") { playerElement = ElementType::Water;    break; }
        else if (input == "2") { playerElement = ElementType::Fire;     break; }
        else if (input == "3") { playerElement = ElementType::Grass;    break; }
        else if (input == "4") { playerElement = ElementType::Electric; break; }
        else if (input == "5") { playerElement = ElementType::Ground;   break; }
        else std::cout << "1~5 중 하나를 입력하세요: ";
    }
    std::cout << elementToKorean(playerElement) << " 속성을 선택했습니다!\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Game::buildSampleWorld()
{
    int pallet = dungeon.addRoom("태초마을", "모험이 시작되는 평화로운 마을이다.", 0, -1);
    int route1 = dungeon.addRoom("1번 도로", "야생 포켓몬이 모습을 드러내는 풀숲 길이다.", 30, -1);
        // 이벤트 트리거를 제거: 걸음 기반 자동 이벤트를 비활성화합니다.
        int viridian = dungeon.addRoom("상록시티", "체육관으로 이어지는 조용한 도시다.", 0, -1);
        int viridianGym = dungeon.addRoom("상록시티 체육관", "상록시티의 정식 체육관입니다.", 0, -1);
        int safari = dungeon.addRoom("사파리존", "희귀한 포켓몬과 아이템이 숨어 있는 넓은 구역이다.", 50, -1);
    roomIdRoute1 = route1;
    roomIdSafari = safari;

    dungeon.connectRooms(pallet, Direction::North, route1, true);
    dungeon.connectRooms(route1, Direction::North, viridian, true);
    dungeon.connectRooms(route1, Direction::East, safari, true);

    dungeon.connectRoomsByGate(pallet, 39, 20, route1, 1, 20);
    dungeon.connectRoomsByGate(route1, 0, 20, pallet, 38, 20);
    dungeon.connectRoomsByGate(route1, 39, 30, viridian, 1, 20);
    dungeon.connectRoomsByGate(viridian, 0, 20, route1, 38, 30);
    dungeon.connectRoomsByGate(route1, 39, 10, safari, 1, 20);
    dungeon.connectRoomsByGate(safari, 0, 20, route1, 38, 10);

    addRoomItem(pallet, 5, 20, createItem("몬스터볼"));
    addRoomItem(pallet, 8, 18, createItem("풀회복약"));
    addRoomItem(pallet, 0, 0, createItem("라이플"));
    addRoomItem(route1, 5, 28, createItem("갑옷"));
    addRandomRoomItem(route1, {"몬스터볼", "풀회복약"});
    addRandomRoomItem(safari, {"몬스터볼", "풀회복약"});
    addRandomRoomItem(viridian, {"몬스터볼", "풀회복약"});

    for (int i = 0; i < 5; ++i)  addRandomEncounterSymbol(route1);
    for (int i = 0; i < 10; ++i) addRandomEncounterSymbol(safari, true);
    addEncounterSymbol(viridian, 18, 18, getRandomPokemonData().name, 'M');

    // 체육관 입구: 빌딩 바로 아래에 게이트('#')가 보이도록 데코를 배치하고 게이트 연결
    int gateX = 20;

    // 상록시티 쪽에 대형 체육관 데코 배치 (사용자 요청 ASCII 스타일, 입구는 building 아래 entranceY)
    const int cx = gateX;
    Room* vroom = dungeon.getRoom(viridian);
    int visualEntranceY = -1;
    if (vroom != nullptr) {
        int roomW = vroom->getWidth();
        int innerWidth = 17; // inner width between vertical borders
        int totalWidth = innerWidth + 2; // including side borders
        int left = std::max(0, cx - totalWidth/2);
        int right = left + totalWidth - 1;
        if (right >= roomW) { right = roomW - 1; left = right - totalWidth + 1; if (left < 0) left = 0; }

        // rows - rows[0]이 y=top(화면 아래), rows[마지막]이 y=top+N(화면 위)가 되도록
        std::vector<std::string> rows;
        // entrance at rows[0] (화면 맨 아래)
        // NOTE: 원래 문자열에 '#'이 들어가면 빌딩 내부에 여러 '#'가 찍힐 수 있음.
        // 따라서 문자열에는 '#'을 넣지 않고, 입구는 아래에서 한 위치만 '#'으로 설정한다.
        rows.push_back(std::string("+") + std::string(17, '=') + "+");
        rows.push_back(std::string("+") + std::string(innerWidth, '-') + "+");
        for (int i = 0; i < 8; ++i) rows.push_back(std::string("|") + std::string(innerWidth, '.') + "|");
        rows.push_back(std::string("|") + std::string(7, '.') + "GYM" + std::string(7, '.') + "|");
        rows.push_back(std::string("/") + std::string(innerWidth, '=') + "\\");
        // roof at rows[마지막] (화면 맨 위)
        rows.push_back(std::string("+") + std::string(7, '=') + "/=\\" + std::string(7, '=') + "+");

        // 화면 아래쪽에서부터 배치
        int top = vroom->getHeight() - (int)rows.size();
        if (top < 0) top = 0;

        for (int ry = 0; ry < (int)rows.size(); ++ry) {
            int y = top + ry;
            const std::string& line = rows[ry];
            for (int j = 0; j < (int)line.size(); ++j) {
                int x = left + j;
                        if (x >= 0 && x < roomW && y >= 0 && y < vroom->getHeight()) {
                            char c = line[j];
                            if (c != ' ') vroom->setDecoration(x, y, c);
                        }
            }
        }
        // 입구 행의 실제 y 좌표 (rows[0]이 top에 배치되므로)
        visualEntranceY = top;
        if (gateX >= 0 && gateX < roomW && visualEntranceY >= 0 && visualEntranceY < vroom->getHeight()) {
            vroom->setDecoration(gateX, visualEntranceY, '#');
        }
        // 빌딩 내부에 의도치 않게 남은 다른 '#'을 제거하고, 입구 '#'만 남김
        for (int yy = top; yy <= top + (int)rows.size() - 1; ++yy) {
            for (int xx = left; xx <= left + (int)rows[0].size() - 1; ++xx) {
                if (xx < 0 || xx >= roomW || yy < 0 || yy >= vroom->getHeight()) continue;
                char dc = vroom->getDecoration(xx, yy);
                if (dc == '#') {
                    if (!(xx == gateX && yy == visualEntranceY)) {
                        // 상단의 잘못된 '#'은 '=', '-' 또는 '.'로 대체
                        // 여기서는 주변 장식과 어울리도록 '='로 바꿉니다.
                        vroom->setDecoration(xx, yy, '=');
                    }
                }
            }
        }

            // 이제 실제로 시각적 입구 위치(visualEntranceY)를 사용해 던전 게이트를 등록합니다.
            // 이전에 사용하던 gateY_hint와 달리, 여기서는 정확한 y 좌표를 매핑합니다.
            dungeon.connectRoomsByGate(viridian, gateX, visualEntranceY, viridianGym, gateX, 0);
            dungeon.connectRoomsByGate(viridianGym, gateX, 0, viridian, gateX, visualEntranceY);
            gateRecords.push_back({viridian, gateX, visualEntranceY, viridianGym, gateX, 0});
            gateRecords.push_back({viridianGym, gateX, 0, viridian, gateX, visualEntranceY});
    }

    // 상록시티 체육관(viridianGym) 룸 중앙에 체육관 NPC 심볼 'G' 추가
    Room* gymRoom = dungeon.getRoom(viridianGym);
    if (gymRoom != nullptr) {
        int gx = gymRoom->getWidth() / 2;
        int gy = gymRoom->getHeight() / 2;
        addEncounterSymbol(viridianGym, gx, gy, "GYM_LEADER", 'G');
    }

    // 디버그: 게이트 좌표와 던전의 게이트 체크 결과를 출력
    {
        int outR=-1, outX=-1, outY=-1;
        bool hasGate = dungeon.checkGate(viridian, gateX, visualEntranceY, outR, outX, outY);
        std::cout << "[DEBUG] viridian gate at (" << gateX << "," << visualEntranceY << ") -> ";
        if (hasGate) {
            std::cout << "maps to room " << outR << " at (" << outX << "," << outY << ")\n";
        } else {
            std::cout << "NO_GATE_FOUND\n";
        }
    }
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

void Game::addRandomRoomItem(int roomId, const std::vector<std::string>& pool, int spawnChancePercent)
{
    if (pool.empty()) return;
    if ((std::rand() % 100) >= spawnChancePercent) return;

    Room* room = dungeon.getRoom(roomId);
    if (room == nullptr) return;

    int x = 1 + std::rand() % (room->getWidth()  - 2);
    int y = 1 + std::rand() % (room->getHeight() - 2);
    const std::string& itemName = pool[std::rand() % pool.size()];
    addRoomItem(roomId, x, y, createItem(itemName));
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

void Game::resetRoomEncounters(int roomId, int count, bool safariRates)
{
    // 해당 방의 encounter를 배열에서 제거 (compact)
    int write = 0;
    for (int read = 0; read < encounterCount; ++read)
    {
        if (encounters[read].roomId != roomId)
            encounters[write++] = encounters[read];
    }
    encounterCount = write;

    for (int i = 0; i < count; ++i)
        addRandomEncounterSymbol(roomId, safariRates);
}

void Game::addRandomEncounterSymbol(int roomId, bool safariRates)
{
    Room* room = dungeon.getRoom(roomId);
    if (room == nullptr) return;
    int x = 1 + std::rand() % (room->getWidth()  - 2);
    int y = 1 + std::rand() % (room->getHeight() - 2);
    const PokemonData& pokemon = safariRates ? getRandomPokemonDataSafari() : getRandomPokemonData();
    addEncounterSymbol(roomId, x, y, pokemon.name, 'M');
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

    // Decorations (walls/building art) — show decoration char if present.
    const Room* decoRoom = dungeon.getRoom(roomId);
    if (decoRoom != nullptr)
    {
        char deco = decoRoom->getDecoration(x, y);
        if (deco != '\0')
        {
            return deco;
        }
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
    std::cout << "P 플레이어 | M 몬스터 | G 체육관 | I 아이템 | # 게이트\n";
    std::cout << "방향키/WASD 이동 | T 줍기 | I 가방 | G 장착 | C 동행 | O 도감 | H 도움말 | Q 종료\n";

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

    const PokemonData* companion = getCompanionPokemon();
    std::cout << "동행: "
              << (companion == nullptr ? "없음" : companion->name)
              << " | 장비 공격 +" << player.getEquipmentAttackBonus()
              << " / 방어 +" << player.getEquipmentDefenseBonus() << "\n";

    int bAtk = 0, bDef = 0, bSpd = 0;
    computeElementBuffs(bAtk, bDef, bSpd);
    std::cout << "속성: " << elementToKorean(playerElement) << " | 버프:";
    if (bAtk == 0 && bDef == 0 && bSpd == 0) std::cout << " 없음";
    if (bAtk > 0) std::cout << " 공격 +" << bAtk;
    if (bDef > 0) std::cout << " 방어 +" << bDef;
    if (bSpd > 0) std::cout << " 속도 +" << bSpd;
    std::cout << "\n";
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
        player.getInventory().print(player.getEquippedWeaponName(), player.getEquippedArmorName(), true);
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Equip)
    {
        promptEquipItem();
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
    if (input.action == InputAction::Companion)
    {
        showCompanionMenu();
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
    std::cout << "M 심볼 접촉       : 해당 포켓몬과 배틀 시작\n";
    std::cout << "I                 : 인벤토리 확인\n";
    std::cout << "G                 : 가방의 장비 아이템 장착\n";
    std::cout << "T                 : 현재 지역의 아이템 줍기\n";
    std::cout << "U                 : 이전 위치로 되돌리기\n";
    std::cout << "P                 : 플레이어 상태 확인\n";
    std::cout << "R                 : 현재 지역 아이템 가치순 보기\n";
    std::cout << "E                 : 대기 중인 이벤트 처리\n";
    std::cout << "B                 : 테스트 배틀 시작\n";
    std::cout << "O                 : 잡은 포켓몬 도감 보기 / 도감 번호 순 정렬\n";
    std::cout << "C                 : 동행 포켓몬 확인 / 변경\n";
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

void Game::showCompanionMenu()
{
    clearScreen();
    std::cout << "[동행 포켓몬]\n";

    if (pokedexCount == 0)
    {
        std::cout << "아직 동행할 포켓몬이 없습니다. 포획에 성공하면 자동으로 동행 후보에 추가됩니다.\n";
        waitForEnter();
        return;
    }

    for (int i = 0; i < pokedexCount; ++i)
    {
        const PokemonData* data = pokedex[i].data;
        if (data == nullptr)
        {
            continue;
        }

        std::cout << (i + 1) << ". " << data->name
                  << " HP " << data->maxHp
                  << " 공격 " << data->attack
                  << " 방어 " << data->defense
                  << " 속도 " << data->speed;

        if (i == companionIndex)
        {
            std::cout << " [현재 동행]";
        }

        std::cout << "\n";
    }

    std::cout << "0. 변경하지 않기\n";
    std::cout << "선택: ";

    std::string choice;
    std::getline(std::cin, choice);

    int selected = std::atoi(choice.c_str());
    if (selected == 0)
    {
        return;
    }

    if (selected < 1 || selected > pokedexCount || pokedex[selected - 1].data == nullptr)
    {
        std::cout << "올바르지 않은 선택입니다.\n";
        waitForEnter();
        return;
    }

    companionIndex = selected - 1;
    std::cout << pokedex[companionIndex].data->name << "을(를) 동행 포켓몬으로 설정했습니다.\n";
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

        // 동행 버프 표시
        std::cout << "동행 버프: ";
        switch (data->element)
        {
            case ElementType::Water:    std::cout << "방어 +" << data->defense / 10; break;
            case ElementType::Fire:     std::cout << "공격 +" << data->attack  / 10; break;
            case ElementType::Grass:    std::cout << "공격 +" << data->attack  / 10
                                                  << ", 방어 +" << data->defense / 10; break;
            case ElementType::Electric: std::cout << "속도 +" << data->speed   / 10; break;
            case ElementType::Ground:   std::cout << "방어 +" << data->defense / 10; break;
        }
        std::cout << "\n";

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

    for (int i = 0; i < pokedexCount; ++i)
    {
        if (pokedex[i].data == data)
        {
            std::cout << data->name << "은(는) 이미 도감에 있습니다.\n";
            return;
        }
    }

    if (pokedexCount >= MAX_POKEDEX)
    {
        std::cout << "도감 공간이 가득 차서 " << data->name << "을(를) 기록하지 못했습니다.\n";
        return;
    }

    int newEntryIndex = pokedexCount;
    pokedex[newEntryIndex].data = data;
    pokedex[newEntryIndex].caughtOrder = pokedexCount + 1;
    ++pokedexCount;

    player.addScore(100);
    std::cout << data->name << "이(가) 도감에 추가되었습니다. 현재 도감: "
              << pokedexCount << "마리 | 점수 +100 (현재 점수: " << player.getScore() << ")\n";

    std::cout << "C 키로 동행 포켓몬을 설정할 수 있습니다.\n";
}

const PokemonData* Game::getCompanionPokemon() const
{
    if (companionIndex < 0 || companionIndex >= pokedexCount)
    {
        return nullptr;
    }

    return pokedex[companionIndex].data;
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

    // 먼저 다음 좌표를 계산하고, 게이트가 아닌 경우 데코(벽)인지 확인
    int proposedX = player.getX() + getDeltaX(direction);
    int proposedY = player.getY() + getDeltaY(direction);

    // 경계 체크
    if (proposedX < 0 || proposedX >= room->getWidth() || proposedY < 0 || proposedY >= room->getHeight()) {
        std::cout << "벽에 부딪혔습니다. 더 이상 이동할 수 없습니다.\n";
        return;
    }

    int tgtRoom = -1, tgtX = -1, tgtY = -1;
    bool isGate = dungeon.checkGate(player.getCurrentRoomId(), proposedX, proposedY, tgtRoom, tgtX, tgtY);
    if (!isGate) {
        // fallback: check our recorded gates (in case of mismatch between visual deco and DungeonGraph)
        // Debug: if there's a '#' decoration here but no gate found, print info to help diagnose
        const Room* debugRoom = dungeon.getRoom(player.getCurrentRoomId());
        if (debugRoom != nullptr) {
            char decoHere = debugRoom->getDecoration(proposedX, proposedY);
            if (decoHere == '#') {
                std::cout << "[DEBUG] Stepping onto '#' at (" << proposedX << "," << proposedY << ") in room " << player.getCurrentRoomId() << " but checkGate returned false.\n";
                // print recorded gates for this room
                for (const auto& gr : gateRecords) {
                    if (gr.roomId == player.getCurrentRoomId()) {
                        std::cout << "[DEBUG] recorded gate: (" << gr.x << "," << gr.y << ") -> room " << gr.targetRoomId << " at (" << gr.targetX << "," << gr.targetY << ")\n";
                    }
                }
            }
        }
        for (const auto& gr : gateRecords) {
            if (gr.roomId == player.getCurrentRoomId() && gr.x == proposedX && gr.y == proposedY) {
                isGate = true;
                tgtRoom = gr.targetRoomId;
                tgtX = gr.targetX;
                tgtY = gr.targetY;
                break;
            }
        }

        if (!isGate && room->isBlocked(proposedX, proposedY)) {
            std::cout << "벽에 부딪혔습니다. 더 이상 이동할 수 없습니다.\n";
            return;
        }
    }

    if (!player.move(direction, room->getWidth(), room->getHeight()))
    {
        // player.move already prints a message for out-of-bounds; don't wait for Enter
        return;
    }

    player.saveMoveHistory(oldRoomId, oldX, oldY);
    player.addScore(1);

    int nextRoomId = -1;
    int nextX = -1;
    int nextY = -1;
    if (!dungeon.checkGate(player.getCurrentRoomId(), player.getX(), player.getY(), nextRoomId, nextX, nextY)) {
        // fallback: search recorded gates
        for (const auto& gr : gateRecords) {
            if (gr.roomId == player.getCurrentRoomId() && gr.x == player.getX() && gr.y == player.getY()) {
                nextRoomId = gr.targetRoomId;
                nextX = gr.targetX;
                nextY = gr.targetY;
                break;
            }
        }
    }

    if (nextRoomId != -1) {
        player.setCurrentRoomId(nextRoomId);
        player.setPosition(nextX, nextY);
        player.resetSteps();
        if (nextRoomId == roomIdRoute1)
            resetRoomEncounters(nextRoomId, 5, false);
        else if (nextRoomId == roomIdSafari)
            resetRoomEncounters(nextRoomId, 10, true);
        return;
    }

    EncounterSymbol* encounter = findEncounterAt(player.getCurrentRoomId(), player.getX(), player.getY());
    if (encounter != nullptr)
    {
        startEncounterBattle(*encounter);
        return;
    }

    // Removed automatic step-based event notification per user request.
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
    printItemSprite(item.getName());
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

void Game::promptEquipItem()
{
    clearScreen();
    player.getInventory().print(player.getEquippedWeaponName(), player.getEquippedArmorName(), true);
    std::cout << "\n장착/해제할 아이템 이름을 입력하세요 (취소: Enter): ";

    std::string itemName;
    std::getline(std::cin, itemName);

    if (itemName.empty())
    {
        return;
    }

    if (player.isItemEquipped(itemName))
    {
        player.unequipItem(itemName);
    }
    else
    {
        player.equipItem(itemName);
    }
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
    const PokemonData* companion = getCompanionPokemon();

    std::string battleName = "플레이어";
    int maxHp = 100;
    int attack = 15;
    int defense = 8;
    int speed = 10;
    ElementType element = playerElement;

    if (companion != nullptr)
    {
        battleName = companion->name;
        maxHp = companion->maxHp;
        attack = companion->attack;
        defense = companion->defense;
        speed = companion->speed;
        element = companion->element;
        std::cout << "[동행] " << companion->name << "이(가) 전투에 나섭니다!\n";
    }

    attack += player.getEquipmentAttackBonus();
    defense += player.getEquipmentDefenseBonus();

    if (player.getEquipmentAttackBonus() > 0 || player.getEquipmentDefenseBonus() > 0)
    {
        std::cout << "[장비] 공격 +" << player.getEquipmentAttackBonus()
                  << " / 방어 +" << player.getEquipmentDefenseBonus()
                  << " 효과가 적용됩니다.\n";
    }

    // 속성 버프 계산
    int totalBuffAtk = 0, totalBuffDef = 0, totalBuffSpd = 0;
    computeElementBuffs(totalBuffAtk, totalBuffDef, totalBuffSpd);

    attack  += totalBuffAtk;
    defense += totalBuffDef;
    speed   += totalBuffSpd;

    if (totalBuffAtk > 0 || totalBuffDef > 0 || totalBuffSpd > 0)
    {
        std::cout << "[속성 버프] " << elementToKorean(playerElement) << " 속성";
        if (companion != nullptr) std::cout << " / 동행 " << elementToKorean(companion->element) << " 속성";
        std::cout << " →";
        if (totalBuffAtk > 0) std::cout << " 공격 +" << totalBuffAtk;
        if (totalBuffDef > 0) std::cout << " 방어 +" << totalBuffDef;
        if (totalBuffSpd > 0) std::cout << " 속도 +" << totalBuffSpd;
        std::cout << "\n";
    }

    PlayerBattle playerBattle(
        battleName,
        maxHp,
        attack,
        defense,
        speed,
        element,
        monsterBalls,
        fullHeals);
    EnemyBattle enemy = createEnemyBattle(*pokemonData);

    BattleSystem battleSystem;
    bool caught = battleSystem.startBattle(playerBattle, enemy);

    if (!playerBattle.isAlive())
    {
        player.addScore(-100);
        std::cout << "패배... 점수 -100 (현재 점수: " << player.getScore() << ")\n";
    }

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
