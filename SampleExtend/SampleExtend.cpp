#include "pch.h"

#include "../QJS/Extend.h"

QJS_API int entry(ContextHandle ctx)
{
	return 0;
}

QJS_API const char* function_list()
{
	return 
		"testFoo1\0"
		"testFoo2\0"
		"\0";
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