#pragma once

#include <string>
#include <vector>

/*
 * gui�`��֌W���ȒP�Ɏg����悤�ɂ���N���X
 *
 * parent �͊e��E�B���h�E�I�u�W�F�N�g�n���h���̉��Z�q�I�[�o�[���[�h�������Ă��邱��
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

	// �N�_�ʒu �A�N�Z�T
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
	// �J�����g�|�W�V���� �A�N�Z�T
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
	// �ꎞ���Z �A�N�Z�T
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


	// �ʒu�ύX
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

	// �F�ύX
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


	// �ĕ`��
	void redraw(int x = 0, int y = 0, int srcWidth = INT_MAX, int srcHeight = INT_MAX)
	{
		int width = INT_MAX == srcWidth ? GetDeviceCaps((HDC)*p, HORZRES) : srcWidth;
		int height = INT_MAX == srcHeight ? GetDeviceCaps((HDC)*p, VERTRES) : srcHeight;

		BitBlt((HDC)p->getDC(), x, y, width, height, (HDC)*p, x, y, SRCCOPY);
	}


	// ����(�P��s)
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
	// ����(�����g��,���[�U�[�ό`�t��)
	void mes(char* text)
	{
		RECT rc = {this->cx(), this->cy(), 0, 0};
		int len = lstrlen(text);
		DrawText((HDC)*p, text, len, &rc, DT_EXPANDTABS | DT_CALCRECT);
		DrawText((HDC)*p, text, len, &rc, DT_WORDBREAK | DT_EXPANDTABS | DT_CALCRECT);
		this->cy(current.y + rc.bottom - rc.top);

		DrawText((HDC)*p, text, len, &rc, DT_WORDBREAK | DT_EXPANDTABS);
	}
	// ����(�t�H�[�}�b�g�t��)
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
	// �֐�,�t�H�[�}�b�g�t��
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


	// �_
	void spix()
	{
		SetPixelV((HDC)*p, this->cx(), this->cy(), GetDCPenColor((HDC)*p));
	}
	// �_ (�擾)
	const COLORREF gpix()
	{
		const COLORREF crColor = GetPixel((HDC)*p, this->cx(), this->cy());
		color(crColor);
		return crColor;
	}


	// ��
	void line(int x1, int y1, int x2, int y2)
	{
		MoveToEx((HDC)*p, this->ax(x1), this->ay(y1), NULL);
		LineTo((HDC)*p, this->cx(x2), this->cy(y2));
	}

	// �l�p�`
	void box(int x1, int y1, int x2, int y2)
	{
		RECT rc = { this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2) };
		FrameRect((HDC)*p, &rc, (HBRUSH)*p);
	}
	// �l�p�`(�h��Ԃ�)
	void fillBox(int x1, int y1, int x2, int y2)
	{
		Rectangle((HDC)*p, this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2));
	}

	// �~
	void circle(int x1, int y1, int x2, int y2)
	{
		Arc((HDC)*p,
			this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2),
			this->ax(x2 - x1), this->ay(y1), this->ax(x2 - x1), this->ay(y1)
		);
	}
	// �~(�h��Ԃ�)
	void fillCircle(int x1, int y1, int x2, int y2)
	{
		Ellipse((HDC)*p, this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2));
	}
};



#include <unordered_map>

// �y�����ȒP�Ɉ�����悤�ɂ���N���X
template<typename parent, typename nameType> class pen
{
	parent* p;
	HPEN hOldPen;
	std::vector<HPEN> pens;
	std::unordered_map<nameType, size_t> name;
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

	// �y���̍쐬
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

	// �y���̓K�p
	void use(nameType name)
	{
		SelectObject((HDC)*p, pens.at(this->name[name]));
	}

	// ���̃y����K�p
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

// �摜�̈������ȒP�Ɏg����悤�ɂ���N���X
template<typename parent, typename nameType> class image
{
	parent* p;

	Gdiplus::GdiplusStartupInput gpSI;
	ULONG_PTR lpToken;
	Gdiplus::Graphics* gdi;

	std::vector<Gdiplus::Bitmap*> bitmaps;
	std::unordered_map<nameType, size_t> name;


private:
	// �摜��ǂݍ��񂾃������u���b�N��IStream���쐬
	IStream* getFileIStream(char* lpszPath) {

		HANDLE hFile;
		HGLOBAL hBuf;
		LPVOID lpBuf;

		IStream* isFile;

		DWORD dwFileSize, dwLoadSize;

		// �摜�t�@�C���I�[�v��
		hFile = CreateFile(lpszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		// �t�@�C���T�C�Y�擾
		dwFileSize = GetFileSize(hFile, NULL);

		// �摜�t�@�C���f�[�^�i�[�p�������u���b�N�m��
		hBuf = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

		// �������u���b�N�����b�N���A�h���X���擾
		lpBuf = GlobalLock(hBuf);

		// �摜�t�@�C���̃f�[�^���������u���b�N�ɓǂݍ���
		ReadFile(hFile, lpBuf, GetFileSize(hFile, NULL), &dwLoadSize, NULL);

		CloseHandle(hFile);

		// �������u���b�N�̃��b�N����
		GlobalUnlock(hBuf);

		// �������u���b�N����IStream���쐬
		CreateStreamOnHGlobal(hBuf, TRUE, &isFile);

		return isFile;

	}

public:
	~image()
	{
		// �r�b�g�}�b�v�폜
		while (bitmaps.size())
		{
			delete bitmaps.back();
			bitmaps.pop_back();
		}

		// GDI+�I������
		delete gdi;
		Gdiplus::GdiplusShutdown(lpToken);
	}

	// ������
	void init(parent* p)
	{
		// GDI+������
		Gdiplus::GdiplusStartup(&lpToken, &gpSI, NULL);

		this->p = p;
		gdi = new Gdiplus::Graphics((HDC)*p);
	}

	// �摜�ǂݍ���
	void load(char* fileName, nameType name)
	{
		this->name[name] = bitmaps.size();

		IStream* is = getFileIStream(fileName);

		bitmaps.push_back(new Gdiplus::Bitmap(is));

		is->Release();
	}

	// �����ǂݍ���,�f�B���N�g���̏ꍇ�ċA�I�ɒT��
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

	// �`��
	void draw(nameType name, int x = 0, int y = 0)
	{
		gdi->DrawImage(bitmaps[this->name[name]], x, y);
	}
	void draw(nameType name, int x, int y, int width, int height, int srcx = 0, int srcy = 0)
	{
		gdi->DrawImage(bitmaps[this->name[name]], x, y, srcx, srcy, width, height);
	}
	void drawCenter(nameType name, int x = 0, int y = 0)
	{
		gdi->DrawImage(bitmaps[this->name[name]], x-width(name)/2, y-height(name)/2);
	}
};

