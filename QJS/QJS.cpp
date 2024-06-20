#include "pch.h"
#include "QJS.h"
#include <set>
#include <map>
#include <vector>
#include <quickjs.h>
#include <quickjs-libc.h>
#include <cassert>

#pragma region Extend
#include "Extend.h"
#pragma endregion


/* quickjs参数为JSValue的内部会自动JS_FreeValue；为JSValueConst则不会 */

#undef __has_attribute
#include <cutils.h>

#if defined(__APPLE__)
#define MALLOC_OVERHEAD  0
#else
#define MALLOC_OVERHEAD  8
#endif

//InnerContext* == ContextHandle
struct InnerContext
{
	bool temp;
	JSContext* context;
	std::vector<uint64_t> values;

	InnerContext()
	{
		temp = false;
		context = NULL;
		extendHandles = new std::map<std::string, HMODULE>();
		extendFuncionsCache = new std::map<std::string, std::map<std::string, FN_JsFunctionCallback*>>;
	}

	~InnerContext()
	{
		if (!temp) 
		{
			if (extendHandles)
			{
				//不需要，在ResetContext中已做
				//for (auto it = extendHandles->begin(); it != extendHandles->end(); ++it)
				//	unloadExtend(it->first);
				//extendHandles->clear();
				delete extendHandles;
			}
			if (extendFuncionsCache)
				delete extendFuncionsCache;
		}
	}

	void addValue(uint64_t v)
	{
		values.push_back(v);
	}

#pragma region Extend
	std::map<std::string, HMODULE>* extendHandles;
	std::map<std::string, std::map<std::string, FN_JsFunctionCallback*>>* extendFuncionsCache;

	HMODULE getOrAddExtend(const std::string& filename)
	{
		if (filename.empty())
			return NULL;

		std::map<std::string, HMODULE>::iterator itFinder = extendHandles->find(filename.c_str());
		if (itFinder != extendHandles->end())
		{
			return itFinder->second;
		}

		//设置dll搜索路径到扩展dll所在的目录
		int i = (int)filename.size() - 1;
		for (; i >= 0; --i)
		{
			if (filename[i] == '/' || filename[i] == '\\')
				break;
		}
		if (i > 0)
		{
			std::string dllDir(filename, i);
			::SetDllDirectoryA(dllDir.c_str());
		}

		HMODULE hDll = LoadLibraryA(filename.c_str());

		if (i > 0)
			::SetDllDirectoryA(NULL);

		extendHandles->insert(std::make_pair(filename.c_str(), hDll));

		return hDll;
	}

	void unloadExtend(const std::string& filename)
	{
		std::map<std::string, HMODULE>::iterator itFinder = extendHandles->find(filename.c_str());
		if (itFinder != extendHandles->end())
		{
			//卸载扩展时，只是把扩展的函数设置为NULL，不能直接delete(因为还有可能被调用)，delete在FreeContext里
			auto itFunc = extendFuncionsCache->find(filename.c_str());
			if (itFunc != extendFuncionsCache->end())
			{
				for (auto itFunc2 = itFunc->second.begin();
					itFunc2 != itFunc->second.end(); ++itFunc2)
				{
					*(itFunc2->second) = NULL;
				}
			}

			HMODULE hDll = itFinder->second;			
			FreeLibrary(hDll);
			extendHandles->erase(itFinder);
		}
	}

	FN_JsFunctionCallback* addExtendFunction(const std::string& filename, 
		const std::string& funcName, FN_JsFunctionCallback func)
	{
		FN_JsFunctionCallback* pFn = NULL;
		auto itFinder = (*extendFuncionsCache)[filename.c_str()].find(funcName.c_str());
		if (itFinder != (*extendFuncionsCache)[filename.c_str()].end())
		{
			*itFinder->second = func;
			pFn = itFinder->second;
		}
		else
		{
			pFn = new FN_JsFunctionCallback;
			*pFn = func;
			(*extendFuncionsCache)[filename.c_str()].insert(std::make_pair(funcName.c_str(), pFn));
		}
		return pFn;
	}
#pragma endregion
};
#define ADD_AUTO_FREE(v) if (v.ctx && v.value)((InnerContext*)v.ctx)->addValue(v.value)

#define _INNER_RT(rt) (JSRuntime*)rt
#define _OUTER_RT(rt) (RuntimeHandle)rt

#define _INNER_CTX(ctx) (JSContext*)(((InnerContext*)ctx)->context)
#define _OUTER_CTX(ctx) (ContextHandle)ctx

#define _INNER_VAL(val) (JSValue)val.value
#define _OUTER_VAL(ctx, val) { (ContextHandle)ctx, (uint64_t)val }

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

inline JSRefCountHeader* getValuePtr(JSValue value)
{
	JSRefCountHeader* p = (JSRefCountHeader*)JS_VALUE_GET_PTR(value);
	return p;
}

inline JSRefCountHeader* getValuePtr(ValueHandle value)
{
	JSRefCountHeader* p = getValuePtr(_INNER_VAL(value));
	return p;
}

inline int getValueRefCount(ValueHandle value)
{
	JSRefCountHeader* p = getValuePtr(value);
	if (!p)
		return 0;
	return p->ref_count;
}

RuntimeHandle NewRuntime()
{
	// Create a runtime. 
	JSRuntime* runtime = NULL;
	runtime = JS_NewRuntime2(&qjs_malloc_funcs, NULL);

	//js_std_init_handlers(runtime);
	//JS_SetModuleLoaderFunc(runtime, nullptr, js_module_loader, nullptr);

	return _OUTER_RT(runtime);
}

void FreeRuntime(RuntimeHandle runtime)
{	
	if (!runtime)
		return;

	JS_FreeRuntime(_INNER_RT(runtime));
}

void SetRuntimeUserData(RuntimeHandle runtime, void* user_data)
{
	JS_SetRuntimeOpaque(_INNER_RT(runtime), user_data);
}

void* GetRuntimeUserData(RuntimeHandle runtime)
{
	return JS_GetRuntimeOpaque(_INNER_RT(runtime));
}

struct ContextUserData
{
	InnerContext* innerContext;

	void* outerData;

	ContextUserData()
	{
		innerContext = NULL;
		outerData = NULL;
	}
};

ContextHandle NewContext(RuntimeHandle runtime)
{
	if (runtime == NULL)
		return NULL;

	JSContext* context = JS_NewContext(_INNER_RT(runtime));

	//js_init_module_std(context, "std");
	//js_init_module_os(context, "os");
#ifdef CONFIG_BIGNUM
		JS_AddIntrinsicBigFloat(context);
		JS_AddIntrinsicBigDecimal(context);
		JS_AddIntrinsicOperators(context);
		JS_EnableBignumExt(context, TRUE);
#endif

	InnerContext* _ctx = new InnerContext();
	_ctx->context = context;
	_ctx->values.reserve(1024);

	ContextUserData* contextUserData = new ContextUserData;
	contextUserData->innerContext = _ctx;
	JS_SetContextOpaque(context, contextUserData);

	return _OUTER_CTX(_ctx);
}

ContextUserData* getContextUserData(JSContext* ctx)
{
	ContextUserData* contextUserData = (ContextUserData*)JS_GetContextOpaque(ctx);
	return contextUserData;
}

ContextUserData* getContextUserData(ContextHandle ctx)
{
	return getContextUserData(_INNER_CTX(ctx));
}

ContextHandle rawContextToContextHandle(JSContext* ctx)
{
	ContextUserData* contextUserData = getContextUserData(ctx);
	if (!contextUserData)
		return NULL;
	return (ContextHandle)contextUserData->innerContext;
}

void ResetContext(ContextHandle ctx)
{
	if (!ctx)
		return;

	InnerContext* innerCtx = (InnerContext*)ctx;
	for (size_t i = 0; i < innerCtx->values.size(); i++)
	{
		JS_FreeValue(_INNER_CTX(ctx), (JSValue)innerCtx->values[i]);
	}

#pragma region Extend
	for (std::map<std::string, HMODULE>::iterator it = innerCtx->extendHandles->begin();
		it != innerCtx->extendHandles->end(); ++it)
	{
		FreeLibrary(it->second);
	}
	innerCtx->extendHandles->clear();
		
	for (auto it2 = innerCtx->extendFuncionsCache->begin(); 
		it2 != innerCtx->extendFuncionsCache->end(); ++it2)
	{
		for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
		{
			delete it3->second;
		}
	}
	innerCtx->extendFuncionsCache->clear();
#pragma endregion
}

void FreeContext(ContextHandle ctx)
{
	if (!ctx)
		return;

	ResetContext(ctx);
		
	ContextUserData* contextUserData = (ContextUserData*)JS_GetContextOpaque(_INNER_CTX(ctx));
	if (contextUserData)
	{
		delete contextUserData;
	}


	JS_FreeContext(_INNER_CTX(ctx));

	InnerContext* innerCtx = (InnerContext*)ctx;
	delete innerCtx;
}

void SetContextUserData(ContextHandle ctx, void* user_data)
{
	ContextUserData* contextUserData = (ContextUserData*)JS_GetContextOpaque(_INNER_CTX(ctx));
	if (contextUserData)
	{
		contextUserData->outerData = user_data;
	}
}

void* GetContextUserData(ContextHandle ctx)
{
	ContextUserData* contextUserData = (ContextUserData*)JS_GetContextOpaque(_INNER_CTX(ctx));
	if (contextUserData)
	{
		return contextUserData->outerData;
	}
	return NULL;
}

ValueHandle GetGlobalObject(ContextHandle ctx)
{
	ValueHandle ret = _OUTER_VAL(ctx, JS_GetGlobalObject(_INNER_CTX(ctx)));
	ADD_AUTO_FREE(ret);
	return ret;
}

static int __magicIdx = 0;
static std::map<int, std::pair<FN_JsFunctionCallback, void*> > __NewFunctionFunctions;
static JSValue __NewFunctionCallHelper(JSContext* rawCtx, JSValue this_val, int argc, JSValue* argv, int magic)
{
	auto itFinder = __NewFunctionFunctions.find(magic);
	if (itFinder != __NewFunctionFunctions.end())
	{
		if (itFinder->second.first)
		{
			InnerContext* globalCtx = (InnerContext*)rawContextToContextHandle(rawCtx);
			if (!globalCtx)
				return JS_UNDEFINED;

			InnerContext innerCtx;
			innerCtx.temp = true;
			innerCtx.context = rawCtx;
			innerCtx.extendHandles = globalCtx->extendHandles;
			innerCtx.extendFuncionsCache = globalCtx->extendFuncionsCache;

			ValueHandle* _innerArgv = NULL;
			if (argc > 0)
				_innerArgv = new ValueHandle[argc];
			for (size_t i = 0; i < argc; i++)
			{
				_innerArgv[i] = _OUTER_VAL(rawCtx, argv[i]);
			}

			ValueHandle ret = itFinder->second.first( 
				&innerCtx, { rawCtx, this_val }, argc, (ValueHandle*)_innerArgv, itFinder->second.second);

			if (_innerArgv)
				delete[] _innerArgv;

			//这里的值是传给内部的，由内部自己释放，防止在下面被释放
			JS_DupValue(rawCtx, _INNER_VAL(ret));
			//释放函数内部所申请的内存
			for (size_t i = 0; i < innerCtx.values.size(); i++)
			{
				JS_FreeValue(rawCtx, (JSValue)innerCtx.values[i]);
			}

			return _INNER_VAL(ret);
		}

		return JS_UNDEFINED;
	}

	return JS_UNDEFINED;
}

ValueHandle NewFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc, void* user_data)
{
	if (!ctx)
	{
		return { NULL, NULL };
	}

	JSValue fv = JS_NewCFunctionMagic(_INNER_CTX(ctx), 
		(JSCFunctionMagic*)__NewFunctionCallHelper, NULL, argc, JS_CFUNC_generic_magic, __magicIdx);

	if (JS_IsFunction(_INNER_CTX(ctx), fv))
	{
		__NewFunctionFunctions.insert(std::make_pair(__magicIdx, std::make_pair(cb, user_data)));
		__magicIdx++;
	}

	ValueHandle ret = { ctx, fv };

	ADD_AUTO_FREE(ret);

	return ret;
}

void FreeValueHandle(ValueHandle* value)
{
	if (!value || !value->ctx)
		return;
	
	InnerContext* innerCtx = (InnerContext*)value->ctx;
	std::vector<uint64_t>::iterator itFinder = std::find(
		innerCtx->values.begin(),
		innerCtx->values.end(), value->value);
	if (itFinder != innerCtx->values.end())
		innerCtx->values.erase(itFinder);

	JS_FreeValue(_INNER_CTX(value->ctx), (JSValue)value->value);

	value->ctx = NULL;
	value->value = NULL;
}

bool DefineGetterSetter(ContextHandle ctx, ValueHandle parent, 
	const char* propName, ValueHandle getter, ValueHandle setter)
{
	JSValue globalObj = JS_GetGlobalObject(_INNER_CTX(ctx));
	bool bIsGlobalObj = getValuePtr(parent) == getValuePtr(globalObj);
	JS_FreeValue(_INNER_CTX(ctx), globalObj);
	if (bIsGlobalObj)
		return false;

	int flags = JS_PROP_HAS_CONFIGURABLE | JS_PROP_HAS_ENUMERABLE;
	if (JS_IsFunction(_INNER_CTX(ctx), _INNER_VAL(getter)))
	{
		flags |= JS_PROP_HAS_GET;
	}
	if (JS_IsFunction(_INNER_CTX(ctx), _INNER_VAL(setter)))
	{
		flags |= JS_PROP_HAS_SET;
	}

	JSAtom atom = JS_NewAtom(_INNER_CTX(ctx), propName);
	int ret = JS_DefineProperty(_INNER_CTX(ctx), _INNER_VAL(parent), atom, 
		JS_UNDEFINED, _INNER_VAL(getter), _INNER_VAL(setter), flags);
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	return ret == TRUE;
}

ValueHandle GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue val = JS_GetPropertyStr(_INNER_CTX(ctx), _this, varName);

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	ValueHandle ret = { ctx, val };

	ADD_AUTO_FREE(ret);

	return ret;
}

bool SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));
	
	JSValue v = JS_DupValue(_INNER_CTX(ctx), _INNER_VAL(varValue));
	bool b = JS_SetPropertyStr(_INNER_CTX(ctx), _this, varName, v) == TRUE;
	
	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return b;
}

bool DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSAtom atom = JS_NewAtom(_INNER_CTX(ctx), varName);	
	int res = JS_DeleteProperty(_INNER_CTX(ctx), _this, atom, 0);	
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return res == TRUE;
}

bool HasNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSAtom atom = JS_NewAtom(_INNER_CTX(ctx), varName);
	bool b = JS_HasProperty(_INNER_CTX(ctx), _this, atom) == TRUE;
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return b;
}

ValueHandle GetObjectPropertyKeys(ContextHandle ctx, ValueHandle jObj, bool onlyEnumerable, bool enableSymbol)
{
	JSPropertyEnum* tab = NULL;
	uint32_t len = 0;
	int flags = JS_GPN_STRING_MASK;
	if (onlyEnumerable) 
		flags |= JS_GPN_ENUM_ONLY;
	if (enableSymbol) 
		flags |= JS_GPN_SYMBOL_MASK;
	if (JS_GetOwnPropertyNames(_INNER_CTX(ctx), &tab, &len, _INNER_VAL(jObj), flags) != 0)
		return TheJsUndefined();

	ValueHandle jRetArr = NewArrayJsValue(ctx);

	for (uint32_t n = 0; n < len; ++n) {
		JSValue name = JS_AtomToValue(_INNER_CTX(ctx), tab[n].atom);
		if (JS_IsException(name))
		{
			JS_FreeValue(_INNER_CTX(ctx), name);
			js_free_prop_enum(_INNER_CTX(ctx), tab, len);
			return qjs.TheJsUndefined();
		}

		ValueHandle jname = _OUTER_VAL(ctx, name);
		ADD_AUTO_FREE(jname);
		SetIndexedJsValue(ctx, n, jname, jRetArr);
	}

	js_free_prop_enum(_INNER_CTX(ctx), tab, len);

	return jRetArr;
}

ValueHandle GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue val = JS_GetPropertyUint32(_INNER_CTX(ctx), _this, idx);

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	ValueHandle ret = _OUTER_VAL(ctx, val);
	ADD_AUTO_FREE(ret);
	return ret;
}

bool SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue v = JS_DupValue(_INNER_CTX(ctx), _INNER_VAL(varValue));
	bool b = JS_SetPropertyUint32(_INNER_CTX(ctx), _this, idx, v) == TRUE;

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return b;
}

bool DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent)
{
	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSAtom atom = JS_NewAtomUInt32(_INNER_CTX(ctx), idx);
	int res = JS_DeleteProperty(_INNER_CTX(ctx), _this, atom, 0);
	JS_FreeAtom(_INNER_CTX(ctx), atom);

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	return res == TRUE;
}

ValueHandle GetPrototype(ContextHandle ctx, ValueHandle jObj)
{
	ValueHandle ret = { NULL,NULL };

	if (JsValueIsDate(ctx, jObj))
		ret = _OUTER_VAL(ctx, JS_GetPrototypeOfDate(_INNER_CTX(ctx)));
	else
		ret = _OUTER_VAL(ctx, JS_GetPrototype(_INNER_CTX(ctx), _INNER_VAL(jObj)));

	ADD_AUTO_FREE(ret);
	return ret;
}

bool SetPrototype(ContextHandle ctx, ValueHandle jObj, ValueHandle protoJVal)
{
	return (JS_SetPrototype(_INNER_CTX(ctx), _INNER_VAL(jObj), _INNER_VAL(protoJVal)) == TRUE);
}

ValueHandle TheJsUndefined()
{
	return _OUTER_VAL(NULL, JS_UNDEFINED);
}

ValueHandle TheJsNull()
{
	return _OUTER_VAL(NULL, JS_NULL);
}

ValueHandle TheJsTrue()
{
	return _OUTER_VAL(NULL, JS_TRUE);
}

ValueHandle TheJsFalse()
{
	return _OUTER_VAL(NULL, JS_FALSE);
}

ValueHandle TheJsException()
{
	return _OUTER_VAL(NULL, JS_EXCEPTION);
}

void FreeJsPointer(ContextHandle ctx, void* ptr)
{
	if (ptr)
		js_free(_INNER_CTX(ctx), ptr);
}

uint8_t* LoadFile(ContextHandle ctx, size_t* outLen, const char* filename)
{
	uint8_t* buf = js_load_file(_INNER_CTX(ctx), outLen, filename);
	return buf;
}

ValueHandle RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/* = ""*/)
{
	JSValue res = JS_UNDEFINED;
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		res = JS_Eval(_INNER_CTX(ctx), script, strlen(script), filename, 0);
	else
		res = JS_EvalThis(_INNER_CTX(ctx), _INNER_VAL(parent), script, strlen(script), filename, 0);

	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle RunScriptFile(ContextHandle ctx, const char* filename)
{
	size_t buf_len = 0;
	uint8_t* buf = LoadFile(_INNER_CTX(ctx), &buf_len, filename);
	if (buf)
	{
		ValueHandle ret = RunScript(ctx, (const char*)buf, TheJsNull(), filename);
		FreeJsPointer(ctx, buf);
		return ret;
	}

	return TheJsUndefined();
}

ValueHandle CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent)
{
	JSValue func = _INNER_VAL(jsFunction);
	if (JS_IsFunction(_INNER_CTX(ctx), func) != TRUE)
		return { NULL,NULL };

	JSValue _this = _INNER_VAL(parent);
	bool isUseGlobal = JsValueIsNull(parent) || JsValueIsUndefined(parent);
	if (isUseGlobal)
		_this = JS_GetGlobalObject(_INNER_CTX(ctx));

	JSValue* innerArr = NULL;
	if (argc > 0)
	{
		innerArr = new JSValue[argc];
		for (size_t i = 0; i < argc; i++)
		{
			innerArr[i] = args[i].value;
		}
	}

	JSValue res = JS_Call(_INNER_CTX(ctx), func, _this, argc, innerArr);

	if (innerArr)
	{
		delete[] innerArr;
	}

	if (isUseGlobal)
		JS_FreeValue(_INNER_CTX(ctx), _this);

	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen)
{
	JSValue res = js_std_eval_binary(_INNER_CTX(ctx), byteCode, byteCodeLen, 0);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewIntJsValue(ContextHandle ctx, int intValue)
{
	JSValue res = JS_NewInt32(_INNER_CTX(ctx), intValue);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewInt64JsValue(ContextHandle ctx, int64_t intValue)
{
	JSValue res = JS_NewInt64(_INNER_CTX(ctx), intValue);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewDoubleJsValue(ContextHandle ctx, double doubleValue)
{
	JSValue res = JS_NewFloat64(_INNER_CTX(ctx), doubleValue);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewStringJsValue(ContextHandle ctx, const char* stringValue)
{
	JSValue res = JS_NewString(_INNER_CTX(ctx), stringValue);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewBoolJsValue(ContextHandle ctx, bool boolValue)
{
	JSValue res = JS_NewBool(_INNER_CTX(ctx), (int)boolValue);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewObjectJsValue(ContextHandle ctx)
{
	JSValue res = JS_NewObject(_INNER_CTX(ctx));
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewArrayJsValue(ContextHandle ctx)
{
	JSValue res = JS_NewArray(_INNER_CTX(ctx));
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewThrowJsValue(ContextHandle ctx, ValueHandle throwWhat)
{
	JSValue v = JS_DupValue(_INNER_CTX(ctx), _INNER_VAL(throwWhat));//JS_Throw will free the second param, so do it
	JSValue res = JS_Throw(_INNER_CTX(ctx), v);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle NewDateJsValue(ContextHandle ctx, uint64_t ms_since_1970)
{
	JSValue res = JS_NewDate(_INNER_CTX(ctx), (double)ms_since_1970);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle CopyJsValue(ContextHandle ctx, ValueHandle val)
{
	JSValue res = JS_DupValue(_INNER_CTX(ctx), _INNER_VAL(val));
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

int64_t GetLength(ContextHandle ctx, ValueHandle obj)
{
	int64_t len = -1;
	int res = JS_GetPropertyLength(_INNER_CTX(ctx), &len, _INNER_VAL(obj));
	return len;
}

//void FreeValueHandle(ContextHandle ctx, ValueHandle v)
//{
//	JS_FreeValue(_INNER_CTX(ctx), _INNER_VAL(v));
//}
//
//void AddValueHandleRefCount(ContextHandle ctx, ValueHandle v)
//{
//	JS_DupValue(_INNER_CTX(ctx), _INNER_VAL(v));
//}

bool SetObjectUserData(ValueHandle value, void* user_data)
{
	if (!JsValueIsObject(value))
		return false;
	JS_SetOpaque(_INNER_VAL(value), user_data);
	return true;
}

void* GetObjectUserData(ValueHandle value)
{
	void* userdata = NULL;
	JS_GetClassID(_INNER_VAL(value), &userdata);
	return userdata;
}

const char* JsValueToString(ContextHandle ctx, ValueHandle value)
{
	if (value.value == NULL)
		return NULL;

	if (JsValueIsException(value))
	{
		value = GetAndClearJsLastException(ctx);
	}

	const char* buf = JS_ToCString(_INNER_CTX(ctx), _INNER_VAL(value));
	return buf;
}

void FreeJsValueToStringBuffer(ContextHandle ctx, const char* buff)
{
	if (!buff)
		return;
	JS_FreeCString(_INNER_CTX(ctx), buff);
}

int JsValueToInt(ContextHandle ctx, ValueHandle value, int defVal = 0)
{
	if (value.value == NULL)
		return defVal;

	int e = defVal;
	int res = JS_ToInt32(_INNER_CTX(ctx), &e, _INNER_VAL(value));
	if (res >= 0)
		return e;
	return defVal;
}

int64_t JsValueToInt64(ContextHandle ctx, ValueHandle value, int64_t defVal = 0)
{
	if (value.value == NULL)
		return defVal;

	int64_t e = defVal;
	int res = JS_ToInt64Ext(_INNER_CTX(ctx), &e, _INNER_VAL(value));
	if (res >= 0)
		return e;
	return defVal;
}

double JsValueToDouble(ContextHandle ctx, ValueHandle value, double defVal = 0.0)
{
	if (value.value == NULL)
		return defVal;

	double e = defVal;
	int res = JS_ToFloat64(_INNER_CTX(ctx), &e, _INNER_VAL(value));
	if (res >= 0)
		return e;
	return defVal;
}

bool JsValueToBool(ContextHandle ctx, ValueHandle value, bool defVal = false)
{
	if (value.value == NULL)
		return false;

	int res = JS_ToBool(_INNER_CTX(ctx), _INNER_VAL(value));
	if (res == -1)
		return defVal;

	return (res > 0);
}

uint64_t JsValueToTimestamp(ContextHandle ctx, ValueHandle value)
{
	double timestamp = 0;
	JS_IsDate(_INNER_CTX(ctx), _INNER_VAL(value), &timestamp);
	return (uint64_t)timestamp;
}

ValueHandle CompileScript(ContextHandle ctx, const char* script, const char* filename/* = ""*/)
{
	int eval_flags;
	eval_flags = JS_EVAL_FLAG_COMPILE_ONLY | JS_EVAL_TYPE_GLOBAL;
	JSValue res = JS_Eval(_INNER_CTX(ctx), script, strlen(script), filename, eval_flags);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

uint8_t* JsValueToByteCode(ContextHandle ctx, ValueHandle value, size_t* outByteCodeLen, bool byte_swap/* = false*/)
{
	uint8_t* out_buf = NULL;
	int flags;
	flags = JS_WRITE_OBJ_BYTECODE;
	if (byte_swap)
		flags |= JS_WRITE_OBJ_BSWAP;
	*outByteCodeLen = 0;
	out_buf = JS_WriteObject(_INNER_CTX(ctx), outByteCodeLen, _INNER_VAL(value), flags);
	//failed: out_buf = NULL
	return out_buf;
}

ValueHandle ByteCodeToJsValue(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen)
{
	int flags;
	flags = JS_READ_OBJ_BYTECODE;
	JSValue res = JS_ReadObject(_INNER_CTX(ctx), byteCode, byteCodeLen, flags);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

QJS_API bool SaveByteCodeToFile(const uint8_t* byteCode, size_t byteCodeLen, const char* filepath)
{
	bool res = false;
	FILE* file = fopen(filepath, "wb"); 
	if (file)
	{
		fwrite(byteCode, sizeof(uint8_t), byteCodeLen, file);
		res = true;
		fclose(file);
	}
	return res;
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

bool JsValueIsException(ValueHandle value)
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

ValueHandle JsonStringify(ContextHandle ctx, ValueHandle value)
{
	JSValue jstr = JS_JSONStringify(_INNER_CTX(ctx), _INNER_VAL(value), JS_UNDEFINED, JS_UNDEFINED);
	ValueHandle ret = _OUTER_VAL(ctx, jstr);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle JsonParse(ContextHandle ctx, const char* json)
{
	JSValue obj = JS_ParseJSON2(_INNER_CTX(ctx), json, strlen(json), "<json>", JS_PARSE_JSON_EXT);
	ValueHandle ret = _OUTER_VAL(ctx, obj);
	ADD_AUTO_FREE(ret);
	return ret;
}

bool JsValueIsFunction(ContextHandle ctx, ValueHandle value)
{
	return JS_IsFunction(_INNER_CTX(ctx), _INNER_VAL(value)) == TRUE;
}

ValueHandle GetAndClearJsLastException(ContextHandle ctx)
{
	JSValue res = JS_GetException(_INNER_CTX(ctx));
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

int ExecutePendingJob(RuntimeHandle runtime, void*& outCurCtx)
{
	JSContext* curCtx = NULL;
	int res = JS_ExecutePendingJob(_INNER_RT(runtime), (JSContext**)curCtx);
	outCurCtx = curCtx;
	return res;
}

void SetDebuggerMode(ContextHandle ctx, bool onoff)
{
	JS_SetDebuggerMode(_INNER_CTX(ctx), (onoff ? 1 : 0));
}

static std::map<JSContext*, std::pair<FN_DebuggerLineCallback, void*> > __SetDebuggerCheckLineNoCallbackFunctions;
JS_BOOL __SetDebuggerCheckLineNoCallbackHelper(JSContext* rawCtx, JSAtom file_name, uint32_t line_no, const uint8_t* pc)
{
	//先禁用调试模式，防止在调试函数中执行脚本形成死循环
	JS_SetDebuggerMode(rawCtx, 0);

	do
	{
		auto itFinder = __SetDebuggerCheckLineNoCallbackFunctions.find(rawCtx);
		if (itFinder != __SetDebuggerCheckLineNoCallbackFunctions.end())
		{
			if (itFinder->second.first)
			{
				InnerContext innerCtx;
				innerCtx.context = rawCtx;

				itFinder->second.first(&innerCtx, line_no, pc, itFinder->second.second);

				//释放调试内部所申请的内存
				for (size_t i = 0; i < innerCtx.values.size(); i++)
				{
					JS_FreeValue(rawCtx, (JSValue)innerCtx.values[i]);
				}
			}
		}
	} while (0);

	//恢复调试模式
	JS_SetDebuggerMode(rawCtx, 1);

	return TRUE;
}
void SetDebuggerLineCallback(ContextHandle ctx, FN_DebuggerLineCallback cb, void* user_data)
{
	__SetDebuggerCheckLineNoCallbackFunctions[_INNER_CTX(ctx)] = std::make_pair(cb, user_data);
	JS_SetBreakpointHandler(_INNER_CTX(ctx), __SetDebuggerCheckLineNoCallbackHelper);
}

uint32_t GetDebuggerStackDepth(ContextHandle ctx)
{
	return js_debugger_stack_depth(_INNER_CTX(ctx));
}

ValueHandle GetDebuggerBacktrace(ContextHandle ctx, const uint8_t* pc)
{
	JSValue stack = js_debugger_build_backtrace(_INNER_CTX(ctx), pc);
	ValueHandle ret = _OUTER_VAL(ctx, stack);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle GetDebuggerClosureVariables(ContextHandle ctx, int stack_idx)
{
	JSValue res = js_debugger_closure_variables(_INNER_CTX(ctx), stack_idx);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

ValueHandle GetDebuggerLocalVariables(ContextHandle ctx, int stack_idx)
{
	JSValue res = js_debugger_local_variables(_INNER_CTX(ctx), stack_idx);
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

#pragma region Extend
bool GetExportNames(const char* dllPath, std::vector<std::string>& outExportNames)
{
	HANDLE hFile, hFileMap;//文件句柄和内存映射文件句柄
	DWORD fileAttrib = 0;//存储文件属性用，在createfile中用到。
	void* mod_base;//内存映射文件的起始地址，也是模块的起始地址
	typedef PVOID(CALLBACK* PFNEXPORTFUNC)(PIMAGE_NT_HEADERS, PVOID, ULONG, PIMAGE_SECTION_HEADER*);
	//首先取得ImageRvaToVa函数本来只要#include <Dbghelp.h>就可以使用这个函数，但是可能没有这个头文件
	PFNEXPORTFUNC ImageRvaToVax = NULL;
	HMODULE hModule = ::LoadLibraryA("DbgHelp.dll");
	if (hModule != NULL)
	{
		ImageRvaToVax = (PFNEXPORTFUNC)::GetProcAddress(hModule, "ImageRvaToVa");
		if (ImageRvaToVax == NULL)
		{
			::FreeLibrary(hModule);
			return false;
		}
	}
	else
	{
		return false;
	}

	hFile = CreateFileA(dllPath, GENERIC_READ, 0, 0, OPEN_EXISTING, fileAttrib, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::FreeLibrary(hModule);
		return false;
	}
	hFileMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
	if (hFileMap == NULL)
	{
		CloseHandle(hFile);
		::FreeLibrary(hModule);
		return false;
	}
	mod_base = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (mod_base == NULL)
	{
		CloseHandle(hFileMap);
		CloseHandle(hFile);
		::FreeLibrary(hModule);
		return false;
	}
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)mod_base;
	IMAGE_NT_HEADERS* pNtHeader =
		(IMAGE_NT_HEADERS*)((BYTE*)mod_base + pDosHeader->e_lfanew);//得到NT头首址
	IMAGE_OPTIONAL_HEADER* pOptHeader =
		(IMAGE_OPTIONAL_HEADER*)((BYTE*)mod_base + pDosHeader->e_lfanew + 24);//optional头首址
	IMAGE_EXPORT_DIRECTORY* pExportDesc = (IMAGE_EXPORT_DIRECTORY*)
		ImageRvaToVax(pNtHeader, mod_base, pOptHeader->DataDirectory[0].VirtualAddress, 0);
	if (pExportDesc != NULL)
	{
		//导出表首址。函数名称表首地址每个DWORD代表一个函数名字字符串的地址
		PDWORD nameAddr = (PDWORD)ImageRvaToVax(pNtHeader, mod_base, pExportDesc->AddressOfNames, 0);
		DWORD i = 0;
		DWORD unti = pExportDesc->NumberOfNames;
		for (i = 0; i < unti; i++)
		{
			const char* func_name = (const char*)ImageRvaToVax(pNtHeader, mod_base, (DWORD)nameAddr[i], 0);
			if (func_name)
				outExportNames.push_back(func_name);
		}
	}

	::FreeLibrary(hModule);
	UnmapViewOfFile(mod_base);
	CloseHandle(hFileMap);
	CloseHandle(hFile);

	return true;
}

//这里做一次代理是防止插件被提前卸载后崩溃
ValueHandle _extendCallHelper(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	FN_JsFunctionCallback* pFn = (FN_JsFunctionCallback*)user_data;
	if (!pFn || !(*pFn))
		return TheJsUndefined();

	return (*pFn)(ctx, this_val, argc, argv, NULL);
}

ValueHandle LoadExtend(ContextHandle ctx, const char* extendFile, ValueHandle parent)
{
	std::vector<std::string> function_list;
	if (!GetExportNames(extendFile, function_list) || function_list.size() == 0)
		return TheJsUndefined();

	InnerContext* _ctx = (InnerContext*)ctx;
	HMODULE hDll = _ctx->getOrAddExtend(extendFile);
	if (!hDll)
		return TheJsUndefined();

	if (!JsValueIsObject(parent))
		parent = NewObjectJsValue(ctx);
	for (size_t i = 0; i < function_list.size(); i++)
	{
		std::string funcName = function_list[i];

		FARPROC func = ::GetProcAddress(hDll, funcName.c_str());
		if (!func)
			continue;

		//入口
		if (funcName == "_entry")
		{
			FN_entry entry = (FN_entry)func;
			int entryRes = entry(ctx);
			if (entryRes != 0)
			{
				_ctx->unloadExtend(extendFile);
				return TheJsUndefined();
			}
		}
		else if (funcName == "_completed")
		{
			FN_completed completed = (FN_completed)func;
			completed(ctx);
		}
		else
		{
			FN_JsFunctionCallback* pFn = _ctx->addExtendFunction(
				extendFile, funcName, (FN_JsFunctionCallback)func);//记录以管理释放
			ValueHandle jsFunc = NewFunction(ctx, _extendCallHelper, 0, pFn);
			SetNamedJsValue(ctx, funcName.c_str(), jsFunc, parent);
		}
	}

	return parent;
}

void UnloadExtend(ContextHandle ctx, const char* extendFile)
{
	InnerContext* _ctx = (InnerContext*)ctx;
	_ctx->unloadExtend(extendFile);
}

#pragma endregion

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
