#include "ds/DungeonGraph.h"
#include <iostream>

DungeonGraph::DungeonGraph() : roomCount(0) {
    for (int i = 0; i < MAX_ROOMS; ++i) {
        rooms[i] = nullptr;
        gateCounts[i] = 0;
        for (int d = 0; d < DIRECTION_COUNT; ++d) {
            adjacency[i][d] = -1;
        }
    }
}

DungeonGraph::~DungeonGraph() {
    for (int i = 0; i < roomCount; ++i) {
        delete rooms[i];
        rooms[i] = nullptr;
    }
}

bool DungeonGraph::isValidRoomId(int roomId) const {
    return roomId >= 0 && roomId < roomCount && rooms[roomId] != nullptr;
}

int DungeonGraph::addRoom(const std::string& name, const std::string& description) {
    return addRoom(name, description, 0, -1);
}

int DungeonGraph::addRoom(
    const std::string& name,
    const std::string& description,
    int encounterRate,
    int eventTriggerStep) {
    if (roomCount >= MAX_ROOMS) {
        return -1;
    }

    int newId = roomCount;
    rooms[newId] = new Room(newId, name, description, encounterRate, eventTriggerStep);
    ++roomCount;
    return newId;
}

bool DungeonGraph::connectRooms(int fromRoomId, Direction direction, int toRoomId, bool bidirectional) {
    int directionIndex = directionToIndex(direction);
    if (!isValidRoomId(fromRoomId) || !isValidRoomId(toRoomId) || directionIndex == -1) {
        return false;
    }

    adjacency[fromRoomId][directionIndex] = toRoomId;

    if (bidirectional) {
        Direction opposite = oppositeDirection(direction);
        int oppositeIndex = directionToIndex(opposite);
        if (oppositeIndex != -1) {
            adjacency[toRoomId][oppositeIndex] = fromRoomId;
        }
    }

    return true;
}

int DungeonGraph::getNeighbor(int fromRoomId, Direction direction) const {
    int directionIndex = directionToIndex(direction);
    if (!isValidRoomId(fromRoomId) || directionIndex == -1) {
        return -1;
    }

    return adjacency[fromRoomId][directionIndex];
}

void DungeonGraph::connectRoomsByGate(int fromId, int fromX, int fromY, int toId, int toX, int toY) {
    if (!isValidRoomId(fromId) || !isValidRoomId(toId)) {
        return;
    }
    if (gateCounts[fromId] >= MAX_GATES_PER_ROOM) {
        return;
    }

    int index = gateCounts[fromId];
    gates[fromId][index].entryPos = {fromX, fromY};
    gates[fromId][index].targetRoomId = toId;
    gates[fromId][index].exitPos = {toX, toY};
    ++gateCounts[fromId];
}

bool DungeonGraph::checkGate(
    int currentRoomId,
    int x,
    int y,
    int& outTargetRoomId,
    int& outTargetX,
    int& outTargetY) const {
    if (!isValidRoomId(currentRoomId)) {
        return false;
    }

    for (int i = 0; i < gateCounts[currentRoomId]; ++i) {
        const Gate& gate = gates[currentRoomId][i];
        if (gate.entryPos.x == x && gate.entryPos.y == y) {
            outTargetRoomId = gate.targetRoomId;
            outTargetX = gate.exitPos.x;
            outTargetY = gate.exitPos.y;
            return true;
        }
    }

    return false;
}

Room* DungeonGraph::getRoom(int roomId) {
    if (!isValidRoomId(roomId)) {
        return nullptr;
    }
    return rooms[roomId];
}

const Room* DungeonGraph::getRoom(int roomId) const {
    if (!isValidRoomId(roomId)) {
        return nullptr;
    }
    return rooms[roomId];
}

int DungeonGraph::size() const {
    return roomCount;
}

void DungeonGraph::printMap() const {
    if (roomCount == 0) {
        std::cout << "등록된 방이 없습니다.\n";
        return;
    }

    for (int roomId = 0; roomId < roomCount; ++roomId) {
        if (rooms[roomId] == nullptr) {
            continue;
        }

        std::cout << rooms[roomId]->getName() << ":";
        bool hasExit = false;

        for (int i = 0; i < DIRECTION_COUNT; ++i) {
            int neighborId = adjacency[roomId][i];
            if (isValidRoomId(neighborId)) {
                Direction direction = static_cast<Direction>(i);
                std::cout << " " << directionToString(direction)
                          << " -> " << rooms[neighborId]->getName();
                hasExit = true;
            }
        }

        if (!hasExit) {
            std::cout << " 연결 없음";
        }
        if (gateCounts[roomId] > 0) {
            std::cout << " | 게이트 " << gateCounts[roomId] << "개";
        }

        std::cout << "\n";
    }
}
