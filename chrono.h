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
	int minute()
	{
		return time.tm_min;
	}
	// ��
	int hminute()
	{
		return 11 < time.tm_min ? time.tm_min-12 : time.tm_min ;
	}
	// �b
	int second()
	{
		return time.tm_sec;
	}
	// �~���b
	int millisecond()
	{
		return std::chrono::duration_cast<std::chrono::duration<int, std::chrono::milliseconds>>(now.time_since_epoch()).count();
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

