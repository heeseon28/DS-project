#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <string>

struct GameEvent {
    std::string description;
    int scoreDelta;
    int healthDelta;

    GameEvent(const std::string& description = "", int scoreDelta = 0, int healthDelta = 0)
        : description(description), scoreDelta(scoreDelta), healthDelta(healthDelta) {}
};

#endif
