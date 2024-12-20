#include "pch.h"
#include "JsExtendDebugger.h"
#include "../../QJS/Extend.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "DlgDebugger.h"

struct DebuggerInfo;

bool g_debuggerExtendLoaded = false;
bool g_enableDebugger = false;
bool g_old_debugMode = false;
FN_DebuggerLineCallback g_old_cb = NULL;
void* g_old_user_data = NULL;
std::set<std::string> g_ignoredSrcFilter;
DebuggerInfo* g_lastDebugger = NULL;
std::map<std::string, DebuggerInfo*> g_src2debugger;

struct DebuggerInfo
{
	ContextHandle _ctx;
	DlgDebugger* debuggerDlg;

	struct DebuggerInfo(ContextHandle ctx)
		: _ctx(ctx)
	{
		debuggerDlg = new DlgDebugger();
		debuggerDlg->Create(DlgDebugger::IDD);
		debuggerDlg->ShowWindow(SW_HIDE);
		debuggerDlg->m_ignoredSrc = &g_ignoredSrcFilter;
	}

	~DebuggerInfo()
	{
		if (debuggerDlg)
		{
			debuggerDlg->DestroyWindow();
			delete debuggerDlg;
			debuggerDlg = NULL;
		}
	}
};

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

	g_debuggerExtendLoaded = true;
	return 0;
}

void DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data)
{
	ValueHandle backtrace = qjs.GetDebuggerBacktrace(ctx, pc);
	std::wstring funcName;
	if (qjs.JsValueIsArray(backtrace))
	{
		ValueHandle jframe = qjs.GetIndexedJsValue(ctx, 0, backtrace);
		ValueHandle jfilename = qjs.GetNamedJsValue(ctx, "filename", jframe);
		std::string src = qjs.JsValueToStdString(ctx, jfilename);

		//判断是否已忽略
		if (g_ignoredSrcFilter.find(src) != g_ignoredSrcFilter.end())
		{
			return;
		}

		auto itFinder = g_src2debugger.find(src);
		if (itFinder == g_src2debugger.end())
		{
			DebuggerInfo* pDebugger = new DebuggerInfo(ctx);
			g_src2debugger.insert(std::make_pair(src, pDebugger));
			itFinder = g_src2debugger.find(src);
		}

		//隐藏掉其它窗口
		for (auto it = g_src2debugger.begin(); it != g_src2debugger.end(); ++it)
		{
			if (it->second->debuggerDlg != itFinder->second->debuggerDlg)
				it->second->debuggerDlg->ShowWindow(SW_HIDE);
		}

		//如果从其它debugger切换回来的，无论如何都显示出来当前debugger
		if (itFinder->second != g_lastDebugger)
		{
			itFinder->second->debuggerDlg->OnDebuggerChanged();
			g_lastDebugger = itFinder->second;
		}

		itFinder->second->debuggerDlg->ShowWindow(SW_SHOW);
		itFinder->second->debuggerDlg->BringWindowToTop();
		itFinder->second->debuggerDlg->DebuggerLineCallback(
			ctx, line_no, pc, user_data, src.c_str());
	}
}

//加载完成函数
QJS_API void _completed(ContextHandle ctx, void* user_data, int id)
{
	g_old_debugMode = qjs.GetDebuggerMode(ctx);
	g_old_cb = NULL;
	g_old_user_data = NULL;
	qjs.GetDebuggerLineCallback(ctx, &g_old_cb, &g_old_user_data);

	qjs.SetDebuggerLineCallback(ctx, DebuggerLineCallback, user_data);
	qjs.SetDebuggerMode(ctx, true);
}

//卸载函数
QJS_API void _unload(ContextHandle ctx, void* user_data, int id)
{
	qjs.SetDebuggerMode(ctx, g_old_debugMode);
	qjs.SetDebuggerLineCallback(ctx, g_old_cb, g_old_user_data);

	for (auto it = g_src2debugger.begin(); it != g_src2debugger.end(); ++it)
	{
		delete it->second;
	}
	g_src2debugger.clear();

	g_debuggerExtendLoaded = false;
	g_enableDebugger = false;
	g_old_debugMode = false;
	g_old_cb = NULL;
	g_old_user_data = NULL;
	g_ignoredSrcFilter.clear();
	g_lastDebugger = NULL;
}

QJS_API bool SetEnableDebugger(ContextHandle ctx, bool b)
{
	if (!g_debuggerExtendLoaded)
		return false;

	if (g_enableDebugger != b)
	{
		g_enableDebugger = b;

		qjs.SetDebuggerMode(ctx, b);
		g_lastDebugger = NULL;
	}

	return true;
}