#pragma once

#include <Windows.h>

#include <unordered_map>
#include <string>


// DLL �Ǘ��N���X
class dll
{
	std::unordered_map<std::string, HMODULE> dlls;

	// ��������true
	bool is(const char* dllName)
	{
		return dlls.end() != dlls.find(dllName);
	}

public:
	// DLL�J��
	~dll()
	{
		auto it = dlls.begin();
		while (it != dlls.end()) {
			FreeLibrary(it->second);

			dlls.erase(it++);
		}
	}

	// �ǂݍ���
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
