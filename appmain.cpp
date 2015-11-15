
#include <Windows.h>
#include <array>
#include <fstream>
#include <thread>

#include "appmain.h"
#include "resource.h"

#pragma comment(lib, "WindowsCodecs.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma warning(disable:4244)


enum RUN_TIMING {
	RT_SELECT,	// 選択時
	RT_INIT,	// 初期化時(clockex自体の初期化処理の後)	初期変数の改変など
	RT_CALC,	// 計算時(clockex自体の計算処理の後)		変数の改変
	RT_DRAW,	// 描画時(clockex自体の描画処理の前)		描画の中断,フック
	RT_EXIT,	// 終了時(clockex自体の終了処理の前)
};

// ツールの種類を表す
enum TOOL {
	T_NOTSELECTED = -1,		// ツールの動作が不明
	T_EXIT,		// 終了する
	T_ADD,		// 追加する
	T_EXE,		// EXEを呼び出す
	T_FUNC,		// 関数を呼び出す
};

struct appinfo
{
	app* self;
	form* window;
	canvas<form>* client;

	appinfo() {}
	appinfo(app* self, form* window, canvas<form>*client) {
		this->self = self;
		this->window = window;
		this->client = client;
	}
};

class module
{
	HMODULE hModule;	// モジールハンドル
	char* address;		// 実行ファイル名 / 関数名

public:
	void command(char* command)
	{
		this->address = command;
	}
	void library(char* moduleName, char* funcName)
	{
		hModule = LoadLibrary(moduleName);
		address = funcName;
	}

	// 実行ファイルをオプション付きで実行
	int exec(appinfo* ai)
	{
		return system(this->address);
	}
	// module関数にappinfoを渡して呼び出し
	int func(appinfo* ai)
	{
		typedef bool (*proc_type)(appinfo*);
		proc_type proc = (proc_type)GetProcAddress(hModule, address);
		return proc(ai);
	}
};

class tooltip : public module {
	std::string iconName;
	TOOL type;
	std::vector<RUN_TIMING> timing;

public:
	operator int()
	{
		return this->type;
	}

	tooltip() {}
	tooltip(char* iconName, TOOL type, RUN_TIMING timing)
	{
		this->iconName = iconName;
		this->type = type;
		this->timing.push_back(timing);
	}
	tooltip(char* iconName, TOOL type, std::vector<RUN_TIMING>* timing)
	{
		this->iconName = iconName;
		this->type = type;
		this->timing = *timing;
	}

	void init(char* command)
	{
		type = T_EXE;
		this->init(command);
	}

	// 実行
	bool execute(appinfo* ap)
	{
		switch (type){
		case T_EXIT:
			return true;

		case T_ADD:
			break;

		case T_EXE:
			exec(ap);
			break;

		case T_FUNC:
			return (bool)func(ap);
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
	const float openingCountMax = 45, resizeMax = 50,	// ツールの表示速度
		angleMinute = 360 / 60, angleHour = 360 / 12,	// 長針,短針の1角度
		rInitIcons  = 15,								// アイコンの表示する距離
		mergin = 15;
	const byte rowHeight = 2;
	const std::vector<std::string>
		comboText = { "終了する", "追加する", "実行ファイル", "モジュール関数" },
		listText = { "選択時", "初期化時", "計算時", "描画時", "終了時" };

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
	appinfo ai;

	/*
	:ファイルに保存するもの:
	clockex.ini {
		tooltips数(int)
		icons数
	}
	tooltips {
		タイプ(int)							T_EXIT / T_ADD / T_EXE / T_FUNC		T_EXIT,T_ADDは完全パス,オプション名なし/T_FUNCはcanvas,windowのポインタを渡す引数固定
		ファイル完全パス(size_t,char*)		exe名,dll名
		オプション/関数名(size_t,char*)		T_EXE:オプション T_FUNC:関数名
		アイコン名(size_t,char*)			iconsで読み込みしたエイリアス名(おそらく自動的に割り付け)
		動作するタイミング(int)				init,calc,draw,exit それぞれ複数選択可
	}
	icons {
		アイコンパス(size_t,char*)
		エイリアス名(size_t,char*)
	}
	*/
}


// サブウィンドウのWinMsgコールバック
LRESULT __stdcall DlgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg) {
	case WM_INITDIALOG: {
		// 初期値設定
		HWND hCombo = GetDlgItem(hWnd, IDC_COMBO1);
		for (size_t count = 0; count < comboText.size(); count++) {
			SendMessage(hCombo, CB_ADDSTRING, count, (LPARAM)comboText[count].data());
		}
		SendMessage(GetDlgItem(hWnd, IDC_COMBO1), CB_SETCURSEL, (WPARAM)0, 0);

		for (size_t count = 0; count < listText.size(); count++) {
			SendMessage(GetDlgItem(hWnd, IDC_LIST1), LB_INSERTSTRING, count, (LPARAM)listText[count].data());
		}

		break;
	}

	case WM_COMMAND:
		// 子コントロールから
		switch (LOWORD(wp)) {
		case IDC_BUTTON1:	// ...
			break;

		case IDC_BUTTON2: {	// 作成
			int newId = tooltips.size();

			// ツールの種類
			TOOL type = (TOOL)SendMessage(GetDlgItem(hWnd, IDC_COMBO1), CB_GETCURSEL, 0, 0);

			// ツールの実行タイミング
			HWND hList = GetDlgItem(hWnd, IDC_LIST1);
			std::vector<RUN_TIMING> timing;
			for (size_t count = 0; count < listText.size(); ++count) {
				if (SendMessage(hList, LB_GETSEL, count, 0)) {
					timing.push_back((RUN_TIMING)count);
				}
			}

			tooltips.resize(newId + 1);

			// ファイル名とオプション/関数名
			HWND hFileEdit = GetDlgItem(hWnd, IDC_EDIT1), hOptionEdit = GetDlgItem(hWnd, IDC_EDIT2);
			std::string fileName, options;
			fileName.resize(GetWindowTextLength(hFileEdit)+1);
			options.resize(GetWindowTextLength(hOptionEdit)+1);
			GetWindowText(hFileEdit, (LPSTR)fileName.c_str(), fileName.size());
			GetWindowText(hOptionEdit, (LPSTR)options.c_str(), options.size());

			switch (type) {
			case T_FUNC:
				tooltips[newId] = { "close", type, &timing };
				tooltips[newId].library((char*)fileName.data(), (char*)options.data());
				break;

			case T_EXE:
				fileName.append(" ");
				fileName.append(options.data());

				unsigned char* uuidstr;
				UUID uuid;
				UuidCreate(&uuid);
				UuidToString(&uuid, &uuidstr);

				imgs.loadIcon((char*)fileName.data(), (char*)uuidstr);
				tooltips[newId] = { (char*)uuidstr, type, &timing };
				tooltips[newId].command((char*)fileName.data());
				break;

			default:
				tooltips[newId] = { "close", type, &timing };
			}

		}

		case IDCANCEL:	// キャンセル
			EndDialog(hWnd, IDOK);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hWnd, IDOK);
		return true;
	}

	return DefWindowProc(hWnd, uMsg, wp, lp);
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
	ai = { this, window, cf };

	HICON hIcon = LoadIcon(hInst, (LPCSTR)IDI_ICON1);
	window->makeClass(hInst, "clockex", WndProc, 3U, hIcon);
	window->makeWindow(nCmd, "clockex", "clockex", initwidth(), initheight(),
		WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		WS_EX_TOPMOST | WS_EX_LAYERED);
	windowSize(width(), height());

	// 色の定義
	transColor = 0xFEFEFE;
	c.gettime();
	switch (c.dotw()) {
	case 0: appColor = 0x8888FF; break;
	case 1: appColor = 0xFFAA00; break;
	case 2: appColor = 0xFF88AA; break;
	case 3: appColor = 0xFF0000; break;
	case 4: appColor = 0xAAFF00; break;
	case 5: appColor = 0xFFAA00; break;
	case 6: appColor = 0xAA00; break;
	}
	backColor = RGB(GetRValue(appColor) / 5, GetGValue(appColor) / 5, GetBValue(appColor) / 5);
	selBackColor = RGB(GetRValue(appColor) / 2, GetGValue(appColor) / 2, GetBValue(appColor) / 2);

	// 初期描画位置
	basex = initwidth()/4;
	basey = initheight()/4;

	// オブジェクトで使うものを定義, ウィンドウの透過
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
	
	tooltips.push_back({ "close", TOOL::T_EXIT, RT_SELECT });
	tooltips.push_back({ "add", TOOL::T_ADD, RT_SELECT });
}


// 終了
app::~app()
{




}


// メイン
bool app::main()
{
	// ツールの表示
	if (!opened && false == opening && 0x8000 & GetAsyncKeyState(VK_RBUTTON)) {	// 右クリックで表示
		GetCursorPos(&mousePos);
		GetWindowRect(*window, &windowPos);
		
		if (sqrt(pow(windowPos.left + basex + hwidth - mousePos.x, 2) + powf(windowPos.top + basey + hheight - mousePos.y, 2)) < r) {
			opening = true;
		}
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

		if (sqrt(pow(windowPos.left + basex + hwidth - mousePos.x, 2) + powf(windowPos.top + basey + hheight - mousePos.y, 2)) < r) {
			mouseAngle = atan2(windowPos.top + initheight() / 2 - mousePos.y, windowPos.left + initwidth() / 2 - mousePos.x) + M_PI;

			float prevAngle = (float)-M_PI_2, a = 0, mouseModAngle = 0;
			int count = 0;
			iconsAngle = (float)M_PI * 2 / tooltips.size();
			selId = 0;

			for (size_t count = 0; count < tooltips.size(); ++count) {
				// チップ
				a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
				mouseModAngle = fmod(M_PI * 2 - mouseAngle + a + iconsAngle / 2, M_PI * 2);
				if (0 < mouseModAngle && mouseModAngle < fmod(M_PI * 2 - prevAngle + a, M_PI * 2)) {
					selId = count;
				}

				prevAngle = a;
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
			std::thread makeTool([&]() {
				DialogBox((HINSTANCE)*window, (LPCSTR)IDD_DIALOG1, (HWND)*window, DlgProc);
			});
			makeTool.detach();
		} else if (TOOL::T_NOTSELECTED != selId) {
			// ツールの呼び出し
			if (tooltips[selId].execute(&ai)) return true;
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
		for (size_t count = 0; count < tooltips.size(); ++count) {
			a = fmod(iconsAngle*count + M_PI + M_PI_2, M_PI * 2);
			imgs.drawCenter(
				tooltips[count].name(),
				cf->bx() + hwidth + cos(a) * rIcons,
				cf->by() + hheight + sin(a) * rIcons
			);
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


