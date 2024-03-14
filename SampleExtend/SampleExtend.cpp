#include "pch.h"

#include "../QJS/Extend.h"

QJS_API int entry(ContextHandle ctx)
{
	return 0;//加载插件
	//return 1;//不能加载插件
}

QJS_API ValueHandle testFoo1(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	return qjs.TheJsUndefined();
}

QJS_API ValueHandle testFoo2(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	return qjs.TheJsUndefined();
}