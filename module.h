#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <thread>

#include "appmain.h"
#include "resource.h"


LRESULT __stdcall DlgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);


enum RUN_TIMING {
	RT_SELECT,	// �I����
	RT_INIT,	// ��������(clockex���̂̏����������̌�)	�����ϐ��̉��ςȂ�
	RT_CALC,	// �v�Z��(clockex���̂̌v�Z�����̌�)		�ϐ��̉���
	RT_DRAW,	// �`�掞(clockex���̂̕`�揈���̑O)		�`��̒��f,�t�b�N
	RT_EXIT,	// �I����(clockex���̂̏I�������̑O)

};

// �c�[���̎�ނ�\��
enum TOOL {
	T_NOTSELECTED = -1,		// �c�[���̓��삪�s��
	T_EXIT,		// �I������
	T_ADD,		// �ǉ�����
	T_FILE,		// �t�@�C�����Ăяo��
	T_FUNC,		// �֐����Ăяo��
};

// ���W���[���Ǘ��N���X
class module
{
	HMODULE hModule;		// ���W�[���n���h��
	std::string address, iconName;		// �t�@�C���� / �֐���, �A�C�R����
	std::vector<RUN_TIMING> timing;		// ���s�^�C�~���O
	long result = false;	// ����
	TOOL type;				// ���

	// ���s�^�C�~���O�̊m�F
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

	// �t�@�C�����I�v�V�����t���Ŏ��s
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
	// module�֐���appinfo��n���ČĂяo��
	void func(appinfo* ai)
	{
		typedef bool(*proc_type)(appinfo*);
		proc_type proc = (proc_type)GetProcAddress(hModule, address.data());

		std::thread execute([&]() {
			result = proc(ai);
		});
		execute.detach();
	}

	// ���s
	bool execute(appinfo* ap)
	{
		if (!bootMatch(ap->timing)) return false;

		switch (type) {
		case T_EXIT:
			// �I��
			return true;

		case T_ADD: {
			// ���g�̒ǉ�
			std::thread makeTool([&]() {
				DialogBox((HINSTANCE)*ap->window, (LPCSTR)IDD_DIALOG1, (HWND)*ap->window, DlgProc);
			});
			makeTool.detach();
			break;
		}

		case T_FILE:
			// �t�@�C��
			exec(ap);
			break;

		case T_FUNC:
			// ���W���[���֐�
			func(ap);
			break;
		}
		return false;
	}
};


// �����̃��W���[���̊Ǘ�������N���X
class modules {
	std::vector<module> tooltips;	// �c�[���`�b�v�Ǘ�

	// json�ǂݍ���Ńc�[���̒�`
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
	// �v�f�A�N�Z�X�p
	module& operator[](size_t id) {
		return tooltips[id];
	}

	// �V�����v�f��ǉ�
	void add(char* newIconName, TOOL newType, std::vector<RUN_TIMING>* newTiming)
	{
		tooltips.push_back({ newIconName, newType, newTiming });
	}
	// �V�����v�f��ǉ�
	void add(char* newIconName, TOOL newType, RUN_TIMING newTiming)
	{
		tooltips.push_back({ newIconName, newType, newTiming });
	}

	// �Ō��push_back�����v�f��Ԃ�
	module back()
	{
		return tooltips.back();
	}

	// �v�f�̐�
	size_t size()
	{
		return tooltips.size();
	}
	void resize(size_t newSize)
	{
		tooltips.resize(newSize);
	}

	// �t�@�C���̗񋓂�����json�ǂݍ���
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
