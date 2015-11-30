#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <string>
#include <vector>

#include "canvas.h"
#include "form.h"

// �A�v���P�[�V�������C���N���X
class app
{
	form* window;
	canvas<form>* cf;

	SIZE size = { 100, 100 }, initsize = { 200, 200 };

	std::string str;

public:
	static void getLastError(HWND hWnd = nullptr)
	{
		void* lpMsgBuf;

		// �G���[�\��������쐬
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		MessageBox(hWnd, (const char*)lpMsgBuf, NULL, MB_OK);	// ���b�Z�[�W�\��
		LocalFree(lpMsgBuf);
	}

	virtual void init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd);
	virtual void exit();
	virtual bool main();
	virtual int draw();
	virtual void windowSize(int width, int height);

	int width() { return size.cx; }
	int height() { return size.cy; }
	int initwidth() { return initsize.cx; }
	int initheight() { return initsize.cy; }


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


// appmain�Ŏg���N���X�܂Ƃ�
class modules;
struct appinfo
{
	enum RUN_TIMING timing;	// �O��̃^�C�~���O

	app* appClass;					// appmain�S��
	form* window;					// �E�B���h�E���
	canvas<form>* client;			// �N���C�A���g���

	chrono* c;						// ���v
	pen<form, std::string>* p;		// �y���Ǘ�
	image<form, std::string>* imgs;	// �摜�Ǘ�
	modules* tooltips;				// �c�[���`�b�v�Ǘ�
};
