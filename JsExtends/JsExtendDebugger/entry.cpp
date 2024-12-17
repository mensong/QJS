// JsExtendDebugger.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "entry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CJsExtendDebuggerApp

BEGIN_MESSAGE_MAP(CJsExtendDebuggerApp, CWinApp)
END_MESSAGE_MAP()


// CJsExtendDebuggerApp 构造

CJsExtendDebuggerApp::CJsExtendDebuggerApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CJsExtendDebuggerApp 对象

CJsExtendDebuggerApp theApp;


// CJsExtendDebuggerApp 初始化

BOOL CJsExtendDebuggerApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
