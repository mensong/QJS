#include "pch.h"
#include "../../QJS/Extend.h"
#include <sstream>

QJS_API int entry(ContextHandle ctx)
{
	return 0;//���ز��
}


//��Ansi�ַ�ת��ΪUnicode�ַ���
std::wstring AnsiToUnicode(const std::string& multiByteStr)
{
	wchar_t* pWideCharStr; //���巵�صĿ��ַ�ָ��
	int nLenOfWideCharStr; //������ַ�������ע�ⲻ���ֽ���
	const char* pMultiByteStr = multiByteStr.c_str();
	//��ȡ���ַ��ĸ���
	nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, -1, NULL, 0);
	//��ÿ��ַ�ָ��
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//����
	std::wstring wideByteRet(pWideCharStr, nLenOfWideCharStr);
	//�����ڴ��е��ַ���
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return wideByteRet.c_str();
}

//��Unicode�ַ�ת��ΪAnsi�ַ���
std::string UnicodeToAnsi(const std::wstring& wideByteStr)
{
	char* pMultiCharStr; //���巵�صĶ��ַ�ָ��
	int nLenOfMultiCharStr; //������ַ�������ע�ⲻ���ֽ���
	const wchar_t* pWideByteStr = wideByteStr.c_str();
	//��ȡ���ַ��ĸ���
	nLenOfMultiCharStr = WideCharToMultiByte(CP_ACP, 0, pWideByteStr, -1, NULL, 0, NULL, NULL);
	//��ö��ַ�ָ��
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_ACP, 0, pWideByteStr, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//����
	std::string sRet(pMultiCharStr, nLenOfMultiCharStr);
	//�����ڴ��е��ַ���
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
	//Ԥת�����õ�����ռ�Ĵ�С;
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, utf8Str.c_str(), strlen(utf8Str.c_str()), NULL, 0);
	//����ռ�Ҫ��'\0'�����ռ䣬MultiByteToWideChar�����'\0'�ռ�
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	//ת��
	::MultiByteToWideChar(CP_UTF8, NULL, utf8Str.c_str(), strlen(utf8Str.c_str()), wszString, wcsLen);
	//������'\0'
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

QJS_API ValueHandle alert(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::string msg;
	if (argc > 0)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[0]);
		if (sz)
			msg = Utf8ToAnsi(sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	std::string title;
	if (argc < 2 || qjs.JsValueIsUndefined(argv[1]))
		title = "QJS";
	else
	{
		const char* sz = qjs.JsValueToString(ctx, argv[1]);
		if (sz)
			title = Utf8ToAnsi(sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	MessageBoxA(NULL, msg.c_str(), title.c_str(), 0);

	return qjs.TheJsUndefined();
}

QJS_API ValueHandle print(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::stringstream ss;
	for (int i = 0; i < argc; i++)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[i]);
		if (sz)
			ss << Utf8ToAnsi(sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	printf(ss.str().c_str());
	return qjs.TheJsUndefined();
}