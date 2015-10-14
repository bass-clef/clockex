
#include <Windows.h>

#include "appmain.h"

#pragma comment(lib, "WindowsCodecs.lib")
#pragma warning(disable:4244)

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


// �t�@�C���P�ʕϐ�
namespace {
	// �萔
	const float openingCountMax = 60, resizeMax = 50,	// �c�[���̕\�����x
		angleMinute = 360 / 60, angleHour = 360 / 12,	// ���j,�Z�j��1�p�x
		mergin = 15;
	const byte rowHeight = 2;

	// �ϐ�
	chrono c;					// ���v
	pen<form, std::string> p;	// �y���g�p���₷��
	image<form, std::string> img;

	COLORREF transColor, backColor, appColor;
	POINT mousePos;
	RECT windowPos;
	bool opening = true, opened = true;
	float basex = 50, basey = 50, openingCount = 0,		// �`��n�_
		secAngle, minAngle, hourAngle,					// ��,��,�b �j�̊p�x
		mouseAngle,										// ���S�ƃJ�[�\���Ƃ̊p�x
		r, rHour, rMinute,								// �j�̒���
		hwidth, hheight;								// �E�B���h�E�����̃T�C�Y
}


// ������
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	p.init(window);

	window->makeClass(hInst, "clockex", WndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED);


	// �I�u�W�F�N�g�Ŏg�����̂��`, �E�B���h�E�̓���
	transColor = RGB(1, 1, 1);
	appColor = 0x3399FF;
	backColor = RGB(GetRValue(appColor)/5, GetGValue(appColor)/5, GetBValue(appColor)/5);
	

	window->makeFont("�l�r �S�V�b�N", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_ALPHA | LWA_COLORKEY);

	const int hourHand = 3, minuteHand = 2;
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();

	// �摜�̓ǂݍ���
	img.init(window);
	img.load("icons/glyphicons-208-remove-2.png", "close");
}



// ���C��
bool app::main()
{
	// �c�[���̕\��
	if (false == opening && window->isActive() && 0x1 & GetAsyncKeyState(VK_RBUTTON)) {	// �E�N���b�N�ŕ\��
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

	// �c�[���p�J�[�\��
	GetCursorPos(&mousePos);
	GetWindowRect(*window, &windowPos);
	mouseAngle = atan2(windowPos.top + initheight() / 2 - mousePos.y, windowPos.left + initwidth() / 2 - mousePos.x) + M_PI;
	
	// ���ݎ����̎擾
	c.gettime();
	secAngle = degrad(angleMinute * c.second() + angleMinute / 1000.0 * c.milli());
	minAngle = degrad(angleMinute * c.minute());
	hourAngle = degrad(angleHour * c.hhour() + angleMinute / 12.0 * c.minute());

	// �`��
	r = width() / 2 - 2;
	rHour = r / 2;
	rMinute = r / 4 * 3;
	hwidth = width() / 2;
	hheight = height() / 2;
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
	cf->color(backColor);
	cf->fillCircle(1, 1, width()-1, height()-1);

	cf->color(appColor);
	p.use("hour");
	cf->circle(1, 1, width() - 1, height() - 1);
	p.old();

	// �j
	for (float angle=12, rad=0; angle; --angle)
	{
		rad = degrad(angle * angleHour);
		cf->pos(hwidth + cos(rad)*(r-mergin), hheight + sin(rad)*(r-mergin))->spix();
	}

	cf->addpos(hwidth, hheight);
	cf->line(0, 0, cos(secAngle)*r, sin(secAngle)*r);
	p.use("minute");
	cf->line(0, 0, cos(minAngle)*rMinute, sin(minAngle)*rMinute);
	p.use("hour");
	cf->line(0, 0, cos(hourAngle)*rHour, sin(hourAngle)*rHour);
	p.old();

	cf->line(0, 0, cos(mouseAngle)*r, sin(mouseAngle)*r);


	// ���������p
	int x;
	auto centeringFunc = [&](RECT* rc, int* len)->bool {
		x = (width() - window->getFontSize()->cx * (*len)) / 2;
		rc->left += x;
		rc->right += x;
		return false;
	};

	// ���t,���Ԃ̕\��
	cf->white()->pos(0, hheight - rowHeight - window->getFontSize()->cy * 2);
	cf->mesfunc(centeringFunc, "%d", c.year());
	cf->mesfunc(centeringFunc, "%2d / %2d", c.mon(), c.day());

	cf->white()->pos(0, hheight + rowHeight);
	cf->mesfunc(centeringFunc, "%s", c.toName(c.dotw()));
	cf->mesfunc(centeringFunc, "%d:%02d %2d", c.hour(), c.minute(), c.second());

	img.draw("close", 0, 0);

	if (opening) {
		cf->redraw();
	} else {
		cf->redraw(cf->bx(), cf->by(), width(), height());
	}

	return false;
}



