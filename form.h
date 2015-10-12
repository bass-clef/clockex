#pragma once

#include "chrono.h"

/*
 * windows��gui�쐬�N���X
 */
class form
{
	WNDCLASSEX	wce;		// �N���X�쐬�p
	MSG		msg;			// ���b�Z�[�W�����p
	HWND	hWnd;			// �E�B���h�E�n���h��
	HDC		hDC, hMDC;		// �f�o�C�X�R���e�L�X�g/������
	HFONT	hFont;			// �t�H���g
	HPEN	hPen;			// �y��
	HBRUSH	hBrush;			// �u���V
	HBITMAP	hMBitmap;		// �������p�r�b�g�}�b�v

	bool redrawFlag = false;

	SIZE font;

	chrono time;

public:
	form(HWND hWnd)
	{
		this->hWnd = hWnd;
		makeObject();
	}
	form()
	{

	}
	~form()
	{
		DeleteObject(hMBitmap);
		DeleteObject(hBrush);
		DeleteObject(hPen);
		DeleteObject(hFont);
		DeleteDC(hMDC);
		ReleaseDC(hWnd, hDC);
	}

	// overload
	operator HDC()		{
		redrawFlag = true;
		return hMDC;
	}
	operator HWND()		{ return hWnd; }
	operator HFONT()	{ return hFont; }
	operator HPEN()		{ return hPen; }
	operator HBRUSH()	{ return hBrush; }
	operator HBITMAP()	{ return hMBitmap; }


	// �����o�A�N�Z�T
	MSG* getMsg()
	{
		return &msg;
	}
	SIZE* getFontSize()
	{
		return &font;
	}


	// �N���X�̍쐬
	void makeClass(HINSTANCE hInstance = NULL, char* AppName = "static", WNDPROC WindowProc = DefWindowProc, UINT style = CS_HREDRAW | CS_VREDRAW)
	{
		wce.cbSize = sizeof(wce);								// �\���̂�size
		wce.style = style;										// �X�^�C��
		wce.lpfnWndProc = WindowProc;							// �v���[�W���֐�
		wce.cbClsExtra = wce.cbWndExtra = 0;
		wce.hInstance = hInstance;								// �v���O�����̃n���h��
		wce.hIcon = LoadIcon(NULL, IDI_APPLICATION);			// �A�C�R��
		wce.hCursor = LoadCursor(NULL, IDC_ARROW);				// �}�E�X�J�[�\��
		wce.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);// �u���V
		wce.lpszMenuName = NULL;								// ���j���[
		wce.lpszClassName = AppName;							// �N���X��
		wce.hIconSm = LoadIcon(NULL, IDI_APPLICATION);			// �������A�C�R��

		if (!RegisterClassEx(&wce)) {
			throw("not created class.");
		}
	}

	// �E�B���h�E�̍쐬
	void makeWindow(HINSTANCE hInstance, int nCmd, char* className, char* windowName = "", int width = 640, int height = 480, DWORD style = WS_OVERLAPPEDWINDOW, DWORD exStyle = 0, int xPos = CW_USEDEFAULT, int yPos = CW_USEDEFAULT)
	{
		hWnd = CreateWindowEx(
			exStyle, className, windowName, style,
			xPos, yPos, width, height,
			NULL, NULL, hInstance, NULL
			);

		if (!hWnd) {
			throw("not created window");
		}

		ShowWindow(hWnd, nCmd);
		makeObject();
	}

	// �E�B���h�E�I�u�W�F�N�g�̍쐬
	void makeObject()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		hDC = GetDC(hWnd);
		if (!hDC) {
			throw("not got window device context");
		}

		hMDC = CreateCompatibleDC(hDC);
		hMBitmap = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
		if (!hMDC) {
			throw("not got window device context");
		}

		SelectObject(hMDC, hMBitmap);

		hBrush = (HBRUSH)GetStockObject(DC_BRUSH);
		hPen = (HPEN)GetStockObject(DC_PEN);
		SelectObject(hMDC, hBrush);
		SelectObject(hMDC, hPen);
		PatBlt(hMDC, 0, 0, rc.right, rc.bottom, WHITENESS);

		font.cx = 8;
		font.cy = 16;
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		SelectObject(hMDC, hFont);

		time.setprev();
	}

	// �t�H���g�̍쐬
	void makeFont(char* faceName, int height)
	{
		DeleteObject(hFont);

		font.cx = height / 2;
		font.cy = height;
		hFont = CreateFont(font.cy, font.cx, 0, 0, FW_NORMAL, false, false, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, faceName);
		SelectObject(hMDC, hFont);
	}

	// ���b�Z�[�W���[�v (false��Ԃ��ƏI��)
	template<typename MSGLPFUNC> bool messageLoop(MSGLPFUNC appMain, long msFps = 0)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			// �V�X�e�����C��
			if (!GetMessage(&msg, NULL, 0, 0)) return false;// WM_QUIT�����Ă���
			TranslateMessage(&msg);	// �L�[�{�[�h�𗘗p�\�ɂ���
			DispatchMessage(&msg);	// �����Windows�ɖ߂�
			return true;
		}

		if (msFps) {
			// FPS�w��x��
			if (time.diff() < 1000 / msFps) {
				return true;
			}

			time.setprev();
		}

		return appMain(msg);
	}


	// �ĕ`��
	void redraw(int x = 0, int y = 0)
	{
		if (!redrawFlag) {
			return;
		}
		redrawFlag = false;
		BitBlt(hDC, x, y, GetDeviceCaps(hMDC, HORZRES)-x, GetDeviceCaps(hMDC, VERTRES)-y, hMDC, x, y, SRCCOPY);
	}

	// is�n
	bool isActive()
	{
		return GetForegroundWindow() == hWnd;
	}

	// �^�C�g���ύX
	void title(char* text)
	{
		SetWindowText(hWnd, text);
	}
	void titlef(char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr)+1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		this->title(buffer);

		delete[] buffer;
	}

	// �f�[�^�̊֘A�t��
	void setData(char* name, void* hData)
	{
		SetProp(hWnd, name, hData);
	}
	void* getData(char* name)
	{
		return GetProp(hWnd, name);
	}
	void removeData(char* name)
	{
		RemoveProp(hWnd, name);
	}

	// �E�B���h�E�X�^�C���̕ύX
	void setStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_STYLE, style);
	}
	void addStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | style);
	}
	void removeStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) ^ style);
	}

	// �g���E�B���h�E�X�^�C���̕ύX
	void setExStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, style);
	}
	void addExStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | style);
	}
	void removeExStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) ^ style);
	}
};



