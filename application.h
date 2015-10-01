#pragma once

#include "form.h"
#include "canvas.h"

// アプリケーションメインクラス
class app
{
	form* window;
	canvas<form>* client;

	SIZE size = { 100, 100 };
	COLORREF transColor = 0xFEFEFE;

public:
	static LRESULT __stdcall app::wndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
	void init(form* window, canvas<form>* client, HINSTANCE hInst, UINT nCmd);

	int getWidth() { return size.cx; }
	int getHeight() { return size.cy; }

	virtual bool main();
	virtual int draw();
	
};

