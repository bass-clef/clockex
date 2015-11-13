
#include <Windows.h>
#include <array>
#include <fstream>
#include <thread>

#include "appmain.h"


#pragma comment(lib, "WindowsCodecs.lib")
#pragma warning(disable:4244)


enum TOOL { T_NOTSELECTED = -1, T_EXIT, T_ADD, T_EXE};
class module
{
	char* address;

public:
	void init(char* command)
	{
		this->address = command;
	}

	// moduleを引数付きで実行
	int exec()
	{
		return system(this->address);
	}
};

class tooltip : public module {
	std::string iconName;
	TOOL type;

public:
	operator int()
	{
		return this->type;
	}

	tooltip() {}
	tooltip(char* iconName, TOOL type)
	{
		this->iconName = iconName;
		this->type = type;
	}

	void init(char* command)
	{
		type = T_EXE;
		this->init(command);
	}

	// 実行
	bool execute()
	{
		switch (type){
		case T_EXIT:
			return true;

		case T_ADD:
			break;

		case T_EXE:
			return (bool)exec();
		}
		return false;
	}

	const char* name()
	{
		return this->iconName.data();
	}
};

// ファイル単位変数
namespace {
	// 定数
	const float openingCountMax = 30, resizeMax = 50,	// ツールの表示速度
		angleMinute = 360 / 60, angleHour = 360 / 12,	// 長針,短針の1角度
		rInitIcons  = 15,								// アイコンの表示する距離
		mergin = 15;
	const byte rowHeight = 2;

	// 変数
	chrono c;					// 時計
	pen<form, std::string> p;	// ペン管理
	image<form, std::string> imgs;	// 画像管理
	std::vector<tooltip> tooltips;	// ツールチップ管理

	COLORREF transColor, backColor, appColor, selBackColor;
	POINT mousePos, prevSecPos;
	RECT windowPos;
	std::ofstream iniFile;
	bool opening = false, opened = false;
	float basex = 50, basey = 50, openingCount = 0,		// 描画始点
		secAngle, minAngle, hourAngle,					// 時,分,秒 針の角度
		mouseAngle, iconsAngle,							// 中心とカーソルとの角度, ツールチップの表示間隔角度
		r, rHour, rMinute, rIcons = 75,					// 針の長さ
		hwidth, hheight;								// ウィンドウ半分のサイズ
	int selId;											// 選択されたツールチップID
}


// サブウィンドウのWinMsgコールバック
LRESULT __stdcall SubWndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wp, lp);
}

// 新しいツールの登録
void registerTool()
{
	RECT clientSize = { 0, 0, 300, 200 };
	AdjustWindowRect(&clientSize, WS_OVERLAPPEDWINDOW, NULL);

	form subWindow;
	subWindow.makeWindow(
		SW_SHOW, "clockex_sub", "新しいツールの登録",
		clientSize.right - clientSize.left, clientSize.bottom - clientSize.top
		);
	subWindow.makeFont("ＭＳ ゴシック", 14);
	subWindow.makeButton("新しいツールの作成", 10, 0, 300 - 20, 20);

	while (subWindow.messageLoop([](MSG msg) { return true; })) Sleep(1);

	int newId = tooltips.size();

	tooltips.resize(newId + 1);
	tooltips[newId] = { "close", T_EXIT };
}



// WinMsgコールバック
LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_NCLBUTTONDOWN, 2, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wp, lp);
}


// 初期化
void app::init(form* window, canvas<form>* cf, HINSTANCE hInst, UINT nCmd)
{
	this->window = window;
	this->cf = cf;

	window->makeClass(hInst, "clockex", WndProc);
	window->makeClass(hInst, "clockex_sub", SubWndProc);
	window->makeWindow(nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED);
	windowSize(width(), height());

	// オブジェクトで使うものを定義, ウィンドウの透過
	transColor = 0xFEFEFE;
	appColor = 0x3399FF;
	backColor = RGB(GetRValue(appColor) / 5, GetGValue(appColor) / 5, GetBValue(appColor) / 5);
	selBackColor = RGB(GetRValue(appColor) / 2, GetGValue(appColor) / 2, GetBValue(appColor) / 2);

	window->makeFont("ＭＳ ゴシック", 14);
	SetLayeredWindowAttributes((HWND)*window, transColor, 0xB0, LWA_ALPHA | LWA_COLORKEY);

	const int hourHand = 3, minuteHand = 2;
	p.init(window);
	p.make(PS_SOLID, hourHand, appColor, "hour");
	p.make(PS_SOLID, minuteHand, appColor, "minute");
	p.old();

	// 画像の読み込み
	imgs.init(window);
	imgs.load("icons/glyphicons-208-remove-2.png", "close");
	imgs.load("icons/glyphicons-433-plus.png", "add");

	// ツールチップ初期化
//	iniFile.open("", std::ios::binary | std::ios::out | std::ios::app);
	
	tooltips.resize(2);
	tooltips[0] = { "close", TOOL::T_EXIT };
	tooltips[1] = { "add", TOOL::T_ADD };

	

	/*
	:ファイルに保存するもの:
	・個数
	・ファイル完全パス
	・タイプ
	*/
}


// 終了
app::~app()
{




}


// メイン
bool app::main()
{
	// ツールの表示
	if (false == opening && window->isActive() && 0x1 & GetAsyncKeyState(VK_RBUTTON)) {	// 右クリックで表示
		opening = true;
	}
	if (opening) {
		float r = sin(M_PI_2 / openingCountMax * openingCount) * resizeMax, outr = r*2;
		if ((int)openingCount < (int)openingCountMax) {
			openingCount++;
		} else {
			openingCount = 0;
			opening = false;
			opened = !opened;
		}

		if (opening) if (opened) {
			basex = r;
			basey = r;
			windowSize(initwidth() - outr, initheight() - outr);
		} else {
			rIcons = rInitIcons + r;
			basex = 50.0-r;
			basey = 50.0-r;
			windowSize(initwidth() - 100 + outr, initheight() - 100 + outr);
		}
	}

	// ツール選択
	if (!opened && opening || opened && !opening) {
		// ツール用カーソル
		GetCursorPos(&mousePos);
		GetWindowRect(*window, &windowPos);

		if (sqrt(pow(windowPos.left + hwidth - mousePos.x, 2) + powf(windowPos.top + hheight - mousePos.y, 2)) < r) {
			mouseAngle = atan2(windowPos.top + initheight() / 2 - mousePos.y, windowPos.left + initwidth() / 2 - mousePos.x) + M_PI;

			float prevAngle = (float)-M_PI_2, a = 0, mouseModAngle = 0;
			int count = 0;
			iconsAngle = (float)M_PI * 2 / tooltips.size();
			selId = 0;

			for (auto it = tooltips.begin(); it != tooltips.end(); it++) {
				// チップ
				a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
				mouseModAngle = fmod(M_PI * 2 - mouseAngle + a + iconsAngle / 2, M_PI * 2);
				if (0 < mouseModAngle && mouseModAngle < fmod(M_PI * 2 - prevAngle + a, M_PI * 2)) {
					selId = count;
				}

				prevAngle = a;
				count++;
			}
		} else if (TOOL::T_NOTSELECTED != selId) {
			selId = TOOL::T_NOTSELECTED;
		}
	}

	// 表示後に右クリックupで非表示
	if (opened && !GetAsyncKeyState(VK_RBUTTON)) {
		opening = true;

		if (TOOL::T_ADD == selId) {
			// ツールの追加 x16までいける
			std::thread makeTool(registerTool);
			makeTool.detach();
		} else if (TOOL::T_NOTSELECTED != selId) {
			// ツールの呼び出し
			if (tooltips[selId].execute()) return true;
		}
		selId = TOOL::T_NOTSELECTED;
	}

	
	// 現在時刻の取得
	c.gettime();
	secAngle = degrad(angleMinute * c.second() + angleMinute / 1000.0 * c.milli());
	minAngle = degrad(angleMinute * c.minute());
	hourAngle = degrad(angleHour * c.hhour() + angleMinute / 12.0 * c.minute());


	// 描画の抑制
	POINT secPos = { cos(secAngle)*r, sin(secAngle)*r };
	if (prevSecPos.x == secPos.x && prevSecPos.y == secPos.y) {
		return false;
	}
	prevSecPos.x = secPos.x;
	prevSecPos.y = secPos.y;


	// 描画
	draw();

	return false;
}


// 描画
int app::draw()
{
	cf->basepos(0, 0);
	cf->color(transColor);
	cf->fillBox(0, 0, initwidth(), initheight());

	// 枠
	cf->basepos(basex, basey);
	cf->color(backColor);
	cf->fillCircle(1, 1, width()-1, height()-1);

	cf->color(appColor);
	p.use("hour");
	cf->circle(1, 1, width() - 1, height() - 1);
	p.old();

	// 針点
	for (float angle = 12, rad = 0; angle; --angle)
	{
		rad = degrad(angle * angleHour);
		int x = hwidth + cos(rad)*(r - mergin), y = hheight + sin(rad)*(r - mergin),
			xMinor = x - 2, yMinor = y - 2,
			xPlus = x + 2, yPlus = y + 2;
		cf->pos(xMinor, yMinor)->spix();
		cf->pos(xMinor, yPlus)->spix();
		cf->pos(x, y)->spix();
		cf->pos(xPlus, yMinor)->spix();
		cf->pos(xPlus, yPlus)->spix();
	}

	// ツールの表示
	if (!opened && opening || opened && !opening) {
		float prevAngle = (float)-M_PI_2, a = 0, mouseModAngle = 0;
		int count = 0;

		if (TOOL::T_NOTSELECTED != selId) {
			// 選択肢
			cf->color(selBackColor)->fillPie(
				mergin, mergin, width() - mergin, height() - mergin,
				r + cos(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r,
				r + cos(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r
				);

			// 選択
			cf->color(appColor)->fillPie(
				mergin, mergin, width() - mergin, height() - mergin,
				r + cos(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 + iconsAngle / 2)*r,
				r + cos(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r, r + sin(iconsAngle* selId - M_PI_2 - iconsAngle / 2)*r
				);
		} else {
			cf->color(selBackColor)->fillCircle(mergin, mergin, width() - mergin, height() - mergin);
			cf->color(appColor);
		}

		// チップ
		cf->addpos(hwidth, hheight);
		for (auto it = tooltips.begin(); it != tooltips.end(); it++) {
			a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
			imgs.drawCenter(
				it->name(),
				cf->bx() + hwidth + cos(a) * rIcons,
				cf->by() + hheight + sin(a) * rIcons
			);

			count++;
		}
	}

	// 針
	cf->addpos(hwidth, hheight);
	cf->line(0, 0, prevSecPos.x, prevSecPos.y);
	p.use("minute");
	cf->line(0, 0, cos(minAngle)*rMinute, sin(minAngle)*rMinute);
	p.use("hour");
	cf->line(0, 0, cos(hourAngle)*rHour, sin(hourAngle)*rHour);
	p.old();


	// 中央揃え用
	int x;
	auto centeringFunc = [&](RECT* rc, int* len)->bool {
		x = hwidth - window->getFontSize()->cx * (*len) / 2;
		rc->left += x;
		rc->right += x;
		return false;
	};

	// 日付,時間の表示
	cf->white()->pos(0, hheight - rowHeight - window->getFontSize()->cy * 2);
	cf->mesfunc(centeringFunc, "%d", c.year());
	cf->mesfunc(centeringFunc, "%2d / %2d", c.mon(), c.day());

	cf->white()->pos(0, hheight + rowHeight);
	cf->mesfunc(centeringFunc, "%s", c.toName(c.dotw()));
	cf->mesfunc(centeringFunc, "%d:%02d %2d", c.hour(), c.minute(), c.second());

	// 裏画面反映
	cf->redraw();

	return false;
}


// ウィンドウサイズ変更
void app::windowSize(int width, int height)
{
	size.cx = width;
	size.cy = height;

	r = width / 2;
	rHour = r / 2;
	rMinute = r / 4 * 3;
	hwidth = width / 2;
	hheight = height / 2;
}


