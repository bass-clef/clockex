#pragma once

#include <Windows.h>
#include <future>

class thread
{
	std::thread t;


public:
	// íxâÑé¿çs
	template<typename T>
	void delayFunc(DWORD ms, T func)
	{
		t = std::thread([&]() {
			std::this_thread::sleep_for(std::chrono::microseconds(ms));
			func();
		});
		t.detach();
		t.join();
	}
};
