#include "pch.h"
#include <string>
#include "../QJS/Extend.h"

QJS_API int _entry(ContextHandle ctx, int id)
{
	return 0;//加载插件
	//return 1;//不能加载插件
}

QJS_API ValueHandle testFoo1(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
{
	ValueType t = qjs.GetValueType(this_val);
	auto arrNames = qjs.GetObjectPropertyKeys(ctx, this_val, false, true);
	int64_t len = qjs.GetLength(ctx, arrNames);
	for (int i = 0; i < len; i++)
	{
		auto jname = qjs.GetIndexedJsValue(ctx, i, arrNames);
		std::string name = qjs.JsValueToStdString(ctx, jname);
		printf("Property name:%s\n", name.c_str());
	}

	MessageBoxA(NULL, "testFoo1", "SampleExtend", 0);
	return qjs.TheJsUndefined();
}

QJS_API ValueHandle testFoo2(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
{
	MessageBoxA(NULL, "testFoo2", "SampleExtend", 0);
	return qjs.TheJsUndefined();
}

QJS_API void _completed(ContextHandle ctx, int id)
{
	printf("Sample插件加载完成\n");
}

QJS_API void _unload(ContextHandle ctx, int id)
{
	printf("Sample插件已卸载\n");
}