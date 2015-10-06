#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/*
* 描画関係のクラス
*/
template<typename parent> class canvas
{
protected:
	POINT current = {0, 0}, add = { 0, 0 };
	parent* p;

public:
	canvas(parent* p) {
		this->p = p;
	}
	canvas() { }

	void init(parent* p)
	{
		this->p = p;
	}

	// カレントポジション セレクタ
	int cx(int x = INT_MAX)
	{
		if (INT_MAX != x) {
			current.x = x;
		}
		return current.x + add.x;
	}
	int cy(int y = INT_MAX)
	{
		if (INT_MAX != y) {
			current.y = y;
		}
		return current.y + add.y;
	}
	int addx(int x = INT_MAX)
	{
		if (INT_MAX != x) {
			add.x = x;
		}
		return add.x;
	}
	int addy(int y = INT_MAX)
	{
		if (INT_MAX != y) {
			add.y = y;
		}
		return add.y;
	}
	// 加算
	int ax(int x = INT_MAX)
	{
		return add.x + x;
	}
	int ay(int y = INT_MAX)
	{
		return add.y + y;
	}


	// カレントポジション変更
	canvas* pos(int x = INT_MAX, int y = INT_MAX)
	{
		this->cx(x);
		this->cy(y);

		this->addpos(0, 0);

		return this;
	}
	canvas* addpos(int x = INT_MAX, int y = INT_MAX)
	{
		this->addx(x);
		this->addy(y);

		return this;
	}

	// 色変更
	canvas* color(COLORREF crColor)
	{
		SetDCBrushColor((HDC)*p, crColor);
		SetDCPenColor((HDC)*p, crColor);
		SetTextColor((HDC)*p, crColor);
		SetBkColor((HDC)*p, crColor);
		SetBkMode((HDC)*p, TRANSPARENT);

		return this;
	}
	canvas* white()	{ return color(0xFFFFFF); }
	canvas* black()	{ return color(0); }
	canvas* rgb(BYTE r = 0, BYTE g = 0, BYTE b = 0) {
		return color(RGB(r, g, b));
	}

	void redraw()
	{
		p->redraw();
	}


	// 文字(単一行)
	void print(char* text)
	{
		TextOut((HDC)*p, this->cx(), this->cy(current.y), text, lstrlen(text));

		SIZE size;
		GetTextExtentPoint32((HDC)*p, text, lstrlen(text), &size);
		this->cy(size.cy);
	}
	void printf(char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr) + 1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		this->print(buffer);

		delete[] buffer;
	}
	// 文字(自動拡張)
	void mes(char* text)
	{
		RECT rc = {this->cx(), this->cy(), 0, 0};
		DrawText((HDC)*p, text, lstrlen(text), &rc, DT_EXPANDTABS | DT_CALCRECT);
		DrawText((HDC)*p, text, lstrlen(text), &rc, DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);

		DrawText((HDC)*p, text, lstrlen(text), &rc, DT_WORDBREAK | DT_EXPANDTABS);

		this->cy(rc.bottom);
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
		SetPixelV((HDC)*p, this->cx(), this->cy(), GetDCPenColor((HDC)*p));
	}
	// 点 (取得)
	const COLORREF gpix()
	{
		const COLORREF crColor = GetPixel((HDC)*p, this->cx(), this->cy());
		color(crColor);
		return crColor;
	}


	// 線
	void line(int x1, int y1, int x2, int y2)
	{
		MoveToEx((HDC)*p, this->ax(x1), this->ay(y1), NULL);
		LineTo((HDC)*p, this->cx(x2), this->cy(y2));
	}

	// 四角形
	void box(int x1, int y1, int x2, int y2)
	{
		RECT rc = { this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2) };
		FrameRect((HDC)*p, &rc, (HBRUSH)*p);
	}
	// 四角形(塗りつぶし)
	void fillBox(int x1, int y1, int x2, int y2)
	{
		Rectangle((HDC)*p, this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2));
	}

	// 円
	void circle(int x1, int y1, int x2, int y2)
	{
		Arc((HDC)*p,
			this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2),
			this->ax(x2 - x1), this->ay(y1), this->ax(x2 - x1), this->ay(y1)
		);
	}
	// 円(塗りつぶし)
	void fillCircle(int x1, int y1, int x2, int y2)
	{
		Ellipse((HDC)*p, this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2));
	}
};


template<typename parent, typename nameType> class pen : public canvas<parent>
{
	HPEN hOldPen;
	std::vector<HPEN> pens;
	std::unordered_map<nameType, int> name;
	bool changedPen = false;

public:
	~pen()
	{
		while(pens.size())
		{
			DeleteObject(pens.back());
			pens.pop_back();
		}
		old();
	}

	// ペンの作成
	pen* make(int fnPenStyle, int nWidth, COLORREF crColor, nameType name)
	{
		this->name[name] = pens.size();
		HPEN hPen = CreatePen(fnPenStyle, nWidth, crColor);
		pens.push_back(hPen);

		if (changedPen) {
			SelectObject((HDC)*p, hPen);
		}
		else {
			hOldPen = (HPEN)SelectObject((HDC)*p, hPen);
			changedPen = true;
		}

		return this;
	}

	// ペンの適用
	void use(nameType name)
	{
		SelectObject((HDC)*p, pens.at(this->name[name]));
	}

	// 元のペンを適用
	void old()
	{
		if (!changedPen) {
			return;
		}
		SelectObject((HDC)*p, hOldPen);
	}

};
