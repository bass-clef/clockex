
#include <Windows.h>
#include <chrono>
#include <ctime>
#include <time.h>

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
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_COLORKEY | LWA_ALPHA);

	appColor = RGB(255, 128, 0);

	window->titlef("%d x %d : %d", getWidth(), getHeight(), (HDC)*window);
	draw();
}

class Clock
{
	std::time_t t;
	std::chrono::time_point<std::chrono::system_clock> now;
	tm time;

public:
	Clock get()
	{
		now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		localtime_s(&time, &t);

		return *this;
	}
	// éû
	int hour()
	{
		return time.tm_hour;
	}
	// ï™
	int minute()
	{
		return time.tm_min;
	}
	// ïb
	int second()
	{
		return time.tm_sec;
	}
};

namespace {
	Clock c;
}


// ÉÅÉCÉì
bool app::main()
{
	switch (window->getMsg()->message) {
	case WM_RBUTTONDOWN:
		return true;
	}

	c.get();
	draw();

	return false;
}


// ï`âÊ
int app::draw()
{
	LONG x = getWidth() / 2, y = getHeight() / 2;
	/**/
	client->color(transColor).box(0, 0, getWidth(), getHeight());
	
	client->color(appColor).fillCircle(0, 0, getWidth(), getHeight());
	client->color(appColor).pos(x, y).sPix();
	/**/

	client->color(appColor).pos(0, 0).mesf("%d:%d %d", c.hour(), c.minute(), c.second());


	client->redraw();

	return false;
}



