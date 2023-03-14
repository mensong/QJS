#include "pch.h"
#include <set>
#include <map>
#include <vector>
#include <quickjs.h>
#include "QJS.h"
#include "RuntimeManager.h"

#define _FROM_RT(rt) (JSRuntime*)rt
#define _TO_RT(rt) (RuntimeHandle)rt

#define _FROM_CTX(ctx) (JSContext*)ctx
#define _TO_CTX(ctx) (ContextHandle)ctx

#define _FROM_VAL(val) (JSValue)val
#define _TO_VAL(val) (ValueHandle)val

#define QJS_AUTO_FREE 1

extern RuntimeManager* _runtimeManager;

void _DisposeContextInner(ContextHandle ctx, bool removeFromMap)
{
	if (!ctx)
		return;

#if QJS_AUTO_FREE
	auto itStr = _runtimeManager->_stringMap.find(ctx);
	if (itStr != _runtimeManager->_stringMap.end())
	{
		for (auto it = itStr->second.begin(); it != itStr->second.end(); ++it)
		{
			JS_FreeCString(_FROM_CTX(ctx), *it);
		}
		_runtimeManager->_stringMap.erase(itStr);
	}

	auto itValue = _runtimeManager->_valueMap.find(ctx);
	if (itValue != _runtimeManager->_valueMap.end())
	{
		for (auto it = itValue->second.begin(); it != itValue->second.end(); ++it)
		{
			JS_FreeValue(_FROM_CTX(ctx), _FROM_VAL(*it));
		}
		_runtimeManager->_valueMap.erase(itValue);
	}
#endif

#if QJS_AUTO_FREE
	JSRuntime* rt = JS_GetRuntime(_FROM_CTX(ctx));
#endif

	JS_FreeContext(_FROM_CTX(ctx));

#if QJS_AUTO_FREE
	if (removeFromMap)
	{
		auto itFinder = _runtimeManager->_contextMap.find(_TO_RT(rt));
		if (itFinder != _runtimeManager->_contextMap.end())
			_runtimeManager->_contextMap.erase(itFinder);
	}
#endif
}

void _DisposeRuntimeInner(RuntimeHandle runtime, bool removeFromMap)
{
	if (!runtime)
		return;

#if QJS_AUTO_FREE
	auto itContext = _runtimeManager->_contextMap.find(runtime);
	if (itContext != _runtimeManager->_contextMap.end())
	{
		for (auto it = itContext->second.begin(); it != itContext->second.end(); ++it)
		{
			_DisposeContextInner(*it, false);
		}
		_runtimeManager->_contextMap.erase(itContext);
	}
#endif

	JS_FreeRuntime(_FROM_RT(runtime));

#if QJS_AUTO_FREE
	if (removeFromMap)
	{
		auto itFinder = _runtimeManager->_runtimeSet.find(runtime);
		if (itFinder != _runtimeManager->_runtimeSet.end())
			_runtimeManager->_runtimeSet.erase(itFinder);
	}
#endif
}

RuntimeHandle CreateRuntime()
{
	// Create a runtime. 
	JSRuntime* runtime = NULL;
	runtime = JS_NewRuntime();

#if QJS_AUTO_FREE
	if (runtime)
	{
		_runtimeManager->_runtimeSet.insert(runtime);
	}
#endif

	return _TO_RT(runtime);
}

void FreeRuntime(RuntimeHandle runtime)
{	
	_DisposeRuntimeInner(runtime, true);
}

ContextHandle CreateContext(RuntimeHandle runtime)
{
	if (runtime == NULL)
		return NULL;

	// Create an execution context. 
	JSContext* context = NULL;
	context = JS_NewContext(_FROM_RT(runtime));

#if QJS_AUTO_FREE
	if (context)
	{
		_runtimeManager->_contextMap[runtime].insert(_TO_CTX(context));
	}
#endif

	return _TO_CTX(context);
}

void FreeContext(ContextHandle ctx)
{
	_DisposeContextInner(ctx, true);
}

ValueHandle GetGlobalObject(ContextHandle ctx)
{
	ValueHandle val = _TO_VAL(JS_GetGlobalObject(_FROM_CTX(ctx)));

#if QJS_AUTO_FREE
	//防止调用多次GetGlobalObject出现的错误
	auto itFinder = _runtimeManager->_valueMap.find(ctx);
	if (itFinder != _runtimeManager->_valueMap.end())
	{
		auto it = itFinder->second.find(val);
		if (it != itFinder->second.end())
		{
			JS_FreeValue(_FROM_CTX(ctx), _FROM_VAL(*it));
		}
	}

	_runtimeManager->_valueMap[ctx].insert(val);
#endif

	return val;
}

ValueHandle CreateFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc)
{
	if (!ctx) return NULL;
	JSValue fv = JS_NewCFunction(_FROM_CTX(ctx), (JSCFunction*)cb, NULL, argc);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(fv));
#endif

	return _TO_VAL(fv);
}

bool DefineGetterSetter(ContextHandle ctx, ValueHandle parent, const char* propName, ValueHandle getter, ValueHandle setter)
{
	int flags = JS_PROP_HAS_CONFIGURABLE | JS_PROP_HAS_ENUMERABLE;
	if (getter && JS_IsFunction(_FROM_CTX(ctx), _FROM_VAL(getter)))
		flags |= JS_PROP_HAS_GET;
	if (setter && JS_IsFunction(_FROM_CTX(ctx), _FROM_VAL(setter)))
		flags |= JS_PROP_HAS_SET;

	JSAtom atom = JS_NewAtom(_FROM_CTX(ctx), propName);
	int ret = JS_DefineProperty(_FROM_CTX(ctx), _FROM_VAL(parent), atom, JS_UNDEFINED, _FROM_VAL(getter), _FROM_VAL(setter), flags);
	JS_FreeAtom(_FROM_CTX(ctx), atom);

	return ret == TRUE;
}

ValueHandle GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _FROM_VAL(parent);
	if (!parent)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	JSValue val = JS_GetPropertyStr(_FROM_CTX(ctx), _this, varName);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(val));
#endif

	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return _TO_VAL(val);
}

bool SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _FROM_VAL(parent);
	if (!parent)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	bool b = JS_SetPropertyStr(_FROM_CTX(ctx), _this, varName, _FROM_VAL(varValue)) == TRUE;

#if QJS_AUTO_FREE
	//varValue转交给gc管理
	if (b)
		_runtimeManager->_valueMap[ctx].erase(varValue);
#endif
	
	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return b;
}

bool DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _FROM_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	JSAtom atom = JS_NewAtom(_FROM_CTX(ctx), varName);	
	int res = JS_DeleteProperty(_FROM_CTX(ctx), _this, atom, 0);	
	JS_FreeAtom(_FROM_CTX(ctx), atom);

	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return res == TRUE;
}

ValueHandle GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent)
{
	JSValue _this = _FROM_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	JSValue val = JS_GetPropertyUint32(_FROM_CTX(ctx), _this, idx);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(val));
#endif

	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return _TO_VAL(val);
}

bool SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _FROM_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	bool b = JS_SetPropertyUint32(_FROM_CTX(ctx), _this, idx, _FROM_VAL(varValue)) == TRUE;

#if QJS_AUTO_FREE
	//varValue转交给gc管理
	if (b)
		_runtimeManager->_valueMap[ctx].erase(varValue);
#endif

	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return b;
}

bool DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _FROM_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	JSAtom atom = JS_NewAtomUInt32(_FROM_CTX(ctx), idx);
	int res = JS_DeleteProperty(_FROM_CTX(ctx), _this, atom, 0);
	JS_FreeAtom(_FROM_CTX(ctx), atom);

	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return res == TRUE;
}

ValueHandle TheJsUndefined()
{
	return _TO_VAL(JS_UNDEFINED);
}

ValueHandle TheJsNull()
{
	return _TO_VAL(JS_NULL);
}

ValueHandle TheJsTrue()
{
	return _TO_VAL(JS_TRUE);
}

ValueHandle TheJsFalse()
{
	return _TO_VAL(JS_FALSE);
}

ValueHandle TheJsException()
{
	return _TO_VAL(JS_EXCEPTION);
}

ValueHandle RunScript(ContextHandle ctx, const char* script)
{
	JSValue jsRes = JS_Eval(_FROM_CTX(ctx), script, strlen(script), "<eval>", 0);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(jsRes));
#endif

	return _TO_VAL(jsRes);
}

ValueHandle CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent)
{
	JSValue func = _FROM_VAL(jsFunction);
	if (JS_IsFunction(_FROM_CTX(ctx), func) != TRUE)
		return NULL;

	JSValue _this = _FROM_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_FROM_CTX(ctx));

	JSValue res = JS_Call(_FROM_CTX(ctx), func, _this, argc, (JSValue*)args);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	if (!parent)
		JS_FreeValue(_FROM_CTX(ctx), _this);

	return _TO_VAL(res);
}

ValueHandle CreateIntJsValue(ContextHandle ctx, int intValue)
{
	JSValue res = JS_NewInt64(_FROM_CTX(ctx), intValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

ValueHandle CreateDoubleJsValue(ContextHandle ctx, double doubleValue)
{
	JSValue res = JS_NewFloat64(_FROM_CTX(ctx), doubleValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

ValueHandle CreateStringJsValue(ContextHandle ctx, const char* stringValue)
{
	JSValue res = JS_NewString(_FROM_CTX(ctx), stringValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

QJS_API ValueHandle CreateBoolJsValue(ContextHandle ctx, bool boolValue)
{
	JSValue res = JS_NewBool(_FROM_CTX(ctx), (int)boolValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

ValueHandle CreateObjectJsValue(ContextHandle ctx)
{
	JSValue res = JS_NewObject(_FROM_CTX(ctx));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

ValueHandle CreateArrayJsValue(ContextHandle ctx)
{
	JSValue res = JS_NewArray(_FROM_CTX(ctx));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

ValueHandle CreateThrowJsValue(ContextHandle ctx, ValueHandle throwWhat)
{
	JSValue res = JS_Throw(_FROM_CTX(ctx), _FROM_VAL(throwWhat));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}

void FreeValueHandle(ContextHandle ctx, ValueHandle v)
{
	JS_FreeValue(_FROM_CTX(ctx), _FROM_VAL(v));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].erase(v);
#endif
}

const char* JsValueToString(ContextHandle ctx, ValueHandle value, const char* defVal = "")
{
	if (value == NULL)
		return defVal;

	const char* buf = JS_ToCString(_FROM_CTX(ctx), _FROM_VAL(value));

#if QJS_AUTO_FREE
	_runtimeManager->_stringMap[ctx].insert(buf);
#endif

	return buf;
}

int JsValueToInt(ContextHandle ctx, ValueHandle value, int defVal = 0)
{
	if (value == NULL)
		return defVal;

	int e = defVal;
	int res = JS_ToInt32(_FROM_CTX(ctx), &e, _FROM_VAL(value));
	if (res == TRUE)
		return e;
	return defVal;
}

int64_t JsValueToInt64(ContextHandle ctx, ValueHandle value, int64_t defVal = 0)
{
	if (value == NULL)
		return defVal;

	int64_t e = defVal;
	int res = JS_ToInt64Ext(_FROM_CTX(ctx), &e, _FROM_VAL(value));
	if (res == TRUE)
		return e;
	return defVal;
}

double JsValueToDouble(ContextHandle ctx, ValueHandle value, double defVal = 0.0)
{
	if (value == NULL)
		return defVal;

	double e = defVal;
	int res = JS_ToFloat64(_FROM_CTX(ctx), &e, _FROM_VAL(value));
	if (res == TRUE)
		return e;
	return defVal;
}

bool JsValueToBool(ContextHandle ctx, ValueHandle value, bool defVal = false)
{
	if (value == NULL)
		return false;

	int res = JS_ToBool(_FROM_CTX(ctx), _FROM_VAL(value));
	if (res == -1)
		return defVal;

	return (res == TRUE);
}

ValueType GetValueType(ValueHandle value)
{
	return (ValueType)JS_VALUE_GET_TAG(_FROM_VAL(value));
}

bool JsValueIsString(ValueHandle value)
{
	return JS_IsString(_FROM_VAL(value)) == TRUE;
}

bool JsValueIsInt(ValueHandle value)
{
	return GetValueType(value) == JS_TYPE_INT;
}

bool JsValueIsNumber(ValueHandle value)
{
	return JS_IsNumber(_FROM_VAL(value)) == TRUE;
}

bool JsValueIsDouble(ValueHandle value)
{
	return GetValueType(value) == JS_TYPE_FLOAT64;
}

bool JsValueIsBool(ValueHandle value)
{
	return JS_IsBool(_FROM_VAL(value)) == TRUE;
}

bool JsValueIsObject(ValueHandle value)
{
	return JS_IsObject(_FROM_VAL(value)) == TRUE;
}

bool JsValueIsArray(ContextHandle ctx, ValueHandle value)
{
	return JS_IsArray(_FROM_CTX(ctx), _FROM_VAL(value)) == TRUE;
}

QJS_API bool JsValueIsException(ValueHandle value)
{
	return JS_IsException(_FROM_VAL(value));
}

bool JsValueIsUndefined(ValueHandle value)
{
	return JS_IsUndefined(_FROM_VAL(value)) == TRUE;
}

bool JsValueIsNull(ValueHandle value)
{
	return JS_IsNull(_FROM_VAL(value)) == TRUE;
}

bool JsValueIsFunction(ContextHandle ctx, ValueHandle value)
{
	return JS_IsFunction(_FROM_CTX(ctx), _FROM_VAL(value)) == TRUE;
}

ValueHandle GetJsLastException(ContextHandle ctx)
{
	JSValue res = JS_GetException(_FROM_CTX(ctx));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_TO_VAL(res));
#endif

	return _TO_VAL(res);
}


std::wstring Ret_AnsiToUnicode;
const wchar_t* AnsiToUnicode(const char* multiByteStr)
{
	wchar_t* pWideCharStr; //定义返回的宽字符指针
	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
	//获取宽字符的个数
	nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, multiByteStr, -1, NULL, 0);
	//获得宽字符指针
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_ACP, 0, multiByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//返回
	Ret_AnsiToUnicode.resize(nLenOfWideCharStr + 1, 0);
	wcscpy_s(&Ret_AnsiToUnicode[0], Ret_AnsiToUnicode.size(), pWideCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return Ret_AnsiToUnicode.c_str();
}

std::string Ret_UnicodeToAnsi;
const char* UnicodeToAnsi(const wchar_t* wideByteRet)
{
	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_ACP, 0, wideByteRet, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_ACP, 0, wideByteRet, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	Ret_UnicodeToAnsi.resize(nLenOfMultiCharStr + 1, 0);
	strcpy_s(&Ret_UnicodeToAnsi[0], Ret_UnicodeToAnsi.size(), pMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return Ret_UnicodeToAnsi.c_str();
}

std::string Ret_UnicodeToUtf8;
const char* UnicodeToUtf8(const wchar_t* wideByteRet)
{
	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_UTF8, 0, wideByteRet, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_UTF8, 0, wideByteRet, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	Ret_UnicodeToUtf8.resize(nLenOfMultiCharStr + 1, 0);
	strcpy_s(&Ret_UnicodeToUtf8[0], Ret_UnicodeToUtf8.size(), pMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return Ret_UnicodeToUtf8.c_str();
}

std::wstring Ret_Utf8ToUnicode;
const wchar_t* Utf8ToUnicode(const char* utf8ByteStr)
{
	wchar_t* pWideCharStr; //定义返回的宽字符指针
	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
	//获取宽字符的个数
	nLenOfWideCharStr = MultiByteToWideChar(CP_UTF8, 0, utf8ByteStr, -1, NULL, 0);
	//获得宽字符指针
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_UTF8, 0, utf8ByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//返回
	Ret_Utf8ToUnicode.resize(nLenOfWideCharStr + 1, 0);
	wcscpy_s(&Ret_Utf8ToUnicode[0], Ret_Utf8ToUnicode.size(), pWideCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return Ret_Utf8ToUnicode.c_str();
}
