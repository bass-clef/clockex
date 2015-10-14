
#include <Windows.h>

#include "appmain.h"

#pragma comment(lib, "WindowsCodecs.lib")

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
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
	chrono c;					// ���v
	pen<form, std::string> p;	// �y���g�p���₷��
	image<form, std::string> img;

	bool opening = true, opened = true;
	float basex = 50, basey = 50, openingCount = 0;	// �`��n�_
	const float openingCountMax = 90, resizeMax = 50,
		angleMinute = 360 / 60, angleHour = 360 / 12;					// ���j,�Z�j��1�p�x
}


// ������
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	p.init(window);
	img.init(window);

	window->makeClass(hInst, "clockex", WndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED, 0, 0);


	// �I�u�W�F�N�g�Ŏg�����̂��`, �E�B���h�E�̓���
	appColor = 0x3399FF;
	transColor = RGB(254, 254, 254);
	
	window->makeFont("�l�r �S�V�b�N", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_ALPHA | LWA_COLORKEY);

	const int hourHand = 3, minuteHand = 2;
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();

	// �摜�̓ǂݍ���
	img.load("icons/glyphicons-208-remove-2.png", "close");
}



// ���C��
bool app::main()
{
/*	if (0x8001 & GetAsyncKeyState(VK_RBUTTON) && window->isActive()) {
		return true;
	}*/

	// �c�[���̕\��
	if (false == opening && 0x1 & GetAsyncKeyState(VK_RBUTTON)) {	// �E�N���b�N�ŕ\��
		opening = true;
	}
	if (opened && !GetAsyncKeyState(VK_RBUTTON)) {					// �\����ɉE�N���b�Nup�Ŕ�\��
		opening = true;
	}

	if (opening) {
		float r = sin(M_PI_2 / openingCountMax * openingCount) * resizeMax, outr = r*2;
		if ((int)openingCount < (int)openingCountMax) {
			openingCount++;
		} else {
			openingCount = 0;
			opening = false;
			opened = !opened;
		}

		if (opening) if (opened) {
			basex = r;
			basey = r;
			windowSize(initwidth() - outr, initheight() - outr);
		} else {
			basex = 50.0-r;
			basey = 50.0-r;
			windowSize(initwidth() - 100 + outr, initheight() - 100 + outr);
		}
	}
	
	// ���ݎ����̎擾
	c.gettime();

	// �`��
	draw();

	return false;
}


// �`��
int app::draw()
{
	if (opening) {
		cf->basepos(0, 0);
		cf->color(transColor);
		cf->fillBox(0, 0, initwidth(), initheight());
	}

	// �g
	cf->basepos(basex, basey);
	cf->black();
	cf->fillCircle(1, 1, width()-1, height()-1);

	cf->color(appColor);
	p.use("hour");
	cf->circle(1, 1, width() - 1, height() - 1);
	p.old();

	// �j
	const float r = width() / 2 - 2, rHour = r / 2, rMinute = r / 4 * 3, mergin = 15, hwidth = width() / 2, hheight = height() / 2;
	for (float angle=12, rad=0; angle; --angle)
	{
		rad = degrad(angle * angleHour);
		cf->pos(hwidth + cos(rad)*(r-mergin), hheight + sin(rad)*(r-mergin))->spix();
	}

	cf->addpos(hwidth, hheight);
	const float secAngle = degrad(angleMinute * c.second() + angleMinute / 1000.0 * c.milli()),
		minAngle = degrad(angleMinute * c.minute()),
		hourAngle = degrad(angleHour * c.hhour() + angleMinute / 12.0 * c.minute());

	cf->line(0, 0, cos(secAngle)*r, sin(secAngle)*r);
	p.use("minute");
	cf->line(0, 0, cos(minAngle)*rMinute, sin(minAngle)*rMinute);
	p.use("hour");
	cf->line(0, 0, cos(hourAngle)*rHour, sin(hourAngle)*rHour);
	p.old();

	// ���t�̕\��
	const auto func = [&](RECT* rc, int* len)->bool {
		int x = (width() - window->getFontSize()->cx * (*len)) / 2;
		rc->left += x;
		rc->right += x;
		return false;
	};

	const byte rowHeight = 2;
	cf->white()->pos(0, hheight - rowHeight - window->getFontSize()->cy * 2);
	cf->mesfunc(func, "%d", c.year());
	cf->mesfunc(func, "%2d / %2d", c.mon(), c.day());

	cf->white()->pos(0, hheight + rowHeight);
	cf->mesfunc(func, "%s", c.toName(c.dotw()));
	cf->mesfunc(func, "%d:%02d %2d", c.hour(), c.minute(), c.second());

	if (opening) {
		cf->redraw();
	} else {
		cf->redraw(cf->bx(), cf->by(), width(), height());
	}

	return false;
}



