#include "pch.h"
#include "../../QJS/Extend.h"
#include <sstream>
#include <iostream>

QJS_API int entry(ContextHandle ctx)
{
	return 0;//¼ÓÔØ²å¼þ
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

	std::wcout << ss.str();
	return qjs.TheJsUndefined();
}

QJS_API ValueHandle debug(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
#ifdef _DEBUG
	std::wstringstream ss;
	for (int i = 0; i < argc; i++)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[i]);
		if (sz)
			ss << qjs.Utf8ToUnicode(sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	MessageBoxW(NULL, ss.str().c_str(), L"Debug", 0);
#endif
	return qjs.TheJsUndefined();
}

QJS_API ValueHandle require(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();

	std::string filename = qjs.JsValueToStdString(ctx, argv[0]);
	std::wstring wfilename = qjs.Utf8ToUnicode(filename.c_str());
	filename = qjs.UnicodeToAnsi(wfilename.c_str());

	ValueHandle jexports = qjs.NewObjectJsValue(ctx);
	if (!qjs.SetNamedJsValue(ctx, "exports", jexports, qjs.GetGlobalObject(ctx)))
		return qjs.TheJsUndefined();

	ValueHandle jmodule = qjs.NewObjectJsValue(ctx);
	if (!qjs.SetNamedJsValue(ctx, "module", jmodule, qjs.GetGlobalObject(ctx)))
		return qjs.TheJsUndefined();

	ValueHandle jret = qjs.RunScriptFile(ctx, filename.c_str());
	if (qjs.JsValueIsException(jret))
		return jret;

	//qjs.GetNamedJsValue(ctx, "exports", qjs.GetGlobalObject(ctx));
	//ValueHandle jmodule = qjs.GetNamedJsValue(ctx, "module", qjs.GetGlobalObject(ctx));
	//qjs.GetNamedJsValue(ctx, "exports", jmodule);
	

	return qjs.TheJsUndefined();
}
