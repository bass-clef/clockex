#pragma once

// appmainで使うクラスまとめ

#include <string>

class modules;
class tasklist;
class app;
class form;
class chrono;
class dll;
template<class> class canvas;
template<class, class> class pen;
template<class, class> class image;
struct appinfo
{
	enum RUN_TIMING timing;	// 前回のタイミング

	app* appClass;					// appmain全部
	form* windowInfo;				// ウィンドウ情報
	canvas<form>* clientInfo;		// クライアント情報

	chrono* chrono;					// 時計
	pen<form, std::string>* pens;	// ペン管理
	image<form, std::string>* imgs;	// 画像管理
	modules* tooltips;				// ツールチップ管理
	tasklist* exque;				// 実行タイミング管理
	dll* dlls;						// DLL管理
};
