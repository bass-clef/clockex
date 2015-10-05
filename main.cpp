
#include <Windows.h>

#include "appmain.h"
#include "canvas.h"
#include "form.h"



int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, char* lpCmd, int nCmd)
{
	form window;
	canvas<form> cf(&window);
	app app;

	app.init(&window, &cf, hInst, nCmd);

	// �I������x��������
	short endCount = 0;
	bool endFlag = false;
	const byte endWaitTime = 10;

	// �A�v���P�[�V�������C��
	auto appMain = [&](MSG msg) {
		// �ҋ@
		if (app.main()) {
			endFlag = true;
		}
		endCount += endFlag;
		if (endWaitTime < endCount) {
			return false;
		}

		return true;
	};

	// �I���R�[�h����܂őҋ@
	while (window.messageLoop(appMain, 60)) Sleep(1);

	return 0;
}
