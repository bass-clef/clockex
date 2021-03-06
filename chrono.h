#pragma once

#include <chrono>
#include <ctime>
#include <time.h>

#define TM_ADD_YAER		1900
#define TM_ADD_MONTH	1

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
	// 年
	int year()
	{
		return time.tm_year + TM_ADD_YAER;
	}
	// 月
	int mon()
	{
		return time.tm_mon + TM_ADD_MONTH;
	}
	// 日
	int day()
	{
		return time.tm_mday;
	}
	// 曜日
	int dotw()
	{
		return time.tm_wday;
	}
	// 時
	int hour()
	{
		return time.tm_hour;
	}
	// 時
	int hhour()
	{
		return time.tm_hour % 12;
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
	// ミリ秒
	long long milli()
	{
		return micsec() / 10000;
	}
	// 全部
	long long micsec()
	{
		const auto d = now.time_since_epoch();
		return d.count() % decltype(d)::period::den;
	}
	// 曜日を英字に変換
	static const char* toName(int dotw)
	{
		switch (dotw % 7) {
		case 0: return "Sunday";
		case 1: return "Monday";
		case 2: return "Tuesday";
		case 3: return "Wednesday";
		case 4: return "Thursday";
		case 5: return "Friday";
		}
		return "Saturday";
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
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - prev).count();
	}


};

