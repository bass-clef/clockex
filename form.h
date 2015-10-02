#pragma once

#include <Windows.h>
#include <stdio.h>


/*
 * winフォームの作成クラス
 */
class form
{
	WNDCLASSEX	wce;		// クラス作成用
	MSG		msg;			// メッセージ処理用
	HWND	hWnd;			// ウィンドウハンドル
	HDC		hDC, hMDC;		// デバイスコンテキスト/メモリ
	HFONT	hFont;			// フォント
	HPEN	hPen;			// ペン
	HBRUSH	hBrush;			// ブラシ
	HBITMAP	hMBitmap;		// メモリ用ビットマップ

	SIZE font;

public:
	form(HWND hWnd)
	{
		this->hWnd = hWnd;
		makeObject();
	}
	form()
	{

	}
	~form()
	{
		DeleteObject(hMBitmap);
		DeleteObject(hBrush);
		DeleteObject(hPen);
		DeleteObject(hFont);
		DeleteDC(hMDC);
		ReleaseDC(hWnd, hDC);
	}

	// override
	operator HDC()		{ return hMDC; }
	operator HWND()		{ return hWnd; }
	operator HFONT()	{ return hFont; }
	operator HPEN()		{ return hPen; }
	operator HBRUSH()	{ return hBrush; }
	operator HBITMAP()	{ return hMBitmap; }

	// クラスの作成
	void makeClass(HINSTANCE hInstance = NULL, char* AppName = "static", WNDPROC WindowProc = DefWindowProc, UINT style = CS_HREDRAW | CS_VREDRAW)
	{
		wce.cbSize = sizeof(wce);								// 構造体のsize
		wce.style = style;										// スタイル
		wce.lpfnWndProc = WindowProc;							// プロージャ関数
		wce.cbClsExtra = wce.cbWndExtra = 0;
		wce.hInstance = hInstance;								// プログラムのハンドル
		wce.hIcon = LoadIcon(NULL, IDI_APPLICATION);			// アイコン
		wce.hCursor = LoadCursor(NULL, IDC_ARROW);				// マウスカーソル
		wce.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);// ブラシ
		wce.lpszMenuName = NULL;								// メニュー
		wce.lpszClassName = AppName;							// クラス名
		wce.hIconSm = LoadIcon(NULL, IDI_APPLICATION);			// 小さいアイコン

		if (!RegisterClassEx(&wce)) {
			throw("not created class.");
		}
	}

	// ウィンドウの作成
	void makeWindow(HINSTANCE hInstance, int nCmd, char* className, char* windowName = "", int width = 640, int height = 480, DWORD style = WS_OVERLAPPEDWINDOW, DWORD exStyle = 0, int xPos = CW_USEDEFAULT, int yPos = CW_USEDEFAULT)
	{
		hWnd = CreateWindowEx(
			exStyle, className, windowName, style,
			xPos, yPos, width, height,
			NULL, NULL, hInstance, NULL
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

		COLORREF crColor = 0;
		hBrush = CreateSolidBrush(crColor);
		hPen = CreatePen(PS_SOLID, 0, crColor);
		SelectObject(hMDC, hBrush);
		SelectObject(hMDC, hPen);
		PatBlt(hMDC, 0, 0, rc.right, rc.bottom, WHITENESS);

		makeFont("ＭＳ ゴシック", 16);
	}

	// フォントの作成
	void makeFont(char* faceName, int height)
	{
		DeleteObject(hFont);

		font.cx = height / 2;
		font.cy = height;
		hFont = CreateFont(font.cy, font.cx, 0, 0, FW_NORMAL, false, false, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, faceName);
		SelectObject(hMDC, hFont);
	}

	// メッセージループ (falseを返すと終了)
	template<typename MSGLPFUNC> bool messageLoop(MSGLPFUNC appMain)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			// システムメイン
			if (!GetMessage(&msg, NULL, 0, 0)) return false;// WM_QUITが来てたら
			TranslateMessage(&msg);	// キーボードを利用可能にする
			DispatchMessage(&msg);	// 制御をWindowsに戻す
			return true;
		}

		return appMain(msg);
	}

	// MSG取得
	MSG* getMsg()
	{
		return &msg;
	}


	// 再描画
	void redraw()
	{
		BitBlt(hDC, 0, 0, GetDeviceCaps(hMDC, HORZRES), GetDeviceCaps(hMDC, VERTRES), hMDC, 0, 0, SRCCOPY);
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



