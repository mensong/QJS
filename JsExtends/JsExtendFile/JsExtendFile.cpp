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
#include <map>
#include "../StringConvert/StringConvert.h"
#include "../JsExtendBase/JsExtendBase.h"

// 判断文件是否存在
BOOL IsFileExist(const TCHAR* csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件夹是否存在
BOOL IsDirExist(const TCHAR* csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
// 判断文件或文件夹是否存在
BOOL IsPathExist(const TCHAR* csPath)
{
	DWORD dwAttrib = GetFileAttributes(csPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

//// 变变变变变种(听说会更快一点)，见备注1
//BOOL IsPathExist(const TCHAR* csPath)
//{
//	WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 };
//	return 0 != GetFileAttributesEx(csPath, GetFileExInfoStandard, &attrs);
//}

ValueHandle g_jbase;
int g_load_count = 0;

QJS_API int _entry(ContextHandle ctx, void* user_data, int id)
{
	++g_load_count;

	//加载依赖
	if (!qjs.JsValueIsObject(g_jbase))
	{
		g_jbase = qjs.NewObjectJsValue(ctx);
		int idBase = qjs.LoadExtend(ctx, "JsExtendBase.dll", g_jbase, (void*)id);

		//File.dependents = [base]
		ValueHandle jFile = qjs.GetExtendParentObject(ctx, id);
		ValueHandle jdependents = qjs.NewArrayJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "dependents", jdependents, jFile);
		qjs.SetIndexedJsValue(ctx, 0, g_jbase, jdependents);
	}

	return 0;//加载插件
}

QJS_API void _completed(ContextHandle ctx, void* user_data, int id)
{

}

QJS_API void _unload(ContextHandle ctx, void* user_data, int id)
{
	--g_load_count;

	//卸载依赖
	if (g_load_count == 0)
	{
		//
	}
}

std::wstring resolveAndUpdateFilePath(ContextHandle ctx, const std::wstring& filename)
{
	if (IsFileExist(filename.c_str()))
	{
		std::wstring wpath;
		if (filename.find(L':') != std::wstring::npos)
		{//全路径
			wpath = filename;
		}
		else
		{
			wchar_t dir[MAX_PATH] = { 0 };
			::GetCurrentDirectoryW(MAX_PATH, dir);
			wpath = os_pathw::join(dir, filename);
		}
				
		//添加查找路径
		std::wstring wdir = os_pathw::dirname(wpath);
		JsExtendBase::Ins().AddPath(ctx, wdir.c_str(), g_jbase);

		return wpath;
	}

	if (filename.find(L':') != std::wstring::npos)
		return L"";

	ValueHandle jprocess = qjs.GetNamedJsValue(ctx, "process", g_jbase);
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

//readTextFile(filename, [function(){}], [input charset]);
QJS_API ValueHandle F_readTextFile(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
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

	bool hasInputCharset = false;
	std::string sInputCharset = "";
	if (argc > 2)
	{
		sInputCharset = qjs.JsValueToStdString(ctx, argv[2], "");
		hasInputCharset = true;
	}

	auto funcConvert = [&](std::string& inouttxt)->bool {
		if (!hasInputCharset)
		{
			char* inputCharset = NULL;
			if (StringConvert::Ins().DetectCharset(&inputCharset, inouttxt.c_str(), inouttxt.size()))
			{
				sInputCharset = inputCharset;
				std::string sDetectCharset = pystring::upper(pystring::replace(inputCharset, "-", ""));
				if (sDetectCharset != "UTF8")
				{
					if (sDetectCharset.find("UTF") == std::string::npos)
					{
						sInputCharset = "GB18030";
					}
				}
			}
		}

		if (sInputCharset != "")
		{
			char* outStr = NULL;
			size_t outLen = 0;
			StringConvert::Ins().ConvertCharset(&outStr, &outLen,
				inouttxt.c_str(), inouttxt.size(), sInputCharset.c_str(), "UTF-8", true);
			if (outStr)
			{
				inouttxt = outStr;
				//std::wstring ws = qjs.Utf8ToUnicode(ctx, line.c_str());
				StringConvert::Ins().FreeOutStr(outStr);
				return true;
			}
		}
		return false;
	};

	if (argc > 1)
	{//每行返回
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
			funcConvert(line);

			ValueHandle params[] = {
				qjs.NewStringJsValue(ctx, line.c_str()),
				qjs.NewIntJsValue(ctx, lineno)};
			ValueHandle readLineRes = qjs.CallJsFunction(ctx, lineAction, 
				params, sizeof(params)/sizeof(ValueHandle), qjs.TheJsNull());
			if (qjs.JsValueIsBool(readLineRes) &&
				qjs.JsValueToBool(ctx, readLineRes, false))//返回true则退出，返回false则继续读下一行
			{
				break;
			}
			++lineno;
		}
		inputFile.close();
		return qjs.TheJsTrue();
	}
	else
	{//否则返回总结果
		inputFile.seekg(0, std::ios::end);//文件指针指向结尾
		size_t length = inputFile.tellg();//获取文件长度
		inputFile.seekg(0, std::ios::beg);//文件指针从新指向开头
		char* buffer = new char[length + 1];
		buffer[length] = '\0';
		inputFile.read(buffer, length);//读取

		std::string sBuff = buffer;
		funcConvert(sBuff);

		delete[] buffer;
		inputFile.close();

		ValueHandle ret = qjs.NewStringJsValue(ctx, sBuff.c_str());
		return ret;
	}	
	
	inputFile.close();
	return qjs.TheJsFalse();
}

//writeTextFile(filename, text, [out charset]);
QJS_API ValueHandle F_writeTextFile(
	ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id)
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
		outCharset = qjs.JsValueToStdString(ctx, argv[2], "UTF-8");
		outCharset = pystring::upper(pystring::replace(outCharset, "-", ""));
		if (outCharset != "UTF8")
		{
			char* outText = NULL;
			size_t outLen = 0;
			bool b = StringConvert::Ins().ConvertCharset(&outText, &outLen, 
				text.c_str(), text.size(), "UTF-8", outCharset.c_str(), true);
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
