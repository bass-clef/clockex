#pragma once

#include <deque>

#include "module.h"

// module のポインタを管理するクラス
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

	// moduleList に module のポインタを追加
	void add(module* m)
	{
		if (!moduleList.empty()) {
			if (addFirst(m)) return;
			if (addLast(m)) return;
			if (moduleList.back()->isLast()) {
				// 最後の要素に RT_LAST が登録されていた場合、最後からひとつ前に追加
				module* back = moduleList.back();
				moduleList.pop_back();

				moduleList.push_back(m);
				moduleList.push_back(back);

				return;
			}
		}

		moduleList.push_back(m);
	}

	// 優先追加
	bool addFirst(module* m)
	{
		if (!m->isFirst()) {
			return false;
		}

		moduleList.push_front(m);
		return true;
	}

	// 最後に追加
	bool addLast(module* m)
	{
		if (!m->isLast()) {
			return false;
		}

		moduleList.push_front(m);
		return true;
	}


	// moduleList に入ってる拡張を実行
	void execute(appinfo *ap)
	{
		for (const auto& module : moduleList) {
			module->execute(ap);
		}
	}
};

// task を RUN_TIMING で参照できるようにするためのクラス
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

	// 複数の RUN_TIMING を module から登録
	void add(module* m)
	{
		const std::vector<RUN_TIMING>* timing = &m->runTiming();
		for (auto count = 0; count < timing->size(); ++count) {
			list[timing->at(count)]->add(m);
		}
	}

	// 複数の module から 複数の RUN_TIMING を登録
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
