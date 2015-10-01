#pragma once

#include "form.h"
#include "canvas.h"

// アプリケーションメインクラス
class app
{
	form* window;
	canvas<form>* client;

	SIZE size = { 100, 100 };
	COLORREF transColor = 0xFEFEFE, appColor;

public:
	static LRESULT __stdcall app::wndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);

	virtual void init(form* window, canvas<form>* client, HINSTANCE hInst, UINT nCmd);
	virtual bool main();
	virtual int draw();

	int getWidth() { return size.cx; }
	int getHeight() { return size.cy; }
};


