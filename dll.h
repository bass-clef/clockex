#pragma once

#include <Windows.h>

#include <unordered_map>
#include <string>


// DLL 管理クラス
class dll
{
	std::unordered_map<std::string, HMODULE> dlls;

	// あったらtrue
	bool is(const char* dllName)
	{
		return dlls.end() != dlls.find(dllName);
	}

public:
	// DLL開放
	~dll()
	{
		auto it = dlls.begin();
		while (it != dlls.end()) {
			FreeLibrary(it->second);

			dlls.erase(it++);
		}
	}

	// 読み込み
	HMODULE load(const char* dllName)
	{
		if (is(dllName)) {
			return dlls[dllName];
		}

		HMODULE hModule = LoadLibrary(dllName);
		if (nullptr == hModule) return nullptr;

		dlls[dllName] = hModule;
		return hModule;
	}
};
