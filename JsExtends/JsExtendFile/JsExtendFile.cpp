#include "pch.h"
#include <windows.h>
#include "../../QJS/Extend.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <tchar.h>
#include <vector>
#include "../StringConvert/StringConvert.h"

// �ж��ļ��Ƿ����
BOOL IsFileExist(const TCHAR* csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// �ж��ļ����Ƿ����
BOOL IsDirExist(const TCHAR* csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// �ж��ļ����ļ����Ƿ����
BOOL IsPathExist(const TCHAR* csPath)
{
	DWORD dwAttrib = GetFileAttributes(csPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

//// ��������(��˵�����һ��)������ע1
//BOOL IsPathExist(const TCHAR* csPath)
//{
//	WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
//	return 0 != GetFileAttributesEx(csPath, GetFileExInfoStandard, &attrs);
//}

QJS_API int _entry(ContextHandle ctx)
{
	//��������
	qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx));

	return 0;//���ز��
}

std::wstring resolveAndUpdateFilePath(ContextHandle ctx, const std::wstring& filename)
{
	if (IsFileExist(filename.c_str()))
	{
		std::wstring wpath;
		if (filename.find(L':') != std::wstring::npos)
		{//ȫ·��
			wpath = filename;
		}
		else
		{
			wchar_t dir[MAX_PATH] = { 0 };
			::GetCurrentDirectoryW(MAX_PATH, dir);
			wpath = os_pathw::join(dir, filename);
		}

		//��Ӳ���·��
		ValueHandle jaddPath = qjs.RunScript(ctx, "process.addPath", qjs.TheJsNull(), "");
		if (qjs.JsValueIsFunction(jaddPath))
		{
			std::wstring wdir = os_pathw::dirname(wpath);
			std::string dir = qjs.UnicodeToUtf8(ctx, wdir.c_str());
			ValueHandle param[] = { qjs.NewStringJsValue(ctx, dir.c_str()) };
			qjs.CallJsFunction(ctx, jaddPath, param, sizeof(param) / sizeof(ValueHandle), qjs.TheJsNull());
		}

		return wpath;
	}

	if (filename.find(L':') != std::wstring::npos)
		return L"";

	ValueHandle jprocess = qjs.GetNamedJsValue(ctx, "process", qjs.GetGlobalObject(ctx));
	if (!qjs.JsValueIsObject(jprocess))
		return L"";

	ValueHandle jpaths = qjs.GetNamedJsValue(ctx, "paths", jprocess);
	if (!qjs.JsValueIsArray(jpaths))
		return L"";

	int64_t len = qjs.GetLength(ctx, jpaths);
	for (int i = len - 1; i >= 0; --i)
	{
		ValueHandle jitem = qjs.GetIndexedJsValue(ctx, i, jpaths);
		std::string item = qjs.JsValueToStdString(ctx, jitem);
		std::wstring path = qjs.Utf8ToUnicode(ctx, item.c_str());
		std::wstring filepath = os_pathw::join(path, filename);
		if (IsFileExist(filepath.c_str()))
			return filepath;
	}

	return L"";
}

//readTextFile(filename, [function(){}]);
QJS_API ValueHandle readTextFile(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc == 0)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "no filename input");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::string filename = qjs.JsValueToStdString(ctx, argv[0]);
	std::wstring wfilename = qjs.Utf8ToUnicode(ctx, filename.c_str());
	wfilename = resolveAndUpdateFilePath(ctx, wfilename);
	if (wfilename == L"")
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, (filename + " not exist.").c_str());
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::ifstream inputFile(wfilename);
	if (!inputFile) 
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "open file error");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	if (argc > 1)
	{//ÿ�з���
		ValueHandle lineAction = argv[1];
		if (!qjs.JsValueIsFunction(lineAction))
		{
			inputFile.close();
			ValueHandle ex = qjs.NewStringJsValue(ctx, "line action is not function");
			return qjs.NewThrowJsValue(ctx, ex);
		}

		std::string line;
		int lineno = 0;
		while (std::getline(inputFile, line))
		{
			ValueHandle params[] = {
				qjs.NewStringJsValue(ctx, line.c_str()),
				qjs.NewIntJsValue(ctx, lineno)};
			ValueHandle readLineRes = qjs.CallJsFunction(ctx, lineAction, 
				params, sizeof(params)/sizeof(ValueHandle), qjs.TheJsNull());
			if (qjs.JsValueIsBool(readLineRes) &&
				qjs.JsValueToBool(ctx, readLineRes, false))//����true���˳�������false���������һ��
			{
				break;
			}
			++lineno;
		}
		inputFile.close();
		return qjs.TheJsTrue();
	}
	else
	{//���򷵻��ܽ��
		inputFile.seekg(0, std::ios::end);//�ļ�ָ��ָ���β
		size_t length = inputFile.tellg();//��ȡ�ļ�����
		inputFile.seekg(0, std::ios::beg);//�ļ�ָ�����ָ��ͷ
		char* buffer = new char[length + 1];
		buffer[length] = '\0';
		inputFile.read(buffer, length);//��ȡ
		ValueHandle ret = qjs.NewStringJsValue(ctx, buffer);
		delete[] buffer;
		inputFile.close();
		return ret;
	}	
	
	inputFile.close();
	return qjs.TheJsFalse();
}

//writeTextFile(filename, text, [out charset]);
QJS_API ValueHandle writeTextFile(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc < 2)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "no filename output");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::string filename = qjs.JsValueToStdString(ctx, argv[0]);
	std::wstring wfilename = qjs.Utf8ToUnicode(ctx, filename.c_str());

	std::ofstream outputFile(wfilename);
	if (!outputFile)
	{
		ValueHandle ex = qjs.NewStringJsValue(ctx, "open file error");
		return qjs.NewThrowJsValue(ctx, ex);
	}

	std::string text = qjs.JsValueToStdString(ctx, argv[1]);

	std::string outCharset;
	if (argc > 2)
	{
		outCharset = qjs.JsValueToStdString(ctx, argv[2], "UTF8");

		if (outCharset != "UTF8")
		{
			char* outText = NULL;
			size_t outLen = 0;
			bool b = StringConvert::Ins().ConvertCharset(&outText, &outLen, 
				text.c_str(), text.size(), "UTF8", outCharset.c_str(), true);
			if (!outText)
			{
				ValueHandle ex = qjs.NewStringJsValue(ctx, "output charset error");
				return qjs.NewThrowJsValue(ctx, ex);
			}

			text = outText;
			StringConvert::Ins().FreeOutStr(outText);
		}
		
	}

	outputFile << text;
	size_t pos = outputFile.tellp();
	outputFile.close();

	return qjs.NewInt64JsValue(ctx, pos);
}

QJS_API void _completed(ContextHandle ctx)
{
	
}
