#include "pch.h"
#include <windows.h>
#include "../../QJS/Extend.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include <sstream>
#include <iostream>
#include <tchar.h>
#include <vector>
#include "JsExtendBase.h"

// 判断文件是否存在
BOOL IsFileExist(const TCHAR* csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件夹是否存在
BOOL IsDirExist(const TCHAR* csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件或文件夹是否存在
BOOL IsPathExist(const TCHAR* csPath)
{
	DWORD dwAttrib = GetFileAttributes(csPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

//// 变变变变变种(听说会更快一点)，见备注1
//BOOL IsPathExist(const TCHAR* csPath)
//{
//	WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
//	return 0 != GetFileAttributesEx(csPath, GetFileExInfoStandard, &attrs);
//}

ValueHandle _jprocess_cwd(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	wchar_t dir[MAX_PATH] = { 0 };
	::GetCurrentDirectoryW(MAX_PATH, dir);

	std::string cwd = qjs.UnicodeToUtf8(ctx, dir);
	return qjs.NewStringJsValue(ctx, cwd.c_str());
}

ValueHandle _jprocess_addPath(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc != 1)
		return qjs.TheJsFalse();

	ValueHandle jprocess = qjs.GetNamedJsValue(ctx, "process", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jprocess))
		return qjs.TheJsFalse();
	
	ValueHandle jpaths = qjs.GetNamedJsValue(ctx, "paths", jprocess);
	if (!qjs.JsValueIsArray(jpaths))
		return qjs.TheJsFalse();

	auto jindexOf = qjs.GetNamedJsValue(ctx, "indexOf", jpaths);
	if (!qjs.JsValueIsFunction(jindexOf))
		return qjs.TheJsFalse();
	ValueHandle params[] = { argv[0] };
	ValueHandle jidx = qjs.CallJsFunction(ctx, jindexOf, params, sizeof(params) / sizeof(ValueHandle), jpaths);
	if (qjs.JsValueIsInt(jidx))
	{
		int idx = qjs.JsValueToInt(ctx, jidx, -1);
		if (idx != -1)//已有则跳过
			return qjs.TheJsFalse();
	}

	auto jpush = qjs.GetNamedJsValue(ctx, "push", jpaths);
	if (!qjs.JsValueIsFunction(jpush))
		return qjs.TheJsFalse();	
	qjs.CallJsFunction(ctx, jpush, params, sizeof(params) / sizeof(ValueHandle), jpaths);
	return qjs.TheJsTrue();
}

static void init_process(ContextHandle ctx)
{
	//process全局变量
	ValueHandle jprocess = qjs.GetNamedJsValue(ctx, "process", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jprocess))
	{
		jprocess = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "process", jprocess, qjs.GetGlobalObject(ctx));
	}

	//构造process.env到系统的所有变量
	std::vector<std::string> paths;
	ValueHandle jenv = qjs.GetNamedJsValue(ctx, "env", jprocess);
	if (!qjs.JsValueIsObject(jenv))
	{
		jenv = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "env", jenv, jprocess);
	}
	//环境变量字符串是以NULL分隔的，内存块以NULL结尾
	LPTCH lpvEnv = GetEnvironmentStrings();
	LPTSTR lpszVariable = (LPTSTR)lpvEnv;
	while (*lpszVariable)
	{
		//_tprintf(TEXT("%s\n"), lpszVariable);
		std::string var = qjs.UnicodeToUtf8(ctx, lpszVariable);
		size_t idx = var.find('=');
		if (idx != std::string::npos)
		{
			std::string key = var.substr(0, idx);
			if (strlen(key.c_str()) > 0)
			{
				std::string val = var.substr(idx + 1);
				ValueHandle item = qjs.NewStringJsValue(ctx, val.c_str());
				qjs.SetNamedJsValue(ctx, key.c_str(), item, jenv);

				//构造paths
				if (pystring::equal(key, "Path", true))
				{
					pystring::split(val, paths, ";");
				}
			}
		}

		lpszVariable += lstrlen(lpszVariable) + 1;   //移动指针
	}
	FreeEnvironmentStrings(lpvEnv);

	//构造process.exePath
	wchar_t wexePath[MAX_PATH + 1] = {0};
	::GetModuleFileNameW(NULL, wexePath, MAX_PATH);
	std::string exePath = qjs.UnicodeToUtf8(ctx, wexePath);
	ValueHandle jexePath = qjs.NewStringJsValue(ctx, exePath.c_str());
	qjs.SetNamedJsValue(ctx, "exePath", jexePath, jprocess);

	//构造process.pid
	DWORD pid = ::GetCurrentProcessId();
	ValueHandle jpid = qjs.NewInt64JsValue(ctx, pid);
	qjs.SetNamedJsValue(ctx, "pid", jpid, jprocess);
	
	//构造process.paths
	//  把exe所在的目录也算在里面
	paths.push_back(os_path::dirname(exePath));
	ValueHandle jpaths = qjs.GetNamedJsValue(ctx, "paths", jprocess);
	if (!qjs.JsValueIsArray(jpaths))
	{
		jpaths = qjs.NewArrayJsValue(ctx);
		for (size_t i = 0; i < paths.size(); i++)
		{
			qjs.SetIndexedJsValue(ctx, i, 
				qjs.NewStringJsValue(ctx, paths[i].c_str()), jpaths);
		}
		qjs.SetNamedJsValue(ctx, "paths", jpaths, jprocess);
	}
	//构造process.addPath
	ValueHandle jprocess_addPath = qjs.NewFunction(ctx, _jprocess_addPath, 1, NULL);
	qjs.SetNamedJsValue(ctx, "addPath", jprocess_addPath, jprocess);

	//构造process.cwd()
	ValueHandle jprocess_cwd = qjs.NewFunction(ctx, _jprocess_cwd, 0, NULL);
	qjs.SetNamedJsValue(ctx, "cwd", jprocess_cwd, jprocess);
}

QJS_API int _entry(ContextHandle ctx)
{
	init_process(ctx);

	return 0;//加载插件
}

QJS_API ValueHandle alert(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::wstring msg;
	if (argc > 0)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[0]);
		if (sz)
		{
			msg = qjs.Utf8ToUnicode(ctx, sz);
		}
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	std::wstring title;
	if (argc < 2 || qjs.JsValueIsUndefined(argv[1]))
		title = L"QJS";
	else
	{
		const char* sz = qjs.JsValueToString(ctx, argv[1]);
		if (sz)
			title = qjs.Utf8ToUnicode(ctx, sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	MessageBoxW(NULL, msg.c_str(), title.c_str(), 0);

	return qjs.TheJsUndefined();
}

QJS_API ValueHandle print(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::wstringstream ss;
	for (int i = 0; i < argc; i++)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[i]);
		if (sz)
			ss << qjs.Utf8ToUnicode(ctx, sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	std::wcout.imbue(std::locale("chs"));
	std::wcout << ss.str();
	return qjs.TheJsUndefined();
}

//可设定process.isDebug=true/false;来控制开关debug
QJS_API ValueHandle debug(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	ValueHandle jprocess = qjs.GetNamedJsValue(ctx, "process", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jprocess))
		return qjs.TheJsUndefined();
	
	ValueHandle jisDebug = qjs.GetNamedJsValue(ctx, "isDebug", jprocess);
	if (!qjs.JsValueIsBool(jisDebug))
		return qjs.TheJsUndefined();

	bool isDebug = qjs.JsValueToBool(ctx, jisDebug, false);
	if (isDebug) 
	{
		std::wstringstream ss;
		for (int i = 0; i < argc; i++)
		{
			const char* sz = qjs.JsValueToString(ctx, argv[i]);
			if (sz)
				ss << qjs.Utf8ToUnicode(ctx, sz);
			qjs.FreeJsValueToStringBuffer(ctx, sz);
		}

		MessageBoxW(NULL, ss.str().c_str(), L"Debug", 0);
	}

	return qjs.TheJsUndefined();
}

QJS_API ValueHandle debugObject(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.NewStringJsValue(ctx, "");

	std::string json;
	ValueHandle arrKeys = qjs.GetObjectPropertyKeys(ctx, argv[0], false, true);
	int64_t len = qjs.GetLength(ctx, arrKeys);
	for (size_t i = 0; i < len; i++)
	{
		ValueHandle jkey = qjs.GetIndexedJsValue(ctx, i, arrKeys);
		std::string key = qjs.JsValueToStdString(ctx, jkey);
		ValueHandle jval = qjs.GetNamedJsValue(ctx, key.c_str(), argv[0]);
		std::string val = qjs.JsValueToStdString(ctx, jval);
		json += key + ":" + val + "\n";
	}

	return qjs.NewStringJsValue(ctx, json.c_str());
}

std::wstring resolveAndUpdateFilePath(ContextHandle ctx, const std::wstring& filename)
{
	if (IsFileExist(filename.c_str()))
	{
		std::wstring wpath;
		if (filename.find(L':') != std::wstring::npos)
		{//全路径
			wpath = filename;
		}
		else
		{
			wchar_t dir[MAX_PATH] = { 0 };
			::GetCurrentDirectoryW(MAX_PATH, dir);
			wpath = os_pathw::join(dir, filename);
		}

		//添加查找路径
		ValueHandle jaddPath = qjs.RunScript(ctx, "process.addPath", qjs.TheJsNull(), "");
		if (qjs.JsValueIsFunction(jaddPath))
		{
			std::wstring wdir = os_pathw::dirname(wpath);
			std::string dir = qjs.UnicodeToUtf8(ctx, wdir.c_str());
			ValueHandle param[] = { qjs.NewStringJsValue(ctx, dir.c_str()) };
			qjs.CallJsFunction(ctx, jaddPath, param, sizeof(param) / sizeof(ValueHandle), qjs.TheJsNull());
		}

		return wpath;
	}

	if (filename.find(L':') != std::wstring::npos)
		return L"";

	ValueHandle jprocess = qjs.GetNamedJsValue(ctx, "process", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jprocess))
		return L"";

	ValueHandle jpaths = qjs.GetNamedJsValue(ctx, "paths", jprocess);
	if (!qjs.JsValueIsArray(jpaths))
		return L"";

	int64_t len = qjs.GetLength(ctx, jpaths);
	for (int i = len - 1; i >= 0; --i)
	{
		ValueHandle jitem = qjs.GetIndexedJsValue(ctx, i, jpaths);
		std::string item = qjs.JsValueToStdString(ctx, jitem);
		std::wstring path = qjs.Utf8ToUnicode(ctx, item.c_str());
		std::wstring filepath = os_pathw::join(path, filename);
		if (IsFileExist(filepath.c_str()))
			return filepath;
	}

	return L"";
}

QJS_API ValueHandle require(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "no filename input");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::string filename = qjs.JsValueToStdString(ctx, argv[0]);
	std::wstring wfilename = qjs.Utf8ToUnicode(ctx, filename.c_str());
	wfilename = resolveAndUpdateFilePath(ctx, wfilename);
	if (wfilename == L"")
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, (filename + " not exist.").c_str());
		return qjs.NewThrowJsValue(ctx, ex);
	}
	filename = qjs.UnicodeToAnsi(ctx, wfilename.c_str());

	ValueHandle jexports = qjs.NewObjectJsValue(ctx);
	if (!qjs.SetNamedJsValue(ctx, "exports", jexports, qjs.GetGlobalObject(ctx)))
		return qjs.TheJsUndefined();

	ValueHandle jmodule = qjs.NewObjectJsValue(ctx);
	if (!qjs.SetNamedJsValue(ctx, "module", jmodule, qjs.GetGlobalObject(ctx)))
		return qjs.TheJsUndefined();
	qjs.SetNamedJsValue(ctx, "exports", jexports, jmodule);

	ValueHandle jret = qjs.RunScriptFile(ctx, filename.c_str());
	if (qjs.JsValueIsException(jret))
		return jret;

	jexports = qjs.GetNamedJsValue(ctx, "exports", qjs.GetGlobalObject(ctx));
	qjs.DeleteNamedJsValue(ctx, "exports", qjs.GetGlobalObject(ctx));
	qjs.DeleteNamedJsValue(ctx, "module", qjs.GetGlobalObject(ctx));

	return jexports;
}

QJS_API ValueHandle include(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "input filename");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::string filename = qjs.JsValueToStdString(ctx, argv[0]);
	std::wstring wfilename = qjs.Utf8ToUnicode(ctx, filename.c_str());
	wfilename = resolveAndUpdateFilePath(ctx, wfilename);
	if (wfilename == L"")
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, (filename + " not exist.").c_str());
		return qjs.NewThrowJsValue(ctx, ex);
	}
	filename = qjs.UnicodeToAnsi(ctx, wfilename.c_str());

	ValueHandle jret = qjs.RunScriptFile(ctx, filename.c_str());

	return jret;
}

QJS_API void _completed(ContextHandle ctx)
{
	//console
	ValueHandle jconsole = qjs.GetNamedJsValue(ctx, "console", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jconsole))
	{
		jconsole = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "console", jconsole, qjs.GetGlobalObject(ctx));
	}

	//console.log
	ValueHandle jprint = qjs.NewFunction(ctx, print, 0, NULL);
	qjs.SetNamedJsValue(ctx, "log", jprint, jconsole);
}

QJS_API bool AddPath(ContextHandle ctx, const wchar_t* dir)
{
	//全路径却存在时需要添加查找路径
	ValueHandle jaddPath = qjs.RunScript(ctx, "process.addPath", qjs.TheJsNull(), "");
	if (qjs.JsValueIsFunction(jaddPath))
	{
		std::string sdir = qjs.UnicodeToUtf8(ctx, dir);
		ValueHandle param[] = { qjs.NewStringJsValue(ctx, sdir.c_str()) };
		ValueHandle jret = qjs.CallJsFunction(ctx, jaddPath, param, sizeof(param) / sizeof(ValueHandle), qjs.TheJsNull());
		if (qjs.JsValueIsBool(jret))
		{
			return qjs.JsValueToBool(ctx, jret, false);
		}
	}

	return false;
}

void OnFreeingContextCallback(ContextHandle ctx)
{

}

//第一个参数为触发函数，第二个参数为时间间隔，单位：毫秒，第三个参数为当前时间
ValueHandle setTimeoutCallback(ContextHandle ctx, int argc, ValueHandle* argv)
{
	if (argc != 3)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "setTimeout execute error");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	//检查是否已被取消
	auto jcancel = qjs.GetNamedJsValue(ctx, "cancel", argv[2]);
	if (qjs.JsValueToBool(ctx, jcancel, true))
		return qjs.TheJsUndefined();

	//如果start time未0则先不执行，继续计时
	auto jstart = qjs.GetNamedJsValue(ctx, "start", argv[2]);
	if (!qjs.JsValueIsInt(jstart))
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "setTimeout execute error");
		return qjs.NewThrowJsValue(ctx, ex);
	}
	int64_t start = qjs.JsValueToInt64(ctx, jstart, 0);
	if (start == 0)
	{
		jstart = qjs.NewInt64JsValue(ctx, ::GetTickCount64());
		qjs.SetNamedJsValue(ctx, "start", jstart, argv[2]);
		qjs.EnqueueJob(ctx, setTimeoutCallback, argv, argc);
		return qjs.TheJsUndefined();
	}
	
	ValueHandle jInterval = argv[1];
	int64_t interval = qjs.JsValueToInt64(ctx, jInterval, 0);
	if (::GetTickCount64() - start >= interval)
	{
		ValueHandle jFunc = argv[0];
		qjs.CallJsFunction(ctx, jFunc, NULL, 0, qjs.TheJsNull());
	}

	jstart = qjs.NewInt64JsValue(ctx, ::GetTickCount64());
	qjs.SetNamedJsValue(ctx, "start", jstart, argv[2]);
	qjs.EnqueueJob(ctx, setTimeoutCallback, argv, argc);

	return qjs.TheJsUndefined();
}

//第一个参数为触发函数，第二个参数为时间间隔，单位：毫秒
QJS_API ValueHandle setTimeout(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc != 2)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "setTimeout(function, interval)");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	//1.添加一个辅助参数，用于传递运行数据，例如：开始时间、取消对象等
	int argc_new = argc + 1;
	ValueHandle* argv_new = new ValueHandle[argc_new];
	size_t i;
	for (i = 0; i < argc; i++)
	{
		argv_new[i] = argv[i];
	}

	ValueHandle jsetTimeoutObj = qjs.NewObjectJsValue(ctx);
	
	ValueHandle jIsCanceled = qjs.NewBoolJsValue(ctx, false);
	qjs.SetNamedJsValue(ctx, "cancel", jIsCanceled, jsetTimeoutObj);

	ValueHandle jstartTime = qjs.NewInt64JsValue(ctx, ::GetTickCount64());
	qjs.SetNamedJsValue(ctx, "start", jstartTime, jsetTimeoutObj);
		
	argv_new[i++] = jsetTimeoutObj;

	if (!qjs.EnqueueJob(ctx, setTimeoutCallback, argv_new, argc_new))
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "setTimeout error");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	return jsetTimeoutObj;
}