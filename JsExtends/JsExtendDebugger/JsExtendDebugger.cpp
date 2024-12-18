#include "pch.h"
#include "JsExtendDebugger.h"
#include "../../QJS/Extend.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "DlgDebugger.h"

//入口函数
QJS_API int _entry(ContextHandle ctx, void* user_data, int id)
{
	return 0;
}

//加载完成函数
QJS_API void _completed(ContextHandle ctx, void* user_data, int id)
{
	
}

//卸载函数
QJS_API void _unload(ContextHandle ctx, void* user_data, int id)
{

}

void switchDebuggerMode(ContextHandle ctx, bool debugMode, FN_DebuggerLineCallback cb, void* user_data,
	bool* old_debugMode, FN_DebuggerLineCallback* old_cb, void** old_user_data)
{
	*old_debugMode = qjs.GetDebuggerMode(ctx);
	*old_cb = NULL;
	*old_user_data = NULL;
	qjs.GetDebuggerLineCallback(ctx, old_cb, old_user_data);

	qjs.SetDebuggerLineCallback(ctx, cb, user_data);
	qjs.SetDebuggerMode(ctx, debugMode);
}

ValueHandle RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/)
{
	DlgDebugger* debuggerDlg = new DlgDebugger();
	debuggerDlg->Create(DlgDebugger::IDD);
	debuggerDlg->ShowWindow(SW_SHOW);

	bool old_debugMode; FN_DebuggerLineCallback old_cb; void* old_user_data;
	switchDebuggerMode(ctx, true, DlgDebugger::DebuggerLineCallback, debuggerDlg,
		&old_debugMode, &old_cb, &old_user_data);

	std::string sFileSrc = script;
	ValueHandle jret = qjs.RunScript(ctx, script, parent, sFileSrc.c_str());

	debuggerDlg->AppendResultText(_T("\r\n运行结果："), true);
	debuggerDlg->AppendResultText(ctx, jret, false);

	qjs.SetDebuggerMode(ctx, false);//防止在运行后的表达式调试中又重复进入调试
	while (debuggerDlg->m_debugMode)
	{
		if (!DlgDebugger::DoEvent(debuggerDlg, ctx))
			break;
		Sleep(10);
	}	
	//debuggerDlg->DestroyWindow();
	delete debuggerDlg;

	qjs.SetDebuggerMode(ctx, old_debugMode);
	qjs.SetDebuggerLineCallback(ctx, old_cb, old_user_data);

	return jret;
}

ValueHandle RunScriptFile(ContextHandle ctx, const char* filename, ValueHandle parent)
{
	if (!filename || filename[0] == '\0')
		return qjs.TheJsUndefined();

	size_t buf_len = 0;
	uint8_t* buf = qjs.LoadFile(ctx, &buf_len, filename);
	if (buf)
	{
		ValueHandle ret = RunScript(ctx, (const char*)buf, parent, NULL);
		qjs.FreeJsPointer(ctx, buf);
		return ret;
	}

	return qjs.TheJsUndefined();
}

ValueHandle CompileScript(ContextHandle ctx, const char* script, const char* filename/* = ""*/)
{
	//必须使用调试模式编译脚本才有效
	bool old_debugMode; FN_DebuggerLineCallback old_cb; void* old_user_data;
	switchDebuggerMode(ctx, true, NULL, NULL,
		&old_debugMode, &old_cb, &old_user_data);

	std::string sFileSrc = script;
	ValueHandle jret = qjs.CompileScript(ctx, script, sFileSrc.c_str());

	qjs.SetDebuggerMode(ctx, old_debugMode);
	qjs.SetDebuggerLineCallback(ctx, old_cb, old_user_data);

	return jret;
}

ValueHandle RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen)
{
	DlgDebugger* debuggerDlg = new DlgDebugger();
	debuggerDlg->Create(DlgDebugger::IDD);
	debuggerDlg->ShowWindow(SW_SHOW);
	
	bool old_debugMode; FN_DebuggerLineCallback old_cb; void* old_user_data;
	switchDebuggerMode(ctx, true, DlgDebugger::DebuggerLineCallback, debuggerDlg,
		&old_debugMode, &old_cb, &old_user_data);

	ValueHandle jret = qjs.RunByteCode(ctx, byteCode, byteCodeLen);

	debuggerDlg->AppendResultText(_T("\r\n运行结果："), true);
	debuggerDlg->AppendResultText(ctx, jret, false);

	qjs.SetDebuggerMode(ctx, false);//防止在运行后的表达式调试中又重复进入调试
	while (debuggerDlg->m_debugMode)
	{
		if (!DlgDebugger::DoEvent(debuggerDlg, ctx))
			break;
		Sleep(10);
	}

	//debuggerDlg->DestroyWindow();
	delete debuggerDlg;

	qjs.SetDebuggerMode(ctx, old_debugMode);
	qjs.SetDebuggerLineCallback(ctx, old_cb, old_user_data);

	return jret;
}
