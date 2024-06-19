// JsExtendPath.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../../QJS/Extend.h"
#include "pystring.h"

QJS_API int _entry(ContextHandle ctx)
{
	return 0;
}

QJS_API ValueHandle basename(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	std::string dir = os_path::basename(path);
	return qjs.NewStringJsValue(ctx, dir.c_str());
}

QJS_API ValueHandle basename_no_ext(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	std::string dir = os_path::basename_no_ext(path);
	return qjs.NewStringJsValue(ctx, dir.c_str());
}

QJS_API ValueHandle extension(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	std::string dir = os_path::extension(path);
	return qjs.NewStringJsValue(ctx, dir.c_str());
}

QJS_API ValueHandle dirname(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	std::string dir = os_path::dirname(path);
	return qjs.NewStringJsValue(ctx, dir.c_str());
}

QJS_API ValueHandle isabs(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	bool b = os_path::isabs(path);
	return qjs.NewBoolJsValue(ctx, b);
}

QJS_API ValueHandle abspath(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	std::string pwd = qjs.JsValueToStdString(ctx, argv[1]);
	std::string res = os_path::abspath(path, pwd);
	return qjs.NewStringJsValue(ctx, res.c_str());
}

QJS_API ValueHandle join(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsUndefined();
	std::string path1 = qjs.JsValueToStdString(ctx, argv[0]);
	std::string path2 = qjs.JsValueToStdString(ctx, argv[1]);
	std::string res = os_path::join(path1, path2);
	return qjs.NewStringJsValue(ctx, res.c_str());
}

QJS_API ValueHandle normpath(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();
	std::string path = qjs.JsValueToStdString(ctx, argv[0]);
	std::string res = os_path::normpath(path);
	return qjs.NewStringJsValue(ctx, res.c_str());
}

QJS_API ValueHandle equal_path(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsUndefined();
	std::string path1 = qjs.JsValueToStdString(ctx, argv[0]);
	std::string path2 = qjs.JsValueToStdString(ctx, argv[1]);
	bool res = os_path::equal_path(path1, path2);
	return qjs.NewBoolJsValue(ctx, res);
}
