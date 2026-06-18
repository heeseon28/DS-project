#include "ds/DungeonGraph.h"
#include <iostream>

DungeonGraph::DungeonGraph() : roomCount(0) {
    // adjacency의 -1은 해당 방향으로 연결된 방이 없다는 뜻이다.
    // gateCounts는 각 방에 등록된 좌표형 gate 개수를 따로 관리한다.
    for (int i = 0; i < MAX_ROOMS; ++i) {
        rooms[i] = nullptr;
        gateCounts[i] = 0;
        for (int d = 0; d < DIRECTION_COUNT; ++d) {
            adjacency[i][d] = -1;
        }
    }
}

DungeonGraph::~DungeonGraph() {
    // addRoom에서 new로 만든 Room 객체를 Graph가 소유하므로 소멸자에서 해제한다.
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

    // roomId는 배열 인덱스와 같게 만든다. 이렇게 하면 현재 방 id로 Room*을
    // O(1)에 찾을 수 있다.
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

    // 방향은 네 개로 고정되어 있으므로 adjacency[from][direction] 조회는 O(1)이다.
    adjacency[fromRoomId][directionIndex] = toRoomId;

    if (bidirectional) {
        // 양방향 통로라면 반대편 방에도 reverse edge를 자동으로 등록한다.
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

    // gate는 "방향"이 아니라 특정 좌표를 밟는 이벤트형 edge이다.
    // 예: Route 1의 특정 입구 좌표 -> Viridian City의 시작 좌표.
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

    // 현재 방에 등록된 gate만 확인한다. 방마다 gate 수를 작게 제한했기 때문에
    // 선형 탐색이어도 이동 처리 비용이 안정적이다.
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
        std::cout << "등록된 지역이 없습니다.\n";
        return;
    }

    for (int roomId = 0; roomId < roomCount; ++roomId) {
        if (rooms[roomId] == nullptr) {
            continue;
        }

        std::cout << rooms[roomId]->getName() << ":";
        bool hasConnection = false;

        for (int d = 0; d < DIRECTION_COUNT; ++d) {
            int neighborId = adjacency[roomId][d];
            if (isValidRoomId(neighborId)) {
                Direction direction = static_cast<Direction>(d);
                std::cout << " " << directionToString(direction)
                          << " -> " << rooms[neighborId]->getName();
                hasConnection = true;
            }
        }

        if (gateCounts[roomId] > 0) {
            std::cout << " | 게이트 " << gateCounts[roomId] << "개";
            hasConnection = true;
        }

        if (!hasConnection) {
            std::cout << " 연결 없음";
        }

        std::cout << "\n";
    }
}
