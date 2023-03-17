#include "pch.h"
#include <set>
#include <map>
#include <vector>
#include <quickjs.h>
#include <quickjs-libc.h>
#include "QJS.h"
#include "RuntimeManager.h"
#include <cassert>



#undef __has_attribute
#include <cutils.h>

#if defined(__APPLE__)
#define MALLOC_OVERHEAD  0
#else
#define MALLOC_OVERHEAD  8
#endif




#define _INNER_RT(rt) (JSRuntime*)rt
#define _OUTER_RT(rt) (RuntimeHandle)rt

#define _INNER_CTX(ctx) (JSContext*)ctx
#define _OUTER_CTX(ctx) (ContextHandle)ctx

#define _INNER_VAL(val) (JSValue)val
#define _OUTER_VAL(val) (ValueHandle)val

extern RuntimeManager* _runtimeManager;




/* default memory allocation functions with memory limitation */
static inline size_t js_qjs_malloc_usable_size(void* ptr)
{
#if defined(__APPLE__)
	return malloc_size(ptr);
#elif defined(_WIN32)
	return _msize(ptr);
#elif defined(EMSCRIPTEN)
	return 0;
#elif defined(__linux__)
	return malloc_usable_size(ptr);
#else
	/* change this to `return 0;` if compilation fails */
	return malloc_usable_size(ptr);
#endif
}

static void* js_qjs_malloc(JSMallocState* s, size_t size)
{
	void* ptr;

	/* Do not allocate zero bytes: behavior is platform dependent */
	assert(size != 0);

	if (unlikely(s->malloc_size + size > s->malloc_limit))
		return NULL;

	ptr = malloc(size);
	if (!ptr)
		return NULL;

	s->malloc_count++;
	s->malloc_size += js_qjs_malloc_usable_size(ptr) + MALLOC_OVERHEAD;
	return ptr;
}


static void js_qjs_free(JSMallocState* s, void* ptr)
{
	if (!ptr)
		return;

	s->malloc_count--;
	s->malloc_size -= js_qjs_malloc_usable_size(ptr) + MALLOC_OVERHEAD;
	free(ptr);
}

static void* js_qjs_realloc(JSMallocState* s, void* ptr, size_t size)
{
	size_t old_size;

	if (!ptr) {
		if (size == 0)
			return NULL;
		return js_qjs_malloc(s, size);
	}
	old_size = js_qjs_malloc_usable_size(ptr);
	if (size == 0) {
		s->malloc_count--;
		s->malloc_size -= old_size + MALLOC_OVERHEAD;
		free(ptr);
		return NULL;
	}
	if (s->malloc_size + size - old_size > s->malloc_limit)
		return NULL;

	ptr = realloc(ptr, size);
	if (!ptr)
		return NULL;

	s->malloc_size += js_qjs_malloc_usable_size(ptr) - old_size;
	return ptr;
}

static const JSMallocFunctions qjs_malloc_funcs = {
	js_qjs_malloc,
	js_qjs_free,
	js_qjs_realloc,
#if defined(__APPLE__)
	malloc_size,
#elif defined(_WIN32)
	(size_t(*)(const void*))_msize,
#elif defined(EMSCRIPTEN)
	NULL,
#elif defined(__linux__)
	(size_t(*)(const void*))malloc_usable_size,
#else
	/* change this to `NULL,` if compilation fails */
	malloc_usable_size,
#endif
};


void _DisposeContextInner(ContextHandle ctx, bool removeFromMap)
{
	if (!ctx)
		return;

#if QJS_AUTO_FREE
	auto itValue = _runtimeManager->_valueMap.find(ctx);
	if (itValue != _runtimeManager->_valueMap.end())
	{
		for (auto it = itValue->second.begin(); it != itValue->second.end(); ++it)
		{
			JS_FreeValue(_INNER_CTX(ctx), _INNER_VAL(*it));
		}
		_runtimeManager->_valueMap.erase(itValue);
	}
#endif

	JS_FreeContext(_INNER_CTX(ctx));

#if QJS_AUTO_FREE
	if (removeFromMap)
	{
		JSRuntime* rt = JS_GetRuntime(_INNER_CTX(ctx));

		auto itFinder = _runtimeManager->_contextMap.find(_OUTER_RT(rt));
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

	JS_FreeRuntime(_INNER_RT(runtime));

#if QJS_AUTO_FREE
	if (removeFromMap)
	{
		auto itFinder = _runtimeManager->_runtimeSet.find(runtime);
		if (itFinder != _runtimeManager->_runtimeSet.end())
			_runtimeManager->_runtimeSet.erase(itFinder);
	}
#endif
}

RuntimeHandle NewRuntime()
{
	// Create a runtime. 
	JSRuntime* runtime = NULL;
	runtime = JS_NewRuntime2(&qjs_malloc_funcs, NULL);

	//js_std_init_handlers(runtime);
	//JS_SetModuleLoaderFunc(runtime, nullptr, js_module_loader, nullptr);

#if QJS_AUTO_FREE
	if (runtime)
	{
		_runtimeManager->_runtimeSet.insert(runtime);
	}
#endif

	return _OUTER_RT(runtime);
}

void FreeRuntime(RuntimeHandle runtime)
{	
	_DisposeRuntimeInner(runtime, true);
}

ContextHandle NewContext(RuntimeHandle runtime)
{
	if (runtime == NULL)
		return NULL;

	JSContext* context = JS_NewContext(_INNER_RT(runtime));

	//js_init_module_std(context, "std");
	//js_init_module_os(context, "os");

#if QJS_AUTO_FREE
	if (context)
	{
		_runtimeManager->_contextMap[runtime].insert(_OUTER_CTX(context));
	}
#endif

	return _OUTER_CTX(context);
}

void FreeContext(ContextHandle ctx)
{
	_DisposeContextInner(ctx, true);
}

ValueHandle GetGlobalObject(ContextHandle ctx)
{
	ValueHandle val = _OUTER_VAL(JS_GetGlobalObject(_INNER_CTX(ctx)));

#if QJS_AUTO_FREE
	//防止调用多次GetGlobalObject出现的错误
	auto itFinder = _runtimeManager->_valueMap.find(ctx);
	if (itFinder != _runtimeManager->_valueMap.end())
	{
		auto it = itFinder->second.find(val);
		if (it != itFinder->second.end())
		{
			JS_FreeValue(_INNER_CTX(ctx), _INNER_VAL(*it));
		}
	}

	_runtimeManager->_valueMap[ctx].insert(val);
#endif

	return val;
}

static int __magicIdx = 0;
static std::map<int, std::pair<FN_JsFunctionCallback, void*> > __NewFunctionFunctions;
static JSValue __NewFunctionHelper(JSContext* ctx, JSValue this_val, int argc, JSValue* argv, int magic)
{
	auto itFinder = __NewFunctionFunctions.find(magic);
	if (itFinder != __NewFunctionFunctions.end())
	{
		if (itFinder->second.first)
		{
			ValueHandle ret = itFinder->second.first(_OUTER_CTX(ctx), _OUTER_VAL(this_val), argc, (ValueHandle*)argv, itFinder->second.second);
			//return JS_DupValue(ctx,  _INNER_VAL(ret));

#if QJS_AUTO_FREE
		//varValue转交给gc管理
			_runtimeManager->_valueMap[ctx].erase(ret);
#endif

			return _INNER_VAL(ret);
		}

		return JS_UNDEFINED;
	}

	return JS_UNDEFINED;
}
ValueHandle NewFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc, void* user_data)
{
	if (!ctx) return NULL;
	JSValue fv = JS_NewCFunctionMagic(_INNER_CTX(ctx), (JSCFunctionMagic*)__NewFunctionHelper, NULL, argc, JS_CFUNC_generic_magic, __magicIdx);

	if (JS_IsFunction(_INNER_CTX(ctx), fv))
	{
#if QJS_AUTO_FREE
		_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(fv));
#endif

		__NewFunctionFunctions.insert(std::make_pair(__magicIdx, std::make_pair(cb, user_data)));
		__magicIdx++;
	}

	return _OUTER_VAL(fv);
}

bool DefineGetterSetter(ContextHandle ctx, ValueHandle parent, const char* propName, ValueHandle getter, ValueHandle setter)
{
	int flags = JS_PROP_HAS_CONFIGURABLE | JS_PROP_HAS_ENUMERABLE;
	if (getter && JS_IsFunction(_INNER_CTX(ctx), _INNER_VAL(getter)))
		flags |= JS_PROP_HAS_GET;
	if (setter && JS_IsFunction(_INNER_CTX(ctx), _INNER_VAL(setter)))
		flags |= JS_PROP_HAS_SET;

	JSAtom atom = JS_NewAtom(_INNER_CTX(ctx), propName);
	int ret = JS_DefineProperty(_INNER_CTX(ctx), _INNER_VAL(parent), atom, JS_UNDEFINED, _INNER_VAL(getter), _INNER_VAL(setter), flags);
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	return ret == TRUE;
}

ValueHandle GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	if (!parent)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue val = JS_GetPropertyStr(_INNER_CTX(ctx), _this, varName);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(val));
#endif

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return _OUTER_VAL(val);
}

bool SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	if (!parent)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));
		
	bool b = JS_SetPropertyStr(_INNER_CTX(ctx), _this, varName, _INNER_VAL(varValue)) == TRUE;

#if QJS_AUTO_FREE
	//varValue转交给gc管理
	if (b)
		_runtimeManager->_valueMap[ctx].erase(varValue);
#endif

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return b;
}

bool DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSAtom atom = JS_NewAtom(_INNER_CTX(ctx), varName);	
	int res = JS_DeleteProperty(_INNER_CTX(ctx), _this, atom, 0);	
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return res == TRUE;
}

ValueHandle GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue val = JS_GetPropertyUint32(_INNER_CTX(ctx), _this, idx);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(val));
#endif

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return _OUTER_VAL(val);
}

bool SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	bool b = JS_SetPropertyUint32(_INNER_CTX(ctx), _this, idx, _INNER_VAL(varValue)) == TRUE;

#if QJS_AUTO_FREE
	//varValue转交给gc管理
	if (b)
		_runtimeManager->_valueMap[ctx].erase(varValue);
#endif

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return b;
}

bool DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSAtom atom = JS_NewAtomUInt32(_INNER_CTX(ctx), idx);
	int res = JS_DeleteProperty(_INNER_CTX(ctx), _this, atom, 0);
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return res == TRUE;
}

ValueHandle TheJsUndefined()
{
	return _OUTER_VAL(JS_UNDEFINED);
}

ValueHandle TheJsNull()
{
	return _OUTER_VAL(JS_NULL);
}

ValueHandle TheJsTrue()
{
	return _OUTER_VAL(JS_TRUE);
}

ValueHandle TheJsFalse()
{
	return _OUTER_VAL(JS_FALSE);
}

ValueHandle TheJsException()
{
	return _OUTER_VAL(JS_EXCEPTION);
}

ValueHandle RunScript(ContextHandle ctx, const char* script)
{
	JSValue jsRes = JS_Eval(_INNER_CTX(ctx), script, strlen(script), "<eval>", 0);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(jsRes));
#endif

	return _OUTER_VAL(jsRes);
}

ValueHandle CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent)
{
	JSValue func = _INNER_VAL(jsFunction);
	if (JS_IsFunction(_INNER_CTX(ctx), func) != TRUE)
		return NULL;

	JSValue _this = _INNER_VAL(parent);
	if (!_this)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue res = JS_Call(_INNER_CTX(ctx), func, _this, argc, (JSValue*)args);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	if (!parent)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return _OUTER_VAL(res);
}

ValueHandle NewIntJsValue(ContextHandle ctx, int intValue)
{
	JSValue res = JS_NewInt32(_INNER_CTX(ctx), intValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewInt64JsValue(ContextHandle ctx, int64_t intValue)
{
	JSValue res = JS_NewInt64(_INNER_CTX(ctx), intValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewDoubleJsValue(ContextHandle ctx, double doubleValue)
{
	JSValue res = JS_NewFloat64(_INNER_CTX(ctx), doubleValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewStringJsValue(ContextHandle ctx, const char* stringValue)
{
	JSValue res = JS_NewString(_INNER_CTX(ctx), stringValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

QJS_API ValueHandle NewBoolJsValue(ContextHandle ctx, bool boolValue)
{
	JSValue res = JS_NewBool(_INNER_CTX(ctx), (int)boolValue);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewObjectJsValue(ContextHandle ctx)
{
	JSValue res = JS_NewObject(_INNER_CTX(ctx));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewArrayJsValue(ContextHandle ctx)
{
	JSValue res = JS_NewArray(_INNER_CTX(ctx));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewThrowJsValue(ContextHandle ctx, ValueHandle throwWhat)
{
	JSValue res = JS_Throw(_INNER_CTX(ctx), _INNER_VAL(throwWhat));

#if QJS_AUTO_FREE
	//throwWhat转交给gc管理
		_runtimeManager->_valueMap[ctx].erase(throwWhat);
#endif

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle NewDateJsValue(ContextHandle ctx, uint64_t ms_since_1970)
{
	JSValue res = JS_NewDate(_INNER_CTX(ctx), (double)ms_since_1970);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

ValueHandle CopyJsValue(ContextHandle ctx, ValueHandle val)
{
	JSValue newValue = JS_DupValue(_INNER_CTX(ctx), _INNER_VAL(val));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(newValue));
#endif

	return _OUTER_VAL(newValue);
}

QJS_API int64_t GetLength(ContextHandle ctx, ValueHandle obj)
{
	int64_t len = -1;
	if (JS_GetPropertyLength(_INNER_CTX(ctx), &len, obj) == TRUE)
		return len;
	return -1;
}

void FreeValueHandle(ContextHandle ctx, ValueHandle v)
{
	JS_FreeValue(_INNER_CTX(ctx), _INNER_VAL(v));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].erase(v);
#endif
}

const char* JsValueToString(ContextHandle ctx, ValueHandle value, const char* defVal = "")
{
	if (value == NULL)
		return defVal;

	const char* buf = JS_ToCString(_INNER_CTX(ctx), _INNER_VAL(value));

	return buf;
}

void FreeJsValueToStringBuffer(ContextHandle ctx, const char* buff)
{
	JS_FreeCString(_INNER_CTX(ctx), buff);
}

int JsValueToInt(ContextHandle ctx, ValueHandle value, int defVal = 0)
{
	if (value == NULL)
		return defVal;

	int e = defVal;
	int res = JS_ToInt32(_INNER_CTX(ctx), &e, _INNER_VAL(value));
	if (res >= 0)
		return e;
	return defVal;
}

int64_t JsValueToInt64(ContextHandle ctx, ValueHandle value, int64_t defVal = 0)
{
	if (value == NULL)
		return defVal;

	int64_t e = defVal;
	int res = JS_ToInt64Ext(_INNER_CTX(ctx), &e, _INNER_VAL(value));
	if (res >= 0)
		return e;
	return defVal;
}

double JsValueToDouble(ContextHandle ctx, ValueHandle value, double defVal = 0.0)
{
	if (value == NULL)
		return defVal;

	double e = defVal;
	int res = JS_ToFloat64(_INNER_CTX(ctx), &e, _INNER_VAL(value));
	if (res >= 0)
		return e;
	return defVal;
}

bool JsValueToBool(ContextHandle ctx, ValueHandle value, bool defVal = false)
{
	if (value == NULL)
		return false;

	int res = JS_ToBool(_INNER_CTX(ctx), _INNER_VAL(value));
	if (res == -1)
		return defVal;

	return (res > 0);
}

uint64_t JsValueToTimestamp(ContextHandle ctx, ValueHandle value)
{
	double timestamp = 0;
	JS_IsDate(_INNER_CTX(ctx), _INLINE_VAR(value), &timestamp);
	return (uint64_t)timestamp;
}

ValueType GetValueType(ValueHandle value)
{
	return (ValueType)JS_VALUE_GET_TAG(_INNER_VAL(value));
}

bool JsValueIsString(ValueHandle value)
{
	return JS_IsString(_INNER_VAL(value)) == TRUE;
}

bool JsValueIsInt(ValueHandle value)
{
	return GetValueType(value) == JS_TYPE_INT;
}

bool JsValueIsNumber(ValueHandle value)
{
	return JS_IsNumber(_INNER_VAL(value)) == TRUE;
}

bool JsValueIsDouble(ValueHandle value)
{
	return GetValueType(value) == JS_TYPE_FLOAT64;
}

bool JsValueIsBool(ValueHandle value)
{
	return JS_IsBool(_INNER_VAL(value)) == TRUE;
}

bool JsValueIsObject(ValueHandle value)
{
	return JS_IsObject(_INNER_VAL(value)) == TRUE;
}

bool JsValueIsArray(ContextHandle ctx, ValueHandle value)
{
	return JS_IsArray(_INNER_CTX(ctx), _INNER_VAL(value)) == TRUE;
}

QJS_API bool JsValueIsException(ValueHandle value)
{
	return JS_IsException(_INNER_VAL(value));
}

bool JsValueIsUndefined(ValueHandle value)
{
	return JS_IsUndefined(_INNER_VAL(value)) == TRUE;
}

bool JsValueIsNull(ValueHandle value)
{
	return JS_IsNull(_INNER_VAL(value)) == TRUE;
}

bool JsValueIsDate(ContextHandle ctx, ValueHandle value)
{
	return JS_IsDate(_INNER_CTX(ctx), _INNER_VAL(value), NULL) == TRUE;
}

bool JsValueIsFunction(ContextHandle ctx, ValueHandle value)
{
	return JS_IsFunction(_INNER_CTX(ctx), _INNER_VAL(value)) == TRUE;
}

ValueHandle GetJsLastException(ContextHandle ctx)
{
	JSValue res = JS_GetException(_INNER_CTX(ctx));

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
}

int ExecutePendingJob(RuntimeHandle runtime, ContextHandle* outCurCtx)
{
	return JS_ExecutePendingJob(_INNER_RT(runtime), (JSContext**)outCurCtx);
}

void SetDebuggerMode(ContextHandle ctx, bool onoff)
{
	JS_SetDebuggerMode(_INNER_CTX(ctx), (onoff ? 1 : 0));
}

static std::map<ContextHandle, std::pair<FN_DebuggerLineCallback, void*> > __SetDebuggerCheckLineNoCallbackFunctions;
JS_BOOL __SetDebuggerCheckLineNoCallbackHelper(JSContext* ctx, JSAtom file_name, uint32_t line_no, const uint8_t* pc)
{
	//先禁用调试模式，防止在调试函数中执行脚本形成死循环
	JS_SetDebuggerMode(_INNER_CTX(ctx), 0);

	do
	{
		auto itFinder = __SetDebuggerCheckLineNoCallbackFunctions.find(ctx);
		if (itFinder != __SetDebuggerCheckLineNoCallbackFunctions.end())
		{
			if (itFinder->second.first)
			{
				itFinder->second.first(ctx, line_no, pc, itFinder->second.second);
			}
		}
	} while (0);

	//恢复调试模式
	JS_SetDebuggerMode(_INNER_CTX(ctx), 1);

	return TRUE;
}
void SetDebuggerLineCallback(ContextHandle ctx, FN_DebuggerLineCallback cb, void* user_data)
{
	__SetDebuggerCheckLineNoCallbackFunctions[ctx] = std::make_pair(cb, user_data);
	JS_SetBreakpointHandler(_INNER_CTX(ctx), __SetDebuggerCheckLineNoCallbackHelper);
}

uint32_t GetDebuggerStackDepth(ContextHandle ctx)
{
	return js_debugger_stack_depth(_INNER_CTX(ctx));
}

ValueHandle GetDebuggerClosureVariables(ContextHandle ctx, int stack_idx)
{
	JSValue res = js_debugger_closure_variables(_INNER_CTX(ctx), stack_idx);

#if QJS_AUTO_FREE
	_runtimeManager->_valueMap[ctx].insert(_OUTER_VAL(res));
#endif

	return _OUTER_VAL(res);
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
