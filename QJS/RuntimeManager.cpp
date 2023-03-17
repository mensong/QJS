#include "pch.h"
#include "RuntimeManager.h"
#include <quickjs.h>
#include <cassert>

extern void _DisposeRuntimeInner(RuntimeHandle runtime, bool removeFromMap);

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
