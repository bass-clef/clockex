#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <thread>

#include "appmain.h"
#include "resource.h"


LRESULT __stdcall DlgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);


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
	T_FILE,		// ファイルを呼び出す
	T_FUNC,		// 関数を呼び出す
};

// モジュール管理クラス
class module
{
	HMODULE hModule;		// モジールハンドル
	std::string address, iconName;		// ファイル名 / 関数名, アイコン名
	std::vector<RUN_TIMING> timing;		// 実行タイミング
	long result = false;	// 結果
	TOOL type;				// 種類

	// 実行タイミングの確認
	bool bootMatch(RUN_TIMING nowTiming)
	{
		if (timing.end() != std::find(timing.begin(), timing.end(), nowTiming)) {
			return true;
		}
		return false;
	}
public:

	module() {}
	module(char* iconName, TOOL type, RUN_TIMING timing)
	{
		this->iconName = iconName;
		this->type = type;
		this->timing.push_back(timing);
	}
	module(char* iconName, TOOL type, std::vector<RUN_TIMING>* timing)
	{
		this->iconName = iconName;
		this->type = type;
		this->timing = *timing;
	}

	operator int()
	{
		return this->type;
	}

	const char* name()
	{
		return this->iconName.data();
	}

	void command(char* command)
	{
		address.assign(command);
	}
	void library(char* moduleName, char* funcName)
	{
		hModule = LoadLibrary(moduleName);
		address = funcName;
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
		typedef bool(*proc_type)(appinfo*);
		proc_type proc = (proc_type)GetProcAddress(hModule, address.data());

		std::thread execute([&]() {
			result = proc(ai);
		});
		execute.detach();
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
};


// 複数のモジュールの管理をするクラス
class modules {
	std::vector<module> tooltips;	// ツールチップ管理

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
		std::vector<RUN_TIMING> timing;

		for (auto it = o.begin(); it != o.end(); it++) {
			auto element = it->second.get<picojson::object>();

			if (element["type"].is<double>()) {
				type = (TOOL)(int)element["type"].get<double>();
				OutputDebugString(ap->appClass->strf("modname:[%s] type:%d\n", it->first.data(), type));
			}
			if (element["file"].is<std::string>()) {
				filePath.assign(element["file"].get<std::string>().data());
				OutputDebugString(ap->appClass->strf("file[%s]\n", filePath.data()));
			}
			if (element["icon"].is<std::string>()) {
				iconPath.assign(element["icon"].get<std::string>().data());
				OutputDebugString(ap->appClass->strf("icon[%s]\n", iconPath.data()));
			}
			if (element["option"].is<std::string>()) {
				option.assign(element["option"].get<std::string>().data());
				OutputDebugString(ap->appClass->strf("option[%s]\n", option.data()));
			}
			if (element["timing"].is<double>()) {
				timing = { (RUN_TIMING)(int)element["timing"].get<double>() };
				OutputDebugString(ap->appClass->strf("timing:%d\n", timing.back()));
			} else if (element["timing"].is<picojson::array>()) {
				auto a = element["timing"].get<picojson::array>();
				for (auto count = 0; count < a.size(); ++count) {
					if (a[count].is<double>()) {
						timing.push_back((RUN_TIMING)(int)a[count].get<double>());
						OutputDebugString(ap->appClass->strf("%d", timing.back()));
					}
				}
				OutputDebugString("\n");
			}
			OutputDebugString("\n");

			char iconName[MAX_PATH];
			if (iconPath.empty()) {
				ap->imgs->loadIcon((char*)filePath.data(), iconName);
			} else {
				ap->imgs->load((char*)iconPath.data(), iconName);
			}

			this->add(iconName, type, &timing);

			switch (type) {
			case TOOL::T_FILE:
				if (option.size()) {
					filePath.append(" ");
					filePath.append(option.data());
				}

				this->back().command((char*)filePath.c_str());
				break;

			case TOOL::T_FUNC:
				this->back().library((char*)filePath.data(), (char*)option.data());
			}
		}
	}
public:
	// 要素アクセス用
	module& operator[](size_t id) {
		return tooltips[id];
	}

	// 新しい要素を追加
	void add(char* newIconName, TOOL newType, std::vector<RUN_TIMING>* newTiming)
	{
		tooltips.push_back({ newIconName, newType, newTiming });
	}
	// 新しい要素を追加
	void add(char* newIconName, TOOL newType, RUN_TIMING newTiming)
	{
		tooltips.push_back({ newIconName, newType, newTiming });
	}

	// 最後にpush_backした要素を返す
	module back()
	{
		return tooltips.back();
	}

	// 要素の数
	size_t size()
	{
		return tooltips.size();
	}
	void resize(size_t newSize)
	{
		tooltips.resize(newSize);
	}

	// ファイルの列挙をしてjson読み込み
	void readExtension(appinfo* ap, char* startDirectory, char* extensions)
	{
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
	}
};
