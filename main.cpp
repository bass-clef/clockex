
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

	// 終了から遅延させる
	short endCount = 0;
	bool endFlag = false;
	const byte endWaitTime = 10;

	window.titlef("%d", GetDeviceCaps((HDC)window, RASTERCAPS));

	// アプリケーションメイン
	auto appMain = [&](MSG msg) {
		// 待機
		if (app.main()) {
			endFlag = true;
		}
		endCount += endFlag;
		if (endWaitTime < endCount) {
			return false;
		}

		return true;
	};

	// 終了コード来るまで待機
	while (window.messageLoop(appMain, 60)) Sleep(1);

	return 0;
}
