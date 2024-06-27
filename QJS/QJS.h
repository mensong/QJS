/*
QJS重新封装了quickjs，使得在Windows上使用更方便快捷
*/

#pragma once
#ifndef _AFX
#include <windows.h>
#endif
#include <string>
#include <stdint.h>

#define MULTI_THREAD

#ifdef MULTI_THREAD
#include <mutex>
#endif // MULTI_THREAD

#ifdef QJS_EXPORTS
#define QJS_API extern "C" __declspec(dllexport)
#else
#define QJS_API extern "C" __declspec(dllimport)
#endif

typedef void* RuntimeHandle;
typedef void* ContextHandle;
struct ValueHandle
{
	ContextHandle		ctx;
	uint64_t/*JSValue*/ value;
};

//Js value类型
enum ValueType
{
	JS_TYPE_UNINITIALIZED = 0,
	JS_TYPE_INT = 1,
	JS_TYPE_BOOL = 2,
	JS_TYPE_NULL = 3,
	JS_TYPE_UNDEFINED = 4,
	JS_TYPE_CATCH_OFFSET = 5,
	JS_TYPE_EXCEPTION = 6,
	JS_TYPE_FLOAT64 = 7,

	/* all tags with a reference count have 0b1000 bit */
	JS_TYPE_OBJECT = 8,
	JS_TYPE_FUNCTION_BYTECODE = 9, /* used internally */
	JS_TYPE_MODULE = 10, /* used internally */
	JS_TYPE_STRING = 11,
	JS_TYPE_SYMBOL = 12,
	JS_TYPE_BIG_FLOAT = 13,
	JS_TYPE_BIG_INT = 14,
	JS_TYPE_BIG_DECIMAL = 15,
};

//创建js运行时
QJS_API RuntimeHandle NewRuntime();
//销毁js运行时
QJS_API void FreeRuntime(RuntimeHandle runtime);
//get set runtime userdata
QJS_API void SetRuntimeUserData(RuntimeHandle runtime, int key, void* user_data);
QJS_API void* GetRuntimeUserData(RuntimeHandle runtime, int key);

//创建js上下文
QJS_API ContextHandle NewContext(RuntimeHandle runtime);
//把此前所有申请的C++Value释放
QJS_API void ResetContext(ContextHandle ctx);
//销毁js上下文
QJS_API void FreeContext(ContextHandle ctx);
//销毁js上下文前回调
typedef void (*FN_OnFreeingContextCallback)(ContextHandle ctx);
QJS_API void AddFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb);
QJS_API bool RemoveFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb);
//根据上下文获得Runtime
QJS_API RuntimeHandle GetContextRuntime(ContextHandle ctx);
//get set context userdata
QJS_API void SetContextUserData(ContextHandle ctx, int key, void* user_data);
QJS_API void* GetContextUserData(ContextHandle ctx, int key);

//获得顶层对象
QJS_API ValueHandle GetGlobalObject(ContextHandle ctx);

//释放js指针
QJS_API void FreeJsPointer(ContextHandle ctx, void* ptr);
//读取文件到缓存，成功返回内容，不成功返回NULL，返回的内容需要自行FreeJsPointer
QJS_API uint8_t* LoadFile(ContextHandle ctx, size_t* outLen, const char* filename);

//运行脚本
QJS_API ValueHandle RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/);
//运行文件
QJS_API ValueHandle RunScriptFile(ContextHandle ctx, const char* filename, ValueHandle parent);
//执行js中函数，没有参数时args=NULL并且argc=0
QJS_API ValueHandle CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent);

//根据名称取得一个js变量值
QJS_API ValueHandle GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//根据名称设置一个js变量值
QJS_API bool SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent);
//根据名称删除一个js变量值
QJS_API bool DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//是否有一个名称的js变量值
QJS_API bool HasNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//获得对象属性名列表，成功返回JsArray，失败返回JsUndefined。
//  onlyEnumerable-只获取可列举的属性名；enableSymbol-可以获取Symbol的属性名
QJS_API ValueHandle GetObjectPropertyKeys(ContextHandle ctx, ValueHandle jObj, bool onlyEnumerable/*=true*/, bool enableSymbol/*=false*/);

//根据序号获得一个js变量值
QJS_API ValueHandle GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);
//根据序号设置一个js变量值
QJS_API bool SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent);
//根据序号删除一个js变量值
QJS_API bool DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);

//获得.length属性 失败返回-1
QJS_API int64_t GetLength(ContextHandle ctx, ValueHandle obj);

///原型
//获得一个js变量的原型对象
QJS_API ValueHandle GetPrototype(ContextHandle ctx, ValueHandle jObj);
QJS_API bool SetPrototype(ContextHandle ctx, ValueHandle jObj, ValueHandle protoJVal);

//JS的undefined值
QJS_API ValueHandle TheJsUndefined();
//JS的null值
QJS_API ValueHandle TheJsNull();
//JS的true值
QJS_API ValueHandle TheJsTrue();
//JS的false值
QJS_API ValueHandle TheJsFalse();
//JS的exception值
QJS_API ValueHandle TheJsException();

//设置geter seter
// getter/setter是一个JsFunction。setter方法中argv[0]为设置进来的值
//同一个变量名称，不能分别设置getter和setter，要一起调用DefineGetterSetter
QJS_API bool DefineGetterSetter(ContextHandle ctx, ValueHandle parent, 
	const char* propName, ValueHandle getter, ValueHandle setter);
//int转ValueHandle
QJS_API ValueHandle NewIntJsValue(ContextHandle ctx, int intValue);
QJS_API ValueHandle NewInt64JsValue(ContextHandle ctx, int64_t intValue);
//double转ValueHandle
QJS_API ValueHandle NewDoubleJsValue(ContextHandle ctx, double doubleValue);
//string转ValueHandle
QJS_API ValueHandle NewStringJsValue(ContextHandle ctx, const char* stringValue);
//bool转ValueHandle
QJS_API ValueHandle NewBoolJsValue(ContextHandle ctx, bool boolValue);
//创建一个JS Object
QJS_API ValueHandle NewObjectJsValue(ContextHandle ctx);
//get set object userdata
QJS_API bool SetObjectUserData(ValueHandle value, void* user_data);
QJS_API void* GetObjectUserData(ValueHandle value);
//创建一个JS Array
QJS_API ValueHandle NewArrayJsValue(ContextHandle ctx);
//创建一个抛出异常
QJS_API ValueHandle NewThrowJsValue(ContextHandle ctx, ValueHandle throwWhat);
//日期转ValueHandle
QJS_API ValueHandle NewDateJsValue(ContextHandle ctx, uint64_t ms_since_1970);

//自定义js函数原型
typedef ValueHandle(*FN_JsFunctionCallback)(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data);
//创建一个JS函数
QJS_API ValueHandle NewFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc, void* user_data);

//ValueHandle转string
QJS_API const char* JsValueToString(ContextHandle ctx, ValueHandle value);
//手动释放JsValueToString出来的字符串
QJS_API void FreeJsValueToStringBuffer(ContextHandle ctx, const char* buff);
//ValueHandle转int
QJS_API int JsValueToInt(ContextHandle ctx, ValueHandle value, int defVal/* = 0*/);
//ValueHandle转int64
QJS_API int64_t JsValueToInt64(ContextHandle ctx, ValueHandle value, int64_t defVal/* = 0*/);
//ValueHandle转double
QJS_API double JsValueToDouble(ContextHandle ctx, ValueHandle value, double defVal/* = 0.0*/);
//ValueHandle转bool
QJS_API bool JsValueToBool(ContextHandle ctx, ValueHandle value, bool defVal/* = false*/);
//ValueHandle转timestamp
QJS_API uint64_t JsValueToTimestamp(ContextHandle ctx, ValueHandle value);

//编译脚本
QJS_API ValueHandle CompileScript(ContextHandle ctx, const char* script, const char* filename/* = ""*/);
//ValueHandle转ByteCode
QJS_API uint8_t* JsValueToByteCode(ContextHandle ctx, ValueHandle value, size_t* outByteCodeLen, bool byte_swap/* = false*/);
//ByteCode转ValueHandle
QJS_API ValueHandle ByteCodeToJsValue(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);
//保存ByteCode到文件，可以使用LoadFile读入内存
QJS_API bool SaveByteCodeToFile(const uint8_t* byteCode, size_t byteCodeLen, const char* filepath);
//执行bytecode脚本
QJS_API ValueHandle RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);

//获得ValueHandle的类型
QJS_API ValueType GetValueType(ValueHandle value);
QJS_API bool JsValueIsString(ValueHandle value);
QJS_API bool JsValueIsInt(ValueHandle value);
QJS_API bool JsValueIsNumber(ValueHandle value);
QJS_API bool JsValueIsDouble(ValueHandle value);
QJS_API bool JsValueIsBool(ValueHandle value);
QJS_API bool JsValueIsObject(ValueHandle value);
QJS_API bool JsValueIsArray(ValueHandle value);
QJS_API bool JsValueIsFunction(ValueHandle value);
QJS_API bool JsValueIsException(ValueHandle value);
QJS_API bool JsValueIsUndefined(ValueHandle value);
QJS_API bool JsValueIsNull(ValueHandle value);
QJS_API bool JsValueIsDate(ValueHandle value);
QJS_API bool JsValueIsGlobalObject(ContextHandle ctx, ValueHandle value);

//js value to string
QJS_API ValueHandle JsonStringify(ContextHandle ctx, ValueHandle value);
//json string to js value
QJS_API ValueHandle JsonParse(ContextHandle ctx, const char* json);


//获得异常
//Most C functions can return a Javascript exception.c
//It must be explicitly tested and handled by the C code.
//The specific JSValue JS_EXCEPTION indicates that an exception occurred.
//The actual exception object is stored in the JSContext and can be retrieved with GetException()
QJS_API ValueHandle GetAndClearJsLastException(ContextHandle ctx);

//添加事件
typedef ValueHandle(*FN_JsJobCallback)(ContextHandle ctx, int argc, ValueHandle* argv);
QJS_API bool EnqueueJob(ContextHandle ctx, FN_JsJobCallback funcJob, ValueHandle args[], int argc);
//处理事件，一般可放在应用程序的消息循环timer里。例如申请一个1毫秒的timer，然后在timer里执行ExecutePendingJob
// return < 0 if exception, 
// return 0 if no job pending, 
// return 1 if a job was executed successfully, the context of the job is stored in 'outCurCtx'
QJS_API int ExecutePendingJob(RuntimeHandle runtime, void** outRawCtx);
QJS_API ContextHandle GetContextByRaw(RuntimeHandle runtime, void* rawCtx);

//开启调试模式
QJS_API void SetDebuggerMode(ContextHandle ctx, bool onoff);
//调试行回调
typedef void (*FN_DebuggerLineCallback)(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data);
//设置调试行回调
QJS_API void SetDebuggerLineCallback(ContextHandle ctx, FN_DebuggerLineCallback cb, void* user_data);
//获得调试时的堆栈深度
QJS_API uint32_t GetDebuggerStackDepth(ContextHandle ctx);
//获得回溯
QJS_API ValueHandle GetDebuggerBacktrace(ContextHandle ctx, const uint8_t* pc);
//获得调试变量（只读）
QJS_API ValueHandle GetDebuggerClosureVariables(ContextHandle ctx, int stack_idx);
QJS_API ValueHandle GetDebuggerLocalVariables(ContextHandle ctx, int stack_idx);

//加载扩展
// parent - 如果是JsObject，则把加载到的这个对象里面；否则新建一个对象以存储加载的内容。如果想要直接加载到全局，parent=qjs.GetGlobalObject(ctx)即可。
// 返回插件ID
QJS_API int LoadExtend(ContextHandle ctx, const char* extendFile, ValueHandle parent, void* userData/* = NULL*/);
//获得扩展dll句柄
QJS_API HMODULE GetExtendHandle(ContextHandle ctx, int extendId);
//获得扩展文件名
QJS_API const char* GetExtendFile(ContextHandle ctx, int extendId);
//获得js父对象
QJS_API ValueHandle GetExtendParentObject(ContextHandle ctx, int extendId);
//卸载扩展
QJS_API void UnloadExtend(ContextHandle ctx, int extendId);

//手工释放一个ValueHandle，一般不用
QJS_API void FreeValueHandle(ValueHandle* value);
QJS_API size_t PushRunScope(ContextHandle ctx);
QJS_API size_t PopRunScope(ContextHandle ctx, size_t pushdValueIdx);

//将Ansi字符转换为Unicode字符串
QJS_API const wchar_t* AnsiToUnicode(ContextHandle ctx, const char* multiByteStr);
//将Unicode字符转换为Ansi字符串
QJS_API const char* UnicodeToAnsi(ContextHandle ctx, const wchar_t* wideByteRet);
//将Unicode字符转换为UTF8字符串
QJS_API const char* UnicodeToUtf8(ContextHandle ctx, const wchar_t* wideByteRet);
//将UTF8字符转换为Unicode字符串
QJS_API const wchar_t* Utf8ToUnicode(ContextHandle ctx, const char* utf8ByteStr);

class QJS
{
#define DEF_PROC(name) \
	decltype(::name)* name

#define SET_PROC(hDll, name) \
	this->name = (decltype(::name)*)::GetProcAddress(hDll, #name)

public:
	QJS()
	{
		hDll = LoadLibraryFromCurrentDir("QJS.dll");
		if (!hDll)
			return;

		SET_PROC(hDll, NewRuntime);
		SET_PROC(hDll, FreeRuntime); 
		SET_PROC(hDll, SetRuntimeUserData); 
		SET_PROC(hDll, GetRuntimeUserData);
		SET_PROC(hDll, NewContext);
		SET_PROC(hDll, ResetContext);
		SET_PROC(hDll, FreeContext);
		SET_PROC(hDll, AddFreeingContextCallback);
		SET_PROC(hDll, RemoveFreeingContextCallback);
		SET_PROC(hDll, SetContextUserData); 
		SET_PROC(hDll, GetContextUserData);
		SET_PROC(hDll, GetContextRuntime);
		SET_PROC(hDll, GetGlobalObject);
		SET_PROC(hDll, NewFunction);
		SET_PROC(hDll, DefineGetterSetter);
		SET_PROC(hDll, GetNamedJsValue);
		SET_PROC(hDll, SetNamedJsValue);
		SET_PROC(hDll, DeleteNamedJsValue); 
		SET_PROC(hDll, HasNamedJsValue);
		SET_PROC(hDll, GetObjectPropertyKeys);
		SET_PROC(hDll, GetIndexedJsValue);
		SET_PROC(hDll, SetIndexedJsValue);
		SET_PROC(hDll, DeleteIndexedJsValue); 
		SET_PROC(hDll, GetPrototype);
		SET_PROC(hDll, SetPrototype);
		SET_PROC(hDll, FreeJsPointer);
		SET_PROC(hDll, LoadFile);
		SET_PROC(hDll, RunScript);
		SET_PROC(hDll, RunScriptFile);
		SET_PROC(hDll, CallJsFunction);
		SET_PROC(hDll, CompileScript);
		SET_PROC(hDll, JsValueToByteCode);
		SET_PROC(hDll, ByteCodeToJsValue);		
		SET_PROC(hDll, SaveByteCodeToFile);
		SET_PROC(hDll, RunByteCode);
		SET_PROC(hDll, TheJsUndefined);
		SET_PROC(hDll, TheJsNull);
		SET_PROC(hDll, TheJsTrue);
		SET_PROC(hDll, TheJsFalse);
		SET_PROC(hDll, TheJsException);
		SET_PROC(hDll, NewIntJsValue);
		SET_PROC(hDll, NewInt64JsValue);
		SET_PROC(hDll, NewDoubleJsValue);
		SET_PROC(hDll, NewStringJsValue);
		SET_PROC(hDll, NewBoolJsValue);
		SET_PROC(hDll, NewObjectJsValue);
		SET_PROC(hDll, SetObjectUserData);
		SET_PROC(hDll, GetObjectUserData);
		SET_PROC(hDll, NewArrayJsValue);
		SET_PROC(hDll, NewThrowJsValue); 
		SET_PROC(hDll, NewDateJsValue);
		SET_PROC(hDll, GetLength);
		SET_PROC(hDll, JsValueToString);
		SET_PROC(hDll, FreeJsValueToStringBuffer);
		SET_PROC(hDll, JsValueToInt);
		SET_PROC(hDll, JsValueToInt64);
		SET_PROC(hDll, JsValueToDouble);
		SET_PROC(hDll, JsValueToBool);
		SET_PROC(hDll, GetValueType);
		SET_PROC(hDll, AnsiToUnicode);
		SET_PROC(hDll, UnicodeToAnsi);
		SET_PROC(hDll, UnicodeToUtf8);
		SET_PROC(hDll, Utf8ToUnicode);
		SET_PROC(hDll, JsValueIsString);
		SET_PROC(hDll, JsValueIsInt);
		SET_PROC(hDll, JsValueIsNumber);
		SET_PROC(hDll, JsValueIsDouble);
		SET_PROC(hDll, JsValueIsBool);
		SET_PROC(hDll, JsValueIsObject);
		SET_PROC(hDll, JsValueIsArray);
		SET_PROC(hDll, JsValueIsException);
		SET_PROC(hDll, JsValueIsFunction); 
		SET_PROC(hDll, JsValueToTimestamp); 
		SET_PROC(hDll, JsValueIsGlobalObject);
		SET_PROC(hDll, GetAndClearJsLastException);
		SET_PROC(hDll, JsValueIsUndefined);
		SET_PROC(hDll, JsValueIsNull); 
		SET_PROC(hDll, JsValueIsDate); 
		SET_PROC(hDll, JsonStringify); 
		SET_PROC(hDll, JsonParse);
		SET_PROC(hDll, EnqueueJob);
		SET_PROC(hDll, ExecutePendingJob);
		SET_PROC(hDll, GetContextByRaw);
		SET_PROC(hDll, SetDebuggerMode); 
		SET_PROC(hDll, SetDebuggerLineCallback); 
		SET_PROC(hDll, GetDebuggerStackDepth); 
		SET_PROC(hDll, GetDebuggerBacktrace);
		SET_PROC(hDll, GetDebuggerClosureVariables); 
		SET_PROC(hDll, GetDebuggerLocalVariables);
		SET_PROC(hDll, LoadExtend);
		SET_PROC(hDll, GetExtendHandle);
		SET_PROC(hDll, GetExtendFile);
		SET_PROC(hDll, GetExtendParentObject);
		SET_PROC(hDll, UnloadExtend);
		SET_PROC(hDll, FreeValueHandle);
		SET_PROC(hDll, PushRunScope);
		SET_PROC(hDll, PopRunScope);
	}


	DEF_PROC(NewRuntime);
	DEF_PROC(FreeRuntime); 
	DEF_PROC(SetRuntimeUserData); 
	DEF_PROC(GetRuntimeUserData);
	DEF_PROC(NewContext);
	DEF_PROC(ResetContext);
	DEF_PROC(FreeContext);
	DEF_PROC(AddFreeingContextCallback);
	DEF_PROC(RemoveFreeingContextCallback);
	DEF_PROC(SetContextUserData); 
	DEF_PROC(GetContextUserData);
	DEF_PROC(GetContextRuntime);
	DEF_PROC(GetGlobalObject);
	DEF_PROC(NewFunction);
	DEF_PROC(DefineGetterSetter);
	DEF_PROC(GetNamedJsValue);
	DEF_PROC(SetNamedJsValue);
	DEF_PROC(DeleteNamedJsValue); 
	DEF_PROC(HasNamedJsValue);
	DEF_PROC(GetObjectPropertyKeys);
	DEF_PROC(GetIndexedJsValue);
	DEF_PROC(SetIndexedJsValue);
	DEF_PROC(DeleteIndexedJsValue); 
	DEF_PROC(GetPrototype);
	DEF_PROC(SetPrototype);
	DEF_PROC(FreeJsPointer);
	DEF_PROC(LoadFile);
	DEF_PROC(RunScript);
	DEF_PROC(RunScriptFile);
	DEF_PROC(CallJsFunction);
	DEF_PROC(CompileScript);
	DEF_PROC(JsValueToByteCode);
	DEF_PROC(ByteCodeToJsValue);
	DEF_PROC(SaveByteCodeToFile);
	DEF_PROC(RunByteCode);
	DEF_PROC(TheJsUndefined);
	DEF_PROC(TheJsNull);
	DEF_PROC(TheJsTrue);
	DEF_PROC(TheJsFalse);
	DEF_PROC(TheJsException);
	DEF_PROC(NewIntJsValue);
	DEF_PROC(NewInt64JsValue);
	DEF_PROC(NewDoubleJsValue);
	DEF_PROC(NewStringJsValue);
	DEF_PROC(NewBoolJsValue);
	DEF_PROC(NewObjectJsValue);
	DEF_PROC(SetObjectUserData);
	DEF_PROC(GetObjectUserData);
	DEF_PROC(NewArrayJsValue);
	DEF_PROC(NewThrowJsValue); 
	DEF_PROC(NewDateJsValue);
	DEF_PROC(GetLength);
	DEF_PROC(JsValueToString);
	DEF_PROC(FreeJsValueToStringBuffer);
	DEF_PROC(JsValueToInt);
	DEF_PROC(JsValueToInt64);
	DEF_PROC(JsValueToDouble);
	DEF_PROC(JsValueToBool);
	DEF_PROC(GetValueType);
	DEF_PROC(AnsiToUnicode);
	DEF_PROC(UnicodeToAnsi);
	DEF_PROC(UnicodeToUtf8);
	DEF_PROC(Utf8ToUnicode);
	DEF_PROC(JsValueIsString);
	DEF_PROC(JsValueIsInt);
	DEF_PROC(JsValueIsNumber);
	DEF_PROC(JsValueIsDouble);
	DEF_PROC(JsValueIsBool);
	DEF_PROC(JsValueIsObject);
	DEF_PROC(JsValueIsArray);
	DEF_PROC(JsValueIsException);
	DEF_PROC(JsValueIsFunction); 
	DEF_PROC(JsValueToTimestamp);
	DEF_PROC(JsValueIsGlobalObject);
	DEF_PROC(GetAndClearJsLastException);
	DEF_PROC(JsValueIsUndefined);
	DEF_PROC(JsValueIsNull); 
	DEF_PROC(JsValueIsDate); 
	DEF_PROC(JsonStringify); 
	DEF_PROC(JsonParse);
	DEF_PROC(EnqueueJob);
	DEF_PROC(ExecutePendingJob);
	DEF_PROC(GetContextByRaw);
	DEF_PROC(SetDebuggerMode);
	DEF_PROC(SetDebuggerLineCallback); 
	DEF_PROC(GetDebuggerStackDepth);
	DEF_PROC(GetDebuggerBacktrace);
	DEF_PROC(GetDebuggerClosureVariables); 
	DEF_PROC(GetDebuggerLocalVariables);
	DEF_PROC(LoadExtend);
	DEF_PROC(GetExtendHandle);
	DEF_PROC(GetExtendFile);
	DEF_PROC(GetExtendParentObject);
	DEF_PROC(UnloadExtend);
	DEF_PROC(FreeValueHandle);
	DEF_PROC(PushRunScope);
	DEF_PROC(PopRunScope);

	//ValueHandle转std::string
	std::string JsValueToStdString(ContextHandle ctx, ValueHandle value, const std::string& defVal = "")
	{
		const char* sz = this->JsValueToString(ctx, value);
		if (!sz)
			return defVal;
		std::string ret = sz;
		this->FreeJsValueToStringBuffer(ctx, sz);
		return ret;
	}

public:
	static QJS& Ins() 
	{
#ifdef MULTI_THREAD
		std::lock_guard<std::mutex> _locker(s_insMutex);
#endif // MULTI_THREAD
		
		static QJS s_ins;
		return s_ins; 
	}

	static HMODULE LoadLibraryFromCurrentDir(const char* dllName)
	{
		char selfPath[MAX_PATH];
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE hModule = ((::VirtualQuery(
			LoadLibraryFromCurrentDir, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
		::GetModuleFileNameA(hModule, selfPath, MAX_PATH);
		std::string moduleDir(selfPath);
		size_t idx = moduleDir.find_last_of('\\');
		moduleDir = moduleDir.substr(0, idx);
		std::string modulePath = moduleDir + "\\" + dllName;
		char curDir[MAX_PATH];
		::GetCurrentDirectoryA(MAX_PATH, curDir);
		::SetCurrentDirectoryA(moduleDir.c_str());
		HMODULE hDll = LoadLibraryA(modulePath.c_str());
		::SetCurrentDirectoryA(curDir);
		if (!hDll)
		{
			DWORD err = ::GetLastError();
			char buf[10];
			sprintf_s(buf, "%u", err);
			::MessageBoxA(NULL, ("找不到" + modulePath + "模块:" + buf).c_str(), "找不到模块", MB_OK | MB_ICONERROR);
		}
		return hDll;
	}
	~QJS()
	{
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

private:
#ifdef MULTI_THREAD
	static std::mutex s_insMutex;
#endif // MULTI_THREAD
	
	HMODULE hDll;
};

#ifdef MULTI_THREAD
__declspec(selectany) std::mutex QJS::s_insMutex;
#endif // MULTI_THREAD

#define qjs QJS::Ins()

//自动管理过程新建的ValueHandle，可选
class QJSRunScope 
{
public:
	QJSRunScope(ContextHandle& ctx)
	{
		_ctx = ctx;
		_pushdValueIdx = QJS::Ins().PushRunScope(ctx);
	}
	~QJSRunScope()
	{
		QJS::Ins().PopRunScope(_ctx, _pushdValueIdx);
	}
private:
	size_t _pushdValueIdx;
	ContextHandle _ctx;
};

#define QJS_SCOPE(ctx) QJSRunScope __scope(ctx)