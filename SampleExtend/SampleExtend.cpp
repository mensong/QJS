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