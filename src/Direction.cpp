#include "Direction.h"

Direction parseDirection(const std::string& text) {
    if (text == "north" || text == "n" || text == "북") {
        return Direction::North;
    }
    if (text == "south" || text == "s" || text == "남") {
        return Direction::South;
    }
    if (text == "east" || text == "e" || text == "동") {
        return Direction::East;
    }
    if (text == "west" || text == "w" || text == "서") {
        return Direction::West;
    }
    return Direction::Invalid;
}

std::string directionToString(Direction direction) {
    switch (direction) {
        case Direction::North: return "북";
        case Direction::South: return "남";
        case Direction::East: return "동";
        case Direction::West: return "서";
        default: return "잘못된 방향";
    }
}

Direction oppositeDirection(Direction direction) {
    switch (direction) {
        case Direction::North: return Direction::South;
        case Direction::South: return Direction::North;
        case Direction::East: return Direction::West;
        case Direction::West: return Direction::East;
        default: return Direction::Invalid;
    }
}

int directionToIndex(Direction direction) {
    switch (direction) {
        case Direction::North: return 0;
        case Direction::South: return 1;
        case Direction::East: return 2;
        case Direction::West: return 3;
        default: return -1;
    }
}

int getDeltaX(Direction direction) {
    if (direction == Direction::East) {
        return 1;
    }
    if (direction == Direction::West) {
        return -1;
    }
    return 0;
}

int getDeltaY(Direction direction) {
    if (direction == Direction::North) {
        return 1;
    }
    if (direction == Direction::South) {
        return -1;
    }
    return 0;
}
