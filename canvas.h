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


	// ��_�|�W�V�����ύX (�J�����g�|�W�V����,���Z�|�W�V������������)
	canvas* basepos(int x = INT_MAX, int y = INT_MAX)
	{
		this->bx(x);
		this->by(y);

		this->pos(0, 0);

		return this;
	}
	// �J�����g�|�W�V�����ύX (���Z�|�W�V������������)
	canvas* pos(int x = INT_MAX, int y = INT_MAX)
	{
		this->cx(x);
		this->cy(y);

		this->addpos(0, 0);

		return this;
	}
	// ���Z�|�W�V�����ύX
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

	// �~��(�p�x)
	void arc(int x, int y, long r, float startAngle, float sweepAngle)
	{
		AngleArc((HDC)*p, this->ax(x), this->ay(y), r, startAngle, sweepAngle);
	}

	// ��`(�h��Ԃ�) ��x1,y1,x2,y2 �ɂ�basepos�͉��Z����Ȃ�
	void fillPie(int x1, int y1, int x2, int y2, int xStartRadial, int yStartRadial, int xSweepRadial, int ySweepRadial)
	{
		Pie((HDC)*p,
			this->ax(x1), this->ay(y1), this->ax(x2), this->ay(y2),
			this->ax(xStartRadial), this->ay(yStartRadial), this->ax(xSweepRadial), this->ax(ySweepRadial)
		);
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
	std::unordered_map<nameType, std::pair<size_t, std::pair<unsigned long, unsigned long>>> info;	// �G�C���A�X,�T�C�Y,hashLow,hashHigh


private:
	// �t�@�C���̏d���m�F
	bool isRegistered(HANDLE hFile, std::pair<unsigned long, unsigned long>* hashs)
	{
		// �d�����ĂȂ��ꍇhash�l����
		BY_HANDLE_FILE_INFORMATION bhfi;
		GetFileInformationByHandle(hFile, &bhfi);
		for (auto it = info.begin(); it != info.end(); it++) {
			std::pair<nameType, std::pair<size_t, std::pair<unsigned long, unsigned long>>> infos = *it;
			std::pair<size_t, std::pair<unsigned long, unsigned long>> element = infos.second;
			std::pair<unsigned long, unsigned long> fileHash = element.second;
			if (bhfi.nFileIndexLow == fileHash.first && bhfi.nFileIndexHigh == fileHash.second) {
				CloseHandle(hFile);
				hFile = nullptr;
			}
		}
		if (nullptr == hFile) {
			return true;
		}
		hashs->first = bhfi.nFileIndexLow;
		hashs->second = bhfi.nFileIndexHigh;
		return false;
	}

	// �摜��ǂݍ��񂾃������u���b�N��IStream���쐬
	IStream* getFileIStream(char* lpszPath, std::pair<unsigned long, unsigned long>* hashs)
	{
		// �摜�t�@�C���I�[�v��
		HANDLE hFile;
		hFile = CreateFile(lpszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile) {
			return nullptr;
		}

		// �摜�t�@�C���d���m�F
		if (isRegistered(hFile, hashs)) {
			CloseHandle(hFile);
			return nullptr;
		}

		// �t�@�C���T�C�Y�擾
		DWORD dwFileSize, dwLoadSize;
		dwFileSize = GetFileSize(hFile, NULL);

		// �摜�t�@�C���f�[�^�i�[�p�������u���b�N�m��
		HGLOBAL hBuf;
		hBuf = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

		// �������u���b�N�����b�N���A�h���X���擾
		LPVOID lpBuf;
		lpBuf = GlobalLock(hBuf);

		// �摜�t�@�C���̃f�[�^���������u���b�N�ɓǂݍ���
		ReadFile(hFile, lpBuf, GetFileSize(hFile, NULL), &dwLoadSize, NULL);

		CloseHandle(hFile);

		// �������u���b�N�̃��b�N����
		GlobalUnlock(hBuf);

		// �������u���b�N����IStream���쐬
		IStream* isFile;
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
		std::pair<unsigned long, unsigned long> hashs;
		IStream* is = getFileIStream(fileName, &hashs);
		if (nullptr == is) {
			return;
		}

		this->info[name] = std::make_pair(bitmaps.size(), hashs);

		bitmaps.push_back(new Gdiplus::Bitmap(is));

		is->Release();
	}

	// �摜��ǂݍ���Ŏ����I�ɃG�C���A�X����t��
	void load(char* fileName, char* name)
	{
		std::pair<unsigned long, unsigned long> hashs;
		IStream* is = getFileIStream(fileName, &hashs);
		if (nullptr == is) {
			return;
		}

		GetFileTitle(fileName, name, MAX_PATH);

		this->info[name] = std::make_pair(bitmaps.size(), hashs);
		bitmaps.push_back(new Gdiplus::Bitmap(is));

		is->Release();
	}

	// �t�@�C���A�C�R���擾
	void loadIcon(char* fileName, nameType name)
	{
		HANDLE hFile;
		hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (nullptr == hFile) {
			return;
		}
		std::pair<unsigned long, unsigned long> hashs;
		if (isRegistered(hFile, &hashs)) {
			CloseHandle(hFile);
			return;
		}
		CloseHandle(hFile);

		this->info[name] = std::make_pair(bitmaps.size(), hashs);

		WORD iIcon = 0;
		HICON hIcon = ExtractAssociatedIcon(nullptr, fileName, &iIcon);

		bitmaps.push_back(new Gdiplus::Bitmap(hIcon));
	}
	void loadIcon(char* fileName, char* name)
	{
		HANDLE hFile;
		hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (nullptr == hFile) {
			return;
		}
		std::pair<unsigned long, unsigned long> hashs;
		if (isRegistered(hFile, &hashs)) {
			CloseHandle(hFile);
			return;
		}
		CloseHandle(hFile);

		GetFileTitle(fileName, name, MAX_PATH);

		this->info[name] = std::make_pair(bitmaps.size(), hashs);

		WORD iIcon = 0;
		HICON hIcon = ExtractAssociatedIcon(nullptr, fileName, &iIcon);

		bitmaps.push_back(new Gdiplus::Bitmap(hIcon));
	}

	size_t getIndex(nameType name)
	{
		std::pair<size_t, std::pair<unsigned long, unsigned long>> element = this->info[name];
		return element.first;
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
		return bitmaps[getIndex(name)]->GetWidth();
	}
	int height(nameType name)
	{
		return bitmaps[getIndex(name)]->GetHeight();
	}

	// �`��
	void draw(nameType name, int x = 0, int y = 0)
	{
		gdi->DrawImage(bitmaps[getIndex(name)], x, y);
	}
	void draw(nameType name, int x, int y, int width, int height, int srcx = 0, int srcy = 0)
	{
		gdi->DrawImage(bitmaps[getIndex(name)], x, y, srcx, srcy, width, height);
	}
	void drawCenter(nameType name, int x = 0, int y = 0)
	{
		gdi->DrawImage(bitmaps[getIndex(name)], x-width(name)/2, y-height(name)/2);
	}
};
