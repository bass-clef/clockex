#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <thread>
#include <fstream>

#include "appmain.h"
#include "resource.h"
#include "interchangeable.h"

// プロトタイプ宣言
INT_PTR __stdcall DlgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);


// 実行タイミングを表す
enum RUN_TIMING : int {
	RT_ENUM_BEGIN,

	RT_SELECT,	// 選択時
	RT_INIT,	// 初期化時(clockex自体の初期化処理の後)	初期変数の改変など
	RT_CALC,	// 計算時(clockex自体の計算処理の前)		計算処理の中断
	RT_DRAW,	// 描画時(clockex自体の描画処理の前)		処理後の変数の改変,描画の中断
	RT_EXIT,	// 終了時(clockex自体の終了処理の前)
	RT_ADD,		// 追加時	自分自身の時も呼ばれる
	RT_DELETE,	// 削除時	同上
	RT_FIRST,	// 一番最初に実行する	この後に読み込まれたRT_FIRST持ちの拡張によって上書きされる,上書きされた場合 2番目以降にもなりうる
	RT_LAST,	// 一番最後に実行する	同上

	RT_ENUM_END,
};

// ツールの種類を表す
enum TOOL {
	T_NOTSELECTED = -1,		// ツールの動作が不明
	T_EXIT,		// 終了する
	T_ADD,		// 追加する
	T_FILE,		// ファイルを呼び出す
	T_FUNC,		// 関数を呼び出す
};

// 実行タイミングと文字列の互換用
const interchangeableClass<RUN_TIMING, const std::string> timingAndString = {
	std::make_pair(RT_SELECT, "RT_SELECT"),
	std::make_pair(RT_INIT, "RT_INIT"),
	std::make_pair(RT_CALC, "RT_CALC"),
	std::make_pair(RT_DRAW, "RT_DRAW"),
	std::make_pair(RT_EXIT, "RT_EXIT"),
	std::make_pair(RT_ADD, "RT_ADD"),
	std::make_pair(RT_DELETE, "RT_DELETE"),
	std::make_pair(RT_FIRST, "RT_FIRST"),
	std::make_pair(RT_LAST, "RT_LAST"),
};
// 種類と文字列の互換用
const interchangeableClass<TOOL, const std::string> toolAndString = {
	std::make_pair(T_NOTSELECTED, "T_NOTSELECTED"),
	std::make_pair(T_EXIT, "T_EXIT"),
	std::make_pair(T_ADD, "T_ADD"),
	std::make_pair(T_FILE, "T_FILE"),
	std::make_pair(T_FUNC, "T_FUNC"),
};


// モジュール管理クラス
class module
{
	std::string option;		// typeによって関数名かオプション
	std::string address, iconName;		// ファイル名, アイコン名
	std::vector<RUN_TIMING> timing;		// 実行タイミング
	long result = 0;		// 結果
	TOOL type;				// 種類
	bool saved = false;		// 保存したか

	// 実行タイミングの確認
	bool bootMatch(RUN_TIMING nowTiming)
	{
		if (timing.end() != std::find(timing.begin(), timing.end(), nowTiming)) {
			return true;
		}
		return false;
	}

	// ファイルをオプション付きで実行
	void exec(appinfo* ai)
	{
		std::thread execute([&]() {
			SHELLEXECUTEINFO sei = { 0 };
			sei.cbSize = sizeof(sei);
			sei.hwnd = (HWND)*ai->window;
			sei.nShow = SW_SHOWNORMAL;
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpFile = address.data();
			sei.lpParameters = option.data();
			if (!ShellExecuteEx(&sei) || (const int)sei.hInstApp <= 32) {
				result = (long)sei.hInstApp;
				return;
			}

			WaitForSingleObject(sei.hProcess, INFINITE);
			result = (long)sei.hInstApp;
		});
		execute.detach();
	}
	// module関数にappinfoを渡して呼び出し
	void func(appinfo* ai)
	{
		std::thread execute([&]() {
			typedef bool(*proc_type)(appinfo*);

			HMODULE hModule = LoadLibrary(option.data());

			proc_type proc = (proc_type)GetProcAddress(hModule, address.data());

			result = proc(ai);

			FreeLibrary(hModule);
		});
		execute.detach();
	}
public:

	module() {}
	module(char* iconName, TOOL type, RUN_TIMING timing)
	{
		this->iconName.assign(iconName);
		this->type = type;
		this->timing.push_back(timing);
	}
	module(char* iconName, TOOL type, std::vector<RUN_TIMING>* timing)
	{
		this->iconName.assign(iconName);
		this->type = type;
		this->timing = *timing;
	}
	module(char* iconName, TOOL type, std::vector<RUN_TIMING>* timing, bool saved)
	{
		this->iconName.assign(iconName);
		this->type = type;
		this->timing = *timing;
		this->saved = saved;
	}

	operator int()
	{
		return this->type;
	}

	const char* name()
	{
		return this->iconName.data();
	}

	const char* file()
	{
		if (address.size()) {
			return address.data();
		}
		if (iconName.size()) {
			return iconName.data();
		}

		return nullptr;
	}

	const std::vector<RUN_TIMING>& runTiming()
	{
		return this->timing;
	}

	// ファイルに保存されてるものを読み込んだか
	bool isSaved()
	{
		return saved;
	}

	bool isFirst()
	{
		return timing.end() != std::find(timing.begin(), timing.end(), RT_FIRST);
	}
	bool isLast()
	{
		return timing.end() != std::find(timing.begin(), timing.end(), RT_LAST);
	}
	

	void init(char* address, char* option)
	{
		this->address = address;
		this->option = option;
	}


	// 実行
	bool execute(appinfo* ap)
	{
		if (!bootMatch(ap->timing)) return false;

		switch (type) {
		case T_EXIT:
			// 終了
			return true;

		case T_ADD: {
			// 自身の追加
			std::thread makeTool([&]() {
				DialogBox((HINSTANCE)*ap->window, (LPCSTR)IDD_DIALOG1, (HWND)*ap->window, DlgProc);
			});
			makeTool.detach();
			break;
		}

		case T_FILE:
			// ファイル
			exec(ap);
			break;

		case T_FUNC:
			// モジュール関数
			func(ap);
			break;
		}
		return false;
	}

	// picojsonのオブジェクトに挿入
	void insertObject(picojson::object* o)
	{
		o->insert(std::make_pair("type", picojson::value( toolAndString[type] )));

		if (1 < timing.size()) {
			picojson::array a;

			for (auto count = 0; count < timing.size(); ++count) {
				a.push_back(picojson::value( timingAndString[timing[count]] ));
			}
			o->insert(std::make_pair("timing", picojson::value(a)));
		} else {
			o->insert(std::make_pair("timing", picojson::value( timingAndString[timing.back()] )));
		}

		switch (type) {
		case T_FILE: case T_FUNC:
			o->insert(std::make_pair("file", picojson::value((char*)address.data())));
			o->insert(std::make_pair("option", picojson::value((char*)option.data())));
			if (address == iconName) {
				break;
			}

		default:
			o->insert(std::make_pair("icon", picojson::value((char*)iconName.data())));
		}

		saved = true;
	}
};


// 複数のモジュールの管理をするクラス
class modules {
	std::vector<module> tooltips;	// ツールチップ管理
	bool locked = false;			// ロックされていて実行できない

	// json読み込んでツールの定義
	void readJson(appinfo* ap, char* fileName)
	{
		std::ifstream ifs(fileName);
		picojson::value v;
		ifs >> v;
		std::string err = picojson::get_last_error();
		if (!err.empty()) {
			OutputDebugString(err.data());
			OutputDebugString("\n");
			return;
		}

		picojson::object& o = v.get<picojson::object>();
		
		std::string filePath, iconPath, option;
		TOOL type;
		size_t order;
		std::vector<RUN_TIMING> timing;

		for (auto it = o.begin(); it != o.end(); it++) {
			iconPath.clear();
			order = -1;
			auto element = it->second.get<picojson::object>();

			if (element["type"].is<std::string>()) {
				type = toolAndString[element["type"].get<std::string>()];
			}
			if (element["file"].is<std::string>()) {
				filePath.assign(element["file"].get<std::string>().data());
			}
			if (element["icon"].is<std::string>()) {
				iconPath.assign(element["icon"].get<std::string>().data());
			}
			if (element["option"].is<std::string>()) {
				option.assign(element["option"].get<std::string>().data());
			}
			if (element["order"].is<double>()) {
				order = (size_t)element["order"].get<double>();
			}
			if (element["timing"].is<std::string>()) {
				timing = { timingAndString[element["timing"].get<std::string>()] };
			} else if (element["timing"].is<picojson::array>()) {
				auto a = element["timing"].get<picojson::array>();
				for (auto count = 0; count < a.size(); ++count) {
					if (a[count].is<std::string>()) {
						timing.push_back( timingAndString[a[count].get<std::string>()] );
					}
				}
			}

			char iconName[MAX_PATH] = "";
			if (iconPath.empty()) {
				ap->imgs->loadIcon((char*)filePath.c_str(), iconName);
			} else {
				ap->imgs->load((char*)iconPath.c_str(), iconName);
			}

			module* m;
			if (-1 != order) {
				m = this->add(order, iconName, type, &timing, true);
			} else {
				m = this->make(iconName, type, &timing, true);
			}
			
			switch (type) {
			case T_FILE:
			case T_FUNC:
				m->init((char*)filePath.data(), (char*)option.data());
				break;
			}
		}
	}
public:
	// 要素アクセス用
	module* at(size_t id)
	{
		return &tooltips[id];
	}
	module& operator[](size_t id)
	{
		return *at(id);
	}

	// 新しい要素を追加
	module* make(char* newIconName, TOOL newType, std::vector<RUN_TIMING>* newTiming, bool saved)
	{
		tooltips.push_back({ newIconName, newType, newTiming, saved });
		return &back();
	}
	module* add(char* newIconName, TOOL newType, std::vector<RUN_TIMING>* newTiming)
	{
		return make(newIconName, newType, newTiming, false);
	}
	void add(char* newIconName, TOOL newType, RUN_TIMING newTiming)
	{
		tooltips.push_back({ newIconName, newType, newTiming });
	}
	module* add(size_t id, char* newIconName, TOOL newType, std::vector<RUN_TIMING>* newTiming, bool saved)
	{
		if (tooltips.size() <= id) {
			tooltips.resize(id + 1);
		}
		if (!tooltips[id]) {
			OutputDebugString("module duplex write\n");
		}
		tooltips[id] = { newIconName, newType, newTiming, saved };
		return &tooltips[id];
	}

	// 最後にpush_backした要素を返す
	module& back()
	{
		return tooltips.back();
	}

	// 要素の数
	size_t size()
	{
		return tooltips.size();
	}

	// 空のmoduleの削除
	void eraseEmpty()
	{
		auto it = tooltips.begin();
		while (it != tooltips.end()) {
			if (it->runTiming().empty()) {
				OutputDebugString("erase timing empty module\n");
				it = tooltips.erase(it);
			} else ++it;
		}
	}

	// 実行
	bool execute(size_t id, appinfo* ap)
	{
		if (locked) {
			return false;
		}
		return tooltips[id].execute(ap);
	}

	// ファイルの列挙をしてjson読み込み
	void readExtension(appinfo* ap, char* startDirectory, char* extensions)
	{
		locked = true;

		HANDLE hFind;
		WIN32_FIND_DATA wfd;

		char directory[MAX_PATH], fileName[MAX_PATH];
		strcpy_s(directory, MAX_PATH, startDirectory);
		strcat_s(directory, MAX_PATH, extensions);

		hFind = FindFirstFile(directory, &wfd);
		if (INVALID_HANDLE_VALUE == hFind) {
			return;
		}

		do {
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				strcpy_s(fileName, MAX_PATH, startDirectory);
				strcat_s(fileName, MAX_PATH, wfd.cFileName);

				OutputDebugString(ap->appClass->strf("file:%s\n", fileName));
				readJson(ap, fileName);
			}
		} while (FindNextFile(hFind, &wfd));

		FindClose(hFind);

		this->eraseEmpty();

		locked = false;
	}

	// 保存してないモジュールのみ保存(全部一つのファイルに保存される)
	void saveExtension(appinfo* ap)
	{
		locked = true;

		picojson::object base;
		size_t contentCount = 0;
		std::string representative;

		this->eraseEmpty();
		ap->c->gettime();

		for (auto count = 0; count < tooltips.size(); ++count) {
			if (tooltips[count].isSaved()) {
				continue;
			}
			picojson::object o;
			tooltips[count].insertObject(&o);
			o.insert(std::make_pair("order", picojson::value((double)count)));

			base.insert(std::make_pair(
				ap->appClass->strf("%d_mod_%d_%02d%02d", ++contentCount, ap->c->year(), ap->c->mon(), ap->c->day()),
				picojson::value(o)
			));

			if (representative.empty()) {
				if (nullptr != tooltips[count].file()) {
					representative.assign(tooltips[count].file());
				}
			}
		}

		if (!contentCount) {
			OutputDebugString("nothing is adding extension\n");
			return;
		}

		std::string fileName;
		if (representative.empty()) {
			fileName = ap->appClass->strf("mods\\myex_%d_%02d%02d.json",
				ap->c->year(), ap->c->mon(), ap->c->day(), ap->c->hour(), ap->c->minute());
		} else {
			fileName.resize(MAX_PATH);
			GetFileTitle(representative.data(), (char*)fileName.c_str(), MAX_PATH);
			fileName = ap->appClass->strf("mods\\ex_%s.json", fileName.data());
		}

	retrypoint:
		std::fstream myExtension;
		myExtension.open(fileName, std::ios::in);
		if (myExtension) {
			myExtension.close();
			const char* message = ap->appClass->strf(
				"保存するファイル名が重複しています ->\n[%s]\n\n中止\t: ファイルの上書きをやめる\n再試行\t: ファイルを変更した\n無視\t: 上書きする",
				fileName.data()
			);
			auto result = MessageBox(nullptr,
				message, "clockex - 拡張ツールの保存",
				MB_ABORTRETRYIGNORE | MB_ICONWARNING
			);

			switch (result) {
			case IDABORT:	// 中止
				return;
			case IDRETRY:	// 再試行
				goto retrypoint;
			}
		}

		myExtension.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
		if (!myExtension) {
			OutputDebugString("could't open mods file -> \"");
			OutputDebugString(fileName.data());
			OutputDebugString("\"");
		}

		auto content = picojson::value(base).serialize(true);
		myExtension << content.data();

		locked = false;
	}
};
