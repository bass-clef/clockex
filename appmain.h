#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <string>

#include "canvas.h"
#include "form.h"


// アプリケーションメインクラス
class app
{
	form* window;
	canvas<form>* cf;

	SIZE size = { 100, 100 };
	COLORREF transColor, appColor;

	std::string str;

public:
	static void getLastError(HWND hWnd = nullptr)
	{
		void* lpMsgBuf;

		// エラー表示文字列作成
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		MessageBox(hWnd, (const char*)lpMsgBuf, NULL, MB_OK);	// メッセージ表示
		LocalFree(lpMsgBuf);
	}

	virtual void init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd);
	virtual bool main();
	virtual int draw();

	int width() { return size.cx; }
	int height() { return size.cy; }


	// 度からラジアンへの変換
	inline double degrad(double deg)
	{
		return (deg - 90) * M_PI / 180.0;
	}

	// フォーマットの適用
	const char* strf(char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr) + 1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		str.assign(buffer);

		delete[] buffer;

		return str.data();
	}
};

