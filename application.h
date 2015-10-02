#pragma once

#include "form.h"
#include "canvas.h"

// �A�v���P�[�V�������C���N���X
class app
{
	form* window;
	canvas<form>* client;

	SIZE size = { 100, 100 };
	COLORREF transColor, appColor;

public:
	static LRESULT __stdcall wndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);
	static void getLastError(HWND hWnd = nullptr)
	{
		void* lpMsgBuf;

		// �G���[�\��������쐬
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		MessageBox(hWnd, (const char*)lpMsgBuf, NULL, MB_OK);	// ���b�Z�[�W�\��
		LocalFree(lpMsgBuf);
	}

	virtual void init(form* window, canvas<form>* client, HINSTANCE hInst, UINT nCmd);
	virtual bool main();
	virtual int draw();

	int getWidth() { return size.cx; }
	int getHeight() { return size.cy; }

};


