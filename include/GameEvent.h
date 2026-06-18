#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <string>

// GameEvent는 Queue에 넣어 두었다가 E 명령으로 하나씩 처리되는 간단한 이벤트이다.
// description은 화면에 보여줄 문장이고, scoreDelta/healthDelta는 이벤트가
// 플레이어 점수와 체력에 주는 변화를 의미한다.
struct GameEvent {
    std::string description;
    int scoreDelta;
    int healthDelta;

    GameEvent(const std::string& description = "", int scoreDelta = 0, int healthDelta = 0)
        : description(description), scoreDelta(scoreDelta), healthDelta(healthDelta) {}
};

#endif
