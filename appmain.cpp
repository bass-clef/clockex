
/*

bug:
//�t�@�C�����J���Ȃ�  DlgProc��bool�l��Ԃ�,DefWindowProc�ɂ���đ���ɈႤ����������Ă���?
//exe�t�@�C��,address�̒��g���˔@�������Ď��s�ł��Ȃ�	push_back �� [] �ł̍쐬�ɍ�������,�����炭�ʃX���b�h�ō쐬�������݂̂�����ł��낤

issue:
//�Ecanvas::image::loads�֐��̎���	����ȂɕK�v�����łȂ����ߕۗ�
�Ejson�t�@�C���̓Ǎ�/�ۑ�
	// �ǂݍ���
	json�ۑ�
		��r���ĂȂ����̂���create
�E�e�c�[���̌Ăяo���L���[�̍쐬
	RT_ADD���ɃL���[�̍X�V

*/


#include <Windows.h>
#include <array>
#include <fstream>
#include <thread>
#include <vector>
#include <picojson.h>

#include "appmain.h"
#include "module.h"
#include "resource.h"

#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "WindowsCodecs.lib")
#pragma warning(disable:4244)


// �t�@�C���P�ʕϐ�
namespace {
	// �萔
	constexpr float openingCountMax = 45, resizeMax = 50,	// �c�[���̕\�����x
		angleMinute = 360 / 60, angleHour = 360 / 12,	// ���j,�Z�j��1�p�x
		rInitIcons  = 15,								// �A�C�R���̕\�����鋗��
		mergin = 15;
	constexpr byte rowHeight = 2;
	const std::vector<std::string>
		comboText = { "ClockEx�̏I��", "�c�[���̒ǉ�", "�t�@�C��", "���W���[���֐�" },
		listText = { "�I����", "��������", "�v�Z��", "�`�掞", "�I����" };

	// �ϐ�
	appinfo ai;
	chrono c;						// ���v
	modules tooltips;				// �c�[���`�b�v�Ǘ�
	pen<form, std::string> p;		// �y���Ǘ�
	image<form, std::string> imgs;	// �摜�Ǘ�

	COLORREF transColor, backColor, appColor, selBackColor;
	POINT mousePos, prevSecPos;
	RECT windowPos;
	bool opening = false, opened = false;
	float basex = 50, basey = 50, openingCount = 0,		// �`��n�_
		secAngle, minAngle, hourAngle,					// ��,��,�b �j�̊p�x
		mouseAngle, iconsAngle,							// ���S�ƃJ�[�\���Ƃ̊p�x, �c�[���`�b�v�̕\���Ԋu�p�x
		r, rHour, rMinute, rIcons = 75,					// �j�̒���
		hwidth, hheight;								// �E�B���h�E�����̃T�C�Y
	int selId;											// �I�����ꂽ�c�[���`�b�vID

	/*
	:�t�@�C���ɕۑ��������:
	clockex.ini {
		tooltips��(int)
		icons��
	}
	tooltips {
		�^�C�v(int)							T_EXIT / T_ADD / T_FILE / T_FUNC		T_EXIT,T_ADD�͊��S�p�X,�I�v�V�������Ȃ�/T_FUNC��canvas,window�̃|�C���^��n�������Œ�
		�t�@�C�����S�p�X(size_t,char*)		�t�@�C����,dll��
		�I�v�V����/�֐���(size_t,char*)		T_FILE:�I�v�V���� T_FUNC:�֐���
		�A�C�R����(size_t,char*)			icons�œǂݍ��݂����G�C���A�X��(�����炭�����I�Ɋ���t��)
		���삷��^�C�~���O(int)				init,calc,draw,exit ���ꂼ�ꕡ���I����
	}
	icons {
		�A�C�R���p�X(size_t,char*)
		�G�C���A�X��(size_t,char*)
	}

	:json�`��:
	mods�f�B���N�g������ hoge.json �����ׂēǂݍ���
	{
		type:"0-3|exit|add|file|func",
		path:"�t�@�C����",
		option:"�I�v�V�����܂��͊֐���",
		timing:"0-4|init|calc|draw|exit|selected"
	}
	*/
}




// �T�u�E�B���h�E��WinMsg�R�[���o�b�N
LRESULT __stdcall DlgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg) {
	case WM_INITDIALOG: {
		// �����l�ݒ�
		HWND hCombo = GetDlgItem(hWnd, IDC_COMBO1);
		for (size_t count = 0; count < comboText.size(); count++) {
			SendMessage(hCombo, CB_ADDSTRING, count, (LPARAM)comboText[count].data());
		}
		SendMessage(GetDlgItem(hWnd, IDC_COMBO1), CB_SETCURSEL, (WPARAM)TOOL::T_FILE, 0);

		for (size_t count = 0; count < listText.size(); count++) {
			SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_INSERTSTRING, count, (LPARAM)listText[count].data());
		}

		return true;
	}

	case WM_COMMAND:
		// �q�R���g���[������
		switch (LOWORD(wp)) {
		case IDC_BUTTON1: {// ...
			char fileName[MAX_PATH] = "";

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = "���ׂẴt�@�C��(*.*)\0*.*\0\0";
			ofn.lpstrFile = fileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;
			GetOpenFileName(&ofn);

			SetWindowText(GetDlgItem(hWnd, IDC_EDIT1), (LPCSTR)fileName);
			break;
		}
		case IDC_BUTTON3: {// ...
			char fileName[MAX_PATH] = "";

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = "���ׂẴt�@�C��(*.*)\0*.*\0\0";
			ofn.lpstrFile = fileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;
			GetOpenFileName(&ofn);

			SetWindowText(GetDlgItem(hWnd, IDC_EDIT3), (LPCSTR)fileName);
			break;
		}

		case IDC_BUTTON2: {	// �쐬
			// �c�[���̎��
			TOOL type = (TOOL)SendMessage(GetDlgItem(hWnd, IDC_COMBO1), CB_GETCURSEL, 0, 0);

			// �c�[���̎��s�^�C�~���O
			HWND hList = GetDlgItem(hWnd, IDC_LIST1);
			std::vector<RUN_TIMING> timing;
			for (size_t count = 0; count < listText.size(); ++count) {
				if (SendMessage(hList, LB_GETSEL, count, 0)) {
					timing.push_back((RUN_TIMING)count);
				}
			}

			// �t�@�C�����ƃI�v�V����/�֐���
			HWND hFileEdit = GetDlgItem(hWnd, IDC_EDIT1),
				hOptionEdit = GetDlgItem(hWnd, IDC_EDIT2),
				hIconFileEdit = GetDlgItem(hWnd, IDC_EDIT3);

			std::string fileName, options, iconFileName;
			fileName.resize(GetWindowTextLength(hFileEdit) + 1);
			options.resize(GetWindowTextLength(hOptionEdit) + 1);
			iconFileName.resize(GetWindowTextLength(hIconFileEdit) + 1);
			GetWindowText(hFileEdit, (LPSTR)fileName.c_str(), fileName.size());
			GetWindowText(hOptionEdit, (LPSTR)options.c_str(), options.size());
			GetWindowText(hIconFileEdit, (LPSTR)iconFileName.c_str(), iconFileName.size());

			char iconName[MAX_PATH] = "";
			if (1 < iconFileName.size()) {
				// �A�C�R���t�@�C���ǂݍ���
				imgs.load((char*)iconFileName.data(), iconName);
				OutputDebugString(ai.appClass->strf("%s\n", iconName));
			}
			if (0 == strcmp(iconName, "")) {
				// �t�@�C���A�C�R���p��UUID�̕�����ł��쐬,�t�@�C������A�C�R���擾
				UUID uuid;
				UuidCreate(&uuid);
				UuidToString(&uuid, (unsigned char**)&iconName);
				imgs.loadIcon((char*)fileName.data(), iconName);
			}

			// ��ޕʂ�tooltips�쐬
			module& m = tooltips.add(iconName, type, &timing);

			switch (type) {
			case T_FUNC:
			case T_FILE:
				m.init((char*)fileName.data(), (char*)options.data());
				break;
			}

			EndDialog(hWnd, IDOK);
			break;
		}

		case IDCANCEL:	// �L�����Z��
			EndDialog(hWnd, IDOK);
			break;
		}
		return true;

	case WM_CLOSE:
		EndDialog(hWnd, IDOK);
		return true;
	}
	return false;
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

	ai.timing = RUN_TIMING::RT_INIT;
	ai.appClass = this;
	ai.window = window;
	ai.client = cf;
	ai.c = &c;
	ai.p = &p;
	ai.imgs = &imgs;
	ai.tooltips = &tooltips;

	HICON hIcon = LoadIcon(hInst, (LPCSTR)IDI_ICON1);
	window->makeClass(hInst, "clockex", WndProc, 3U, hIcon);
	window->makeWindow(nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED);
	windowSize(width(), height());

	// �F�̒�`
	transColor = 0xFEFEFE;
	c.gettime();
	switch (c.dotw()) {
	case 0: appColor = 0x8888FF; break;
	case 1: appColor = 0xFFAA00; break;
	case 2: appColor = 0xFF88AA; break;
	case 3: appColor = 0xFF8888; break;
	case 4: appColor = 0xAAFF00; break;
	case 5: appColor = 0xFFAA00; break;
	case 6: appColor = 0xAA00; break;
	}
	backColor = RGB(GetRValue(appColor) / 5, GetGValue(appColor) / 5, GetBValue(appColor) / 5);
	selBackColor = RGB(GetRValue(appColor) / 2, GetGValue(appColor) / 2, GetBValue(appColor) / 2);

	// �����`��ʒu
	basex = initwidth()/4;
	basey = initheight()/4;

	// �I�u�W�F�N�g�̏�����/�ǂݍ���, �E�B���h�E�̓���
	window->makeFont("�l�r �S�V�b�N", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_ALPHA | LWA_COLORKEY);

	const int hourHand = 3, minuteHand = 2;
	p.init(window);
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();
	imgs.init(window);

	// �c�[���`�b�v������
	tooltips.readExtension(&ai, "mods\\", "*.json");
	
	// �c�[���̎��s

}


// �I������
void app::exit()
{
	// �c�[���̎��s
	ai.timing = RUN_TIMING::RT_EXIT;

	tooltips.saveExtension(&ai);
}


// ���C��
bool app::main()
{
	// �c�[���̎��s
	ai.timing = RUN_TIMING::RT_CALC;


	// �c�[���̕\��
	if (!opened && false == opening && 0x8000 & GetAsyncKeyState(VK_RBUTTON)) {	// �E�N���b�N�ŕ\��
		GetCursorPos(&mousePos);
		GetWindowRect(*window, &windowPos);
		
		if (sqrt(pow(windowPos.left + basex + hwidth - mousePos.x, 2) + powf(windowPos.top + basey + hheight - mousePos.y, 2)) < r) {
			opening = true;
		}
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

		if (sqrt(pow(windowPos.left + basex + hwidth - mousePos.x, 2) + powf(windowPos.top + basey + hheight - mousePos.y, 2)) < r) {
			mouseAngle = atan2(windowPos.top + initheight() / 2 - mousePos.y, windowPos.left + initwidth() / 2 - mousePos.x) + M_PI;

			float prevAngle = (float)-M_PI_2, a = 0, mouseModAngle = 0;
			int count = 0;
			iconsAngle = (float)M_PI * 2 / tooltips.size();
			selId = 0;

			for (size_t count = 0; count < tooltips.size(); ++count) {
				// �`�b�v
				a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
				mouseModAngle = fmod(M_PI * 2 - mouseAngle + a + iconsAngle / 2, M_PI * 2);
				if (0 < mouseModAngle && mouseModAngle < fmod(M_PI * 2 - prevAngle + a, M_PI * 2)) {
					selId = count;
				}

				prevAngle = a;
			}
		} else if (TOOL::T_NOTSELECTED != selId) {
			selId = TOOL::T_NOTSELECTED;
		}
	}

	// �\����ɉE�N���b�Nup�Ŕ�\��
	if (opened && !GetAsyncKeyState(VK_RBUTTON)) {
		opening = true;

		ai.timing = RUN_TIMING::RT_SELECT;
		if (TOOL::T_NOTSELECTED != selId) {
			// �c�[���̌Ăяo��
			if (tooltips.execute(selId, &ai)) return true;
		}
		selId = TOOL::T_NOTSELECTED;
	}

	
	// ���ݎ����̎擾
	c.gettime();
	secAngle = degrad(angleMinute * c.second() + angleMinute / 1000.0 * c.milli());
	minAngle = degrad(angleMinute * c.minute());
	hourAngle = degrad(angleHour * c.hhour() + angleMinute / 12.0 * c.minute());


	// �c�[���̎��s
	ai.timing = RUN_TIMING::RT_DRAW;


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
	// �w�i
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
		for (size_t count = 0; count < tooltips.size(); ++count) {
			a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
			imgs.drawCenter(
				tooltips[count].name(),
				cf->bx() + hwidth + cos(a) * rIcons,
				cf->by() + hheight + sin(a) * rIcons
			);
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
