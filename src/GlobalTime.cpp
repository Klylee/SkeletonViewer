#include "GlobalTime.h"

std::chrono::high_resolution_clock::time_point GlobalTime::startTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point GlobalTime::lastFrameTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point GlobalTime::currentFrameTime = std::chrono::high_resolution_clock::now();