// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。

//
////将Ansi字符转换为Unicode字符串
//std::wstring AnsiToUnicode(const std::string& multiByteStr)
//{
//	wchar_t* pWideCharStr; //定义返回的宽字符指针
//	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
//	const char* pMultiByteStr = multiByteStr.c_str();
//	//获取宽字符的个数
//	nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, -1, NULL, 0);
//	//获得宽字符指针
//	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
//	MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, -1, pWideCharStr, nLenOfWideCharStr);
//	//返回
//	std::wstring wideByteRet(pWideCharStr, nLenOfWideCharStr);
//	//销毁内存中的字符串
//	HeapFree(GetProcessHeap(), 0, pWideCharStr);
//	return wideByteRet;
//}
//
////将Unicode字符转换为Ansi字符串
//std::string UnicodeToAnsi(const std::wstring& wideByteRet)
//{
//	char* pMultiCharStr; //定义返回的多字符指针
//	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
//	const wchar_t* pWideByteStr = wideByteRet.c_str();
//	//获取多字符的个数
//	nLenOfMultiCharStr = WideCharToMultiByte(CP_ACP, 0, pWideByteStr, -1, NULL, 0, NULL, NULL);
//	//获得多字符指针
//	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
//	WideCharToMultiByte(CP_ACP, 0, pWideByteStr, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
//	//返回
//	std::string sRet(pMultiCharStr, nLenOfMultiCharStr);
//	//销毁内存中的字符串
//	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
//	return sRet;
//}
//
////将Unicode字符转换为UTF8字符串
//std::string UnicodeToUtf8(const std::wstring& wideByteRet)
//{
//	char* pMultiCharStr; //定义返回的多字符指针
//	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
//	const wchar_t* pWideByteStr = wideByteRet.c_str();
//	//获取多字符的个数
//	nLenOfMultiCharStr = WideCharToMultiByte(CP_UTF8, 0, pWideByteStr, -1, NULL, 0, NULL, NULL);
//	//获得多字符指针
//	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
//	WideCharToMultiByte(CP_UTF8, 0, pWideByteStr, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
//	//返回
//	std::string sRet(pMultiCharStr, nLenOfMultiCharStr);
//	//销毁内存中的字符串
//	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
//	return sRet;
//}
//
////将UTF8字符转换为Unicode字符串
//std::wstring Utf8ToUnicode(const std::string& utf8ByteStr)
//{
//	wchar_t* pWideCharStr; //定义返回的宽字符指针
//	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
//	const char* pUtf8ByteStr = utf8ByteStr.c_str();
//	//获取宽字符的个数
//	nLenOfWideCharStr = MultiByteToWideChar(CP_UTF8, 0, pUtf8ByteStr, -1, NULL, 0);
//	//获得宽字符指针
//	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
//	MultiByteToWideChar(CP_UTF8, 0, pUtf8ByteStr, -1, pWideCharStr, nLenOfWideCharStr);
//	//返回
//	std::wstring wideByteRet(pWideCharStr, nLenOfWideCharStr);
//	//销毁内存中的字符串
//	HeapFree(GetProcessHeap(), 0, pWideCharStr);
//	return wideByteRet;
//}
