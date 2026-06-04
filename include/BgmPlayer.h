#ifndef BGM_PLAYER_H
#define BGM_PLAYER_H

#include <string>

namespace BgmPlayer {
bool playLoop(const std::string& fileName);
void stop();
}

#endif
