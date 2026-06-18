#ifndef DUNGEON_GRAPH_H
#define DUNGEON_GRAPH_H

#include "Direction.h"
#include "Room.h"

struct Coordinate {
    int x;
    int y;
};

struct Gate {
    Coordinate entryPos;
    int targetRoomId;
    Coordinate exitPos;
};

// DungeonGraph는 게임 월드를 그래프로 표현한다. Room은 node이고, 방향 이동
// 또는 좌표 gate는 edge에 해당한다. 덕분에 이동 로직이 "if문으로 방 이름을
// 하나하나 비교"하는 방식이 아니라, connectRooms/connectRoomsByGate로 등록한
// 연결 정보를 조회하는 방식으로 동작한다.
class DungeonGraph {
private:
    static const int MAX_ROOMS = 20;
    static const int DIRECTION_COUNT = 4;
    static const int MAX_GATES_PER_ROOM = 5;

    // rooms는 roomId -> Room* 매핑이다. adjacency는 동서남북 4방향 edge를
    // 저장하고, gates는 특정 좌표를 밟았을 때 다른 방/좌표로 이동하는 edge를 저장한다.
    Room* rooms[MAX_ROOMS];
    int adjacency[MAX_ROOMS][DIRECTION_COUNT];
    Gate gates[MAX_ROOMS][MAX_GATES_PER_ROOM];
    int gateCounts[MAX_ROOMS];
    int roomCount;

    bool isValidRoomId(int roomId) const;

    DungeonGraph(const DungeonGraph& other) = delete;
    DungeonGraph& operator=(const DungeonGraph& other) = delete;

public:
    DungeonGraph();
    ~DungeonGraph();

    int addRoom(const std::string& name, const std::string& description);
    int addRoom(
        const std::string& name,
        const std::string& description,
        int encounterRate,
        int eventTriggerStep);
    bool connectRooms(int fromRoomId, Direction direction, int toRoomId, bool bidirectional = true);
    int getNeighbor(int fromRoomId, Direction direction) const;
    void connectRoomsByGate(int fromId, int fromX, int fromY, int toId, int toX, int toY);
    bool checkGate(
        int currentRoomId,
        int x,
        int y,
        int& outTargetRoomId,
        int& outTargetX,
        int& outTargetY) const;
    Room* getRoom(int roomId);
    const Room* getRoom(int roomId) const;
    int size() const;
    void printMap() const;
};

#endif
