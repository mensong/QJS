#include "pch.h"
#include <string>
#include <map>
#include "../QJS/Extend.h"

//id 为 LoadExtend 后所分配的ID

std::map<int, std::string> aData;

QJS_API int _entry(ContextHandle ctx, void* user_data, int id)
{
	aData[id] = "mensong";
	return 0;//加载插件
	//return 1;//不能加载插件
}

QJS_API ValueHandle F_testFoo(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
{
	MessageBoxA(NULL, "testFoo()", "SampleExtend", 0);
	return qjs.TheJsUndefined();
}

QJS_API ValueHandle G_a(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
{
	MessageBoxA(NULL, "a.getter", "SampleExtend", 0);
	return qjs.NewStringJsValue(ctx, aData[id].c_str());
}

QJS_API ValueHandle S_a(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
{
	MessageBoxA(NULL, "a.setter", "SampleExtend", 0);
	aData[id] = qjs.JsValueToStdString(ctx, argv[0]);
	return this_val;
}

QJS_API void _completed(ContextHandle ctx, void* user_data, int id)
{
	printf("Sample插件加载完成\n");
}

QJS_API void _unload(ContextHandle ctx, void* user_data, int id)
{
	printf("Sample插件已卸载\n");
}