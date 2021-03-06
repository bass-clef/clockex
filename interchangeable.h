#pragma once

#include <initializer_list>
#include <algorithm>

// 互換クラス
template<class classA, class classB>
class interchangeableClass
{
	using pairAB = std::pair<classA, classB>;

	std::vector< pairAB > pairs;

	classB secondFromFirst(classA a) const
	{
		return std::find_if(pairs.begin(), pairs.end(), [&](const pairAB elem) -> bool {
			return elem.first == a;
		})->second;
	}

	classA firstFromSecond(classB b) const
	{
		return std::find_if(pairs.begin(), pairs.end(), [&](const pairAB elem) -> bool {
			return elem.second == b;
		})->first;
	}

public:
	interchangeableClass(std::initializer_list< pairAB > valueList)
	{
		for (const auto& value : valueList) {
			pairs.push_back(value);
		}
	}

	// classAからclassBを取得
	classB operator[](classA a) const
	{
		return secondFromFirst(a);
	}

	// classBからclassAを取得
	classA operator[](classB b) const
	{
		return firstFromSecond(b);
	}
};
