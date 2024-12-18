#pragma once
#ifndef _AFX
#include <windows.h>
#endif
#include <string>
#include <stdint.h>
#include "QJS.h"

QJS_API ValueHandle RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/);
QJS_API ValueHandle RunScriptFile(ContextHandle ctx, const char* filename, ValueHandle parent);

QJS_API ValueHandle CompileScript(ContextHandle ctx, const char* script, const char* filename/* = ""*/);
QJS_API ValueHandle RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);

class JsExtendDebugger
{
#define DEF_PROC(name) \
	decltype(::name)* name

#define SET_PROC(hDll, name) \
	this->name = (decltype(::name)*)::GetProcAddress(hDll, #name)

public:
	JsExtendDebugger()
	{
		hDll = LoadLibraryFromCurrentDir("JsExtendDebugger.dll");
		if (!hDll)
			return;

		SET_PROC(hDll, RunScript);
		SET_PROC(hDll, RunScriptFile);
		SET_PROC(hDll, CompileScript);
		SET_PROC(hDll, RunByteCode);

	}

	DEF_PROC(RunScript);
	DEF_PROC(RunScriptFile);
	DEF_PROC(CompileScript); 
	DEF_PROC(RunByteCode);


public:
	static JsExtendDebugger& Ins()
	{
		static JsExtendDebugger s_ins;
		return s_ins;
	}

	static HMODULE LoadLibraryFromCurrentDir(const char* dllName)
	{
		char selfPath[MAX_PATH];
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE hModule = ((::VirtualQuery(
			LoadLibraryFromCurrentDir, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
		::GetModuleFileNameA(hModule, selfPath, MAX_PATH);
		std::string moduleDir(selfPath);
		size_t idx = moduleDir.find_last_of('\\');
		moduleDir = moduleDir.substr(0, idx);
		std::string modulePath = moduleDir + "\\" + dllName;
		char curDir[MAX_PATH];
		::GetCurrentDirectoryA(MAX_PATH, curDir);
		::SetCurrentDirectoryA(moduleDir.c_str());
		HMODULE hDll = LoadLibraryA(modulePath.c_str());
		::SetCurrentDirectoryA(curDir);
		if (!hDll)
		{
			DWORD err = ::GetLastError();
			char buf[10];
			sprintf_s(buf, "%u", err);
			::MessageBoxA(NULL, ("�Ҳ���" + modulePath + "ģ��:" + buf).c_str(), "�Ҳ���ģ��", MB_OK | MB_ICONERROR);
		}
		return hDll;
	}
	~JsExtendDebugger()
	{
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

private:
	HMODULE hDll;
};

