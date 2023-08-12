#include "SafeWalk.h"

//Set to Multibyte
HANDLE checkMinecraftHandle() {
    HANDLE pHandle = NULL;

    while (!pHandle) {

        if (HWND hwnd = FindWindow("LWJGL", NULL)) {

            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);
            pHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return pHandle;
}

bool isMinecraftAttached() {

    char wnd_title[256];
    HWND hwnd = GetForegroundWindow();

    GetWindowTextA(hwnd, wnd_title, sizeof(wnd_title));

    if (std::string(wnd_title).find("Minecraft") != std::string::npos ||
        std::string(wnd_title).find("Badlion") != std::string::npos || //Better to use on badlion to prevent bac detections
        std::string(wnd_title).find("Feather") != std::string::npos ||
        std::string(wnd_title).find("Lunar") != std::string::npos) return true;
    else {
        HWND mc_hwnd = FindWindow("LWJGL", NULL);

        return (mc_hwnd == hwnd);
    }
}

struct {
    float coordX = 0.0f;
    float coordZ = 0.0f;
    bool safewalk_enabler = NULL;
    int sneak_dealy = 150;
    DWORD processId = NULL;
    uint64_t z_offset = 200;
    uint64_t x_offset = 192;
    uint64_t base_adress_offset = 377608;
    std::string dll_name = "OpenAL64.dll";
}SafeWalkStruct;

void InitializeWalk() {
    HWND hwnd = FindWindow("LWJGL", NULL);

    GetWindowThreadProcessId(hwnd, &SafeWalkStruct.processId);
}

uint64_t GetModuleBaseAddress(uint64_t processId, const std::string& moduleName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry = {};
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(snapshot, &moduleEntry)) {
            do {
                if (_stricmp(moduleEntry.szModule, moduleName.c_str()) == 0) {
                    CloseHandle(snapshot);
                    return (uint64_t)moduleEntry.modBaseAddr;
                }
            } while (Module32Next(snapshot, &moduleEntry));
        }
    }

    CloseHandle(snapshot);
    return 0;
}

void ReadCoordinates(uint64_t offsets, float& point, bool& loop) {

    uint64_t baseAddress = GetModuleBaseAddress(SafeWalkStruct.processId, SafeWalkStruct.dll_name.c_str()) + SafeWalkStruct.base_adress_offset;

    uint64_t pointedAddress = baseAddress;

    ReadProcessMemory(MINECRAFT_HANDLE, (void*)pointedAddress, &pointedAddress, sizeof(pointedAddress), NULL);
    pointedAddress += offsets;

    while (loop)  ReadProcessMemory(MINECRAFT_HANDLE, (void*)pointedAddress, &point, sizeof(point), NULL);
}

void SafeWalk(bool shift_check) {

    std::thread(ReadCoordinates, SafeWalkStruct.z_offset, std::ref(SafeWalkStruct.coordZ), std::ref(SafeWalkStruct.safewalk_enabler)).detach();
    std::thread(ReadCoordinates, SafeWalkStruct.x_offset, std::ref(SafeWalkStruct.coordX), std::ref(SafeWalkStruct.safewalk_enabler)).detach();

    bool doOnceZ = false, checkerZ = false, doOnceX = false, checkerX = false;
    float first_Z_direction_detection, second_Z_direction_detection, first_X_direction_detection, second_X_direction_detection;

    while (SafeWalkStruct.safewalk_enabler) {

        if (!isMinecraftAttached() || (shift_check && !(GetAsyncKeyState(VK_CONTROL) & 0x8000))) { //shift check is used to make it work only while the key is held
            keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
            while (!isMinecraftAttached()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            while (shift_check && !(GetAsyncKeyState(VK_CONTROL) & 0x8000)) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        first_Z_direction_detection = SafeWalkStruct.coordZ; //Read first Z value
        first_X_direction_detection = SafeWalkStruct.coordX; //Same for X

        std::this_thread::sleep_for(std::chrono::milliseconds(15)); //Wait 20 ms to read second value and avoid false detections

        second_Z_direction_detection = SafeWalkStruct.coordZ; //Read second Z value
        second_X_direction_detection = SafeWalkStruct.coordX;

        if (first_Z_direction_detection - second_Z_direction_detection > 0) { //If we are moving along z 
            if (SafeWalkStruct.coordZ > 0) { //Check if we are in a pos of map where z has a positive value then check our placement on the block
                checkerZ = abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) > 0.700000 && abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) < 0.9999999;
            }       //if we are not in a pos of map where z has a positive value, (so negative) then check our placement on the block
            else checkerZ = abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) > 0.000000 && abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) < 0.3000000;
        } //We get the float value and trunc it to convert it to an int, so we can read the position on the block
        if (first_Z_direction_detection - second_Z_direction_detection < 0) { //If we are moving backwards revert the detection
            if (SafeWalkStruct.coordZ > 0) {
                checkerZ = abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) > 0.000000 && abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) < 0.3000000;
            }   //CheckerZ and X are bools that verify thet the positioning is actually correct by verifying the subtraction
            else  checkerZ = abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) > 0.700000 && abs(SafeWalkStruct.coordZ) - trunc(abs(SafeWalkStruct.coordZ)) < 0.9999999;
        }

        //Same as Z for X
        if (first_X_direction_detection - second_X_direction_detection > 0) {
            if (SafeWalkStruct.coordX > 0) {
                checkerX = abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) > 0.700000 && abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) < 0.9999999;
            }
            else checkerX = abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) > 0.000000 && abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) < 0.3000000;
        }
        if (first_X_direction_detection - second_X_direction_detection < 0) {
            if (SafeWalkStruct.coordX > 0) {
                checkerX = abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) > 0.000000 && abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) < 0.3000000;
            }
            else  checkerX = abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) > 0.700000 && abs(SafeWalkStruct.coordX) - trunc(abs(SafeWalkStruct.coordX)) < 0.9999999;
        }

        //doOnce is used to avoid multiple keybd events (used to send only one time the keybd event)
        if (checkerZ && !doOnceZ) doOnceZ = true;
        if (!checkerZ && doOnceZ) doOnceZ = false;
        if (checkerX && !doOnceX) doOnceZ = true;
        if (!checkerX && doOnceX) doOnceZ = false;

        if (doOnceZ || doOnceX)   keybd_event(VK_SHIFT, 0, 0, 0);
        if (!doOnceZ && !doOnceX)   keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
    }
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
}

void EnableSafeWalk(bool shift_check) {
    if (SafeWalkStruct.safewalk_enabler) 
        SafeWalkStruct.safewalk_enabler = false;
    else {
        SafeWalkStruct.safewalk_enabler = true;
        std::thread(SafeWalk, shift_check).detach();
    }
}