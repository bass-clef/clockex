
#define _USE_MATH_DEFINES

#include <Windows.h>
#include <math.h>

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

inline byte degrad(int deg) {
	return deg * M_PI / 180;
}


// ï`âÊ
int app::draw()
{
	cf->color(transColor);
	cf->fillBox(0, 0, 100, 100);

	// òg
	cf->black();
	cf->fillCircle(0, 0, width(), height());

	cf->color(appColor);
	cf->circle(0, 0, width(), height());

	// êj
	const byte r = width() / 2, minir = r/2, longr = r/4*3, mergin = 5, hwidth = width() / 2, hheight = height() / 2;
	for (byte angle=0; angle < 12; ++angle)
	{
		byte rad = degrad(angle * 30);
		cf->pos(hwidth + cos(rad)*(r-mergin), hheight + sin(rad)*(r-mergin))->spix();
	}

	cf->addpos(hwidth, hheight);
	const byte secAngle = degrad(360.0 / 60000.0 * c.millisecond()),
		minAngle = degrad(360 / 60 * c.hminute()),
		hourAngle = degrad(360 / 12 * c.hour());

	cf->line(0, 0, cos(secAngle)*r, sin(secAngle)*r);
	cf->line(0, 0, cos(minAngle)*minir, sin(minAngle)*minir);
	cf->line(0, 0, cos(hourAngle)*longr, sin(hourAngle)*longr);

	// ï∂éö
	cf->white()->pos(0, hheight);
	cf->mesf("%2d:%02d %2d", c.hour(), c.minute(), c.second());

	cf->redraw();

	return false;
}



