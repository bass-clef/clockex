#include <Windows.h>
#include <thread>

#include "appmain.h"


int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, char* lpCmd, int nCmd)
{
	form window;
	canvas<form> cf(&window);
	app app;

	app.init(&window, &cf, hInst, nCmd);

	// 終了から遅延させる
	short endCount = 0;
	bool endFlag = false;
	const byte endWaitTime = 100, fps = 0;

	// アプリケーションメイン
	const auto appMain = [&](MSG msg) {
		// 待機
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

	// 終了コード来るまで待機
	while (window.messageLoop(appMain, fps));

	app.exit();

	return 0;
}
/*EOF*/