#pragma once

#include <Windows.h>
#include <thread>

class thread
{
	std::thread t;

public:
	// �x�����s
	template<typename T, typename P>
	void delayFunc(DWORD ms, T func, P param = nullptr)
	{
		t = std::thread([&]() {
			Sleep(ms);
			func(param);
		});
		t.join();
	}

};

