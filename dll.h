#pragma once

#include <Windows.h>

#include <unordered_map>
#include <string>


// DLL ä«óùÉNÉâÉX
class dll
{
	std::unordered_map<std::string, HMODULE> dlls;

	// Ç†Ç¡ÇΩÇÁtrue
	bool is(const char* dllName)
	{
		return dlls.end() != dlls.find(dllName);
	}

public:
	// DLLäJï˙
	~dll()
	{
		auto it = dlls.begin();
		while (it != dlls.end()) {
			FreeLibrary(it->second);

			dlls.erase(it++);
		}
	}

	// ì«Ç›çûÇ›
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
