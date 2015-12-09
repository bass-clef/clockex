#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <thread>
#include <fstream>

#include "appmain.h"
#include "resource.h"
#include "interchangeable.h"

// �v���g�^�C�v�錾
INT_PTR __stdcall DlgProc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);


// ���s�^�C�~���O��\��
enum RUN_TIMING : int {
	RT_ENUM_BEGIN,

	RT_SELECT,	// �I����
	RT_INIT,	// ��������(clockex���̂̏����������̌�)	�����ϐ��̉��ςȂ�
	RT_CALC,	// �v�Z��(clockex���̂̌v�Z�����̑O)		�v�Z�����̒��f
	RT_DRAW,	// �`�掞(clockex���̂̕`�揈���̑O)		������̕ϐ��̉���,�`��̒��f
	RT_EXIT,	// �I����(clockex���̂̏I�������̑O)
	RT_ADD,		// �ǉ���	�������g�̎����Ă΂��
	RT_DELETE,	// �폜��	����
	RT_FIRST,	// ��ԍŏ��Ɏ��s����	���̌�ɓǂݍ��܂ꂽRT_FIRST�����̊g���ɂ���ď㏑�������,�㏑�����ꂽ�ꍇ 2�Ԗڈȍ~�ɂ��Ȃ肤��
	RT_LAST,	// ��ԍŌ�Ɏ��s����	����

	RT_ENUM_END,
};

// �c�[���̎�ނ�\��
enum TOOL {
	T_NOTSELECTED = -1,		// �c�[���̓��삪�s��
	T_EXIT,		// �I������
	T_ADD,		// �ǉ�����
	T_FILE,		// �t�@�C�����Ăяo��
	T_FUNC,		// �֐����Ăяo��
};

// ���s�^�C�~���O�ƕ�����̌݊��p
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
// ��ނƕ�����̌݊��p
const interchangeableClass<TOOL, const std::string> toolAndString = {
	std::make_pair(T_NOTSELECTED, "T_NOTSELECTED"),
	std::make_pair(T_EXIT, "T_EXIT"),
	std::make_pair(T_ADD, "T_ADD"),
	std::make_pair(T_FILE, "T_FILE"),
	std::make_pair(T_FUNC, "T_FUNC"),
};


// ���W���[���Ǘ��N���X
class module
{
	std::string option;		// type�ɂ���Ċ֐������I�v�V����
	std::string address, iconName;		// �t�@�C����, �A�C�R����
	std::vector<RUN_TIMING> timing;		// ���s�^�C�~���O
	long result = 0;		// ����
	TOOL type;				// ���
	bool saved = false;		// �ۑ�������

	// ���s�^�C�~���O�̊m�F
	bool bootMatch(RUN_TIMING nowTiming)
	{
		if (timing.end() != std::find(timing.begin(), timing.end(), nowTiming)) {
			return true;
		}
		return false;
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
	// module�֐���appinfo��n���ČĂяo��
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

	// �t�@�C���ɕۑ�����Ă���̂�ǂݍ��񂾂�
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

	// picojson�̃I�u�W�F�N�g�ɑ}��
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


// �����̃��W���[���̊Ǘ�������N���X
class modules {
	std::vector<module> tooltips;	// �c�[���`�b�v�Ǘ�
	bool locked = false;			// ���b�N����Ă��Ď��s�ł��Ȃ�

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
	// �v�f�A�N�Z�X�p
	module* at(size_t id)
	{
		return &tooltips[id];
	}
	module& operator[](size_t id)
	{
		return *at(id);
	}

	// �V�����v�f��ǉ�
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

	// �Ō��push_back�����v�f��Ԃ�
	module& back()
	{
		return tooltips.back();
	}

	// �v�f�̐�
	size_t size()
	{
		return tooltips.size();
	}

	// ���module�̍폜
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

	// ���s
	bool execute(size_t id, appinfo* ap)
	{
		if (locked) {
			return false;
		}
		return tooltips[id].execute(ap);
	}

	// �t�@�C���̗񋓂�����json�ǂݍ���
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

	// �ۑ����ĂȂ����W���[���̂ݕۑ�(�S����̃t�@�C���ɕۑ������)
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
				"�ۑ�����t�@�C�������d�����Ă��܂� ->\n[%s]\n\n���~\t: �t�@�C���̏㏑������߂�\n�Ď��s\t: �t�@�C����ύX����\n����\t: �㏑������",
				fileName.data()
			);
			auto result = MessageBox(nullptr,
				message, "clockex - �g���c�[���̕ۑ�",
				MB_ABORTRETRYIGNORE | MB_ICONWARNING
			);

			switch (result) {
			case IDABORT:	// ���~
				return;
			case IDRETRY:	// �Ď��s
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
