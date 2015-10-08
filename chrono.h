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
	// ���v�@�\
	chrono gettime()
	{
		now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		localtime_s(&time, &t);

		return *this;
	}
	// �N
	int year()
	{
		return time.tm_year;
	}
	// ��
	int mon()
	{
		return time.tm_mon;
	}
	// ��
	int day()
	{
		return time.tm_mday;
	}
	// �j��
	int dotw()
	{
		return time.tm_wday;
	}
	// ��
	int hour()
	{
		return time.tm_hour;
	}
	// ��
	int hhour()
	{
		return time.tm_hour % 12;
	}
	// ��
	int minute()
	{
		return time.tm_min;
	}
	// �b
	int second()
	{
		return time.tm_sec;
	}
	// �~���b
	int milli()
	{
		return micsec() / 10000;
	}
	// �S��
	long long micsec()
	{
		const auto d = now.time_since_epoch();
		return d.count() % decltype(d)::period::den;
	}

	// �����@�\
	chrono setprev()
	{
		prev = std::chrono::system_clock::now();

		return *this;
	}
	// ����
	long long diff()
	{
		auto dur = std::chrono::system_clock::now() - prev;
		return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	}


};

