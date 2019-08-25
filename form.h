#pragma once

#include "chrono.h"

/*
 * windowsのgui作成クラス
 */
class form
{
	WNDCLASSEX	wce;			// クラス作成用
	MSG			msg;			// メッセージ処理用
	HINSTANCE	hInst;			// 
	HWND		hWnd;			// ウィンドウハンドル
	HDC			hDC, hMDC;		// デバイスコンテキスト/メモリ
	HFONT		hFont;			// フォント
	HPEN		hPen;			// ペン
	HBRUSH		hBrush;			// ブラシ
	HBITMAP		hMBitmap;		// メモリ用ビットマップ

	bool redrawFlag = false;

	SIZE font;

	chrono time;

public:
	form(HWND hWnd)
	{
		initFromHWND(hWnd);
	}
	form() {}

	~form()
	{
		DeleteObject(hMBitmap);
		DeleteObject(hBrush);
		DeleteObject(hPen);
		DeleteObject(hFont);
		DeleteDC(hMDC);
		ReleaseDC(hWnd, hDC);
	}

	// overload
	operator HINSTANCE()	{ return hInst; }
	operator HWND()			{ return hWnd; }
	operator HDC()			{ return hMDC; }
	operator HFONT()		{ return hFont; }
	operator HPEN()			{ return hPen; }
	operator HBRUSH()		{ return hBrush; }
	operator HBITMAP()		{ return hMBitmap; }


	// メンバアクセサ
	MSG* getMsg()
	{
		return &msg;
	}
	SIZE* getFontSize()
	{
		return &font;
	}
	HDC getDC()
	{
		return hDC;
	}


	// 特定のウィンドウハンドルから初期化
	void initFromHWND(HWND hWnd)
	{
		this->hWnd = hWnd;
		makeObject();
	}

	// クラスの作成
	void makeClass(HINSTANCE hInstance = NULL, const char* AppName = "static", WNDPROC WindowProc = DefWindowProc, UINT style = CS_HREDRAW | CS_VREDRAW, HICON hIcon = nullptr)
	{
		if (nullptr == hIcon) {
			hIcon = LoadIcon(NULL, IDI_APPLICATION);
		}

		wce.cbSize = sizeof(wce);								// 構造体のsize
		wce.style = style;										// スタイル
		wce.lpfnWndProc = WindowProc;							// プロージャ関数
		wce.cbClsExtra = wce.cbWndExtra = 0;
		wce.hInstance = hInstance;								// プログラムのハンドル
		wce.hIcon = hIcon;			// アイコン
		wce.hCursor = LoadCursor(NULL, IDC_ARROW);				// マウスカーソル
		wce.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);// ブラシ
		wce.lpszMenuName = NULL;								// メニュー
		wce.lpszClassName = AppName;							// クラス名
		wce.hIconSm = hIcon;			// 小さいアイコン

		if (!RegisterClassEx(&wce)) {
			throw("not created class.");
		}
		this->hInst = hInstance;
	}

	// ウィンドウの作成
	void makeWindow(int nCmd, const char* className, const char* windowName = "", int width = 640, int height = 480, DWORD style = WS_OVERLAPPEDWINDOW, DWORD exStyle = 0, int xPos = CW_USEDEFAULT, int yPos = CW_USEDEFAULT)
	{
		hWnd = CreateWindowEx(
			exStyle, className, windowName, style,
			xPos, yPos, width, height,
			NULL, NULL, this->hInst, NULL
			);

		if (!hWnd) {
			throw("not created window");
		}

		ShowWindow(hWnd, nCmd);
		makeObject();
	}

	// ウィンドウオブジェクトの作成
	void makeObject()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		hDC = GetDC(hWnd);
		if (!hDC) {
			throw("not got window device context");
		}

		hMDC = CreateCompatibleDC(hDC);
		hMBitmap = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
		if (!hMDC) {
			throw("not got window device context");
		}

		SelectObject(hMDC, hMBitmap);

		hBrush = (HBRUSH)GetStockObject(DC_BRUSH);
		hPen = (HPEN)GetStockObject(DC_PEN);
		SelectObject(hMDC, hBrush);
		SelectObject(hMDC, hPen);
		PatBlt(hMDC, 0, 0, rc.right, rc.bottom, WHITENESS);

		font.cx = 8;
		font.cy = 16;
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		SelectObject(hMDC, hFont);

		time.setprev();
	}

	// フォントの作成
	void makeFont(const char* faceName, int height)
	{
		DeleteObject(hFont);

		font.cx = height / 2;
		font.cy = height;
		hFont = CreateFont(font.cy, font.cx, 0, 0, FW_NORMAL, false, false, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, faceName);
		SelectObject(hMDC, hFont);
		SelectObject(hDC, hFont);
	}

	// メッセージループ (falseを返すと終了)
	template<typename MSGLPFUNC> bool messageLoop(MSGLPFUNC appMain, long msFps = 0)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			// システムメイン
			if (!GetMessage(&msg, NULL, 0, 0)) return false;// WM_QUITが来てたら
			TranslateMessage(&msg);	// キーボードを利用可能にする
			DispatchMessage(&msg);	// 制御をWindowsに戻す
			return true;
		}

		if (msFps) {
			// FPS指定遅延
			if (time.diff() < 1000 / msFps) {
				return true;
			}

			time.setprev();
		}

		return appMain(msg);
	}


	// is系
	bool isActive()
	{
		return GetForegroundWindow() == hWnd;
	}


	// タイトル変更
	void title(char* text)
	{
		SetWindowText(hWnd, text);
	}
	void titlef(char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr)+1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		this->title(buffer);

		delete[] buffer;
	}

	// データの関連付け
	void setData(char* name, void* hData)
	{
		SetProp(hWnd, name, hData);
	}
	void* getData(char* name)
	{
		return GetProp(hWnd, name);
	}
	void removeData(char* name)
	{
		RemoveProp(hWnd, name);
	}

	// ウィンドウスタイルの変更
	void setStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_STYLE, style);
	}
	void addStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | style);
	}
	void removeStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) ^ style);
	}

	// 拡張ウィンドウスタイルの変更
	void setExStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, style);
	}
	void addExStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | style);
	}
	void removeExStyle(LONG style)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) ^ style);
	}
};



