#include "pch.h"
#include "../../QJS/Extend.h"
#include "RegOperator.h"
#include "../pystring/pystring.h"

QJS_API int _entry(ContextHandle ctx)
{
	return 0;
}

//将Ansi字符转换为Unicode字符串
std::wstring AnsiToUnicode(const std::string& multiByteStr)
{
	wchar_t* pWideCharStr; //定义返回的宽字符指针
	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
	const char* pMultiByteStr = multiByteStr.c_str();
	//获取宽字符的个数
	nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, -1, NULL, 0);
	//获得宽字符指针
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//返回
	std::wstring wideByteRet(pWideCharStr, nLenOfWideCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return wideByteRet.c_str();
}

//将Unicode字符转换为Ansi字符串
std::string UnicodeToAnsi(const std::wstring& wideByteStr)
{
	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	const wchar_t* pWideByteStr = wideByteStr.c_str();
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_ACP, 0, pWideByteStr, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_ACP, 0, pWideByteStr, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	std::string sRet(pMultiCharStr, nLenOfMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return sRet.c_str();
}

std::string UnicodeToUtf8(const std::wstring& wideByteStr)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, wideByteStr.c_str(), -1, NULL, 0, NULL, NULL);
	char* szUtf8 = new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wideByteStr.c_str(), -1, szUtf8, len, NULL, NULL);
	std::string s = szUtf8;
	delete[] szUtf8;
	return s.c_str();
}

std::wstring Utf8ToUnicode(const std::string& utf8Str)
{
	//预转换，得到所需空间的大小;
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, utf8Str.c_str(), strlen(utf8Str.c_str()), NULL, 0);
	//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	//转换
	::MultiByteToWideChar(CP_UTF8, NULL, utf8Str.c_str(), strlen(utf8Str.c_str()), wszString, wcsLen);
	//最后加上'\0'
	wszString[wcsLen] = '\0';
	std::wstring s(wszString);
	delete[] wszString;
	return s;
}

std::string AnsiToUtf8(const std::string& multiByteStr)
{
	std::wstring ws = AnsiToUnicode(multiByteStr);
	return UnicodeToUtf8(ws);
}

std::string Utf8ToAnsi(const std::string& utf8Str)
{
	std::wstring ws = Utf8ToUnicode(utf8Str);
	return UnicodeToAnsi(ws);
}

/* helper macro to fixup start/end slice values */
#define ADJUST_INDICES(start, end, len)         \
    if (end > len)                          \
        end = len;                          \
    else if (end < 0) {                     \
        end += len;                         \
        if (end < 0)                        \
        end = 0;                        \
    }                                       \
    if (start < 0) {                        \
        start += len;                       \
        if (start < 0)                      \
        start = 0;                      \
    }

#define MAX_32BIT_INT 2147483647

static int strFind(const std::string& str, const std::string& sub, int start = 0, int end = MAX_32BIT_INT)
{
	ADJUST_INDICES(start, end, (int)str.size());

	std::string::size_type result = str.find(sub, start);

	// If we cannot find the string, or if the end-point of our found substring is past
	// the allowed end limit, return that it can't be found.
	if (result == std::string::npos ||
		(result + sub.size() > (std::string::size_type)end))
	{
		return -1;
	}

	return (int)result;
}

static std::string strReplace(const std::string& str, const std::string& oldstr, const std::string& newstr, int count = -1)
{
	int sofar = 0;
	int cursor = 0;
	std::string s(str);

	std::string::size_type oldlen = oldstr.size(), newlen = newstr.size();

	cursor = strFind(s, oldstr, cursor);

	while (cursor != -1 && cursor <= (int)s.size())
	{
		if (count > -1 && sofar >= count)
		{
			break;
		}

		s.replace(cursor, oldlen, newstr);
		cursor += (int)newlen;

		if (oldlen != 0)
		{
			cursor = strFind(s, oldstr, cursor);
		}
		else
		{
			++cursor;
		}

		++sofar;
	}

	return s;

}

static void normalPath(std::string& sPath)
{
	sPath = strReplace(sPath, "/", "\\");

	do
	{
		size_t oldLen = sPath.size();
		sPath = strReplace(sPath, "\\\\", "\\");
		if (oldLen == sPath.size())
			break;
	} while (true);
}

//createKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", true);
QJS_API ValueHandle createKey(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsFalse();
	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	bool bX64 = false;
	if (argc > 1)
		bX64 = qjs.JsValueToBool(ctx, argv[1], bX64);

	std::vector<std::string> keys;
	pystring::split(regPath, keys, "\\");
	if (keys.size() < 2)
	{
		return qjs.TheJsFalse();
	}

	CRegOperator reg(keys[0].c_str());
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	for (size_t i = 1; i < keys.size(); i++)
	{
		reg = reg.CreateKey(keys[i].c_str(), bX64);
		if (!reg.IsValid())
			return qjs.TheJsFalse();
	}
	
	return qjs.TheJsTrue();
}

//deleteKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", true);
QJS_API ValueHandle deleteKey(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsFalse();
	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	bool bX64 = false;
	if (argc > 1)
		bX64 = qjs.JsValueToBool(ctx, argv[1], bX64);

	size_t idx = regPath.find_last_of("\\");
	if (idx == std::string::npos)
		return qjs.TheJsFalse();

	std::string rootPath = regPath.substr(0, idx);
	std::string deleteKey = regPath.substr(idx + 1);
	CRegOperator reg(rootPath.c_str());
	if (reg.DeleteKey(deleteKey.c_str(), bX64))
		return qjs.TheJsTrue();

	return qjs.TheJsFalse();
}

//deleteTree("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", true);
QJS_API ValueHandle deleteTree(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsFalse();
	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	bool bX64 = false;
	if (argc > 1)
		bX64 = qjs.JsValueToBool(ctx, argv[1], bX64);

	size_t idx = regPath.find_last_of("\\");
	if (idx == std::string::npos)
		return qjs.TheJsFalse();

	std::string rootPath = regPath.substr(0, idx);
	std::string deleteKey = regPath.substr(idx + 1);
	CRegOperator reg(rootPath.c_str());
	if (reg.DeleteTree(deleteKey.c_str()))
		return qjs.TheJsTrue();

	return qjs.TheJsFalse();
}

//deleteValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "KeyName", true);
QJS_API ValueHandle deleteValue(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsFalse();
	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string regValue = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));

	bool bX64 = false;
	if (argc > 2)
		bX64 = qjs.JsValueToBool(ctx, argv[2], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	if (reg.DeleteValue(regValue.c_str()))
		return qjs.TheJsTrue();

	return qjs.TheJsFalse();
}

//saveKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "D:\\1.reg", true);
QJS_API ValueHandle saveKey(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsFalse();
	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string filename = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));

	bool bX64 = false;
	if (argc > 2)
		bX64 = qjs.JsValueToBool(ctx, argv[2], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	if (reg.SaveKey(filename.c_str()))
		return qjs.TheJsTrue();

	return qjs.TheJsFalse();
}

//RestoreKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "D:\\1.reg", true);
QJS_API ValueHandle RestoreKey(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsFalse();
	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string filename = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));

	bool bX64 = false;
	if (argc > 2)
		bX64 = qjs.JsValueToBool(ctx, argv[2], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	if (reg.RestoreKey(filename.c_str()))
		return qjs.TheJsTrue();

	return qjs.TheJsFalse();
}

//ReadStringValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "valueKey", true);
QJS_API ValueHandle ReadStringValue(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsUndefined();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string valueKey = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));

	bool bX64 = false;
	if (argc > 2)
		bX64 = qjs.JsValueToBool(ctx, argv[2], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsUndefined();

	std::string val;
	if (reg.ReadValue(valueKey.c_str(), val))
	{
		std::wstring wval = qjs.AnsiToUnicode(ctx, val.c_str());
		val = qjs.UnicodeToUtf8(ctx, wval.c_str());
		return qjs.NewStringJsValue(ctx, val.c_str());
	}

	return qjs.TheJsUndefined();
}

//ReadDwordValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "valueKey", true);
QJS_API ValueHandle ReadDwordValue(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
		return qjs.TheJsUndefined();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string valueKey = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));

	bool bX64 = false;
	if (argc > 2)
		bX64 = qjs.JsValueToBool(ctx, argv[2], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsUndefined();

	DWORD val = 0;
	if (reg.ReadValue(valueKey.c_str(), val))
	{
		return qjs.NewInt64JsValue(ctx, val);
	}

	return qjs.TheJsUndefined();
}

//WriteStringValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "valueKey", "value", true);
QJS_API ValueHandle WriteStringValue(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 3)
		return qjs.TheJsFalse();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string valueKey = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));
	std::string value = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[2]));

	bool bX64 = false;
	if (argc > 3)
		bX64 = qjs.JsValueToBool(ctx, argv[3], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	if (reg.WriteValue(valueKey.c_str(), value.c_str()))
	{
		return qjs.TheJsTrue();
	}

	return qjs.TheJsFalse();
}

//WriteDwordValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "valueKey", 123, true);
QJS_API ValueHandle WriteDwordValue(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 3)
		return qjs.TheJsFalse();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string valueKey = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));
	DWORD value = qjs.JsValueToInt64(ctx, argv[2], 0);

	bool bX64 = false;
	if (argc > 3)
		bX64 = qjs.JsValueToBool(ctx, argv[3], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	if (reg.WriteValue(valueKey.c_str(), value))
	{
		return qjs.TheJsTrue();
	}

	return qjs.TheJsFalse();
}

//WriteExpandStringValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", "valueKey", "value", true);
QJS_API ValueHandle WriteExpandStringValue(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 3)
		return qjs.TheJsFalse();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	std::string valueKey = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[1]));
	std::string value = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[2]));

	bool bX64 = false;
	if (argc > 3)
		bX64 = qjs.JsValueToBool(ctx, argv[3], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsFalse();

	if (reg.WriteExpandString(valueKey.c_str(), value.c_str()))
	{
		return qjs.TheJsTrue();
	}

	return qjs.TheJsFalse();
}

//EnumSubKeys("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", true);
QJS_API ValueHandle EnumSubKeys(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);
		
	bool bX64 = false;
	if (argc > 1)
		bX64 = qjs.JsValueToBool(ctx, argv[1], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsUndefined();

	std::vector<std::string> subTreeNames;
	if (reg.EnumSub(&subTreeNames))
	{
		ValueHandle jSubKeys = qjs.NewArrayJsValue(ctx);
		for (size_t i = 0; i < subTreeNames.size(); i++)
		{
			std::wstring wval = qjs.AnsiToUnicode(ctx, subTreeNames[i].c_str());
			std::string val = qjs.UnicodeToUtf8(ctx, wval.c_str());

			qjs.SetIndexedJsValue(ctx, i, 
				qjs.NewStringJsValue(ctx, val.c_str()), jSubKeys);
		}
		
		return jSubKeys;
	}

	return qjs.TheJsUndefined();
}

//EnumValueNameKeys("HKEY_LOCAL_MACHINE\\SOFTWARE\\MySoftware", true);
QJS_API ValueHandle EnumValueNameKeys(
	ContextHandle ctx, ValueHandle this_val,
	int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 1)
		return qjs.TheJsUndefined();

	std::string regPath = Utf8ToAnsi(qjs.JsValueToStdString(ctx, argv[0]));
	normalPath(regPath);

	bool bX64 = false;
	if (argc > 1)
		bX64 = qjs.JsValueToBool(ctx, argv[1], bX64);

	CRegOperator reg(regPath.c_str(), bX64);
	if (!reg.IsValid())
		return qjs.TheJsUndefined();

	std::vector<std::string> subValueNames;
	if (reg.EnumSub(NULL, &subValueNames))
	{
		ValueHandle jSubValueNames = qjs.NewArrayJsValue(ctx);
		for (size_t i = 0; i < subValueNames.size(); i++)
		{
			std::wstring wval = qjs.AnsiToUnicode(ctx, subValueNames[i].c_str());
			std::string val = qjs.UnicodeToUtf8(ctx, wval.c_str());

			qjs.SetIndexedJsValue(ctx, i,
				qjs.NewStringJsValue(ctx, val.c_str()), jSubValueNames);
		}

		return jSubValueNames;
	}

	return qjs.TheJsUndefined();
}
