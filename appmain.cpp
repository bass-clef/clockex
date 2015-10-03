
#include <Windows.h>

#include "appmain.h"


LRESULT __stdcall wndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_NCLBUTTONDOWN, 2, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wp, lp);
}


namespace {
	chrono c;
}


// èâä˙âª
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	appColor = 0x3399FF;
	transColor = RGB(254, 254, 254);

	window->makeClass(hInst, "clockex", wndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", width(), height(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED, 0, 0);

	window->makeFont("ÇlÇr ÉSÉVÉbÉN", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_COLORKEY | LWA_ALPHA);


	draw();
}



// ÉÅÉCÉì
bool app::main()
{
	if (0x8001 & GetAsyncKeyState(VK_RBUTTON) && window->isActive()) {
		return true;
	}

	c.gettime();
	draw();

	return false;
}


// ï`âÊ
int app::draw()
{
	cf->color(transColor)->pos(0, 0);
	cf->fillBox(0, 0, 100, 100);

	cf->color(appColor);
	cf->pos(0, 0);
	cf->mesf("%X\n%X", GetDCPenColor(*window), GetDCBrushColor(*window));

	cf->circle(0, 0, width(), height());

	cf->line(0, 0, width(), height());

	cf->redraw();

	return false;
}



