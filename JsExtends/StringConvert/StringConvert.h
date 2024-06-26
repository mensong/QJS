#pragma once
#ifndef _AFX
#include <windows.h>
#endif
#include <string>
#include <stdint.h>

#ifdef STRINGCONVERT_EXPORTS
#define STRINGCONVERT_API extern "C" __declspec(dllexport)
#else
#define STRINGCONVERT_API extern "C" __declspec(dllimport)
#endif

STRINGCONVERT_API void FreeOutStr(void* outStr);

//����ַ������ַ���
STRINGCONVERT_API bool DetectCharset(char** outCharset, const char* inStr, size_t inSize);

//��ͬ�ַ������ת��
STRINGCONVERT_API bool ConvertCharset(char** outStr, size_t* outSize, const char* inStr, size_t inSize, 
	const char* fromCharset, const char* toCharset, bool exact = true);

//unicode תΪ ansi
STRINGCONVERT_API bool Unicode2Ansi(char** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize, const char* locale = "");

//ansi ת Unicode
STRINGCONVERT_API bool Ansi2Unicode(wchar_t** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale = "");

//Unicode ת UTF8
STRINGCONVERT_API bool Unicode2Utf8(char** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize, const char* locale = "");

//UTF8 ת Unicode
STRINGCONVERT_API bool Utf82Unicode(wchar_t** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale = "");

//ansiתUTF8
STRINGCONVERT_API bool Ansi2Utf8(char** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale = "");

//UTF8תansi
STRINGCONVERT_API bool Utf82Ansi(char** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale = "");

//�ַ���תhex�ַ���
STRINGCONVERT_API bool Str2Hex(char** outStr, size_t* outSize, const char* inStr, size_t inSize);
STRINGCONVERT_API bool WStr2Hex(wchar_t** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize);

//hex�ַ���ת�ַ���
STRINGCONVERT_API bool Hex2Str(char** outStr, size_t* outSize, const char* inStr, size_t inSize);
STRINGCONVERT_API bool Hex2WStr(wchar_t** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize);

//base64
STRINGCONVERT_API void Base64Encode(char** outStr, size_t* outSize, const char* inStr, size_t inSize);
STRINGCONVERT_API bool Base64Decode(char** outStr, size_t* outSize, const char* inStr, size_t inSize);

class StringConvert
{
public:
	typedef void (*FN_FreeOutStr)(void* outStr);
	typedef bool (*FN_DetectCharset)(char** outCharset, const char* inStr, size_t inSize);
	typedef bool (*FN_ConvertCharset)(char** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* fromCharset, const char* toCharset, bool exact/*=true*/);
	typedef bool (*FN_Unicode2Ansi)(char** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize, const char* locale/* = ""*/);
	typedef bool (*FN_Ansi2Unicode)(wchar_t** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale/* = ""*/);
	typedef bool (*FN_Unicode2Utf8)(char** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize, const char* locale/* = ""*/);
	typedef bool (*FN_Utf82Unicode)(wchar_t** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale/* = ""*/);
	typedef bool (*FN_Ansi2Utf8)(char** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale/* = ""*/);
	typedef bool (*FN_Utf82Ansi)(char** outStr, size_t* outSize, const char* inStr, size_t inSize, const char* locale/* = ""*/);
	typedef bool (*FN_Str2Hex)(char** outStr, size_t* outSize, const char* inStr, size_t inSize);
	typedef bool (*FN_WStr2Hex)(wchar_t** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize);
	typedef bool (*FN_Hex2Str)(char** outStr, size_t* outSize, const char* inStr, size_t inSize);
	typedef bool (*FN_Hex2WStr)(wchar_t** outStr, size_t* outSize, const wchar_t* inStr, size_t inSize);
	typedef void (*FN_Base64Encode)(char** outStr, size_t* outSize, const char* inStr, size_t inSize);
	typedef bool (*FN_Base64Decode)(char** outStr, size_t* outSize, const char* inStr, size_t inSize);

#define DEF_PROC(hDll, name) \
	name = (FN_##name)::GetProcAddress(hDll, #name)

	StringConvert()
	{
		hDll = LoadLibraryFromCurrentDir("StringConvert.dll");
		if (!hDll)
			return;

		DEF_PROC(hDll, FreeOutStr);
		DEF_PROC(hDll, DetectCharset);
		DEF_PROC(hDll, ConvertCharset);
		DEF_PROC(hDll, Unicode2Ansi);
		DEF_PROC(hDll, Ansi2Unicode);
		DEF_PROC(hDll, Unicode2Utf8);
		DEF_PROC(hDll, Utf82Unicode);
		DEF_PROC(hDll, Ansi2Utf8);
		DEF_PROC(hDll, Utf82Ansi);
		DEF_PROC(hDll, Str2Hex);
		DEF_PROC(hDll, WStr2Hex);
		DEF_PROC(hDll, Hex2Str);
		DEF_PROC(hDll, Hex2WStr);
		DEF_PROC(hDll, Base64Encode);
		DEF_PROC(hDll, Base64Decode);
	}

	FN_FreeOutStr			FreeOutStr;
	FN_DetectCharset		DetectCharset;
	FN_ConvertCharset		ConvertCharset;
	FN_Unicode2Ansi			Unicode2Ansi;
	FN_Ansi2Unicode			Ansi2Unicode;
	FN_Unicode2Utf8			Unicode2Utf8;
	FN_Utf82Unicode			Utf82Unicode;
	FN_Ansi2Utf8			Ansi2Utf8;
	FN_Utf82Ansi			Utf82Ansi;
	FN_Str2Hex				Str2Hex;
	FN_WStr2Hex				WStr2Hex;
	FN_Hex2Str				Hex2Str;
	FN_Hex2WStr				Hex2WStr;
	FN_Base64Encode			Base64Encode;
	FN_Base64Decode			Base64Decode;
	
public:
	~StringConvert()
	{
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

	static StringConvert& Ins()
	{
		static StringConvert s_ins;
		return s_ins;
	}

	static HMODULE LoadLibraryFromCurrentDir(const char* dllName)
	{
		char selfPath[MAX_PATH];
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE hModule = ((::VirtualQuery(LoadLibraryFromCurrentDir, &mbi, sizeof(mbi)) != 0) ?
			(HMODULE)mbi.AllocationBase : NULL);
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
			::MessageBoxA(NULL, ("�Ҳ���" + modulePath + "ģ��:" + buf).c_str(),
				"�Ҳ���ģ��", MB_OK | MB_ICONERROR);
		}
		return hDll;
	}

	HMODULE hDll;
};

