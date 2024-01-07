// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include "..\QJS\QJS.h"

int main()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	ValueHandle result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(L"var a=123;a"), qjs.TheJsNull());
	if (!qjs.JsValueIsException(result))
	{
		const char* sz = qjs.JsValueToString(ctx, result);
		printf("运行成功:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}
	else
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		const char* sz = qjs.JsValueToString(ctx, exception);
		printf("运行错误:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);

	return 0;
}
