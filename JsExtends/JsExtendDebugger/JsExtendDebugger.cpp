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

ValueHandle RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/)
{
	DlgDebugger* debuggerDlg = new DlgDebugger();
	debuggerDlg->Create(DlgDebugger::IDD);
	debuggerDlg->ShowWindow(SW_SHOW);
	qjs.SetDebuggerLineCallback(ctx, DlgDebugger::DebuggerLineCallback, debuggerDlg);
	qjs.SetDebuggerMode(ctx, true);

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
	qjs.SetDebuggerMode(ctx, true);

	//debuggerDlg->DestroyWindow();
	delete debuggerDlg;

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
	qjs.SetDebuggerMode(ctx, true);//必须使用调试模式编译脚本才有效
	std::string sFileSrc = script;
	ValueHandle jret = qjs.CompileScript(ctx, script, sFileSrc.c_str());
	return jret;
}

ValueHandle RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen)
{
	DlgDebugger* debuggerDlg = new DlgDebugger();
	debuggerDlg->Create(DlgDebugger::IDD);
	debuggerDlg->ShowWindow(SW_SHOW);
	qjs.SetDebuggerLineCallback(ctx, DlgDebugger::DebuggerLineCallback, debuggerDlg);
	qjs.SetDebuggerMode(ctx, true);

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
	qjs.SetDebuggerMode(ctx, true);

	//debuggerDlg->DestroyWindow();
	delete debuggerDlg;

	return jret;
}
