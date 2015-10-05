#pragma once

#include "canvas.h"
#include "form.h"


// �A�v���P�[�V�������C���N���X
class app
{
	form* window;
	canvas<form>* cf;

	SIZE size = { 100, 100 };
	COLORREF transColor, appColor;

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
	virtual bool main();
	virtual int draw();

	int width() { return size.cx; }
	int height() { return size.cy; }

};

