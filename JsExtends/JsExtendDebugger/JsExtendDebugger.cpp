#include "pch.h"
#include "JsExtendDebugger.h"
#include "../../QJS/Extend.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "DlgDebugger.h"

struct DebuggerInfo
{
	ContextHandle _ctx;
	DlgDebugger* debuggerDlg;

	bool old_debugMode; 
	FN_DebuggerLineCallback old_cb; 
	void* old_user_data;

	struct DebuggerInfo(ContextHandle ctx)
		: _ctx(ctx)
	{
		debuggerDlg = new DlgDebugger();
		debuggerDlg->Create(DlgDebugger::IDD);
		debuggerDlg->ShowWindow(SW_SHOW);

		old_debugMode = qjs.GetDebuggerMode(ctx);
		old_cb = NULL;
		old_user_data = NULL;
		qjs.GetDebuggerLineCallback(ctx, &old_cb, &old_user_data);

		qjs.SetDebuggerLineCallback(ctx, DlgDebugger::DebuggerLineCallback, debuggerDlg);
		qjs.SetDebuggerMode(ctx, true);
	}

	~DebuggerInfo()
	{
		if (debuggerDlg)
			delete debuggerDlg;
		qjs.SetDebuggerMode(_ctx, old_debugMode);
		qjs.SetDebuggerLineCallback(_ctx, old_cb, old_user_data);
	}
};

DebuggerInfo* g_debugger = NULL;

//入口函数
QJS_API int _entry(ContextHandle ctx, void* user_data, int id)
{
	//获得本插件dll文件名
	char selfPath[MAX_PATH];
	MEMORY_BASIC_INFORMATION mbi;
	HMODULE hModule = ((::VirtualQuery(
		_entry, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
	::GetModuleFileNameA(hModule, selfPath, MAX_PATH);
	std::string selfFileName = os_path::basename(selfPath);

	int extCount = 0;
	const int* extIds = qjs.GetExtendList(ctx, &extCount);
	for (int i = 0; i < extCount; i++)
	{
		if ((i + 1) == id)
			continue;
		const char* extFile = qjs.GetExtendFile(ctx, i + 1);
		if (!extFile)
			continue;

		std::string extFileName = os_path::basename(extFile);
		if (pystring::equal(extFileName, selfFileName, true))
			return 1;
	}
	return 0;
}

//加载完成函数
QJS_API void _completed(ContextHandle ctx, void* user_data, int id)
{
	g_debugger = new DebuggerInfo(ctx);
}

//卸载函数
QJS_API void _unload(ContextHandle ctx, void* user_data, int id)
{
	delete g_debugger;
	g_debugger = NULL;
}

void _waitDebuged(DebuggerInfo* debugger)
{
	if (debugger && debugger->debuggerDlg)
	{
		qjs.SetDebuggerMode(debugger->_ctx, false);//防止在运行后的表达式调试中又重复进入调试
		while (debugger->debuggerDlg->m_debugMode)
		{
			if (!DlgDebugger::DoEvent(debugger->debuggerDlg, debugger->_ctx))
				break;
			Sleep(1);
		}

		debugger->debuggerDlg->m_debugMode = true;
	}
}

ValueHandle RunScript_Debug(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/)
{
	DebuggerInfo debugger(ctx);

	std::string sFileSrc = script;
	ValueHandle jret = qjs.RunScript(ctx, script, parent, sFileSrc.c_str());

	//if (debugger.debuggerDlg)
	//{
	//	debugger.debuggerDlg->AppendResultText(_T("\r\n运行结果："), true);
	//	debugger.debuggerDlg->AppendResultText(ctx, jret, false);

	//	qjs.SetDebuggerMode(ctx, false);//防止在运行后的表达式调试中又重复进入调试
	//	while (debugger.debuggerDlg->m_debugMode)
	//	{
	//		if (!DlgDebugger::DoEvent(debugger.debuggerDlg, ctx))
	//			break;
	//		Sleep(10);
	//	}
	//}
	_waitDebuged(&debugger);

	return jret;
}

ValueHandle RunScriptFile_Debug(ContextHandle ctx, const char* filename, ValueHandle parent)
{
	if (!filename || filename[0] == '\0')
		return qjs.TheJsUndefined();

	size_t buf_len = 0;
	uint8_t* buf = qjs.LoadFile(ctx, &buf_len, filename);
	if (buf)
	{
		ValueHandle ret = RunScript_Debug(ctx, (const char*)buf, parent, NULL);
		qjs.FreeJsPointer(ctx, buf);
		return ret;
	}

	return qjs.TheJsUndefined();
}

ValueHandle CompileScript_Debug(ContextHandle ctx, const char* script, const char* filename/* = ""*/)
{
	//必须使用调试模式编译脚本才有效	
	bool old_debugMode = qjs.GetDebuggerMode(ctx);
	qjs.SetDebuggerMode(ctx, true);
	FN_DebuggerLineCallback old_cb = NULL;
	void* old_user_data = NULL;
	qjs.GetDebuggerLineCallback(ctx, &old_cb, &old_user_data);
	qjs.SetDebuggerLineCallback(ctx, NULL, NULL);

	std::string sFileSrc = script;
	ValueHandle jret = qjs.CompileScript(ctx, script, sFileSrc.c_str());

	qjs.SetDebuggerMode(ctx, old_debugMode);
	qjs.SetDebuggerLineCallback(ctx, old_cb, old_user_data);

	return jret;
}

ValueHandle RunByteCode_Debug(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen)
{
	DebuggerInfo debugger(ctx);

	ValueHandle jret = qjs.RunByteCode(ctx, byteCode, byteCodeLen);

	_waitDebuged(&debugger);

	return jret;
}

QJS_API ValueHandle CallJsFunction_Debug(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent)
{
	DebuggerInfo debugger(ctx);

	ValueHandle jret = qjs.CallJsFunction(ctx, jsFunction, args, argc, parent);

	_waitDebuged(&debugger);

	return jret;
}

QJS_API void WaitDebuged()
{
	_waitDebuged(g_debugger);
}
