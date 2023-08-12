#pragma once
#include <Windows.h>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <tlhelp32.h>
#define MINECRAFT_HANDLE checkMinecraftHandle()

#pragma comment(lib,"winmm.lib")

void InitializeWalk();

void EnableSafeWalk(bool shift_check);