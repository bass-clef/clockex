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
	T_EXE,		// EXE���Ăяo��
	T_FUNC,		// �֐����Ăяo��
};

class module
{
	HMODULE hModule;	// ���W�[���n���h��
	std::string address;		// ���s�t�@�C���� / �֐���
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

	// ���s�t�@�C�����I�v�V�����t���Ŏ��s
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

	// ���s
	bool execute(appinfo* ap)
	{
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

		case T_EXE:
			// ���s�t�@�C��
			exec(ap);
			break;

		case T_FUNC:
			// ���W���[���֐�
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
