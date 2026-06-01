#ifndef DIRECTION_H
#define DIRECTION_H

#include <string>

enum class Direction {
    North = 0,
    South = 1,
    East = 2,
    West = 3,
    Invalid = 4
};

Direction parseDirection(const std::string& text);
std::string directionToString(Direction direction);
Direction oppositeDirection(Direction direction);
int directionToIndex(Direction direction);
int getDeltaX(Direction direction);
int getDeltaY(Direction direction);

#endif
