#include "pch.h"

#define QJS_EXPORTS
#include "../QJS/QJS.h"

QJS_API int entry(ContextHandle ctx)
{
	return 0;
}

QJS_API const char* function_names()
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