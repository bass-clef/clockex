#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/*
 * gui描画関係を簡単に使えるようにするクラス
 *
 * parent は各種ウィンドウオブジェクトハンドルの演算子オーバーロードを持っていること
 */
template<typename parent> class canvas
{
protected:
	POINT current = { 0, 0 }, add = { 0, 0 }, base = { 0, 0 };
	parent* p;

public:
	canvas(parent* p)
	{
		this->p = p;
	}
	canvas() { }

	// 起点位置 アクセサ
	int bx(int x = INT_MAX)
	{
		if (INT_MAX != x) {
			base.x = x;
		}
		return base.x;
	}
	int by(int y = INT_MAX)
	{
		if (INT_MAX != y) {
			base.y = y;
		}
		return base.y;
	}
	// カレントポジション アクセサ
	int cx(int x = INT_MAX)
	{
		if (INT_MAX != x) {
			current.x = x;
		}
		return current.x + add.x + base.x;
	}
	int cy(int y = INT_MAX)
	{
		if (INT_MAX != y) {
			current.y = y;
		}
		return current.y + add.y + base.y;
	}
	// 一時加算 アクセサ
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
	int ax(int x = INT_MAX)
	{
		return base.x + add.x + x;
	}
	int ay(int y = INT_MAX)
	{
		return base.y + add.y + y;
	}


	// 位置変更
	canvas* basepos(int x = INT_MAX, int y = INT_MAX)
	{
		this->bx(x);
		this->by(y);

		this->pos(0, 0);

		return this;
	}
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


	// 再描画
	void redraw(int x = 0, int y = 0, int srcWidth = INT_MAX, int srcHeight = INT_MAX)
	{
		int width = INT_MAX == srcWidth ? GetDeviceCaps((HDC)*p, HORZRES) : srcWidth;
		int height = INT_MAX == srcHeight ? GetDeviceCaps((HDC)*p, VERTRES) : srcHeight;

		BitBlt((HDC)p->getDC(), x, y, width, height, (HDC)*p, x, y, SRCCOPY);
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
	// 文字(自動拡張,ユーザー変形付き)
	void mes(char* text)
	{
		RECT rc = {this->cx(), this->cy(), 0, 0};
		int len = lstrlen(text);
		DrawText((HDC)*p, text, len, &rc, DT_EXPANDTABS | DT_CALCRECT);
		DrawText((HDC)*p, text, len, &rc, DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);
		this->cy(current.y + rc.bottom - rc.top);

		DrawText((HDC)*p, text, len, &rc, DT_WORDBREAK | DT_EXPANDTABS);
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
	// 関数,フォーマット付き
	template<typename callfunc>
	void mesfunc(callfunc func, char* format, ...)
	{
		va_list argptr;

		va_start(argptr, format);

		int size = _vscprintf(format, argptr) + 1;
		char* buffer = new char[size];
		vsprintf_s(buffer, size, format, argptr);

		va_end(argptr);

		RECT rc = { this->cx(), this->cy(), 0, 0 };
		int len = lstrlen(buffer);
		DrawText((HDC)*p, buffer, len, &rc, DT_EXPANDTABS | DT_CALCRECT);
		DrawText((HDC)*p, buffer, len, &rc, DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);

		if (func(&rc, &len)) return;
		this->cy(current.y + rc.bottom - rc.top);

		DrawText((HDC)*p, buffer, len, &rc, DT_WORDBREAK | DT_EXPANDTABS);

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


// ペンを簡単に扱えるようにするクラス
template<typename parent, typename nameType> class pen
{
	parent* p;
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

	void init(parent* p)
	{
		this->p = p;
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


#include <gdiplus.h>
#pragma comment(lib, "Gdiplus.lib")

// 画像の扱いを簡単に使えるようにするクラス
template<typename parent, typename nameType> class image
{
	parent* p;

	Gdiplus::GdiplusStartupInput gpSI;
	ULONG_PTR lpToken;
	Gdiplus::Graphics* gdi;

	std::vector<Gdiplus::Bitmap*> bitmaps;
	std::unordered_map<nameType, int> name;


private:
	// 画像を読み込んだメモリブロックのIStreamを作成
	IStream* getFileIStream(char* lpszPath) {

		HANDLE hFile;
		HGLOBAL hBuf;
		LPVOID lpBuf;

		IStream* isFile;

		DWORD dwFileSize, dwLoadSize;

		// 画像ファイルオープン
		hFile = CreateFile(lpszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		// ファイルサイズ取得
		dwFileSize = GetFileSize(hFile, NULL);

		// 画像ファイルデータ格納用メモリブロック確保
		hBuf = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

		// メモリブロックをロックしアドレスを取得
		lpBuf = GlobalLock(hBuf);

		// 画像ファイルのデータをメモリブロックに読み込む
		ReadFile(hFile, lpBuf, GetFileSize(hFile, NULL), &dwLoadSize, NULL);

		CloseHandle(hFile);

		// メモリブロックのロック解除
		GlobalUnlock(hBuf);

		// メモリブロックからIStreamを作成
		CreateStreamOnHGlobal(hBuf, TRUE, &isFile);

		return isFile;

	}

public:
	~image()
	{
		// ビットマップ削除
		while (bitmaps.size())
		{
			delete bitmaps.back();
			bitmaps.pop_back();
		}

		// GDI+終了処理
		delete gdi;
		Gdiplus::GdiplusShutdown(lpToken);
	}

	// 初期化
	void init(parent* p)
	{
		// GDI+初期化
		Gdiplus::GdiplusStartup(&lpToken, &gpSI, NULL);

		this->p = p;
		gdi = new Gdiplus::Graphics((HDC)*p);
	}

	// 画像読み込み
	void load(char* fileName, nameType name)
	{
		this->name[name] = bitmaps.size();

		IStream* is = getFileIStream(fileName);

		bitmaps.push_back(new Gdiplus::Bitmap(is));

		is->Release();
	}

	// 複数読み込み,ディレクトリの場合再帰的に探索
	void loads(char* files, nameType name)
	{
		if (FILE_ATTRIBUTE_DIRECTORY == GetFileAttributes(files)) {

		}
	}

	int width()
	{
		return bitmaps.back()->GetWidth();
	}
	int height()
	{
		return bitmaps.back()->GetHeight();
	}
	int width(nameType name)
	{
		return bitmaps[this->name[name]]->GetWidth();
	}
	int height(nameType name)
	{
		return bitmaps[this->name[name]]->GetHeight();
	}

	// 描画
	void draw(nameType name, int x = 0, int y = 0)
	{
		gdi->DrawImage(bitmaps[this->name[name]], x, y);
	}
	void draw(nameType name, int x = 0, int y = 0, int width = INT_MAX, int height = INT_MAX, int srcx = 0, int srcy = 0)
	{
		int imageWidth = INT_MAX != width ? width : this->width();
		int imageHeight = INT_MAX != height ? height : this->height();
		gdi->DrawImage(bitmaps[this->name[name]], x, y, srcx, srcy, imageWidth, imageHeight);
	}
};

