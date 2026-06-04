#include "BgmPlayer.h"
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <string>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
#if defined(__APPLE__)
    pid_t bgmLoopPid = -1;
#endif
    std::string currentBgmPath;

    bool fileExists(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        return file.good();
    }

    std::string findBgmFromDirectory(const std::string& startDirectory, const std::string& fileName)
    {
        std::string current = startDirectory;
        for (int depth = 0; depth < 10 && !current.empty(); ++depth)
        {
            const std::string candidates[] = {
                current + "/bgm/" + fileName,
                current + "/src/bgm/" + fileName,
                current + "/" + fileName
            };

            for (const std::string& candidate : candidates)
            {
                if (fileExists(candidate))
                {
                    return candidate;
                }
            }

            std::size_t slash = current.find_last_of('/');
            if (slash == std::string::npos || slash == 0)
            {
                break;
            }
            current = current.substr(0, slash);
        }

        return "";
    }

    std::string findBgmPath(const std::string& fileName)
    {
        const std::string candidates[] = {
            "bgm/" + fileName,
            "src/bgm/" + fileName,
            "./" + fileName,
            "../bgm/" + fileName,
            "../src/bgm/" + fileName,
            "../" + fileName
        };

        for (const std::string& candidate : candidates)
        {
            if (fileExists(candidate))
            {
                return candidate;
            }
        }

        char cwdBuffer[4096];
        if (getcwd(cwdBuffer, sizeof(cwdBuffer)) != nullptr)
        {
            std::string found = findBgmFromDirectory(cwdBuffer, fileName);
            if (!found.empty())
            {
                return found;
            }
        }

#if defined(__APPLE__)
        char executablePathBuffer[4096];
        uint32_t executablePathSize = sizeof(executablePathBuffer);
        if (_NSGetExecutablePath(executablePathBuffer, &executablePathSize) == 0)
        {
            char resolvedExecutablePath[4096];
            const char* executablePath = realpath(executablePathBuffer, resolvedExecutablePath);
            std::string executableDirectory = executablePath != nullptr
                                                  ? resolvedExecutablePath
                                                  : executablePathBuffer;

            std::size_t slash = executableDirectory.find_last_of('/');
            if (slash != std::string::npos)
            {
                executableDirectory = executableDirectory.substr(0, slash);
                std::string found = findBgmFromDirectory(executableDirectory, fileName);
                if (!found.empty())
                {
                    return found;
                }
            }
        }
#endif

        return "";
    }

#if defined(__APPLE__)
    void clearFinishedLoop()
    {
        if (bgmLoopPid <= 0)
        {
            return;
        }

        int status = 0;
        pid_t waited = waitpid(bgmLoopPid, &status, WNOHANG);
        if (waited == bgmLoopPid)
        {
            bgmLoopPid = -1;
            currentBgmPath.clear();
            return;
        }

        if (waited == -1 && errno == ECHILD)
        {
            bgmLoopPid = -1;
            currentBgmPath.clear();
        }
    }
#endif
}

namespace BgmPlayer
{
bool playLoop(const std::string& fileName)
{
    const char* disableBgm = std::getenv("DS_DISABLE_BGM");
    if (disableBgm != nullptr && std::string(disableBgm) == "1")
    {
        return false;
    }

#if defined(__APPLE__)
    clearFinishedLoop();

    std::string bgmPath = findBgmPath(fileName);
    if (bgmPath.empty())
    {
        return false;
    }

    if (bgmLoopPid > 0 && currentBgmPath == bgmPath)
    {
        return true;
    }

    stop();

    if (access("/usr/bin/afplay", X_OK) != 0)
    {
        return false;
    }

    pid_t loopPid = fork();
    if (loopPid < 0)
    {
        return false;
    }

    if (loopPid == 0)
    {
        setpgid(0, 0);

        while (true)
        {
            pid_t playerPid = fork();
            if (playerPid == 0)
            {
                execl("/usr/bin/afplay", "afplay", bgmPath.c_str(), static_cast<char*>(nullptr));
                execlp("afplay", "afplay", bgmPath.c_str(), static_cast<char*>(nullptr));
                _exit(127);
            }

            if (playerPid < 0)
            {
                _exit(1);
            }

            int status = 0;
            pid_t waited = 0;
            do
            {
                waited = waitpid(playerPid, &status, 0);
            } while (waited == -1 && errno == EINTR);

            if (waited == -1)
            {
                _exit(1);
            }

            if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
            {
                _exit(127);
            }

            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
            {
                _exit(1);
            }
        }
    }

    setpgid(loopPid, loopPid);
    bgmLoopPid = loopPid;
    currentBgmPath = bgmPath;
    return true;
#else
    (void)fileName;
    return false;
#endif
}

void stop()
{
#if defined(__APPLE__)
    if (bgmLoopPid <= 0)
    {
        currentBgmPath.clear();
        return;
    }

    kill(-bgmLoopPid, SIGTERM);

    int status = 0;
    while (waitpid(bgmLoopPid, &status, 0) == -1 && errno == EINTR)
    {
    }

    bgmLoopPid = -1;
    currentBgmPath.clear();
#endif
}
}
