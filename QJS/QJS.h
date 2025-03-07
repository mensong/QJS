/*
QJS重新封装了quickjs，使得在Windows上使用更方便快捷
*/

#ifndef QJS_H
#define QJS_H

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
QJS_API RuntimeHandle __cdecl NewRuntime();
//销毁js运行时
QJS_API void __cdecl FreeRuntime(RuntimeHandle runtime);
//get set runtime userdata
QJS_API void __cdecl SetRuntimeUserData(RuntimeHandle runtime, int key, void* user_data);
QJS_API void* __cdecl GetRuntimeUserData(RuntimeHandle runtime, int key);

//创建js上下文
QJS_API ContextHandle __cdecl NewContext(RuntimeHandle runtime);
//把此前所有申请的C++Value释放
QJS_API void __cdecl ResetContext(ContextHandle ctx);
//销毁js上下文
QJS_API void __cdecl FreeContext(ContextHandle ctx);
//销毁js上下文前回调
typedef void (*FN_OnFreeingContextCallback)(ContextHandle ctx);
QJS_API void __cdecl SetFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb);
QJS_API bool __cdecl RemoveFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb);
//根据上下文获得Runtime
QJS_API RuntimeHandle __cdecl GetContextRuntime(ContextHandle ctx);
//get set context userdata
QJS_API void __cdecl SetContextUserData(ContextHandle ctx, int key, void* user_data);
QJS_API void* __cdecl GetContextUserData(ContextHandle ctx, int key);

//获得顶层对象
QJS_API ValueHandle __cdecl GetGlobalObject(ContextHandle ctx);

//释放js指针
QJS_API void __cdecl FreeJsPointer(ContextHandle ctx, void* ptr);
//读取文件到缓存，成功返回内容，不成功返回NULL，返回的内容需要自行FreeJsPointer
QJS_API uint8_t* __cdecl LoadFile(ContextHandle ctx, size_t* outLen, const char* filename);

//运行脚本
QJS_API ValueHandle __cdecl RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/);
//运行文件
QJS_API ValueHandle __cdecl RunScriptFile(ContextHandle ctx, const char* filename, ValueHandle parent);
//执行js中函数，没有参数时args=NULL并且argc=0
QJS_API ValueHandle __cdecl CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent);

//根据名称取得一个js变量值
QJS_API ValueHandle __cdecl GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//根据名称设置一个js变量值
QJS_API bool __cdecl SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent);
//根据名称删除一个js变量值
QJS_API bool __cdecl DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//是否有一个名称的js变量值
QJS_API bool __cdecl HasNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//获得对象属性名列表，成功返回JsArray，失败返回JsUndefined。
//  onlyEnumerable-只获取可列举的属性名；enableSymbol-可以获取Symbol的属性名
QJS_API ValueHandle __cdecl GetObjectPropertyKeys(ContextHandle ctx, ValueHandle jObj, bool onlyEnumerable/*=true*/, bool enableSymbol/*=false*/);
//获得一个函数的名称。只对在脚本中声明的函数才有小，使用SetNamedJsValue指定的函数名无效。
QJS_API ValueHandle __cdecl GetFunctionName(ContextHandle ctx, ValueHandle func);
//获得当前运行中的函数
QJS_API ValueHandle __cdecl GetCurFrameFunction(ContextHandle ctx);

//根据序号获得一个js变量值
QJS_API ValueHandle __cdecl GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);
//根据序号设置一个js变量值
QJS_API bool __cdecl SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent);
//根据序号删除一个js变量值
QJS_API bool __cdecl DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);

//获得.length属性 失败返回-1
QJS_API int64_t __cdecl GetLength(ContextHandle ctx, ValueHandle obj);

///原型
//获得一个js变量的原型对象
QJS_API ValueHandle __cdecl GetPrototype(ContextHandle ctx, ValueHandle jObj);
QJS_API bool __cdecl SetPrototype(ContextHandle ctx, ValueHandle jObj, ValueHandle protoJVal);

//JS的undefined值
QJS_API ValueHandle __cdecl TheJsUndefined();
//JS的null值
QJS_API ValueHandle __cdecl TheJsNull();
//JS的true值
QJS_API ValueHandle __cdecl TheJsTrue();
//JS的false值
QJS_API ValueHandle __cdecl TheJsFalse();
//JS的exception值
QJS_API ValueHandle __cdecl TheJsException();

//设置geter seter
// getter/setter是一个JsFunction。setter方法中argv[0]为设置进来的值
//同一个变量名称，不能分别设置getter和setter，要一起调用DefineGetterSetter
QJS_API bool __cdecl DefineGetterSetter(ContextHandle ctx, ValueHandle parent,
	const char* propName, ValueHandle getter, ValueHandle setter);
//int转ValueHandle
QJS_API ValueHandle __cdecl NewIntJsValue(ContextHandle ctx, int intValue);
QJS_API ValueHandle __cdecl NewInt64JsValue(ContextHandle ctx, int64_t intValue);
QJS_API ValueHandle __cdecl NewUInt64JsValue(ContextHandle ctx, uint64_t intValue);
//double转ValueHandle
QJS_API ValueHandle __cdecl NewDoubleJsValue(ContextHandle ctx, double doubleValue);
//string转ValueHandle
QJS_API ValueHandle __cdecl NewStringJsValue(ContextHandle ctx, const char* stringValue);
//bool转ValueHandle
QJS_API ValueHandle __cdecl NewBoolJsValue(ContextHandle ctx, bool boolValue);
//创建一个JS Object
QJS_API ValueHandle __cdecl NewObjectJsValue(ContextHandle ctx);
//get set object userdata
QJS_API bool __cdecl SetObjectUserData(ValueHandle value, void* user_data);
QJS_API void* __cdecl GetObjectUserData(ValueHandle value);
//创建一个JS Array
QJS_API ValueHandle __cdecl NewArrayJsValue(ContextHandle ctx);
//创建一个抛出异常
QJS_API ValueHandle __cdecl NewThrowJsValue(ContextHandle ctx, ValueHandle throwWhat);
//日期转ValueHandle
QJS_API ValueHandle __cdecl NewDateJsValue(ContextHandle ctx, uint64_t ms_since_1970);

typedef void(*FN_BufferOnceFree)(ContextHandle ctx, uint8_t* buf);
//创建一个ArrayBuffer - ZeroCopy
QJS_API ValueHandle __cdecl NewArrayBufferJsValue(ContextHandle ctx, uint8_t* buf, size_t bufLen, FN_BufferOnceFree freeFunc/* = NULL*/);
//创建一个ArrayBuffer - Copy
QJS_API ValueHandle __cdecl NewArrayBufferJsValueCopy(ContextHandle ctx, const uint8_t* buf, size_t bufLen);
//释放一个ArrayBuffer
QJS_API void __cdecl DetachArrayBufferJsValue(ContextHandle ctx, ValueHandle* arrBuf);
//获得ArrayBuffer的指针
QJS_API uint8_t* __cdecl GetArrayBufferPtr(ContextHandle ctx, ValueHandle arrBuf, size_t* outBufLen);
//根据TypedArrayBuffer获得对应的ArrayBuffer
QJS_API ValueHandle __cdecl GetArrayBufferByTypedArrayBuffer(ContextHandle ctx, ValueHandle typedArrBuf,
	size_t* out_byte_offset,
	size_t* out_byte_length,
	size_t* out_bytes_per_element);
//填充一个ArrayBuffer
QJS_API bool __cdecl FillArrayBuffer(ContextHandle ctx, ValueHandle arrBuf, uint8_t fill);

//自定义js函数原型
typedef ValueHandle(*FN_JsFunctionCallback)(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data);
//创建一个JS函数
QJS_API ValueHandle __cdecl NewFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc, void* user_data);

//ValueHandle转string
QJS_API const char* __cdecl JsValueToString(ContextHandle ctx, ValueHandle value);
//手动释放JsValueToString出来的字符串
QJS_API void __cdecl FreeJsValueToStringBuffer(ContextHandle ctx, const char* buff);
//ValueHandle转int
QJS_API int __cdecl JsValueToInt(ContextHandle ctx, ValueHandle value, int defVal/* = 0*/);
//ValueHandle转int64
QJS_API int64_t __cdecl JsValueToInt64(ContextHandle ctx, ValueHandle value, int64_t defVal/* = 0*/);
//ValueHandle转UINT64
QJS_API uint64_t __cdecl JsValueToUInt64(ContextHandle ctx, ValueHandle value, uint64_t defVal/* = 0*/);
//ValueHandle转double
QJS_API double __cdecl JsValueToDouble(ContextHandle ctx, ValueHandle value, double defVal/* = 0.0*/);
//ValueHandle转bool
QJS_API bool __cdecl JsValueToBool(ContextHandle ctx, ValueHandle value, bool defVal/* = false*/);
//ValueHandle转timestamp
QJS_API uint64_t __cdecl JsValueToTimestamp(ContextHandle ctx, ValueHandle value);

//编译脚本
QJS_API ValueHandle __cdecl CompileScript(ContextHandle ctx, const char* script, const char* filename/* = ""*/);
//ValueHandle转ByteCode
QJS_API uint8_t* __cdecl JsValueToByteCode(ContextHandle ctx, ValueHandle value, size_t* outByteCodeLen, bool byte_swap/* = false*/);
//ByteCode转ValueHandle
QJS_API ValueHandle __cdecl ByteCodeToJsValue(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);
//保存ByteCode到文件，可以使用LoadFile读入内存
QJS_API bool __cdecl SaveByteCodeToFile(const uint8_t* byteCode, size_t byteCodeLen, const char* filepath);
//执行bytecode脚本
QJS_API ValueHandle __cdecl RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);

//获得ValueHandle的类型
QJS_API ValueType __cdecl GetValueType(ValueHandle value);
QJS_API bool __cdecl JsValueIsString(ValueHandle value);
QJS_API bool __cdecl JsValueIsInt(ValueHandle value);
QJS_API bool __cdecl JsValueIsNumber(ValueHandle value);
QJS_API bool __cdecl JsValueIsDouble(ValueHandle value);
QJS_API bool __cdecl JsValueIsBool(ValueHandle value);
QJS_API bool __cdecl JsValueIsObject(ValueHandle value);
QJS_API bool __cdecl JsValueIsArray(ValueHandle value);
QJS_API bool __cdecl JsValueIsFunction(ValueHandle value);
QJS_API bool __cdecl JsValueIsException(ValueHandle value);
QJS_API bool __cdecl JsValueIsUndefined(ValueHandle value);
QJS_API bool __cdecl JsValueIsNull(ValueHandle value);
QJS_API bool __cdecl JsValueIsDate(ValueHandle value);
QJS_API bool __cdecl JsValueIsGlobalObject(ContextHandle ctx, ValueHandle value);

//js value to string
//replacer（可选）
//   - 如果是一个函数，它会在每个属性被序列化之前被调用。函数接收两个参数，属性名（`key`）和属性值（`value`），并返回一个新的value，这个value会被用于序列化。
//   - 如果是一个数组，那么只有包含在这个数组中的属性名会被序列化。
//   - 如果未提供，则所有可枚举的自有属性都会被序列化。
//space（可选）
//   - 如果是一个数字，则表示在生成的 JSON 字符串中每个级别缩进的空格数。
//   - 如果是一个字符串，则该字符串会被用作缩进字符，每一级缩进都会重复这个字符串。
//   - 如果未提供，则不进行缩进，生成的 JSON 字符串是紧凑的。
QJS_API ValueHandle __cdecl JsonStringify(ContextHandle ctx, ValueHandle value, ValueHandle replacer/*=TheJsUndefined()*/, ValueHandle space/*=TheJsUndefined()*/);
//json string to js value
QJS_API ValueHandle __cdecl JsonParse(ContextHandle ctx, const char* json);


//获得异常
//Most C functions can return a Javascript exception.c
//It must be explicitly tested and handled by the C code.
//The specific JSValue JS_EXCEPTION indicates that an exception occurred.
//The actual exception object is stored in the JSContext and can be retrieved with GetException()
QJS_API ValueHandle __cdecl GetAndClearJsLastException(ContextHandle ctx);

//添加后台任务
typedef ValueHandle(*FN_JsJobCallback)(ContextHandle ctx, int argc, ValueHandle* argv);
QJS_API bool __cdecl EnqueueJob(ContextHandle ctx, FN_JsJobCallback funcJob, ValueHandle args[], int argc);
//处理一个后台任务，并把处理过的任务从任务列表里剔除
// 一般可放在应用程序的消息循环timer里。例如申请一个1毫秒的timer，然后在timer里执行ExecutePendingJob
// return < 0 if exception, 
// return 0 if no job pending, 
// return 1 if a job was executed successfully, the context of the job is stored in 'outCurCtx'
QJS_API int __cdecl ExecutePendingJob(RuntimeHandle runtime, void** outRawCtx);
QJS_API ContextHandle __cdecl GetContextByRaw(RuntimeHandle runtime, void* rawCtx);
//等待并执行后台任务
//  等待执行回调。curCtx - 当前执行到的Context。
//  返回true继续等待执行下一个任务直到执行完所有任务才结束等待；返回false则直接结束等待
typedef bool (*FN_WaitForExecutingJobsCallback)(void* rawCurCtx, int resExecutePendingJob, void* user_data);
QJS_API int __cdecl WaitForExecutingJobs(RuntimeHandle runtime,
	DWORD loopIntervalMS, FN_WaitForExecutingJobsCallback cb, void* user_data);

//开启调试模式
QJS_API void __cdecl SetDebuggerMode(ContextHandle ctx, bool onoff);
QJS_API bool __cdecl GetDebuggerMode(ContextHandle ctx);
//调试行回调 line_no - 从0开始
typedef void (*FN_DebuggerLineCallback)(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data);
//设置调试行回调
QJS_API void __cdecl SetDebuggerLineCallback(ContextHandle ctx, FN_DebuggerLineCallback cb, void* user_data);
QJS_API bool __cdecl GetDebuggerLineCallback(ContextHandle ctx, FN_DebuggerLineCallback* out_cb, void** out_user_data);
//获得调试时的堆栈深度
QJS_API uint32_t __cdecl GetDebuggerStackDepth(ContextHandle ctx);
//获得回溯 return - jArray
QJS_API ValueHandle __cdecl GetDebuggerBacktrace(ContextHandle ctx, const uint8_t* pc);
//获得调试变量（只读）
QJS_API ValueHandle __cdecl GetDebuggerClosureVariables(ContextHandle ctx, int stack_idx);
QJS_API ValueHandle __cdecl GetDebuggerLocalVariables(ContextHandle ctx, int stack_idx);

//加载扩展
// parent - 如果是JsObject，则把加载到的这个对象里面；否则新建一个对象以存储加载的内容。如果想要直接加载到全局，parent=qjs.GetGlobalObject(ctx)即可。
// 返回插件ID:extendId。成功则返回大于0的值
QJS_API int __cdecl LoadExtend(ContextHandle ctx, const char* extendFile, ValueHandle parent, void* userData/* = NULL*/);
//获得已加载的插件
// 返回为extendId数组
QJS_API const int* __cdecl GetExtendList(ContextHandle ctx, int* outLen);
//获得扩展dll句柄
QJS_API HMODULE __cdecl GetExtendHandle(ContextHandle ctx, int extendId);
//获得扩展文件名
QJS_API const char* __cdecl GetExtendFile(ContextHandle ctx, int extendId);
//获得js父对象
QJS_API ValueHandle __cdecl GetExtendParentObject(ContextHandle ctx, int extendId);
//卸载扩展
QJS_API void __cdecl UnloadExtend(ContextHandle ctx, int extendId);

//手工释放一个ValueHandle，一般不用。在PushRunScope与PopRunScope范围内（包括快捷管理类QJSRunScope）不能使用
QJS_API void __cdecl FreeValueHandle(ValueHandle* value);
//push一个范围
QJS_API size_t __cdecl PushRunScope(ContextHandle ctx);
//pop一个范围
QJS_API size_t __cdecl PopRunScope(ContextHandle ctx, size_t pushdValueIdx);

//将Ansi字符转换为Unicode字符串
QJS_API const wchar_t* __cdecl AnsiToUnicode(ContextHandle ctx, const char* multiByteStr);
//将Unicode字符转换为Ansi字符串
QJS_API const char* __cdecl UnicodeToAnsi(ContextHandle ctx, const wchar_t* wideByteRet);
//将Unicode字符转换为UTF8字符串
QJS_API const char* __cdecl UnicodeToUtf8(ContextHandle ctx, const wchar_t* wideByteRet);
//将UTF8字符转换为Unicode字符串
QJS_API const wchar_t* __cdecl Utf8ToUnicode(ContextHandle ctx, const char* utf8ByteStr);

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
		SET_PROC(hDll, SetFreeingContextCallback);
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
		SET_PROC(hDll, GetFunctionName); 
		SET_PROC(hDll, GetCurFrameFunction);
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
		SET_PROC(hDll, NewUInt64JsValue);
		SET_PROC(hDll, NewDoubleJsValue);
		SET_PROC(hDll, NewStringJsValue);
		SET_PROC(hDll, NewBoolJsValue);
		SET_PROC(hDll, NewObjectJsValue);
		SET_PROC(hDll, SetObjectUserData);
		SET_PROC(hDll, GetObjectUserData);
		SET_PROC(hDll, NewArrayJsValue);
		SET_PROC(hDll, NewThrowJsValue); 
		SET_PROC(hDll, NewDateJsValue);
		SET_PROC(hDll, NewArrayBufferJsValue); 
		SET_PROC(hDll, NewArrayBufferJsValueCopy);
		SET_PROC(hDll, DetachArrayBufferJsValue);
		SET_PROC(hDll, GetArrayBufferPtr);
		SET_PROC(hDll, GetArrayBufferByTypedArrayBuffer);
		SET_PROC(hDll, FillArrayBuffer);
		SET_PROC(hDll, GetLength);
		SET_PROC(hDll, JsValueToString);
		SET_PROC(hDll, FreeJsValueToStringBuffer);
		SET_PROC(hDll, JsValueToInt);
		SET_PROC(hDll, JsValueToInt64); 
		SET_PROC(hDll, JsValueToUInt64);
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
		SET_PROC(hDll, WaitForExecutingJobs);
		SET_PROC(hDll, SetDebuggerMode); 
		SET_PROC(hDll, GetDebuggerMode);
		SET_PROC(hDll, SetDebuggerLineCallback);
		SET_PROC(hDll, GetDebuggerLineCallback);
		SET_PROC(hDll, GetDebuggerStackDepth); 
		SET_PROC(hDll, GetDebuggerBacktrace);
		SET_PROC(hDll, GetDebuggerClosureVariables); 
		SET_PROC(hDll, GetDebuggerLocalVariables);
		SET_PROC(hDll, LoadExtend);
		SET_PROC(hDll, GetExtendList);
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
	DEF_PROC(SetFreeingContextCallback);
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
	DEF_PROC(GetFunctionName); 
	DEF_PROC(GetCurFrameFunction);
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
	DEF_PROC(NewUInt64JsValue);
	DEF_PROC(NewDoubleJsValue);
	DEF_PROC(NewStringJsValue);
	DEF_PROC(NewBoolJsValue);
	DEF_PROC(NewObjectJsValue);
	DEF_PROC(SetObjectUserData);
	DEF_PROC(GetObjectUserData);
	DEF_PROC(NewArrayJsValue);
	DEF_PROC(NewThrowJsValue); 
	DEF_PROC(NewDateJsValue);
	DEF_PROC(NewArrayBufferJsValue); 
	DEF_PROC(NewArrayBufferJsValueCopy);
	DEF_PROC(DetachArrayBufferJsValue);
	DEF_PROC(GetArrayBufferPtr);
	DEF_PROC(GetArrayBufferByTypedArrayBuffer);
	DEF_PROC(FillArrayBuffer);
	DEF_PROC(GetLength);
	DEF_PROC(JsValueToString);
	DEF_PROC(FreeJsValueToStringBuffer);
	DEF_PROC(JsValueToInt);
	DEF_PROC(JsValueToInt64); 
	DEF_PROC(JsValueToUInt64);
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
	DEF_PROC(WaitForExecutingJobs);
	DEF_PROC(SetDebuggerMode);
	DEF_PROC(GetDebuggerMode);
	DEF_PROC(SetDebuggerLineCallback);
	DEF_PROC(GetDebuggerLineCallback);
	DEF_PROC(GetDebuggerStackDepth);
	DEF_PROC(GetDebuggerBacktrace);
	DEF_PROC(GetDebuggerClosureVariables); 
	DEF_PROC(GetDebuggerLocalVariables);
	DEF_PROC(LoadExtend);
	DEF_PROC(GetExtendList);
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
	inline static QJS& Ins() 
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


#endif // QJS_H