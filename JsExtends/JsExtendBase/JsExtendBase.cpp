#include "pch.h"
#include <windows.h>
#include "../../QJS/Extend.h"
#include <sstream>
#include <iostream>

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

QJS_API int _entry(ContextHandle ctx)
{

	ValueHandle jENV = qjs.GetNamedJsValue(ctx, "ENV", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jENV))
	{
		jENV = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "ENV", jENV, qjs.GetGlobalObject(ctx));
	}

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
			msg = qjs.Utf8ToUnicode(sz);
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
			title = qjs.Utf8ToUnicode(sz);
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
			ss << qjs.Utf8ToUnicode(sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	std::wcout.imbue(std::locale("chs"));
	std::wcout << ss.str();
	return qjs.TheJsUndefined();
}

//可设定ENV.isDebug=true/false;来控制开关debug
QJS_API ValueHandle debug(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	ValueHandle jENV = qjs.GetNamedJsValue(ctx, "ENV", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jENV))
		return qjs.TheJsUndefined();
	
	ValueHandle jisDebug = qjs.GetNamedJsValue(ctx, "isDebug", jENV);
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
				ss << qjs.Utf8ToUnicode(sz);
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

QJS_API ValueHandle require(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "no filename input");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::string filename = qjs.JsValueToStdString(ctx, argv[0]);
	std::wstring wfilename = qjs.Utf8ToUnicode(filename.c_str());
	if (!IsFileExist(wfilename.c_str()))
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, (filename + " not exist.").c_str());
		return qjs.NewThrowJsValue(ctx, ex);
	}
	filename = qjs.UnicodeToAnsi(wfilename.c_str());

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
	std::wstring wfilename = qjs.Utf8ToUnicode(filename.c_str());
	if (!IsFileExist(wfilename.c_str()))
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, (filename + " not exist.").c_str());
		return qjs.NewThrowJsValue(ctx, ex);
	}
	filename = qjs.UnicodeToAnsi(wfilename.c_str());

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
