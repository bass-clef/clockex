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
	T_EXE,		// EXEを呼び出す
	T_FUNC,		// 関数を呼び出す
};

class module
{
	HMODULE hModule;	// モジールハンドル
	std::string address;		// 実行ファイル名 / 関数名
	long result = false;

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

		case T_EXE:
			// 実行ファイル
			exec(ap);
			break;

		case T_FUNC:
			// モジュール関数
			func(ap);
			break;
		}
		return false;
	}

	const char* name()
	{
		return this->iconName.data();
	}
};
