
#include <Windows.h>
#include "application.h"
#include "canvas.h"
#include "form.h"



int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrev, char* lpCmd, int nCmd)
{
	form window;
	canvas<form> client(&window);
	app app;

	app.init(&window, &client, hInst, nCmd);

	// 終了から遅延させる
	short endCount = 0;
	bool endFlag = false;
	const byte endWaitTime = 100;

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
	while (window.messageLoop(appMain)) Sleep(1);

	return 0;
}
