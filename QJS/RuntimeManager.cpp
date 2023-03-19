#include "pch.h"
#include "RuntimeManager.h"
#include <quickjs.h>
#include <cassert>

extern void _DisposeRuntimeInner(RuntimeHandle runtime, bool removeFromMap);

void RuntimeManager::AddRuntime(RuntimeHandle rt)
{
	_runtimeSet.insert(rt);
}

void RuntimeManager::AddContext(RuntimeHandle rt, ContextHandle ctx)
{
	_contextMap[rt].insert(ctx);
}

bool RuntimeManager::AddValue(ContextHandle ctx, ValueHandle value)
{
	auto itFinder = _valueMap[ctx].find(value);

	_valueMap[ctx].insert(value);

	if (itFinder != _valueMap[ctx].end())
	{
		JS_FreeValue((JSContext*)ctx, (JSValue)value);
		return true;
	}

	return false;
}

RuntimeManager::RuntimeManager()
{
}

RuntimeManager::~RuntimeManager()
{
	for (auto it = _runtimeSet.begin(); it != _runtimeSet.end(); ++it)
	{
		_DisposeRuntimeInner(*it, false);
	}
	_runtimeSet.clear();
}
