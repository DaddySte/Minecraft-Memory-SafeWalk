#include "SafeWalk.h"

//Example on How To Use

int main() {
	timeBeginPeriod(1);
	InitializeWalk();

	while (true) {
		if (GetAsyncKeyState('D') & 0x8000) {
			while (GetAsyncKeyState('D') & 0x8000) std::this_thread::sleep_for(std::chrono::milliseconds(1)); //Used to avoid to run multiple safewalk threads
			EnableSafeWalk(false);
		}
		else std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}