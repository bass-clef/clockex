#pragma once

#define _USE_MATH_DEFINES

#include <Windows.h>
#include <math.h>
#include <string>

#include "canvas.h"
#include "form.h"

#ifndef API
#define API __declspec(dllimport)
#endif

// �A�v���P�[�V�������C���N���X
class API app
{
	form* window;
	canvas<form>* canvasForm;

	SIZE size = { 100, 100 }, initsize = { 200, 200 };

	std::string str;

public:
	// �O������Q�Ƃł���悤�ɂ��邽�� public
	std::string penHour = "hour", penMinute = "minute";
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
	char currentDir[MAX_PATH];


	virtual void init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd);
	virtual void exit();
	virtual bool main();
	virtual bool calc();
	virtual bool draw();
	virtual void windowSize(int width, int height);

	int width() { return size.cx; }
	int height() { return size.cy; }
	int initwidth() { return initsize.cx; }
	int initheight() { return initsize.cy; }

	// �g���G���[�̕\��
	static void getLastError(HWND hWnd = nullptr)
	{
		void* lpMsgBuf;

		// �G���[�\��������쐬
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		MessageBox(hWnd, (const char*)lpMsgBuf, NULL, MB_OK);	// ���b�Z�[�W�\��
		LocalFree(lpMsgBuf);
	}

	// �t�@�C���I���_�C�A���O�̕\��
	static void openFileDlgSetWindowText(HWND hWnd = nullptr)
	{
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

		SetWindowText(hWnd, (LPCSTR)fileName);
	}

	// �x���烉�W�A���ւ̕ϊ�
	inline double degrad(double deg)
	{
		return (deg - 90) * M_PI / 180.0;
	}
	// ���W�A������x�ւ̕ϊ�
	inline double raddeg(double rad)
	{
		return rad * 180.0 / M_PI;
	}

	// �t�H�[�}�b�g�̓K�p
	const char* strf(char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr) + 1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		str.assign(buffer);

		delete[] buffer;

		return str.data();
	}
};
