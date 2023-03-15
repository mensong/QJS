#pragma once
#ifndef _AFX
#include <windows.h>
#endif
#include <string>

#ifdef QJS_EXPORTS
#define QJS_API extern "C" __declspec(dllexport)
#else
#define QJS_API extern "C" __declspec(dllimport)
#endif

#define DEF_PROC(name) \
	decltype(::name)* name

#define SET_PROC(hDll, name) \
	this->name = (decltype(::name)*)::GetProcAddress(hDll, #name)

typedef void* RuntimeHandle;
typedef void* ContextHandle;
typedef uint64_t ValueHandle;

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
//创建js上下文
QJS_API ContextHandle NewContext(RuntimeHandle runtime);
//销毁js上下文
QJS_API void FreeContext(ContextHandle ctx);

//获得顶层对象
QJS_API ValueHandle GetGlobalObject(ContextHandle ctx);

//运行脚本
QJS_API ValueHandle RunScript(ContextHandle ctx, const char* script);
//执行js中函数
QJS_API ValueHandle CallJsFunction(ContextHandle ctx, ValueHandle jsFunction, ValueHandle args[], int argc, ValueHandle parent);

//根据名称取得一个js变量值
QJS_API ValueHandle GetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//根据名称设置一个js变量值
QJS_API bool SetNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle varValue, ValueHandle parent);
//根据名称删除一个js变量值
QJS_API bool DeleteNamedJsValue(ContextHandle ctx, const char* varName, ValueHandle parent);
//根据序号获得一个js变量值
QJS_API ValueHandle GetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);
//根据序号设置一个js变量值
QJS_API bool SetIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle varValue, ValueHandle parent);
//根据序号删除一个js变量值
QJS_API bool DeleteIndexedJsValue(ContextHandle ctx, uint32_t idx, ValueHandle parent);

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
QJS_API bool DefineGetterSetter(ContextHandle ctx, ValueHandle parent, const char* propName, ValueHandle getter, ValueHandle setter);
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
//创建一个JS Array
QJS_API ValueHandle NewArrayJsValue(ContextHandle ctx);
//创建一个抛出异常
QJS_API ValueHandle NewThrowJsValue(ContextHandle ctx, ValueHandle throwWhat);

//自定义js函数原型
typedef ValueHandle(*FN_JsFunctionCallback)(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data);
//创建一个JS函数
QJS_API ValueHandle NewFunction(ContextHandle ctx, FN_JsFunctionCallback cb, int argc, void* user_data);

//获得.length属性 失败返回-1
QJS_API int64_t GetLength(ContextHandle ctx, ValueHandle obj);

//手动释放一个New后的ValueHandle
QJS_API void FreeValueHandle(ContextHandle ctx, ValueHandle v);

//ValueHandle转string
QJS_API const char* JsValueToString(ContextHandle ctx, ValueHandle value, const char* defVal/* = ""*/);
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

//获得ValueHandle的类型
QJS_API ValueType GetValueType(ValueHandle value);
QJS_API bool JsValueIsString(ValueHandle value);
QJS_API bool JsValueIsInt(ValueHandle value);
QJS_API bool JsValueIsNumber(ValueHandle value);
QJS_API bool JsValueIsDouble(ValueHandle value);
QJS_API bool JsValueIsBool(ValueHandle value);
QJS_API bool JsValueIsObject(ValueHandle value);
QJS_API bool JsValueIsArray(ContextHandle ctx, ValueHandle value);
QJS_API bool JsValueIsFunction(ContextHandle ctx, ValueHandle value);
QJS_API bool JsValueIsException(ValueHandle value);
QJS_API bool JsValueIsUndefined(ValueHandle value);
QJS_API bool JsValueIsNull(ValueHandle value);

//获得异常
//Most C functions can return a Javascript exception.c
//It must be explicitly tested and handled by the C code.
//The specific JSValue JS_EXCEPTION indicates that an exception occurred.
//The actual exception object is stored in the JSContext and can be retrieved with GetException()
QJS_API ValueHandle GetJsLastException(ContextHandle ctx);

//将Ansi字符转换为Unicode字符串
QJS_API const wchar_t* AnsiToUnicode(const char* multiByteStr);
//将Unicode字符转换为Ansi字符串
QJS_API const char* UnicodeToAnsi(const wchar_t* wideByteRet);
//将Unicode字符转换为UTF8字符串
QJS_API const char* UnicodeToUtf8(const wchar_t* wideByteRet);
//将UTF8字符转换为Unicode字符串
QJS_API const wchar_t* Utf8ToUnicode(const char* utf8ByteStr);

class QJS
{
public:
private:
	static QJS* s_ins;

public:
	static QJS& Ins()
	{
		if (!s_ins) s_ins = new QJS; return *s_ins;
	}

	static void Rel()
	{
		if (s_ins)
		{
			delete s_ins;
			s_ins = NULL;
		}
	}

	static HMODULE LoadLibraryFromCurrentDir(const char* dllName)
	{
		char selfPath[MAX_PATH];
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE hModule = ((::VirtualQuery(LoadLibraryFromCurrentDir, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
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


	QJS()
	{
		hDll = LoadLibraryFromCurrentDir("QJS.dll");
		if (!hDll)
			return;

		SET_PROC(hDll, NewRuntime);
		SET_PROC(hDll, FreeRuntime);
		SET_PROC(hDll, NewContext);
		SET_PROC(hDll, FreeContext);
		SET_PROC(hDll, GetGlobalObject);
		SET_PROC(hDll, NewFunction);
		SET_PROC(hDll, DefineGetterSetter);
		SET_PROC(hDll, GetNamedJsValue);
		SET_PROC(hDll, SetNamedJsValue);
		SET_PROC(hDll, DeleteNamedJsValue);
		SET_PROC(hDll, GetIndexedJsValue);
		SET_PROC(hDll, SetIndexedJsValue);
		SET_PROC(hDll, DeleteIndexedJsValue);
		SET_PROC(hDll, RunScript);
		SET_PROC(hDll, CallJsFunction);
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
		SET_PROC(hDll, NewArrayJsValue);
		SET_PROC(hDll, NewThrowJsValue);
		SET_PROC(hDll, FreeValueHandle);
		SET_PROC(hDll, JsValueToString);
		SET_PROC(hDll, FreeJsValueToStringBuffer);
		SET_PROC(hDll, JsValueToInt);
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
		SET_PROC(hDll, GetJsLastException);
		SET_PROC(hDll, JsValueIsUndefined);
		SET_PROC(hDll, JsValueIsNull);
	}


	DEF_PROC(NewRuntime);
	DEF_PROC(FreeRuntime);
	DEF_PROC(NewContext);
	DEF_PROC(FreeContext);
	DEF_PROC(GetGlobalObject);
	DEF_PROC(NewFunction);
	DEF_PROC(DefineGetterSetter);
	DEF_PROC(GetNamedJsValue);
	DEF_PROC(SetNamedJsValue);
	DEF_PROC(DeleteNamedJsValue);
	DEF_PROC(GetIndexedJsValue);
	DEF_PROC(SetIndexedJsValue);
	DEF_PROC(DeleteIndexedJsValue);
	DEF_PROC(RunScript);
	DEF_PROC(CallJsFunction);
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
	DEF_PROC(NewArrayJsValue);
	DEF_PROC(NewThrowJsValue);
	DEF_PROC(FreeValueHandle);
	DEF_PROC(JsValueToString);
	DEF_PROC(FreeJsValueToStringBuffer);
	DEF_PROC(JsValueToInt);
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
	DEF_PROC(GetJsLastException);
	DEF_PROC(JsValueIsUndefined);
	DEF_PROC(JsValueIsNull);

	~QJS()
	{
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

	HMODULE hDll;
};
__declspec(selectany) QJS* QJS::s_ins = NULL;

#define qjs QJS::Ins()