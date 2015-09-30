#pragma once

#include <Windows.h>
#include <string>

/*
* 描画関係のクラス
*/
template<typename parent> class canvas
{
	parent* p;
	POINT current;

public:
	canvas(parent* p) {
		this->p = p;
		current = {0, 0};
	}

	// カレントポジション セレクタ
	int cx(int x = INT_MAX)
	{
		if (INT_MAX != x) {
			current.x = x;
		}
		return current.x;
	}
	int cy(int y = INT_MAX)
	{
		if (INT_MAX != y) {
			current.y = y;
		}
		return current.y;
	}

	// カレントポジション変更
	canvas pos(int x = INT_MAX, int y = INT_MAX)
	{
		this->cx(x);
		this->cx(y);

		return *this;
	}

	// 色変更
	canvas color(BYTE r, BYTE g, BYTE b)
	{
		SetDCBrushColor((HDC)*p, RGB(r, g, b));
		SetDCPenColor((HDC)*p, RGB(r, g, b));

		return *this;
	}
	canvas color(COLORREF crColor)
	{
		SetDCBrushColor((HDC)*p, crColor);
		SetDCPenColor((HDC)*p, crColor);

		return *this;
	}



	// 文字(単一行)
	void print(char* text)
	{
		TextOut((HDC)*p, current.x, current.y, text, lstrlen(text));
	}
	// 文字(自動拡張)
	void mes(char* text)
	{
		SIZE size;
		GetTextExtentPoint32((HDC)*p, text, lstrlen(text), &size);

		RECT rc = { current.x, current.y, size.cx, size.cy };
		DrawText((HDC)*p, text, lstrlen(text), &rc, DT_NOCLIP | DT_WORDBREAK |
			DT_LEFT | DT_TOP | DT_EXPANDTABS);

		this->cy(current.y + size.cy);
	}
	// 文字(フォーマット付き)
	void mesf(char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr)+1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		this->mes(buffer);

		delete[] buffer;
	}

	// 点
	void spix()
	{
		SetPixel((HDC)*p, current.x, current.y, GetDCPenColor((HDC)p));
	}
	// 点 (取得)
	const COLORREF gpix()
	{
		const COLORREF crColor = GetPixel((HDC)*p, current.x, current.y);
		color(crColor);
		return crColor;
	}


	// 線
	void line(int xFrom, int yFrom, int xTo, int yTo)
	{
		MoveToEx((HDC)*p, xFrom, yFrom, NULL);
		ListTo((HDC)*p, xTo, this->cy(yTo));
	}

	// 四角形
	void box(int x1, int y1, int x2, int y2)
	{
		RECT rc = { x1, y1, x2, this->cy(y2) };
		FrameRect((HDC)*p, &rc, (HBRUSH)GetCurrentObject((HDC)*p, OBJ_PEN));
	}

	// 四角形(塗りつぶし)
	void fillBox(int x1, int y1, int x2, int y2)
	{
		Rectangle((HDC)*p, x1, y1, x2, this->cy(y2));
	}

	// 円
	void circle(int x1, int y1, int x2, int y2)
	{
		Arc((HDC)*p, x1, y1, x2, this->cy(y2), x2 - x1, y1, x2 - x1, y1);
	}

	// 円(塗りつぶし)
	void fillCircle(int x1, int y1, int x2, int y2)
	{
		Ellipse((HDC)*p, x1, y1, x2, this->cy(y2));
	}
};
