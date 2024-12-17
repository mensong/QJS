#include "pch.h"
#include "JsExtendDebugger.h"
#include "../../QJS/Extend.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "DlgDebugger.h"

DlgDebugger* g_debugerDlg = NULL;

//入口函数
QJS_API int _entry(ContextHandle ctx, void* user_data, int id)
{
	return 0;
}

//加载完成函数
QJS_API void _completed(ContextHandle ctx, void* user_data, int id)
{
	if (g_debugerDlg)
		return;

	g_debugerDlg = new DlgDebugger();
	g_debugerDlg->Create(DlgDebugger::IDD);
	g_debugerDlg->ShowWindow(SW_SHOW);

	qjs.SetDebuggerLineCallback(ctx, DlgDebugger::DebuggerLineCallback, g_debugerDlg);
}

//卸载函数
QJS_API void _unload(ContextHandle ctx, void* user_data, int id)
{
	if (!g_debugerDlg)
		return;
	g_debugerDlg->DestroyWindow();
	delete g_debugerDlg;
	g_debugerDlg = NULL;
}