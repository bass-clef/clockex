
#include <Windows.h>

#include "appmain.h"

#pragma comment(lib, "WindowsCodecs.lib")
#pragma warning(disable:4244)


enum TOOL { T_NOTSELECTED = -1, T_EXIT, T_ADD, };

struct tooltip {
	std::string iconName;
	int type;
	void* pointer;
};

// �t�@�C���P�ʕϐ�
namespace {
	// �萔
	const float openingCountMax = 30, resizeMax = 50,	// �c�[���̕\�����x
		angleMinute = 360 / 60, angleHour = 360 / 12,	// ���j,�Z�j��1�p�x
		rInitIcons  = 15,								// �A�C�R���̕\�����鋗��
		mergin = 15;
	const byte rowHeight = 2;

	// �ϐ�
	chrono c;					// ���v
	pen<form, std::string> p;	// �y���Ǘ�
	image<form, std::string> imgs;	// �摜�Ǘ�
	std::vector<tooltip> tooltips;	// �c�[���`�b�v�Ǘ�

	COLORREF transColor, backColor, appColor, selBackColor;
	POINT mousePos, prevSecPos;
	RECT windowPos;
	TOOL selId;											// �I�����ꂽ�c�[���`�b�vID
	bool opening = true, opened = false;
	float basex = 50, basey = 50, openingCount = 0,		// �`��n�_
		secAngle, minAngle, hourAngle,					// ��,��,�b �j�̊p�x
		mouseAngle, iconsAngle,							// ���S�ƃJ�[�\���Ƃ̊p�x, �c�[���`�b�v�̕\���Ԋu�p�x
		r, rHour, rMinute, rIcons = 75,					// �j�̒���
		hwidth, hheight;								// �E�B���h�E�����̃T�C�Y
}



// WinMsg�R�[���o�b�N
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


// ������
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	window->makeClass(hInst, "clockex", WndProc);
	window->makeWindow(hInst, nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED);
	windowSize(width(), height());

	// �I�u�W�F�N�g�Ŏg�����̂��`, �E�B���h�E�̓���
	transColor = 0xFEFEFE;
	appColor = 0x3399FF;
	backColor = RGB(GetRValue(appColor) / 5, GetGValue(appColor) / 5, GetBValue(appColor) / 5);
	selBackColor = RGB(GetRValue(appColor) / 2, GetGValue(appColor) / 2, GetBValue(appColor) / 2);

	window->makeFont("�l�r �S�V�b�N", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_ALPHA | LWA_COLORKEY);

	const int hourHand = 3, minuteHand = 2;
	p.init(window);
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();

	// �摜�̓ǂݍ���
	imgs.init(window);
	imgs.load("icons/glyphicons-208-remove-2.png", "close");
	imgs.load("icons/glyphicons-433-plus.png", "add");

	// �c�[���`�b�v������
	tooltips.resize(2);
	tooltips[0] = { "close", TOOL::T_EXIT, nullptr };
	tooltips[1] = { "add", TOOL::T_ADD, nullptr };

}



// ���C��
bool app::main()
{
	// �c�[���̕\��
	if (false == opening && window->isActive() && 0x1 & GetAsyncKeyState(VK_RBUTTON)) {	// �E�N���b�N�ŕ\��
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
			rIcons = rInitIcons + r;
			basex = 50.0-r;
			basey = 50.0-r;
			windowSize(initwidth() - 100 + outr, initheight() - 100 + outr);
		}
	}

	// �c�[���I��
	if (!opened && opening || opened && !opening) {
		// �c�[���p�J�[�\��
		GetCursorPos(&mousePos);
		GetWindowRect(*window, &windowPos);

		if (sqrt(pow(windowPos.left + hwidth - mousePos.x, 2) + powf(windowPos.top + hheight - mousePos.y, 2)) < r) {
			mouseAngle = atan2(windowPos.top + initheight() / 2 - mousePos.y, windowPos.left + initwidth() / 2 - mousePos.x) + M_PI;

			float prevAngle = (float)-M_PI_2, a = 0, mouseModAngle = 0;
			int count = 0;
			iconsAngle = (float)M_PI * 2 / tooltips.size();
			selId = TOOL::T_EXIT;

			for (auto it = tooltips.begin(); it != tooltips.end(); it++) {
				// �`�b�v
				a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
				mouseModAngle = fmod(M_PI * 2 - mouseAngle + a + iconsAngle / 2, M_PI * 2);
				if (0 < mouseModAngle && mouseModAngle < fmod(M_PI * 2 - prevAngle + a, M_PI * 2)) {
					selId = (TOOL)count;
				}

				prevAngle = a;
				count++;
			}
		} else if (TOOL::T_NOTSELECTED != selId) {
			selId = TOOL::T_NOTSELECTED;
		}
	}

	// �\����ɉE�N���b�Nup�Ŕ�\��
	if (opened && !GetAsyncKeyState(VK_RBUTTON)) {
		opening = true;

		switch (selId) {
		case TOOL::T_EXIT:
			return true;

		case TOOL::T_ADD:
			/*

			*/
			break;
		}

	}

	
	// ���ݎ����̎擾
	c.gettime();
	secAngle = degrad(angleMinute * c.second() + angleMinute / 1000.0 * c.milli());
	minAngle = degrad(angleMinute * c.minute());
	hourAngle = degrad(angleHour * c.hhour() + angleMinute / 12.0 * c.minute());

	
	// �`��̗}��
	POINT secPos = { cos(secAngle)*r, sin(secAngle)*r };
	if (prevSecPos.x == secPos.x && prevSecPos.y == secPos.y) {
		return false;
	}
	prevSecPos.x = secPos.x;
	prevSecPos.y = secPos.y;

	// �`��
	draw();

	return false;
}


// �`��
int app::draw()
{
	cf->basepos(0, 0);
	cf->color(transColor);
	cf->fillBox(0, 0, initwidth(), initheight());

	// �g
	cf->basepos(basex, basey);
	cf->color(backColor);
	cf->fillCircle(1, 1, width()-1, height()-1);

	cf->color(appColor);
	p.use("hour");
	cf->circle(1, 1, width() - 1, height() - 1);
	p.old();

	// �j�_
	for (float angle = 12, rad = 0; angle; --angle)
	{
		rad = degrad(angle * angleHour);
		int x = hwidth + cos(rad)*(r - mergin), y = hheight + sin(rad)*(r - mergin),
			xMinor = x - 2, yMinor = y - 2,
			xPlus = x + 2, yPlus = y + 2;
		cf->pos(xMinor, yMinor)->spix();
		cf->pos(xMinor, yPlus)->spix();
		cf->pos(x, y)->spix();
		cf->pos(xPlus, yMinor)->spix();
		cf->pos(xPlus, yPlus)->spix();
	}

	// �c�[���̕\��
	if (!opened && opening || opened && !opening) {
		float prevAngle = (float)-M_PI_2, a = 0, mouseModAngle = 0;
		int count = 0;

		if (TOOL::T_NOTSELECTED != selId) {
			// �I����
			cf->color(selBackColor)->fillPie(
				mergin, mergin, width() - mergin, height() - mergin,
				r + cos(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r,
				r + cos(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r
				);

			// �I��
			cf->color(appColor)->fillPie(
				mergin, mergin, width() - mergin, height() - mergin,
				r + cos(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r,
				r + cos(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r
				);
		} else {
			cf->color(selBackColor)->fillCircle(mergin, mergin, width() - mergin, height() - mergin);
			cf->color(appColor);
		}

		// �`�b�v
		cf->addpos(hwidth, hheight);
		for (auto it = tooltips.begin(); it != tooltips.end(); it++) {
			a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
			imgs.drawCenter(
				it->iconName,
				cf->bx() + hwidth + cos(a) * rIcons,
				cf->by() + hheight + sin(a) * rIcons
			);

			count++;
		}
	}

	// �j
	cf->addpos(hwidth, hheight);
	cf->line(0, 0, prevSecPos.x, prevSecPos.y);
	p.use("minute");
	cf->line(0, 0, cos(minAngle)*rMinute, sin(minAngle)*rMinute);
	p.use("hour");
	cf->line(0, 0, cos(hourAngle)*rHour, sin(hourAngle)*rHour);
	p.old();


	// ���������p
	int x;
	auto centeringFunc = [&](RECT* rc, int* len)->bool {
		x = hwidth - window->getFontSize()->cx * (*len) / 2;
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

	// ����ʔ��f
	cf->redraw();

	return false;
}




// �E�B���h�E�T�C�Y�ύX
void app::windowSize(int width, int height)
{
	size.cx = width;
	size.cy = height;

	r = width / 2;
	rHour = r / 2;
	rMinute = r / 4 * 3;
	hwidth = width / 2;
	hheight = height / 2;
}
