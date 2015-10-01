
#include <Windows.h>

#include "application.h"
#include "canvas.h"
#include "form.h"


LRESULT __stdcall app::wndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
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



// èâä˙âª
void app::init(form* window, canvas<form>* client, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->client = client;

	window->makeClass("clockex", app::wndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", getWidth(), getHeight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED, 0, 0);

	window->makeFont("ÇlÇr ÉSÉVÉbÉN", 14);
	SetLayeredWindowAttributes(*window, 0xFEFEFE, 0xB0, LWA_COLORKEY | LWA_ALPHA);


	LONG x = getWidth() / 2, y = getHeight() / 2;

	client->color(transColor).fillBox(0, 0, getWidth(), getHeight());
	client->white().pos(x, y).sPix();

	client->white().printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

	window->titlef("%d x %d : %d", getWidth(), getHeight(), (HDC)*window);

	draw();
}


// ÉÅÉCÉì
bool app::main()
{
	switch (window->getMsg()->message) {
	case WM_RBUTTONDOWN:
		return true;
	}




	return false;
}


// ï`âÊ
int app::draw()
{
	
	client->repaint();
	return false;
}



