/*
QJS���·�װ��quickjs��ʹ����Windows��ʹ�ø�������
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

//Js value����
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

//����js����ʱ
QJS_API RuntimeHandle NewRuntime();
//����js����ʱ
QJS_API void FreeRuntime(RuntimeHandle runtime);
//get set runtime userdata
QJS_API void SetRuntimeUserData(RuntimeHandle runtime, int key, void* user_data);
QJS_API void* GetRuntimeUserData(RuntimeHandle runtime, int key);

//����js������
QJS_API ContextHandle NewContext(RuntimeHandle runtime);
//�Ѵ�ǰ���������C++Value�ͷ�
QJS_API void ResetContext(ContextHandle ctx);
//����js������
QJS_API void FreeContext(ContextHandle ctx);
//����js������ǰ�ص�
typedef void (*FN_OnFreeingContextCallback)(ContextHandle ctx);
QJS_API void AddFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb);
QJS_API bool RemoveFreeingContextCallback(ContextHandle ctx, FN_OnFreeingContextCallback cb);
//���������Ļ��Runtime
QJS_API RuntimeHandle GetContextRuntime(ContextHandle ctx);
//get set context userdata
QJS_API void SetContextUserData(ContextHandle ctx, int key, void* user_data);
QJS_API void* GetContextUserData(ContextHandle ctx, int key);

//��ö������
QJS_API ValueHandle GetGlobalObject(ContextHandle ctx);

//�ͷ�jsָ��
QJS_API void FreeJsPointer(ContextHandle ctx, void* ptr);
//��ȡ�ļ������棬�ɹ��������ݣ����ɹ�����NULL�����ص�������Ҫ����FreeJsPointer
QJS_API uint8_t* LoadFile(ContextHandle ctx, size_t* outLen, const char* filename);

//���нű�
QJS_API ValueHandle RunScript(ContextHandle ctx, const char* script, ValueHandle parent, const char* filename/*=""*/);
//�����ļ�
QJS_API ValueHandle RunScriptFile(ContextHandle ctx, const char* filename, ValueHandle parent);
//ִ��js�к�����û�в���ʱargs=NULL����argc=0
QJS_API ValueHandle CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent);

//��������ȡ��һ��js����ֵ
QJS_API ValueHandle GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//������������һ��js����ֵ
QJS_API bool SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent);
//��������ɾ��һ��js����ֵ
QJS_API bool DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//�Ƿ���һ�����Ƶ�js����ֵ
QJS_API bool HasNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//��ö����������б��ɹ�����JsArray��ʧ�ܷ���JsUndefined��
//  onlyEnumerable-ֻ��ȡ���оٵ���������enableSymbol-���Ի�ȡSymbol��������
QJS_API ValueHandle GetObjectPropertyKeys(ContextHandle ctx, ValueHandle jObj, bool onlyEnumerable/*=true*/, bool enableSymbol/*=false*/);

//������Ż��һ��js����ֵ
QJS_API ValueHandle GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);
//�����������һ��js����ֵ
QJS_API bool SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent);
//�������ɾ��һ��js����ֵ
QJS_API bool DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);

//���.length���� ʧ�ܷ���-1
QJS_API int64_t GetLength(ContextHandle ctx, ValueHandle obj);

///ԭ��
//���һ��js������ԭ�Ͷ���
QJS_API ValueHandle GetPrototype(ContextHandle ctx, ValueHandle jObj);
QJS_API bool SetPrototype(ContextHandle ctx, ValueHandle jObj, ValueHandle protoJVal);

//JS��undefinedֵ
QJS_API ValueHandle TheJsUndefined();
//JS��nullֵ
QJS_API ValueHandle TheJsNull();
//JS��trueֵ
QJS_API ValueHandle TheJsTrue();
//JS��falseֵ
QJS_API ValueHandle TheJsFalse();
//JS��exceptionֵ
QJS_API ValueHandle TheJsException();

//����geter seter
// getter/setter��һ��JsFunction��setter������argv[0]Ϊ���ý�����ֵ
//ͬһ���������ƣ����ֱܷ�����getter��setter��Ҫһ�����DefineGetterSetter
QJS_API bool DefineGetterSetter(ContextHandle ctx, ValueHandle parent, 
	const char* propName, ValueHandle getter, ValueHandle setter);
//intתValueHandle
QJS_API ValueHandle NewIntJsValue(ContextHandle ctx, int intValue);
QJS_API ValueHandle NewInt64JsValue(ContextHandle ctx, int64_t intValue);
//doubleתValueHandle
QJS_API ValueHandle NewDoubleJsValue(ContextHandle ctx, double doubleValue);
//stringתValueHandle
QJS_API ValueHandle NewStringJsValue(ContextHandle ctx, const char* stringValue);
//boolתValueHandle
QJS_API ValueHandle NewBoolJsValue(ContextHandle ctx, bool boolValue);
//����һ��JS Object
QJS_API ValueHandle NewObjectJsValue(ContextHandle ctx);
//get set object userdata
QJS_API bool SetObjectUserData(ValueHandle value, void* user_data);
QJS_API void* GetObjectUserData(ValueHandle value);
//����һ��JS Array
QJS_API ValueHandle NewArrayJsValue(ContextHandle ctx);
//����һ���׳��쳣
QJS_API ValueHandle NewThrowJsValue(ContextHandle ctx, ValueHandle throwWhat);
//����תValueHandle
QJS_API ValueHandle NewDateJsValue(ContextHandle ctx, uint64_t ms_since_1970);

//�Զ���js����ԭ��
typedef ValueHandle(*FN_JsFunctionCallback)(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data);
//����һ��JS����
QJS_API ValueHandle NewFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc, void* user_data);

//ValueHandleתstring
QJS_API const char* JsValueToString(ContextHandle ctx, ValueHandle value);
//�ֶ��ͷ�JsValueToString�������ַ���
QJS_API void FreeJsValueToStringBuffer(ContextHandle ctx, const char* buff);
//ValueHandleתint
QJS_API int JsValueToInt(ContextHandle ctx, ValueHandle value, int defVal/* = 0*/);
//ValueHandleתint64
QJS_API int64_t JsValueToInt64(ContextHandle ctx, ValueHandle value, int64_t defVal/* = 0*/);
//ValueHandleתdouble
QJS_API double JsValueToDouble(ContextHandle ctx, ValueHandle value, double defVal/* = 0.0*/);
//ValueHandleתbool
QJS_API bool JsValueToBool(ContextHandle ctx, ValueHandle value, bool defVal/* = false*/);
//ValueHandleתtimestamp
QJS_API uint64_t JsValueToTimestamp(ContextHandle ctx, ValueHandle value);

//����ű�
QJS_API ValueHandle CompileScript(ContextHandle ctx, const char* script, const char* filename/* = ""*/);
//ValueHandleתByteCode
QJS_API uint8_t* JsValueToByteCode(ContextHandle ctx, ValueHandle value, size_t* outByteCodeLen, bool byte_swap/* = false*/);
//ByteCodeתValueHandle
QJS_API ValueHandle ByteCodeToJsValue(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);
//����ByteCode���ļ�������ʹ��LoadFile�����ڴ�
QJS_API bool SaveByteCodeToFile(const uint8_t* byteCode, size_t byteCodeLen, const char* filepath);
//ִ��bytecode�ű�
QJS_API ValueHandle RunByteCode(ContextHandle ctx, const uint8_t* byteCode, size_t byteCodeLen);

//���ValueHandle������
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


//����쳣
//Most C functions can return a Javascript exception.c
//It must be explicitly tested and handled by the C code.
//The specific JSValue JS_EXCEPTION indicates that an exception occurred.
//The actual exception object is stored in the JSContext and can be retrieved with GetException()
QJS_API ValueHandle GetAndClearJsLastException(ContextHandle ctx);

//����¼�
typedef ValueHandle(*FN_JsJobCallback)(ContextHandle ctx, int argc, ValueHandle* argv);
QJS_API bool EnqueueJob(ContextHandle ctx, FN_JsJobCallback funcJob, ValueHandle args[], int argc);
//�����¼���һ��ɷ���Ӧ�ó������Ϣѭ��timer���������һ��1�����timer��Ȼ����timer��ִ��ExecutePendingJob
// return < 0 if exception, 
// return 0 if no job pending, 
// return 1 if a job was executed successfully, the context of the job is stored in 'outCurCtx'
QJS_API int ExecutePendingJob(RuntimeHandle runtime, void** outRawCtx);
QJS_API ContextHandle GetContextByRaw(RuntimeHandle runtime, void* rawCtx);

//��������ģʽ
QJS_API void SetDebuggerMode(ContextHandle ctx, bool onoff);
//�����лص�
typedef void (*FN_DebuggerLineCallback)(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data);
//���õ����лص�
QJS_API void SetDebuggerLineCallback(ContextHandle ctx, FN_DebuggerLineCallback cb, void* user_data);
//��õ���ʱ�Ķ�ջ���
QJS_API uint32_t GetDebuggerStackDepth(ContextHandle ctx);
//��û���
QJS_API ValueHandle GetDebuggerBacktrace(ContextHandle ctx, const uint8_t* pc);
//��õ��Ա�����ֻ����
QJS_API ValueHandle GetDebuggerClosureVariables(ContextHandle ctx, int stack_idx);
QJS_API ValueHandle GetDebuggerLocalVariables(ContextHandle ctx, int stack_idx);

//������չ
// parent - �����JsObject����Ѽ��ص�������������棻�����½�һ�������Դ洢���ص����ݡ������Ҫֱ�Ӽ��ص�ȫ�֣�parent=qjs.GetGlobalObject(ctx)���ɡ�
// ���ز��ID
QJS_API int LoadExtend(ContextHandle ctx, const char* extendFile, ValueHandle parent, void* userData/* = NULL*/);
//�����չdll���
QJS_API HMODULE GetExtendHandle(ContextHandle ctx, int extendId);
//�����չ�ļ���
QJS_API const char* GetExtendFile(ContextHandle ctx, int extendId);
//���js������
QJS_API ValueHandle GetExtendParentObject(ContextHandle ctx, int extendId);
//ж����չ
QJS_API void UnloadExtend(ContextHandle ctx, int extendId);

//�ֹ��ͷ�һ��ValueHandle��һ�㲻��
QJS_API void FreeValueHandle(ValueHandle* value);
QJS_API size_t PushRunScope(ContextHandle ctx);
QJS_API size_t PopRunScope(ContextHandle ctx, size_t pushdValueIdx);

//��Ansi�ַ�ת��ΪUnicode�ַ���
QJS_API const wchar_t* AnsiToUnicode(ContextHandle ctx, const char* multiByteStr);
//��Unicode�ַ�ת��ΪAnsi�ַ���
QJS_API const char* UnicodeToAnsi(ContextHandle ctx, const wchar_t* wideByteRet);
//��Unicode�ַ�ת��ΪUTF8�ַ���
QJS_API const char* UnicodeToUtf8(ContextHandle ctx, const wchar_t* wideByteRet);
//��UTF8�ַ�ת��ΪUnicode�ַ���
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

	//ValueHandleתstd::string
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
			::MessageBoxA(NULL, ("�Ҳ���" + modulePath + "ģ��:" + buf).c_str(), "�Ҳ���ģ��", MB_OK | MB_ICONERROR);
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

//�Զ���������½���ValueHandle����ѡ
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