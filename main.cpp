#include <Windows.h>
#include <thread>

#include "appmain.h"


int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, char* lpCmd, int nCmd)
{
	form window;
	canvas<form> cf(&window);
	app app;

	app.init(&window, &cf, hInst, nCmd);

	// �I������x��������
	short endCount = 0;
	bool endFlag = false;
	const byte endWaitTime = 100, fps = 0;

	// �A�v���P�[�V�������C��
	const auto appMain = [&](MSG msg) {
		// �ҋ@
		if (app.main()) {
			endFlag = true;
		}

		if (endFlag) {
			endCount++;
			if (endWaitTime < endCount) {
				return false;
			}
		}

		Sleep(1);
		return true;
	};

	// �I���R�[�h����܂őҋ@
	while (window.messageLoop(appMain, fps));

	app.exit();

	return 0;
}
/*EOF*/