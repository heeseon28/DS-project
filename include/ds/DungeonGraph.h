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

class DungeonGraph {
private:
    static const int MAX_ROOMS = 20;
    static const int DIRECTION_COUNT = 4;
    static const int MAX_GATES_PER_ROOM = 5;

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
