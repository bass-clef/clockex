#pragma once

#include <deque>

#include "module.h"

// module �̃|�C���^���Ǘ�����N���X
class task
{
	std::deque<module*> moduleList;

public:
	module* operator[](size_t id)
	{
		return moduleList[id];
	}

	void clear()
	{
		moduleList.clear();
	}

	// moduleList �� module �̃|�C���^��ǉ�
	void add(module* m)
	{
		if (!moduleList.empty()) {
			if (addFirst(m)) return;
			if (addLast(m)) return;
			if (moduleList.back()->isLast()) {
				// �Ō�̗v�f�� RT_LAST ���o�^����Ă����ꍇ�A�Ōォ��ЂƂO�ɒǉ�
				module* back = moduleList.back();
				moduleList.pop_back();

				moduleList.push_back(m);
				moduleList.push_back(back);

				return;
			}
		}

		moduleList.push_back(m);
	}

	// �D��ǉ�
	bool addFirst(module* m)
	{
		if (!m->isFirst()) {
			return false;
		}

		moduleList.push_front(m);
		return true;
	}

	// �Ō�ɒǉ�
	bool addLast(module* m)
	{
		if (!m->isLast()) {
			return false;
		}

		moduleList.push_front(m);
		return true;
	}


	// moduleList �ɓ����Ă�g�������s
	void execute(appinfo *ap)
	{
		for (const auto& module : moduleList) {
			module->execute(ap);
		}
	}
};

// task �� RUN_TIMING �ŎQ�Ƃł���悤�ɂ��邽�߂̃N���X
class tasklist
{
	std::unordered_map<RUN_TIMING, task*> list;

public:
	tasklist()
	{
		for (int count = RT_ENUM_BEGIN+1; count < RT_ENUM_END; ++count) {
			list[(RUN_TIMING)count] = new task;
		}
	}
	~tasklist()
	{
		for (int count = RT_ENUM_BEGIN+1; count < RT_ENUM_END; ++count) {
			delete list[(RUN_TIMING)count];
		}
	}

	task& operator[](RUN_TIMING rt)
	{
		return *list[rt];
	}

	// ������ RUN_TIMING �� module ����o�^
	void add(module* m)
	{
		const std::vector<RUN_TIMING>* timing = &m->runTiming();
		for (auto count = 0; count < timing->size(); ++count) {
			list[timing->at(count)]->add(m);
		}
	}

	// ������ module ���� ������ RUN_TIMING ��o�^
	void allocation(modules* ms)
	{
		for (const auto& l : list) {
			l.second->clear();
		}
		for (auto count = 0; count < ms->size(); ++count) {
			add(ms->at(count));
		}
	}
};
