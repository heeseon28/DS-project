#include "Game.h"
#include "BattleSystem.h"
#include "BgmPlayer.h"
#include "ItemFactory.h"
#include "PokemonFactory.h"
#include "ds/HuffmanCodec.h"
#include "ds/Sorting.h"
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <termios.h>
#include <unistd.h>

namespace
{
    // Game.cpp 내부에서만 사용하는 기본 아이템 풀과 맵 장식 데이터.
    const char *const BASIC_ITEM_POOL[] = {"몬스터볼", "풀회복약"};
    const int BASIC_ITEM_POOL_COUNT = sizeof(BASIC_ITEM_POOL) / sizeof(BASIC_ITEM_POOL[0]);

    // 상록시티 체육관 외형. 아래 행이 화면 아래쪽, 마지막 행이 지붕이다.
    const char *const VIRIDIAN_GYM_ROWS[] = {
        "+=================+",
        "+-----------------+",
        "|.................|",
        "|.................|",
        "|.................|",
        "|.................|",
        "|.................|",
        "|.................|",
        "|.................|",
        "|.................|",
        "|.......GYM.......|",
        "/=================\\",
        "+=======/=\\=======+"};
    const int VIRIDIAN_GYM_ROW_COUNT = sizeof(VIRIDIAN_GYM_ROWS) / sizeof(VIRIDIAN_GYM_ROWS[0]);
    const int VIRIDIAN_GYM_ROW_WIDTH = 19;

    // 키 입력을 게임 명령으로 정규화하기 위한 내부 액션 enum.
    enum class InputAction
    {
        None,
        Move,
        Look,
        Help,
        Inventory,
        Take,
        Undo,
        Map,
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

    // 방향 이동과 일반 명령을 하나의 입력 결과로 묶는다.
    struct GameInput
    {
        InputAction action;
        Direction direction;
    };

    // 터미널을 raw mode로 잠깐 전환해 방향키를 즉시 읽고, 소멸자에서 원복한다.
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

    // ANSI escape sequence로 화면을 지운다.
    void clearScreen()
    {
        std::cout << "\033[2J\033[H";
    }

    // 입력 처리 함수에서 간단히 GameInput을 만들기 위한 작은 helper.
    GameInput makeInput(InputAction action, Direction direction = Direction::Invalid)
    {
        GameInput input = {action, direction};
        return input;
    }

    // ElementType enum을 화면 표시용 한국어 문자열로 변환한다.
    const char *elementToKorean(ElementType element)
    {
        switch (element)
        {
        case ElementType::Water:
            return "물";
        case ElementType::Fire:
            return "불";
        case ElementType::Grass:
            return "풀";
        case ElementType::Electric:
            return "전기";
        case ElementType::Ground:
            return "땅";
        default:
            return "알 수 없음";
        }
    }

    // WASD와 단축키 문자를 내부 InputAction으로 변환한다.
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
        if (lower == 'l')
        {
            return makeInput(InputAction::Look);
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
        if (lower == 'm')
        {
            return makeInput(InputAction::Map);
        }
        if (lower == 'p')
        {
            return makeInput(InputAction::Status);
        }
        if (lower == 'v')
        {
            return makeInput(InputAction::Scores);
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

    // 터미널/비터미널 입력을 모두 처리한다. 방향키 escape sequence도 여기서 해석한다.
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

    // 상호작용 후 사용자가 내용을 읽을 시간을 주기 위한 공통 대기 함수.
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

    // 오박사 이벤트 대사를 한 줄씩 천천히 보여주기 위한 출력 helper.
    void printDelayedLine(const std::string &line)
    {
        std::cout << line << "\n";
        std::cout.flush();
        sleep(1);
    }
}

// 게임 전체 상태를 초기화하고 인트로, 속성 선택, 월드 생성, 기본 아이템 지급을 수행한다.
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
      gateRecordCount(0),
      companionIndex(-1),
      running(true),
      roomIdPallet(-1),
      roomIdRoute1(-1),
      roomIdViridian(-1),
      roomIdViridianGym(-1),
      roomIdSafari(-1),
      playerElement(ElementType::Water),
      professorCaught(false),
      mewSpawned(false)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    showIntro();
    choosePlayerElement();
    buildSampleWorld();

    // 초반 진행이 막히지 않도록 기본 포획/회복 아이템을 지급한다.
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

// 시작 화면과 intro BGM을 출력하고, Enter 입력 후 실제 게임으로 넘어간다.
void Game::showIntro()
{
    BgmPlayer::playLoop("intro.mp3");
    clearScreen();

    std::cout << R"INTRO(+=============================================+========+============================================
+===+==++===++==++===++===++==++===++===++===++===++===++===++===++===++===++===++===++===+====+====
+===+==++===++===+===++==++===++====+===++===++===++===++===++===++===++===++====+===++===+====+====
+=====*########*===*#######==+#####=########################+====#######+=#####*+###############+===
=====###########+=##########++###############################*=+#########=######################+===
=====**==---=-*#**##+=---=*#*#*=-=###--=#+--===--=##=--==--=##**#+=---=+#*#+=+###+--*#=--==--=##+===
+===+**---===-+**##*=-==--+****--=*#*--=#+---=====##---==---###*#=--==-+**#+-=*##=--##----====*#+===
+===+#*::=###::=###+::*#=::=##*::-##-.:##+..:#######:.:##=.:*##*#::+#+::+##=:=##*:.+##:.:=######+===
=====**..=###:..*#-...*#=...=##:.-#*:-###=...#######:..##=...+#+...+#+:..=#=:=##:.*###:..=#####=====
=====**..-##*:-+#*:...+*=...=**:...:***=+=......-***:..*#+--*##=...+*+...-*=....:##+=*:......=#=====
+===+**::-===-+*##-:::*#+:::=#*--:--=#*+*+:::*+++**#-:-==--=###+:::+#+:::=#+-----**+=#-::=+++*#+====
+===+#*----==-*###=---*#+---+##=---==###*+---#####*#---==--=###+---*#*---=#+---==*##+#=--+#####+====
====+#*--=########=---*#+---+##=-=#*-=###+-=#%#%##%#---########+---*#*---=#+-+#*--*###=--+#####+====
=====**--=#####+=+#+--*#+--=##*--=##=--##+--%##%##%#---#####*#%%+--+#+--+##+-+##*--+##---+#####*====
+===+**--=**++++=+#*=-=+=-+****--=*#+--+*+-=##%@%%@*---**+*@%#*=*+-===-=***+-+*#*=-=*#=--=++++**===+
+==++**--=#*++==++##+-----*###*=-=###--=#+---=*@@@#%@%=##=##=+**+=-----*###+-+###+--*#=-------##====
+===+######*+===+++#########++###############%%%%###@%###=+%++=+#*+#####*=######################====
+====######*=======+#######+=+#####=###########%%##%%@%%@#*=====**+#####+=#####*+##############*====
================================================+@@+=+@#====--======-===============================
-------------------------------------------------=##--*%#------=------------------------------------
-----....::--------:....-:--------:...:--------::.:::--=-------:::..==--------....::--------:...:-:-
:::.:..--::::..:::.:.:-::::...::.::.--::::..:::.:.:--:::..==::::.:-::.:.:.::.:.:--:::..::::::.--::.:
.....:-------:.....:::------:.....::-------......:---===-:.....::-------......:-------:.....:-------
.......::.............::.............:............:::.............:..............:............:::...
::.............::.............::............:..............::.............:.............::..........
....................................................................................................
....................................................................................................
:.:...............................................................................................::
)INTRO";

    std::cout << "\n\n"
              << "                                      PRESS ENTER TO START\n";
    std::cout.flush();

    std::string unused;
    std::getline(std::cin, unused);

    BgmPlayer::stop();
    clearScreen();
}

// 뮤 포획 후 임시 엔딩 화면과 ending BGM을 출력한다.
void Game::showEnding()
{
    BgmPlayer::stop();
    BgmPlayer::playLoop("ending.mp3");
    clearScreen();

    std::cout << R"ENDING(+=============================================+========+============================================
+===+==++===++==++===++===++==++===++===++===++===++===++===++===++===++===++===++===++===+====+====
+===+==++===++===+===++==++===++====+===++===++===++===++===++===++===++===++====+===++===+====+====
+=====*########*===*#######==+#####=########################+====#######+=#####*+###############+===
=====###########+=##########++###############################*=+#########=######################+===
=====**==---=-*#**##+=---=*#*#*=-=###--=#+--===--=##=--==--=##**#+=---=+#*#+=+###+--*#=--==--=##+===
+===+**---===-+**##*=-==--+****--=*#*--=#+---=====##---==---###*#=--==-+**#+-=*##=--##----====*#+===
+===+#*::=###::=###+::*#=::=##*::-##-.:##+..:#######:.:##=.:*##*#::+#+::+##=:=##*:.+##:.:=######+===
=====**..=###:..*#-...*#=...=##:.-#*:-###=...#######:..##=...+#+...+#+:..=#=:=##:.*###:..=#####=====
=====**..-##*:-+#*:...+*=...=**:...:***=+=......-***:..*#+--*##=...+*+...-*=....:##+=*:......=#=====
+===+**::-===-+*##-:::*#+:::=#*--:--=#*+*+:::*+++**#-:-==--=###+:::+#+:::=#+-----**+=#-::=+++*#+====
+===+#*----==-*###=---*#+---+##=---==###*+---#####*#---==--=###+---*#*---=#+---==*##+#=--+#####+====
====+#*--=########=---*#+---+##=-=#*-=###+-=#%#%##%#---########+---*#*---=#+-+#*--*###=--+#####+====
=====**--=#####+=+#+--*#+--=##*--=##=--##+--%##%##%#---#####*#%%+--+#+--+##+-+##*--+##---+#####*====
+===+**--=**++++=+#*=-=+=-+****--=*#+--+*+-=##%@%%@*---**+*@%#*=*+-===-=***+-+*#*=-=*#=--=++++**===+
+==++**--=#*++==++##+-----*###*=-=###--=#+---=*@@@#%@%=##=##=+**+=-----*###+-+###+--*#=-------##====
+===+######*+===+++#########++###############%%%%###@%###=+%++=+#*+#####*=######################====
+====######*=======+#######+=+#####=###########%%##%%@%%@#*=====**+#####+=#####*+##############*====
================================================+@@+=+@#====--======-===============================
-------------------------------------------------=##--*%#------=------------------------------------
-----....::--------:....-:--------:...:--------::.:::--=-------:::..==--------....::--------:...:-:-
:::.:..--::::..:::.:.:-::::...::.::.--::::..:::.:.:--:::..==::::.:-::.:.:.::.:.:--:::..::::::.--::.:
.....:-------:.....:::------:.....::-------......:---===-:.....::-------......:-------:.....:-------
.......::.............::.............:............:::.............:..............:............:::...
::.............::.............::............:..............::.............:.............::..........
....................................................................................................
....................................................................................................
:.:...............................................................................................::
)ENDING";

    std::cout << "\n\n"
              << "                                  THE END - PRESS ENTER\n";
    std::cout.flush();

    std::string unused;
    std::getline(std::cin, unused);

    BgmPlayer::stop();
    clearScreen();
}

// 플레이어 속성과 동행 포켓몬 속성을 합쳐 전투 버프 수치를 계산한다.
void Game::computeElementBuffs(int &outAtk, int &outDef, int &outSpd) const
{
    outAtk = 0;
    outDef = 0;
    outSpd = 0;

    int pAtk = 0, pDef = 0, pSpd = 0;
    switch (playerElement)
    {
    case ElementType::Water:
        pDef = 3;
        break;
    case ElementType::Fire:
        pAtk = 3;
        break;
    case ElementType::Grass:
        pAtk = 2;
        pDef = 2;
        break;
    case ElementType::Electric:
        pSpd = 3;
        break;
    case ElementType::Ground:
        pDef = 4;
        break;
    }

    const PokemonData *companion = getCompanionPokemon();
    int cAtk = 0, cDef = 0, cSpd = 0;
    if (companion != nullptr)
    {
        switch (companion->element)
        {
        case ElementType::Water:
            cDef = companion->defense / 10;
            break;
        case ElementType::Fire:
            cAtk = companion->attack / 10;
            break;
        case ElementType::Grass:
            cAtk = companion->attack / 10;
            cDef = companion->defense / 10;
            break;
        case ElementType::Electric:
            cSpd = companion->speed / 10;
            break;
        case ElementType::Ground:
            cDef = companion->defense / 10;
            break;
        }

        // 플레이어와 동행 타입이 같으면 같은 능력치 버프 중 더 큰 쪽을 2배로 강화한다.
        if (playerElement == companion->element)
        {
            auto doublelarge = [](int &a, int &b)
            {
                if (a >= b)
                {
                    a *= 2;
                    b = 0;
                }
                else
                {
                    b *= 2;
                    a = 0;
                }
            };
            if (pAtk > 0 || cAtk > 0)
                doublelarge(pAtk, cAtk);
            if (pDef > 0 || cDef > 0)
                doublelarge(pDef, cDef);
            if (pSpd > 0 || cSpd > 0)
                doublelarge(pSpd, cSpd);
        }
    }

    outAtk = pAtk + cAtk;
    outDef = pDef + cDef;
    outSpd = pSpd + cSpd;
}

// 게임 시작 직후 플레이어의 기본 속성을 선택한다.
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
        if (!(std::cin >> input))
            break;
        if (input == "1")
        {
            playerElement = ElementType::Water;
            break;
        }
        else if (input == "2")
        {
            playerElement = ElementType::Fire;
            break;
        }
        else if (input == "3")
        {
            playerElement = ElementType::Grass;
            break;
        }
        else if (input == "4")
        {
            playerElement = ElementType::Electric;
            break;
        }
        else if (input == "5")
        {
            playerElement = ElementType::Ground;
            break;
        }
        else
            std::cout << "1~5 중 하나를 입력하세요: ";
    }
    std::cout << elementToKorean(playerElement) << " 속성을 선택했습니다!\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 현재 방 ID에 대응하는 필드 BGM 파일명을 반환한다.
std::string Game::getFieldBgmFileName() const
{
    int currentRoomId = player.getCurrentRoomId();

    if (currentRoomId == roomIdPallet)
    {
        return "태초마을.mp3";
    }
    if (currentRoomId == roomIdRoute1)
    {
        return "1번도로.mp3";
    }
    if (currentRoomId == roomIdSafari)
    {
        return "사파리존.mp3";
    }
    if (currentRoomId == roomIdViridianGym)
    {
        return "상록시티.mp3";
    }
    if (currentRoomId == roomIdViridian)
    {
        return "상록시티.mp3";
    }

    return "";
}

// 방 이동 후 필드 BGM을 현재 지역에 맞게 갱신한다.
void Game::updateFieldBgm() const
{
    std::string bgmFileName = getFieldBgmFileName();
    if (bgmFileName.empty())
    {
        BgmPlayer::stop();
        return;
    }

    BgmPlayer::playLoop(bgmFileName);
}

// 점수, 체력, 위치, 장비 등 플레이어 상태를 한 화면에 출력한다.
void Game::showPlayerStatus() const
{
    const Room *room = dungeon.getRoom(player.getCurrentRoomId());
    std::string roomName = room != nullptr ? room->getName() : "알 수 없음";

    std::cout << "점수: " << player.getScore()
              << " | 체력: " << player.getHealth()
              << " | 위치: " << roomName
              << " (" << player.getX() << ", " << player.getY() << ")"
              << " | 걸음: " << player.getSteps() << "\n";
    std::cout << "장비: 무기 "
              << (player.getEquippedWeaponName().empty() ? "없음" : player.getEquippedWeaponName())
              << " (공격 +" << player.getEquipmentAttackBonus() << ")"
              << " | 방어구 "
              << (player.getEquippedArmorName().empty() ? "없음" : player.getEquippedArmorName())
              << " (방어 +" << player.getEquipmentDefenseBonus() << ")\n";
}

// 방, 게이트, 아이템, 인카운터, 체육관 이벤트까지 초기 월드를 구성한다.
void Game::buildSampleWorld()
{
    // addRoom은 DungeonGraph에 node를 추가하고 roomId를 돌려준다.
    // 이후 방향 edge와 gate edge는 이 id를 기준으로 연결된다.
    int pallet = dungeon.addRoom("태초마을", "모험이 시작되는 평화로운 마을이다.", 0, -1);
    int route1 = dungeon.addRoom("1번 도로", "야생 포켓몬이 모습을 드러내는 풀숲 길이다.", 30, -1);
    // 이벤트 트리거를 제거: 걸음 기반 자동 이벤트를 비활성화합니다.
    int viridian = dungeon.addRoom("상록시티", "체육관으로 이어지는 조용한 도시다.", 0, -1);
    int viridianGym = dungeon.addRoom("상록시티 체육관", "상록시티의 정식 체육관입니다.", 0, -1);
    int safari = dungeon.addRoom("사파리존", "희귀한 포켓몬과 아이템이 숨어 있는 넓은 구역이다.", 50, -1);
    roomIdPallet = pallet;
    roomIdRoute1 = route1;
    roomIdViridian = viridian;
    roomIdViridianGym = viridianGym;
    roomIdSafari = safari;

    // 일반 방향 연결은 그래프 구조에서 인접 지역을 설명할 때 사용된다.
    dungeon.connectRooms(pallet, Direction::North, route1, true);
    dungeon.connectRooms(route1, Direction::North, viridian, true);
    dungeon.connectRooms(route1, Direction::East, safari, true);

    // 좌표 기반 게이트는 40x40 맵에서 실제로 밟는 타일 이동을 담당한다.
    dungeon.connectRoomsByGate(pallet, 39, 20, route1, 1, 20);
    dungeon.connectRoomsByGate(route1, 0, 20, pallet, 38, 20);
    dungeon.connectRoomsByGate(route1, 39, 30, viridian, 1, 20);
    dungeon.connectRoomsByGate(viridian, 0, 20, route1, 38, 30);
    dungeon.connectRoomsByGate(route1, 39, 10, safari, 1, 20);
    dungeon.connectRoomsByGate(safari, 0, 20, route1, 38, 10);

    // 고정 아이템은 요구사항상 항상 같은 위치에 등장한다.
    addRoomItem(pallet, 5, 20, createItem("몬스터볼"));
    addRoomItem(pallet, 8, 18, createItem("풀회복약"));
    addRoomItem(pallet, 0, 0, createItem("라이플"));
    addRoomItem(route1, 5, 28, createItem("갑옷"));

    // 랜덤 아이템은 지역별 탐험 보상을 조금씩 다르게 만든다.
    addRandomRoomItem(route1, BASIC_ITEM_POOL, BASIC_ITEM_POOL_COUNT);
    addRandomRoomItem(safari, BASIC_ITEM_POOL, BASIC_ITEM_POOL_COUNT);
    addRandomRoomItem(viridian, BASIC_ITEM_POOL, BASIC_ITEM_POOL_COUNT);

    // Queue 자료구조 시연용 이벤트: E 키로 하나씩 dequeue되어 점수/체력에 반영된다.
    // 현재 모든 NPC/아이템 발생을 총괄하는 완전한 이벤트 시스템은 아니고,
    // FIFO 이벤트 처리 구조를 게임 상태 변화에 연결한 예시이다.
    eventQueue.enqueue(GameEvent("태초마을 게시판에서 탐험 팁을 확인했다. 점수 +10", 10, 0));
    eventQueue.enqueue(GameEvent("상비약을 정리하며 컨디션을 회복했다. 체력 +10", 0, 10));

    // 필드 몬스터 심볼은 방 진입/초기화 시 다시 배치된다.
    for (int i = 0; i < 5; ++i)
        addRandomEncounterSymbol(route1);
    for (int i = 0; i < 10; ++i)
        addRandomEncounterSymbol(safari, true);
    addEncounterSymbol(viridian, 18, 18, getRandomPokemonData().name, 'M');

    // 체육관 입구: 빌딩 바로 아래에 게이트('#')가 보이도록 데코를 배치하고 게이트 연결
    int gateX = 20;

    // 상록시티 쪽에 대형 체육관 데코 배치 (사용자 요청 ASCII 스타일, 입구는 building 아래 entranceY)
    const int cx = gateX;
    Room *vroom = dungeon.getRoom(viridian);
    int visualEntranceY = -1;
    if (vroom != nullptr)
    {
        int roomW = vroom->getWidth();
        int innerWidth = 17;             // inner width between vertical borders
        int totalWidth = innerWidth + 2; // including side borders
        int left = cx - totalWidth / 2;
        if (left < 0)
            left = 0;
        int right = left + totalWidth - 1;
        if (right >= roomW)
        {
            right = roomW - 1;
            left = right - totalWidth + 1;
            if (left < 0)
                left = 0;
        }

        // 화면 아래쪽에서부터 배치
        int top = vroom->getHeight() - VIRIDIAN_GYM_ROW_COUNT;
        if (top < 0)
            top = 0;

        for (int ry = 0; ry < VIRIDIAN_GYM_ROW_COUNT; ++ry)
        {
            int y = top + ry;
            const char *line = VIRIDIAN_GYM_ROWS[ry];
            const int lineLength = static_cast<int>(std::strlen(line));
            for (int j = 0; j < lineLength; ++j)
            {
                int x = left + j;
                if (x >= 0 && x < roomW && y >= 0 && y < vroom->getHeight())
                {
                    char c = line[j];
                    if (c != ' ')
                        vroom->setDecoration(x, y, c);
                }
            }
        }
        // 입구 행의 실제 y 좌표 (rows[0]이 top에 배치되므로)
        visualEntranceY = top;
        if (gateX >= 0 && gateX < roomW && visualEntranceY >= 0 && visualEntranceY < vroom->getHeight())
        {
            vroom->setDecoration(gateX, visualEntranceY, '#');
        }
        // 빌딩 내부에 의도치 않게 남은 다른 '#'을 제거하고, 입구 '#'만 남김
        for (int yy = top; yy <= top + VIRIDIAN_GYM_ROW_COUNT - 1; ++yy)
        {
            for (int xx = left; xx <= left + VIRIDIAN_GYM_ROW_WIDTH - 1; ++xx)
            {
                if (xx < 0 || xx >= roomW || yy < 0 || yy >= vroom->getHeight())
                    continue;
                char dc = vroom->getDecoration(xx, yy);
                if (dc == '#')
                {
                    if (!(xx == gateX && yy == visualEntranceY))
                    {
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
        addGateRecord(viridian, gateX, visualEntranceY, viridianGym, gateX, 0);
        addGateRecord(viridianGym, gateX, 0, viridian, gateX, visualEntranceY);
    }

    // 상록시티 체육관 중앙에 오박사 심볼 'T' 추가
    Room *gymRoom = dungeon.getRoom(viridianGym);
    if (gymRoom != nullptr)
    {
        int gx = gymRoom->getWidth() / 2;
        int gy = gymRoom->getHeight() / 2;
        addEncounterSymbol(viridianGym, gx, gy, "오박사", 'T');
    }
}

// BST 자료구조 시연을 위해 기본 점수 데이터를 넣는다.
void Game::seedScores()
{
    // BST 시연용 초기 점수 기록. V 키와 종료 화면에서 right-root-left 순회로
    // 내림차순 출력된다. 점수 삽입 순서에 따라 트리가 편향될 수 있다는 한계는 남아 있다.
    scoreTree.insert(ScoreRecord("Red", 80));
    scoreTree.insert(ScoreRecord("Blue", 65));
    scoreTree.insert(ScoreRecord("Green", 95));
}

// Room의 아이템 목록과 화면용 ItemSymbol 배열을 동시에 갱신한다.
void Game::addRoomItem(int roomId, int x, int y, const Item &item)
{
    Room *room = dungeon.getRoom(roomId);
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

// 지정된 아이템 풀에서 확률적으로 하나를 뽑아 현재 방의 임의 좌표에 배치한다.
void Game::addRandomRoomItem(int roomId, const char *const itemPool[], int poolCount, int spawnChancePercent)
{
    if (itemPool == nullptr || poolCount <= 0)
        return;
    if ((std::rand() % 100) >= spawnChancePercent)
        return;

    Room *room = dungeon.getRoom(roomId);
    if (room == nullptr)
        return;

    int x = 1 + std::rand() % (room->getWidth() - 2);
    int y = 1 + std::rand() % (room->getHeight() - 2);
    const char *itemName = itemPool[std::rand() % poolCount];
    addRoomItem(roomId, x, y, createItem(itemName));
}

// 그래프 게이트와 화면 장식 좌표가 어긋날 때를 대비한 보조 게이트 기록이다.
void Game::addGateRecord(int roomId, int x, int y, int targetRoomId, int targetX, int targetY)
{
    // DungeonGraph가 1차 권위이고, 이 배열은 화면 장식과 게이트 좌표를 맞추는 보조 기록이다.
    if (gateRecordCount >= MAX_GATE_RECORDS)
    {
        return;
    }

    gateRecords[gateRecordCount] = {roomId, x, y, targetRoomId, targetX, targetY};
    ++gateRecordCount;
}

// 맵에 보이는 M/T 심볼과 실제 전투 데이터를 연결한다.
void Game::addEncounterSymbol(int roomId, int x, int y, const char *pokemonName, char symbol)
{
    if (encounterCount >= MAX_ENCOUNTERS)
    {
        return;
    }

    encounters[encounterCount] = {roomId, x, y, pokemonName, symbol, true};
    ++encounterCount;
}

// 특정 방의 인카운터 심볼을 지우고 새로 배치한다.
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

// 방 크기 안의 랜덤 좌표에 포켓몬 심볼을 하나 만든다.
void Game::addRandomEncounterSymbol(int roomId, bool safariRates)
{
    Room *room = dungeon.getRoom(roomId);
    if (room == nullptr)
        return;
    int x = 1 + std::rand() % (room->getWidth() - 2);
    int y = 1 + std::rand() % (room->getHeight() - 2);
    const PokemonData &pokemon = safariRates ? getRandomPokemonDataSafari() : getRandomPokemonData();
    addEncounterSymbol(roomId, x, y, pokemon.name, 'M');
}

// 현재 좌표에 있는 활성 아이템 심볼을 찾는다.
Game::ItemSymbol *Game::findItemAt(int roomId, int x, int y)
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

// const 맥락에서 현재 좌표의 활성 아이템 심볼을 찾는다.
const Game::ItemSymbol *Game::findItemAt(int roomId, int x, int y) const
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

// 이름으로 특정 방의 아이템 심볼을 찾아 획득 처리에 사용한다.
Game::ItemSymbol *Game::findItemSymbol(int roomId, const std::string &itemName)
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

// 현재 좌표에 있는 활성 인카운터 심볼을 찾는다.
Game::EncounterSymbol *Game::findEncounterAt(int roomId, int x, int y)
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

// const 맥락에서 현재 좌표의 활성 인카운터 심볼을 찾는다.
const Game::EncounterSymbol *Game::findEncounterAt(int roomId, int x, int y) const
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

// 맵 한 칸을 어떤 문자로 그릴지 우선순위에 따라 결정한다.
char Game::tileAt(int roomId, int x, int y) const
{
    if (player.getCurrentRoomId() == roomId && player.getX() == x && player.getY() == y)
    {
        return 'P';
    }

    const EncounterSymbol *encounter = findEncounterAt(roomId, x, y);
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

    const ItemSymbol *item = findItemAt(roomId, x, y);
    if (item != nullptr)
    {
        return 'I';
    }

    // Decorations (walls/building art) — show decoration char if present.
    const Room *decoRoom = dungeon.getRoom(roomId);
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

// 현재 방의 40x40 맵 중 플레이어 주변 20줄을 콘솔에 렌더링한다.
void Game::displayMap() const
{
    clearScreen();

    const Room *room = dungeon.getRoom(player.getCurrentRoomId());
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
    std::cout << "P 플레이어 | M 몬스터 | T 오박사 | I 아이템 | # 게이트\n";
    std::cout << "방향키/WASD 이동 | L 보기 | M 지도 | T 줍기 | I 가방 | G 장착 | H 도움말 | Q 종료\n";

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
              << " | 지역: " << (room != nullptr ? room->getName() : "알 수 없음") << "\n";

    const PokemonData *companion = getCompanionPokemon();
    std::cout << "동행: "
              << (companion == nullptr ? "없음" : companion->name)
              << " | 장비 공격 +" << player.getEquipmentAttackBonus()
              << " / 방어 +" << player.getEquipmentDefenseBonus() << "\n";

    int bAtk = 0, bDef = 0, bSpd = 0;
    computeElementBuffs(bAtk, bDef, bSpd);
    std::cout << "속성: " << elementToKorean(playerElement) << " | 버프:";
    if (bAtk == 0 && bDef == 0 && bSpd == 0)
        std::cout << " 없음";
    if (bAtk > 0)
        std::cout << " 공격 +" << bAtk;
    if (bDef > 0)
        std::cout << " 방어 +" << bDef;
    if (bSpd > 0)
        std::cout << " 속도 +" << bSpd;
    std::cout << "\n";
}

// 입력 하나를 읽어 해당 게임 명령으로 분기한다.
void Game::handleInput()
{
    GameInput input = readGameInput();

    // 이동은 가장 자주 발생하므로 별도 화면 대기 없이 바로 처리한다.
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
    if (input.action == InputAction::Look)
    {
        look();
        waitForEnter();
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
    if (input.action == InputAction::Map)
    {
        clearScreen();
        dungeon.printMap();
        waitForEnter();
        return;
    }
    if (input.action == InputAction::Status)
    {
        showPlayerStatus();
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
        // 테스트용 전투는 기본 포켓몬으로 전투 시스템을 빠르게 확인하기 위한 경로다.
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

// 조작키와 각 기능을 간단히 안내한다.
void Game::printHelp() const
{
    clearScreen();
    std::cout << "[도움말]\n";
    std::cout << "방향키 또는 WASD : 40x40 맵에서 한 칸 이동\n";
    std::cout << "맵의 M 심볼 접촉  : 해당 포켓몬과 배틀 시작\n";
    std::cout << "L                 : 현재 지역 설명 보기\n";
    std::cout << "I                 : 인벤토리 확인\n";
    std::cout << "G                 : 가방의 장비 아이템 장착\n";
    std::cout << "T                 : 현재 지역의 아이템 줍기\n";
    std::cout << "U                 : 이전 위치로 되돌리기\n";
    std::cout << "M                 : 지역 연결 그래프 보기\n";
    std::cout << "P                 : 플레이어 상태 확인\n";
    std::cout << "V                 : 점수 기록 보기\n";
    std::cout << "R                 : 현재 지역 아이템 가치순 보기\n";
    std::cout << "E                 : 대기 중인 이벤트 처리\n";
    std::cout << "B                 : 테스트 배틀 시작\n";
    std::cout << "O                 : 잡은 포켓몬 도감 보기 / 도감 번호 순 정렬\n";
    std::cout << "C                 : 동행 포켓몬 확인 / 변경\n";
    std::cout << "Q                 : 게임 종료\n";
    waitForEnter();
}

// 잡은 포켓몬을 포획 순서 또는 도감 번호 순으로 출력한다.
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
        // 도감 번호 순 정렬은 STL sort 대신 직접 선택 정렬로 구현한다.
        // 원본 pokedex 배열은 포획 순서를 보존해야 하므로 임시 배열 sorted에 복사한 뒤 정렬한다.
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

// 포획한 포켓몬 중 하나를 전투 동행으로 선택한다.
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
        const PokemonData *data = pokedex[i].data;
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

// 도감 항목의 능력치, 동행 버프, Huffman 복원 스프라이트를 출력한다.
void Game::printPokedexEntries(const PokedexEntry *entries, int count) const
{
    for (int i = 0; i < count; ++i)
    {
        const PokemonData *data = entries[i].data;
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
        case ElementType::Water:
            std::cout << "방어 +" << data->defense / 10;
            break;
        case ElementType::Fire:
            std::cout << "공격 +" << data->attack / 10;
            break;
        case ElementType::Grass:
            std::cout << "공격 +" << data->attack / 10
                      << ", 방어 +" << data->defense / 10;
            break;
        case ElementType::Electric:
            std::cout << "속도 +" << data->speed / 10;
            break;
        case ElementType::Ground:
            std::cout << "방어 +" << data->defense / 10;
            break;
        }
        std::cout << "\n";

        if (data->sprite != nullptr)
        {
            std::cout << HuffmanCodec::decodeSprite(data->sprite) << "\n";
        }
    }
    std::cout << "----------------------------------------\n";
}

// 포획 성공 시 중복을 검사하고 도감 배열에 새 포켓몬을 기록한다.
void Game::recordCaughtPokemon(const PokemonData *data)
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

// 현재 선택된 동행 포켓몬 데이터를 반환한다.
const PokemonData *Game::getCompanionPokemon() const
{
    if (companionIndex < 0 || companionIndex >= pokedexCount)
    {
        return nullptr;
    }

    return pokedex[companionIndex].data;
}

// 오박사 격파 후 태초마을에 뮤 심볼을 한 번만 생성한다.
void Game::spawnMewInPallet()
{
    if (mewSpawned || roomIdPallet == -1)
    {
        return;
    }

    addEncounterSymbol(roomIdPallet, 20, 20, "뮤", 'M');
    mewSpawned = true;

    printDelayedLine("오박사: 훌륭하구나. 태초마을에 신비로운 포켓몬이 나타났다는 소식이 들려왔단다.");
    printDelayedLine("오박사: 어서 돌아가 보거라. 그 포켓몬은 분명 너를 기다리고 있을 게다.");
}

// 현재 방 설명과 방 내부 아이템/몬스터 정보를 보여준다.
void Game::look() const
{
    const Room *room = dungeon.getRoom(player.getCurrentRoomId());
    if (room == nullptr)
    {
        std::cout << "현재 지역 정보를 찾을 수 없습니다.\n";
        return;
    }

    room->printDescription();
}

// 이동, 벽 충돌, 게이트 전환, 심볼 인카운터까지 한 번의 걸음에서 처리한다.
void Game::move(Direction direction)
{
    const Room *room = dungeon.getRoom(player.getCurrentRoomId());
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
    if (proposedX < 0 || proposedX >= room->getWidth() || proposedY < 0 || proposedY >= room->getHeight())
    {
        std::cout << "벽에 부딪혔습니다. 더 이상 이동할 수 없습니다.\n";
        return;
    }

    int tgtRoom = -1, tgtX = -1, tgtY = -1;
    bool isGate = dungeon.checkGate(player.getCurrentRoomId(), proposedX, proposedY, tgtRoom, tgtX, tgtY);
    if (!isGate)
    {
        // 장식 좌표와 그래프 게이트 좌표를 맞추기 위한 보조 기록도 확인한다.
        for (int i = 0; i < gateRecordCount; ++i)
        {
            const GateRecord &gr = gateRecords[i];
            if (gr.roomId == player.getCurrentRoomId() && gr.x == proposedX && gr.y == proposedY)
            {
                isGate = true;
                tgtRoom = gr.targetRoomId;
                tgtX = gr.targetX;
                tgtY = gr.targetY;
                break;
            }
        }

        if (!isGate && room->isBlocked(proposedX, proposedY))
        {
            std::cout << "벽에 부딪혔습니다. 더 이상 이동할 수 없습니다.\n";
            return;
        }
    }

    if (!player.move(direction, room->getWidth(), room->getHeight()))
    {
        // player.move already prints a message for out-of-bounds; don't wait for Enter
        return;
    }

    // 이동 성공 후, 이동 전 위치를 Stack에 저장한다. undo 명령은 이 기록을 pop해서
    // 가장 최근 이동부터 되돌린다.
    player.saveMoveHistory(oldRoomId, oldX, oldY);
    player.addScore(1);

    int nextRoomId = -1;
    int nextX = -1;
    int nextY = -1;
    if (!dungeon.checkGate(player.getCurrentRoomId(), player.getX(), player.getY(), nextRoomId, nextX, nextY))
    {
        // DungeonGraph 조회가 실패하면 화면 장식 기반 보조 기록에서 한 번 더 찾는다.
        for (int i = 0; i < gateRecordCount; ++i)
        {
            const GateRecord &gr = gateRecords[i];
            if (gr.roomId == player.getCurrentRoomId() && gr.x == player.getX() && gr.y == player.getY())
            {
                nextRoomId = gr.targetRoomId;
                nextX = gr.targetX;
                nextY = gr.targetY;
                break;
            }
        }
    }

    if (nextRoomId != -1)
    {
        // 게이트에 올라섰다면 목적지 방과 좌표로 이동하고 방별 인카운터를 새로 배치한다.
        player.setCurrentRoomId(nextRoomId);
        player.setPosition(nextX, nextY);
        player.resetSteps();
        if (nextRoomId == roomIdRoute1)
            resetRoomEncounters(nextRoomId, 5, false);
        else if (nextRoomId == roomIdSafari)
            resetRoomEncounters(nextRoomId, 10, true);
        return;
    }

    EncounterSymbol *encounter = findEncounterAt(player.getCurrentRoomId(), player.getX(), player.getY());
    if (encounter != nullptr)
    {
        startEncounterBattle(*encounter);
        return;
    }

    // Removed automatic step-based event notification per user request.
}

// Stack에 저장된 이전 위치를 꺼내 되돌아간다.
void Game::undoMove()
{
    int roomId = -1;
    int x = -1;
    int y = -1;
    if (player.undoMove(roomId, x, y))
    {
        const Room *room = dungeon.getRoom(roomId);
        std::cout << "이전 위치로 돌아왔습니다: "
                  << (room != nullptr ? room->getName() : "알 수 없음")
                  << " (" << x << ", " << y << ")\n";
    }
    waitForEnter();
}

// 전투에서 사용한 소모품 개수만큼 인벤토리에서 제거한다.
void Game::removeInventoryItems(const std::string &itemName, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if (!player.getInventory().removeItem(itemName))
        {
            return;
        }
    }
}

// 방에서 아이템을 꺼내 인벤토리에 넣고, 화면 심볼을 비활성화한다.
void Game::takeItem(const std::string &itemName)
{
    if (itemName.empty())
    {
        std::cout << "아이템 이름이 비어 있습니다.\n";
        return;
    }

    Room *room = dungeon.getRoom(player.getCurrentRoomId());
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
    ItemSymbol *itemSymbol = findItemSymbol(player.getCurrentRoomId(), itemName);
    if (itemSymbol != nullptr)
    {
        itemSymbol->active = false;
    }
    player.addScore(item.getValue());
    printItemSprite(item.getName());
    std::cout << "획득: ";
    item.print();
}

// 현재 칸 아이템을 우선 줍고, 없으면 이름 입력 방식으로 획득을 시도한다.
void Game::promptTakeItem()
{
    ItemSymbol *itemHere = findItemAt(
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

// 인벤토리에서 장비 아이템을 장착하거나 이미 장착된 장비를 해제한다.
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

// 일반 포켓몬 전투를 생성하고, 장비/속성/동행 버프와 소모품 수량을 반영한다.
bool Game::startPokemonBattle(const std::string &pokemonName, bool resumeFieldBgmAfterBattle)
{
    const PokemonData *pokemonData = findPokemonData(pokemonName);
    if (pokemonData == nullptr)
    {
        std::cout << "'" << pokemonName << "' 포켓몬 데이터를 찾을 수 없어 기본 포켓몬으로 전투를 시작합니다.\n";
        pokemonData = &getDefaultPokemonData();
    }

    int monsterBalls = player.getInventory().countItem("몬스터볼");
    int fullHeals = player.getInventory().countItem("풀회복약");
    const PokemonData *companion = getCompanionPokemon();

    std::string battleName = "플레이어"; // 플레이어 능력치
    int maxHp = 100;
    int attack = 15;
    int defense = 8;
    int speed = 10;
    ElementType element = playerElement;

    // 동행 포켓몬이 있으면 플레이어 대신 동행 포켓몬의 기본 스탯으로 전투한다.
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

    // 장비 보너스는 포켓몬/플레이어 기본 스탯 위에 더한다.
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

    attack += totalBuffAtk;
    defense += totalBuffDef;
    speed += totalBuffSpd;

    if (totalBuffAtk > 0 || totalBuffDef > 0 || totalBuffSpd > 0)
    {
        std::cout << "[속성 버프] " << elementToKorean(playerElement) << " 속성";
        if (companion != nullptr)
            std::cout << " / 동행 " << elementToKorean(companion->element) << " 속성";
        std::cout << " →";
        if (totalBuffAtk > 0)
            std::cout << " 공격 +" << totalBuffAtk;
        if (totalBuffDef > 0)
            std::cout << " 방어 +" << totalBuffDef;
        if (totalBuffSpd > 0)
            std::cout << " 속도 +" << totalBuffSpd;
        std::cout << "\n";
    }

    // BattleSystem에 넘길 플레이어/적 전투 객체를 구성한다.
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
    if (resumeFieldBgmAfterBattle)
    {
        updateFieldBgm();
    }

    if (!playerBattle.isAlive())
    {
        player.addScore(-100);
        std::cout << "패배... 점수 -100 (현재 점수: " << player.getScore() << ")\n";
    }

    // BattleSystem 내부에서 감소한 소모품 수량을 실제 인벤토리에 반영한다.
    removeInventoryItems("몬스터볼", monsterBalls - playerBattle.getMonsterBallCount());
    removeInventoryItems("풀회복약", fullHeals - playerBattle.getFullHealCount());
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return caught;
}

// 체육관 보스인 오박사 전투를 별도 능력치와 대사로 시작한다.
bool Game::startProfessorBattle()
{
    printDelayedLine("오박사: 여기까지 왔구나.");
    printDelayedLine("오박사: 네가 자료구조의 길을 제대로 걸어왔는지 마지막으로 확인해 보마.");
    printDelayedLine("오박사: 준비가 되었다면, 이 배틀을 받아라!");
    std::cout << "\n";

    int monsterBalls = player.getInventory().countItem("몬스터볼");
    int fullHeals = player.getInventory().countItem("풀회복약");
    const PokemonData *companion = getCompanionPokemon();

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

    int totalBuffAtk = 0;
    int totalBuffDef = 0;
    int totalBuffSpd = 0;
    computeElementBuffs(totalBuffAtk, totalBuffDef, totalBuffSpd);

    attack += totalBuffAtk;
    defense += totalBuffDef;
    speed += totalBuffSpd;

    PlayerBattle playerBattle(
        battleName,
        maxHp,
        attack,
        defense,
        speed,
        element,
        monsterBalls,
        fullHeals);
    EnemyBattle enemy("오박사", 120, 45, 38, 55, ElementType::Electric); // 오박사 능력치

    // 오박사는 포획 성공 또는 HP 0 처리 모두 격파로 인정한다.
    BattleSystem battleSystem;
    bool caught = battleSystem.startBattle(playerBattle, enemy);
    updateFieldBgm();
    bool defeatedProfessor = caught || (playerBattle.isAlive() && !enemy.isAlive());

    if (!playerBattle.isAlive())
    {
        player.addScore(-100);
        std::cout << "패배... 점수 -100 (현재 점수: " << player.getScore() << ")\n";
    }

    removeInventoryItems("몬스터볼", monsterBalls - playerBattle.getMonsterBallCount());
    removeInventoryItems("풀회복약", fullHeals - playerBattle.getFullHealCount());
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return defeatedProfessor;
}

// 맵 심볼과 접촉했을 때 오박사/뮤/일반 포켓몬 이벤트를 분기한다.
void Game::startEncounterBattle(EncounterSymbol &encounter)
{
    clearScreen();
    if (std::string(encounter.pokemonName) == "오박사")
    {
        std::cout << "체육관 중앙에서 오박사가 당신을 기다리고 있습니다.\n";
        bool defeatedProfessor = startProfessorBattle();
        if (defeatedProfessor)
        {
            professorCaught = true;
            encounter.active = false;
            spawnMewInPallet();
        }
        else
        {
            printDelayedLine("오박사: 아직 끝난 것은 아니란다. 다시 준비해서 오거라.");
        }
        waitForEnter();
        return;
    }

    std::cout << "야생의 " << encounter.pokemonName << " 심볼과 마주쳤습니다!\n";

    bool isMewEncounter = std::string(encounter.pokemonName) == "뮤";
    bool caught = startPokemonBattle(encounter.pokemonName, !isMewEncounter);
    if (caught)
    {
        recordCaughtPokemon(findPokemonData(encounter.pokemonName));
        if (isMewEncounter)
        {
            encounter.active = false;
            BgmPlayer::stop();
            showEnding();
            running = false;
            return;
        }
    }

    if (!isMewEncounter || caught)
    {
        encounter.active = false;
    }
    std::cout << "\n심볼 인카운터가 처리되었습니다.\n";
    waitForEnter();
}

// Queue에 쌓인 이벤트 하나를 꺼내 점수/체력에 반영한다.
void Game::processOneEvent()
{
    GameEvent event;
    if (!eventQueue.dequeue(event))
    {
        std::cout << "대기 중인 이벤트가 없습니다.\n";
        return;
    }

    // dequeue에 성공한 이벤트만 적용한다. FIFO 구조라서 준비된 이벤트가
    // 등록된 순서 그대로 플레이어 상태에 반영된다.
    std::cout << "이벤트: " << event.description << "\n";
    player.addScore(event.scoreDelta);
    player.changeHealth(event.healthDelta);
    showPlayerStatus();
}

// BST에 저장된 점수 기록을 내림차순으로 출력한다.
void Game::showScores() const
{
    std::cout << "점수 기록:\n";
    scoreTree.printDescending();
}

// 현재 방의 아이템을 복사해 가치 기준으로 정렬해서 보여준다.
void Game::showSortedRoomItems() const
{
    const Room *room = dungeon.getRoom(player.getCurrentRoomId());
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

    // Room 내부 DynamicArray의 원본 순서는 그대로 두고, 출력용 배열만 복사해 정렬한다.
    // 따라서 방의 실제 아이템 저장 구조와 화면 표시 순서를 분리할 수 있다.
    Item *items = new Item[count];
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

// 메인 게임 루프. BGM 갱신, 화면 출력, 입력 처리를 반복하고 종료 시 점수를 기록한다.
void Game::run()
{
    while (running && player.isAlive())
    {
        updateFieldBgm();
        displayMap();
        handleInput();
    }

    BgmPlayer::stop();
    // 종료 시 현재 플레이어 점수를 BST에 삽입한 뒤 내림차순 랭킹을 출력한다.
    scoreTree.insert(ScoreRecord(player.getName(), player.getScore()));
    clearScreen();
    std::cout << "최종 상태:\n";
    showPlayerStatus();
    std::cout << "\n최종 점수 기록:\n";
    scoreTree.printDescending();
    std::cout << "게임을 종료합니다.\n";
}
