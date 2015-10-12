
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


// 初期化
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	p.init(window);

	window->makeClass(hInst, "clockex", wndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED, 0, 0);


	// オブジェクトで使うものを定義, ウィンドウの透過
	appColor = 0x3399FF;
	transColor = RGB(254, 254, 254);

	window->makeFont("ＭＳ ゴシック", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_ALPHA | LWA_COLORKEY);

	const int hourHand = 3, minuteHand = 2;
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();

	draw();
}


struct {
	void func() {

	}
};


// メイン
bool app::main()
{
	if (0x8001 & GetAsyncKeyState(VK_RBUTTON) && window->isActive()) {
		return true;
	}

	c.gettime();
	draw();

	return false;
}


// 描画
int app::draw()
{
	cf->basepos(0, 0);
	cf->color(transColor);
	cf->fillBox(0, 0, initwidth(), initheight());
	cf->color(appColor)->box(0, 0, initwidth(), initheight());

	// 枠
	cf->basepos(50, 50);
	cf->black();
	cf->fillCircle(1, 1, width()-1, height()-1);

	cf->color(appColor);
	p.use("hour");
	cf->circle(1, 1, width()-1, height()-1);
	p.old();


	// 針
	const double r = width() / 2, rHour = r/2, rMinute = r/4*3, mergin = 15, hwidth = width() / 2, hheight = height() / 2;
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

	// 日付の表示
	const byte rowHeight = 2;
	const auto func = [&](RECT* rc, int* len)->bool {
		int x = (width() - window->getFontSize()->cx * (*len)) / 2;
		rc->left += x;
		rc->right += x;
		return false;
	};

	cf->white()->pos(0, hheight - rowHeight - window->getFontSize()->cy * 2);
	cf->mesfunc(func, "%d", c.year());
	cf->mesfunc(func, "%d月%d日", c.mon(), c.day());
	cf->mesf("%d", window->getFontSize()->cy);

	cf->white()->pos(0, hheight + rowHeight);
	cf->mesfunc(func, "%s", c.toName(c.dotw()));
	cf->mesfunc(func, "%d:%02d %2d", c.hour(), c.minute(), c.second());

	cf->basepos(0, 0);
	cf->redraw();

	return false;
}



