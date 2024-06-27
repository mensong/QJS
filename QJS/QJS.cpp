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


/* quickjs中参数为JSValue的内部会自动JS_FreeValue；为JSValueConst则不会需要自己调用JS_FreeValue */

#undef __has_attribute
#include <cutils.h>

#if defined(__APPLE__)
#define MALLOC_OVERHEAD  0
#else
#define MALLOC_OVERHEAD  8
#endif

struct QJSContext;
struct QJSRuntime;

//QJSRuntime* == RuntimeHandle
struct QJSRuntime
{
	JSRuntime* raw;
	std::set<QJSContext*> contexts;
	
	std::map<int, void*> userData;

	QJSRuntime()
	{
		raw = NULL;
	}

	~QJSRuntime();
};

//QJSContext* == ContextHandle
struct QJSContext
{
	JSContext* raw;
	QJSRuntime* runtime;
	std::vector<uint64_t> values;

	//这里的全部都要提取到一个对象中，并且使用指针引用;
	std::map<int, void*> userData;

	int __magicIdx;
	std::map<int, std::pair<FN_JsFunctionCallback, void*> > __NewFunctionFunctions;

	int __JobFunctionIdx;
	std::map<
		int, /*__EnqueueJobCallHelper argv第一个参数的值代表函数标识*/
		FN_JsJobCallback> __JobFunctions;

	std::wstring Ret_AnsiToUnicode;
	std::string Ret_UnicodeToAnsi;
	std::string Ret_UnicodeToUtf8;
	std::wstring Ret_Utf8ToUnicode;

	//上下文释放前的回调
	std::set<FN_OnFreeingContextCallback> onFreeingContextCallbacks;

	QJSContext()
	{
		raw = NULL;
		runtime = NULL;

		__magicIdx = 0;
		__JobFunctionIdx = 0;

	}

	~QJSContext()
	{
		for (size_t i = 0; i < values.size(); i++)
		{
			JS_FreeValue(raw, (JSValue)values[i]);
		}
		values.clear();
		
#pragma region Extend
		clearExtends();
#pragma endregion

		JS_FreeContext(raw);

		//清除runtime中context的引用
		if (runtime)
		{
			runtime->contexts.erase(this);
		}
	}

	void addValue(uint64_t v)
	{
		values.push_back(v);
	}

#pragma region Extend

	int extendIdx = 0;
	//插件函数UserData
	struct ExtendFunctionUserData
	{
		int extendId;
		FN_JsExtendFunction nativeFunc;
		void* nativeUserData;

		ExtendFunctionUserData()
		{
			extendId = 0;
			nativeFunc = NULL;
			nativeUserData = NULL;
		}
	};
	//插件信息
	struct ExtendInfo
	{
		HMODULE hDll;
		std::string filename;
		int extendId;

		ValueHandle parentObj;

		//<funcion name, Function callback pointer>
		std::map<std::string, FN_JsFunctionCallback> functions;

		ExtendInfo()
		{
			hDll = NULL;
			extendId = 0;
		}
	};
	//<extend id, ExtendInfo pointer>
	std::map<int, ExtendInfo*> extends;
	//插件函数UserData缓存，用于最后释放
	std::vector<ExtendFunctionUserData*> extendFunctionUserDatasHolder;

	//返回值-插件ID。返回0失败，非0成功
	int loadExtend(const std::string& filename)
	{
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

		if (!hDll)
			return 0;

		++extendIdx;//id递增

		ExtendInfo* pEI = new ExtendInfo();
		pEI->extendId = extendIdx;
		pEI->hDll = hDll;
		pEI->filename = filename;

		extends.insert(std::make_pair(extendIdx, pEI));

		return extendIdx;
	}

	ExtendInfo* getExtendInfo(int extendId)
	{
		auto itFinder = extends.find(extendId);
		if (itFinder == extends.end())
			return NULL;
		return itFinder->second;
	}

	ExtendFunctionUserData* addExtendFunction(int extendId, const std::string& funcName, 
		FN_JsExtendFunction nativeFunc, void* userData)
	{
		ExtendFunctionUserData* pUserData = new ExtendFunctionUserData();
		pUserData->extendId = extendId;
		//pUserData->parentObj = parentObj;
		pUserData->nativeFunc = nativeFunc;
		pUserData->nativeUserData = userData;
		extendFunctionUserDatasHolder.push_back(pUserData);
		return pUserData;
	}

	bool unloadExtend(int extendId)
	{
		ExtendInfo* pEI = getExtendInfo(extendId);
		if (!pEI)
			return false;

		//卸载
		FN_unload _unload = (FN_unload)::GetProcAddress(pEI->hDll, "_unload");
		if (_unload)
			_unload((ContextHandle)this, extendId);
				
		::FreeLibrary(pEI->hDll);
		extends.erase(extendId);
		delete pEI;
		return true;
	}

	void clearExtends()
	{
		std::vector<int> extendIds;
		for (auto it = extends.begin(); it != extends.end(); ++it)
		{
			if (!it->second)
				continue;
			extendIds.push_back(it->second->extendId);
		}
		for (size_t i = 0; i < extendIds.size(); i++)
		{
			unloadExtend(extendIds[i]);
		}
		extends.clear();

		for (size_t i = 0; i < extendFunctionUserDatasHolder.size(); i++)
		{
			ExtendFunctionUserData* p = extendFunctionUserDatasHolder[i];
			if (p)
				delete p;
		}
		extendFunctionUserDatasHolder.clear();
	}
#pragma endregion

};

QJSRuntime::~QJSRuntime()
{
	for (auto it = contexts.begin(); it != contexts.end(); ++it)
	{
		delete *it;
	}
	contexts.clear();
}



#define ADD_AUTO_FREE(v) if ((v).ctx && (v).value)((QJSContext*)(v).ctx)->addValue((v).value)

#define _INNER_RT(rt) (JSRuntime*)(((QJSRuntime*)(rt))->raw)
//#define _OUTER_RT(rt) (RuntimeHandle)(rt)

#define _INNER_CTX(ctx) (JSContext*)(((QJSContext*)(ctx))->raw)
//#define _OUTER_CTX(ctx) (ContextHandle)(ctx)

#define _INNER_VAL(val) (JSValue)(val).value
inline ValueHandle _OUTER_VAL(ContextHandle ctx, JSValue val)
{
	ValueHandle ret;
	ret.ctx = ctx;
	ret.value = val;
	return ret;
}
//#define _OUTER_VAL(ctx, val) ValueHandle((ContextHandle)(ctx), (uint64_t)(val))

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

RuntimeHandle JSContextToContextHandle(JSRuntime* rt)
{
	QJSRuntime* qjsRt = (QJSRuntime*)JS_GetRuntimeOpaque(rt);
	return (RuntimeHandle)qjsRt;
}

ContextHandle JSContextToContextHandle(JSContext* ctx)
{
	QJSContext* qjsCtx = (QJSContext*)JS_GetContextOpaque(ctx);
	return (ContextHandle)qjsCtx;
}

RuntimeHandle NewRuntime()
{
	// Create a runtime. 
	JSRuntime* runtime =  JS_NewRuntime2(&qjs_malloc_funcs, NULL);

	//js_std_init_handlers(runtime);
	//JS_SetModuleLoaderFunc(runtime, nullptr, js_module_loader, nullptr);

	QJSRuntime* _rt = new QJSRuntime();
	_rt->raw = runtime;

	//JSRuntime*的Opaque是QJSRuntime*
	JS_SetRuntimeOpaque(runtime, _rt);

	return (RuntimeHandle)_rt;
}

void FreeRuntime(RuntimeHandle runtime)
{	
	if (!runtime)
		return;

	JS_FreeRuntime(_INNER_RT(runtime));
}

void SetRuntimeUserData(RuntimeHandle runtime, int key, void* user_data)
{
	QJSRuntime* thisRt = (QJSRuntime*)runtime;
	thisRt->userData[key] = user_data;
}

void* GetRuntimeUserData(RuntimeHandle runtime, int key)
{
	QJSRuntime* thisRt = (QJSRuntime*)runtime;
	auto itFinder = thisRt->userData.find(key);
	if (itFinder == thisRt->userData.end())
		return NULL;
	return itFinder->second;
}

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

	QJSContext* _ctx = new QJSContext();
	_ctx->raw = context;
	_ctx->runtime = (QJSRuntime*)runtime;
	_ctx->values.reserve(1024);

	//JSContext*的Opaque为QJSContext*
	JS_SetContextOpaque(context, _ctx);

	return (ContextHandle)_ctx;
}

void ResetContext(ContextHandle ctx)
{
	if (!ctx)
		return;

	QJSContext* innerCtx = (QJSContext*)ctx;
	for (size_t i = 0; i < innerCtx->values.size(); i++)
	{
		JS_FreeValue(_INNER_CTX(ctx), (JSValue)innerCtx->values[i]);
	}
	innerCtx->values.clear();

#pragma region Extend
	innerCtx->clearExtends();
#pragma endregion
}

void FreeContext(ContextHandle ctx)
{
	if (!ctx)
		return;

	QJSContext* thisCtx = (QJSContext*)ctx;

	//回调
	for (auto it = thisCtx->onFreeingContextCallbacks.begin();
		it != thisCtx->onFreeingContextCallbacks.end(); ++it)
	{
		if (*it)
			(*it)(ctx);
	}

	ResetContext(ctx);
	
	//删除context内存
	delete thisCtx;
}

void AddFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	thisCtx->onFreeingContextCallbacks.insert(cb);
}

bool RemoveFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	auto itFinder = thisCtx->onFreeingContextCallbacks.find(cb);
	if (itFinder == thisCtx->onFreeingContextCallbacks.end())
		return false;
	thisCtx->onFreeingContextCallbacks.erase(itFinder);
	return true;
}

void SetContextUserData(ContextHandle ctx, int key, void* user_data)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	thisCtx->userData[key] = user_data;
}

QJS_API void* GetContextUserData(ContextHandle ctx, int key)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	auto itFinder = thisCtx->userData.find(key);
	if (itFinder == thisCtx->userData.end())
		return NULL;
	return itFinder->second;
}

RuntimeHandle GetContextRuntime(ContextHandle ctx)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	return (RuntimeHandle)(thisCtx->runtime);
}

ValueHandle GetGlobalObject(ContextHandle ctx)
{
	ValueHandle ret = _OUTER_VAL(ctx, JS_GetGlobalObject(_INNER_CTX(ctx)));
	ADD_AUTO_FREE(ret);
	return ret;
}

static JSValue __NewFunctionCallHelper(JSContext* rawCtx, JSValue this_val, int argc, JSValue* argv, int magic)
{
	QJSContext* thisCtx = (QJSContext*)JSContextToContextHandle(rawCtx);
	if (thisCtx == NULL)
		return JS_EXCEPTION;

	auto itFinder = thisCtx->__NewFunctionFunctions.find(magic);
	if (itFinder != thisCtx->__NewFunctionFunctions.end())
	{
		if (itFinder->second.first)
		{
			//push value
			size_t pushdValueIdx = thisCtx->values.size();

			ValueHandle* _innerArgv = NULL;
			if (argc > 0)
				_innerArgv = new ValueHandle[argc];
			for (size_t i = 0; i < argc; i++)
			{
				_innerArgv[i] = _OUTER_VAL(rawCtx, argv[i]);
			}

			ValueHandle ret = itFinder->second.first( 
				(ContextHandle)thisCtx, {rawCtx, this_val}, argc, (ValueHandle*)_innerArgv, itFinder->second.second);

			if (_innerArgv)
				delete[] _innerArgv;

			//这里的值是传给内部的，由内部自己释放，防止在下面被释放
			JS_DupValue(rawCtx, _INNER_VAL(ret));

			//pop value
			for (int i = thisCtx->values.size() - 1; i >= pushdValueIdx; i--)
			{
				JSValue jv = (JSValue)thisCtx->values.back();
				JS_FreeValue(rawCtx, jv);
				thisCtx->values.pop_back();
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
		return TheJsException();
	}

	QJSContext* thisCtx = (QJSContext*)ctx;
	if (thisCtx == NULL)
		return TheJsException();

	JSValue fv = JS_NewCFunctionMagic(_INNER_CTX(ctx), 
		(JSCFunctionMagic*)__NewFunctionCallHelper, NULL, argc, JS_CFUNC_generic_magic, thisCtx->__magicIdx);

	if (JS_IsFunction(_INNER_CTX(ctx), fv))
	{
		thisCtx->__NewFunctionFunctions.insert(std::make_pair(thisCtx->__magicIdx, std::make_pair(cb, user_data)));
		thisCtx->__magicIdx++;
	}

	ValueHandle ret = { ctx, fv };

	ADD_AUTO_FREE(ret);

	return ret;
}

void FreeValueHandle(ValueHandle* value)
{
	if (!value || !value->ctx)
		return;
	
	QJSContext* innerCtx = (QJSContext*)value->ctx;
	std::vector<uint64_t>::iterator itFinder = std::find(
		innerCtx->values.begin(),
		innerCtx->values.end(), value->value);
	if (itFinder != innerCtx->values.end())
		innerCtx->values.erase(itFinder);

	JS_FreeValue(_INNER_CTX(value->ctx), (JSValue)value->value);

	value->ctx = NULL;
	value->value = NULL;
}

size_t PushRunScope(ContextHandle ctx)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	return thisCtx->values.size();
}

size_t PopRunScope(ContextHandle ctx, size_t pushdValueIdx)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	size_t curSize = thisCtx->values.size();
	for (int i = curSize - 1; i >= pushdValueIdx; i--)
	{
		JSValue jv = (JSValue)thisCtx->values.back();
		JS_FreeValue(thisCtx->raw, jv);
		thisCtx->values.pop_back();
	}

	return curSize - pushdValueIdx;
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

	if (JsValueIsDate(jObj))
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
	//QJSContext* thisCtx = (QJSContext*)ctx;
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

ValueHandle RunScriptFile(ContextHandle ctx, const char* filename, ValueHandle parent)
{
	size_t buf_len = 0;
	uint8_t* buf = LoadFile(ctx, &buf_len, filename);
	if (buf)
	{
		ValueHandle ret = RunScript(ctx, (const char*)buf, parent, filename);
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

bool SaveByteCodeToFile(const uint8_t* byteCode, size_t byteCodeLen, const char* filepath)
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

bool JsValueIsArray(ValueHandle value)
{
	if (!value.ctx)
		return false;
	return JS_IsArray(_INNER_CTX(value.ctx), _INNER_VAL(value)) == TRUE;
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

bool JsValueIsDate(ValueHandle value)
{
	if (!value.ctx)
		return false;
	return JS_IsDate(_INNER_CTX(value.ctx), _INNER_VAL(value), NULL) == TRUE;
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

bool JsValueIsFunction(ValueHandle value)
{
	if (!value.ctx)
		return false;
	return JS_IsFunction(_INNER_CTX(value.ctx), _INNER_VAL(value)) == TRUE;
}

ValueHandle GetAndClearJsLastException(ContextHandle ctx)
{
	JSValue res = JS_GetException(_INNER_CTX(ctx));
	ValueHandle ret = _OUTER_VAL(ctx, res);
	ADD_AUTO_FREE(ret);
	return ret;
}

JSValue __EnqueueJobCallHelper(JSContext* ctx, int argc, JSValueConst* argv)
{
	if (argc == 0)
		return JS_EXCEPTION;

	QJSContext* thisCtx = (QJSContext*)JSContextToContextHandle(ctx);
	if (!thisCtx)
		return JS_EXCEPTION;

	int jobIdx = 0;
	if (JS_ToInt32(ctx, &jobIdx, argv[0]) != 0)
		return JS_EXCEPTION;
	auto itFinder = thisCtx->__JobFunctions.find(jobIdx);
	if (itFinder == thisCtx->__JobFunctions.end())
		return JS_EXCEPTION;

	//push value
	size_t pushdValueIdx = thisCtx->values.size();

	int argc_real = argc - 1;
	ValueHandle* argv_real = NULL;
	if (argc_real > 0)
		argv_real = new ValueHandle[argc_real];
	for (int i = 1; i < argc; i++)
	{
		argv_real[i - 1] = _OUTER_VAL(thisCtx, argv[i]);
	}
	ValueHandle ret = itFinder->second((ContextHandle)thisCtx, argc_real, argv_real);
	if (argv_real)
		delete[] argv_real;

	//这里的值是传给内部的，由内部自己释放，防止在下面被释放
	JS_DupValue(ctx, _INNER_VAL(ret));

	//pop value
	for (int i = thisCtx->values.size() - 1; i >= pushdValueIdx; i--)
	{
		JSValue jv = (JSValue)thisCtx->values.back();
		JS_FreeValue(ctx, jv);
		thisCtx->values.pop_back();
	}

	return _INNER_VAL(ret);
}

bool EnqueueJob(ContextHandle ctx, FN_JsJobCallback funcJob, ValueHandle args[], int argc)
{
	QJSContext* thisCtx = (QJSContext*)ctx;

	JSValue* innerArr = new JSValue[argc + 1/*+1多加了一个函数标识*/];

	//构造函数标识
	int jobIdx = thisCtx->__JobFunctionIdx++;
	innerArr[0] = JS_NewInt32(_INNER_CTX(ctx), jobIdx);
	thisCtx->__JobFunctions.insert(std::make_pair(jobIdx, funcJob));

	if (argc > 0)
	{
		for (size_t i = 0; i < argc; i++)
		{
			innerArr[i + 1] = args[i].value;
		}
	}

	int res = JS_EnqueueJob(_INNER_CTX(ctx), __EnqueueJobCallHelper, argc + 1/*+1多加了一个函数标识*/, innerArr);

	if (innerArr)
	{
		delete[] innerArr;
	}

	return res == 0;
}

int ExecutePendingJob(RuntimeHandle runtime, void** outRawCtx)
{
	JSContext* curCtx = NULL;
	int res = JS_ExecutePendingJob(_INNER_RT(runtime), (JSContext**)&curCtx);
	*outRawCtx = curCtx;
	return res;
}

ContextHandle GetContextByRaw(RuntimeHandle runtime, void* rawCtx)
{
	auto& contexts = ((QJSRuntime*)runtime)->contexts;
	for (auto it = contexts.begin(); it != contexts.end(); ++it)
	{
		if ((*it)->raw == rawCtx)
		{
			return (*it);
		}
	}

	return NULL;
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
				QJSContext* thisCtx = (QJSContext*)JSContextToContextHandle(rawCtx);

				//push value
				size_t pushdValueIdx = thisCtx->values.size();

				itFinder->second.first(&rawCtx, line_no, pc, itFinder->second.second);

				//pop value
				for (int i = thisCtx->values.size() - 1; i >= pushdValueIdx; i--)
				{
					JSValue jv = (JSValue)thisCtx->values.back();
					JS_FreeValue(rawCtx, jv);
					thisCtx->values.pop_back();
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
	QJSContext::ExtendFunctionUserData* pFuncUserData = (QJSContext::ExtendFunctionUserData*)user_data;
	if (!pFuncUserData)
		return TheJsUndefined();

	QJSContext* thisCtx = (QJSContext*)ctx;
	if (thisCtx->extends.find(pFuncUserData->extendId) == thisCtx->extends.end())
	{
		std::string err = "Extend#" + std::to_string(pFuncUserData->extendId) + " has be unloaded";
		return NewThrowJsValue(ctx, NewStringJsValue(ctx, err.c_str()));
	}

	if (!pFuncUserData->nativeFunc)
	{
		return NewThrowJsValue(ctx, NewStringJsValue(ctx, "No native function"));
	}

	return pFuncUserData->nativeFunc(ctx, this_val, argc, argv, 
		pFuncUserData->nativeUserData, pFuncUserData->extendId);
}

int LoadExtend(ContextHandle ctx, const char* extendFile, ValueHandle parent, void* userData/*=NULL*/)
{
	std::vector<std::string> function_list;
	if (!GetExportNames(extendFile, function_list) || function_list.size() == 0)
		return 0;

	bool bDupGlobal = false;
	if (!JsValueIsObject(parent))
	{
		parent = GetGlobalObject(ctx);
		bDupGlobal = true;
	}

	QJSContext* _ctx = (QJSContext*)ctx;

	int extendId = _ctx->loadExtend(extendFile);
	if (extendId <= 0)
	{
		if (bDupGlobal)
			FreeValueHandle(&parent);
		return extendId;
	}

	QJSContext::ExtendInfo* pEI = _ctx->getExtendInfo(extendId);
	if (!pEI)
	{
		if (bDupGlobal)
			FreeValueHandle(&parent);
		return 0;
	}
	pEI->parentObj = parent;

	FN_entry _entry = (FN_entry)GetProcAddress(pEI->hDll, "_entry");
	FN_completed _completed = (FN_completed)GetProcAddress(pEI->hDll, "_completed");
	if (_entry)
	{
		int entryRes = _entry(ctx, extendId);
		if (entryRes != 0)
		{
			_ctx->unloadExtend(extendId);

			if (bDupGlobal)
				FreeValueHandle(&parent);
			return 0;
		}
	}

	for (size_t i = 0; i < function_list.size(); i++)
	{
		std::string funcName = function_list[i];

		FARPROC func = ::GetProcAddress(pEI->hDll, funcName.c_str());
		if (!func)
			continue;

		//
		if (funcName != "_entry" && funcName != "_completed" && funcName != "_unload")
		{
			QJSContext::ExtendFunctionUserData* pFuncUserData = _ctx->addExtendFunction(
				extendId, funcName, (FN_JsExtendFunction)func, userData);
			ValueHandle jsFunc = NewFunction(ctx, _extendCallHelper, 0, pFuncUserData);
			SetNamedJsValue(ctx, funcName.c_str(), jsFunc, parent);
		}
	}

	if (_completed)
		_completed(ctx, extendId);

	return extendId;
}

void UnloadExtend(ContextHandle ctx, int extendId)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	thisCtx->unloadExtend(extendId);
}

HMODULE GetExtendHandle(ContextHandle ctx, int extendId)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	QJSContext::ExtendInfo* pEI = thisCtx->getExtendInfo(extendId);
	if (!pEI)
		return NULL;
	return pEI->hDll;
}

const char* GetExtendFile(ContextHandle ctx, int extendId)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	QJSContext::ExtendInfo* pEI = thisCtx->getExtendInfo(extendId);
	if (!pEI)
		return NULL;
	return pEI->filename.c_str();
}

ValueHandle GetExtendParentObject(ContextHandle ctx, int extendId)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	QJSContext::ExtendInfo* pEI = thisCtx->getExtendInfo(extendId);
	if (!pEI)
		return TheJsUndefined();
	return pEI->parentObj;
}

#pragma endregion


const wchar_t* AnsiToUnicode(ContextHandle ctx, const char* multiByteStr)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	if (thisCtx == NULL)
		return L"";

	wchar_t* pWideCharStr; //定义返回的宽字符指针
	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
	//获取宽字符的个数
	nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, multiByteStr, -1, NULL, 0);
	//获得宽字符指针
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_ACP, 0, multiByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//返回
	thisCtx->Ret_AnsiToUnicode.resize(nLenOfWideCharStr + 1, 0);
	wcscpy_s(&thisCtx->Ret_AnsiToUnicode[0], thisCtx->Ret_AnsiToUnicode.size(), pWideCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return thisCtx->Ret_AnsiToUnicode.c_str();
}

const char* UnicodeToAnsi(ContextHandle ctx, const wchar_t* wideByteRet)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	if (thisCtx == NULL)
		return "";

	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_ACP, 0, wideByteRet, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_ACP, 0, wideByteRet, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	thisCtx->Ret_UnicodeToAnsi.resize(nLenOfMultiCharStr + 1, 0);
	strcpy_s(&thisCtx->Ret_UnicodeToAnsi[0], thisCtx->Ret_UnicodeToAnsi.size(), pMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return thisCtx->Ret_UnicodeToAnsi.c_str();
}

const char* UnicodeToUtf8(ContextHandle ctx, const wchar_t* wideByteRet)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	if (thisCtx == NULL)
		return "";

	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_UTF8, 0, wideByteRet, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_UTF8, 0, wideByteRet, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	thisCtx->Ret_UnicodeToUtf8.resize(nLenOfMultiCharStr + 1, 0);
	strcpy_s(&thisCtx->Ret_UnicodeToUtf8[0], thisCtx->Ret_UnicodeToUtf8.size(), pMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return thisCtx->Ret_UnicodeToUtf8.c_str();
}

const wchar_t* Utf8ToUnicode(ContextHandle ctx, const char* utf8ByteStr)
{
	QJSContext* thisCtx = (QJSContext*)ctx;
	if (thisCtx == NULL)
		return L"";

	wchar_t* pWideCharStr; //定义返回的宽字符指针
	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
	//获取宽字符的个数
	nLenOfWideCharStr = MultiByteToWideChar(CP_UTF8, 0, utf8ByteStr, -1, NULL, 0);
	//获得宽字符指针
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_UTF8, 0, utf8ByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//返回
	thisCtx->Ret_Utf8ToUnicode.resize(nLenOfWideCharStr + 1, 0);
	wcscpy_s(&thisCtx->Ret_Utf8ToUnicode[0], thisCtx->Ret_Utf8ToUnicode.size(), pWideCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return thisCtx->Ret_Utf8ToUnicode.c_str();
}

