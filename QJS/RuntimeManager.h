#pragma once
#include <set>
#include <map>
#include <vector>
#include "QJS.h"

class RuntimeManager
{
public:
	std::set<RuntimeHandle> _runtimeSet;
	std::map<RuntimeHandle, std::set<ContextHandle> > _contextMap;
	std::map<ContextHandle, std::set<ValueHandle> > _valueMap;

	void AddRuntime(RuntimeHandle rt);
	void AddContext(RuntimeHandle rt, ContextHandle ctx);
	bool AddValue(ContextHandle ctx, ValueHandle value);

public:
	RuntimeManager();
	~RuntimeManager();
};

