
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
	pen<form, std::string> p;
}


// èâä˙âª
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	p.init(window);

	window->makeClass(hInst, "clockex", wndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", width(), height(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED, 0, 0);


	appColor = 0x3399FF;
	transColor = RGB(254, 254, 254);

	window->makeFont("ÇlÇr ÉSÉVÉbÉN", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_COLORKEY | LWA_ALPHA);

	const int hourHand = 3, minuteHand = 2;
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();

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
	cf->color(transColor);
	cf->fillBox(0, 0, 100, 100);

	// òg
	cf->black();
	cf->fillCircle(0, 0, width(), height());

	cf->color(appColor);
	cf->circle(0, 0, width(), height());

	// êj
	const double r = width() / 2, rHour = r/2, rMinute = r/4*3, mergin = 10, hwidth = width() / 2, hheight = height() / 2;
	for (byte angle=0; angle < 12; ++angle)
	{
		double rad = degrad((double)angle * 30);
		cf->pos(hwidth + cos(rad)*(r-mergin), hheight + sin(rad)*(r-mergin))->spix();
	}

	cf->addpos(hwidth, hheight);
	const double secAngle = degrad(360.0 / 60.0 * c.second() + 360.0 / 60.0 / 1000.0 * c.milli()),
		minAngle = degrad(360.0 / 60.0 * c.minute()),
		hourAngle = degrad(360.0 / 12.0 * c.hhour() + 360.0 / 12.0 / 60.0 * c.minute());

	cf->line(0, 0, cos(secAngle)*r, sin(secAngle)*r);
	p.use("minute");
	cf->line(0, 0, cos(minAngle)*rMinute, sin(minAngle)*rMinute);
	p.use("hour");
	cf->line(0, 0, cos(hourAngle)*rHour, sin(hourAngle)*rHour);
	p.old();

	// ï∂éö
	const char* clockString = strf("%2d:%02d %2d", c.hour(), c.minute(), c.second());
	int clockSize = lstrlen(clockString) * window->getFontSize()->cx;
	cf->white()->pos(hwidth - clockSize / 2, hheight + window->getFontSize()->cx + mergin);
	cf->mesf((char*)clockString);


	cf->redraw();

	return false;
}



