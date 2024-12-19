#pragma once
#ifndef _AFX
#include <windows.h>
#endif
#include <string>
#include <stdint.h>
#include "QJS.h"

/** ע�⣺
* Ҫ��ʹ��debugger��������·����
  1.ʹ�������XXX_Debug�����Ӧ��ԭ��ִ�к�����RunScript/RunScriptFile/CompileScript/RunByteCode/CallJsFunction��������ÿ�ε��ö������һ���µ�debugger
  2.ʹ��ȫ��debugger�����ԣ�StartDebugger() -> RunScript/RunScriptFile/CompileScript/RunByteCode/CallJsFunction -> WaitDebuged()
*/

/* �÷�1 */
QJS_API ValueHandle RunScript_Debug(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/);
QJS_API ValueHandle RunScriptFile_Debug(ContextHandle ctx, const char* filename, ValueHandle parent);
QJS_API ValueHandle CompileScript_Debug(ContextHandle ctx, const char* script, const char* filename/* = ""*/);
QJS_API ValueHandle RunByteCode_Debug(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);
QJS_API ValueHandle CallJsFunction_Debug(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent);

/* �÷�2 */
//1.StartDebugger()
//2.RunScript/RunScriptFile/CompileScript/RunByteCode/CallJsFunction
//3.JsExtendDebugger::Ins().WaitDebuged();
QJS_API void StartDebugger();
QJS_API void WaitDebuged();

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

		SET_PROC(hDll, RunScript_Debug);
		SET_PROC(hDll, RunScriptFile_Debug);
		SET_PROC(hDll, CompileScript_Debug);
		SET_PROC(hDll, RunByteCode_Debug); 
		SET_PROC(hDll, CallJsFunction_Debug);
		SET_PROC(hDll, StartDebugger);
		SET_PROC(hDll, WaitDebuged);

	}

	DEF_PROC(RunScript_Debug);
	DEF_PROC(RunScriptFile_Debug);
	DEF_PROC(CompileScript_Debug);
	DEF_PROC(RunByteCode_Debug);
	DEF_PROC(CallJsFunction_Debug); 
	DEF_PROC(StartDebugger);
	DEF_PROC(WaitDebuged);


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

