#pragma once

#include <chrono>
#include <ctime>
#include <time.h>


class chrono
{
	std::time_t t;
	std::chrono::time_point<std::chrono::system_clock> now, prev;
	tm time;

public:
	// 時計機能
	chrono gettime()
	{
		now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		localtime_s(&time, &t);

		return *this;
	}
	// 時
	int hour()
	{
		return time.tm_hour;
	}
	// 分
	int minute()
	{
		return time.tm_min;
	}
	// 秒
	int second()
	{
		return time.tm_sec;
	}

	// 差分機能
	chrono setprev()
	{
		prev = std::chrono::system_clock::now();

		return *this;
	}
	// 差分
	long long diff()
	{
		auto dur = std::chrono::system_clock::now() - prev;
		return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	}


};

