#pragma once
#ifndef GLOBALTIME_H
#define GLOBALTIME_H

#include <chrono>

class GlobalTime
{
public:
    static void Init()
    {
        startTime = std::chrono::high_resolution_clock::now();
        lastFrameTime = startTime;
        currentFrameTime = startTime;
    }

    static float GetTime()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float>(now - startTime).count();
    }

    static float GetFrameDeltaTime()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
    }
    static void UpdateLastFrameTime()
    {
        lastFrameTime = currentFrameTime;
    }
    static void UpdateCurrentFrameTime()
    {
        currentFrameTime = std::chrono::high_resolution_clock::now();
    }

private:
    static std::chrono::high_resolution_clock::time_point startTime;
    static std::chrono::high_resolution_clock::time_point lastFrameTime;
    static std::chrono::high_resolution_clock::time_point currentFrameTime;
};

#endif // GLOBALTIME_H
