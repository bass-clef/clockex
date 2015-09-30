
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include "canvas.h"
#include "form.h"
#include "thread.h"


int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, char* lpCmd, int nCmd)
{
	form f;
	canvas<form> c(&f);
	thread s;
	SIZE window = { 100, 100 };
	LONG x = window.cx / 2, y = window.cy / 2;

	WNDPROC wndProc = [](HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp) -> LRESULT {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			SendMessage(hWnd, WM_NCLBUTTONDOWN, 2, 0);
			break;
		case WM_RBUTTONDOWN: {
			SendMessage(hWnd, WM_CLOSE, 0, 0);
/*			form f(hWnd);
			((thread*)f.getData("threadHandle"))->delayFunc(100, [](void* var) {
				PostQuitMessage(0);
			}, nullptr);*/

			return 0;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hWnd, uMsg, wp, lp);
	};

	f.makeClass("clockex", wndProc);
	f.makeWindow(hInst, SW_SHOW, "clockex", "clockex", window.cx, window.cy,
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 0, 0);

//	SetLayeredWindowAttributes(f, 0xFFFFFF, 0xB0, LWA_COLORKEY | LWA_ALPHA);


	auto appMain = [&]() {
		c.color(0).fillBox(0, 0, window.cx, window.cy);
//		c.color(0xFFFFFF).pos(x, y).spix();

		f.repaint();
	};

	f.titlef("%d x %d : %d", window.cx, window.cy, (HDC)f);

	while (f.messageLoop(appMain)) Sleep(1);

	return 0;
}
