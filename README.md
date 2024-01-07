# QJS
quickjs(https://github.com/mensong/quickjs-win) 在windows上的封装

# 使用
1.把QJS.dll放到调用者exe同目录

2.代码包含QJS.h

3.使用qjs.XXX就可以内嵌js引擎了

# 示例

```
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
```

