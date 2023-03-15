
// TestQJSDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TestQJS.h"
#include "TestQJSDlg.h"
#include "afxdialogex.h"
#include "../QJS/QJS.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestQJSDlg 对话框



CTestQJSDlg::CTestQJSDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTQJS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestQJSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editScript);
	DDX_Control(pDX, IDC_EDIT2, m_editResult);
}

void CTestQJSDlg::AppendResultText(const CString& msg, bool endl)
{
	CString txt;
	m_editResult.GetWindowText(txt);
	if (endl && !txt.IsEmpty())
		txt += "\r\n";
	txt += msg;
	m_editResult.SetWindowText(txt);

	m_editResult.LineScroll(m_editResult.GetLineCount());
}

BEGIN_MESSAGE_MAP(CTestQJSDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CTestQJSDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CTestQJSDlg 消息处理程序

BOOL CTestQJSDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestQJSDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTestQJSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

ValueHandle JsAlert(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::string msg;
	if (argc > 0)
		msg = qjs.JsValueToString(ctx, argv[0], "");

	std::string title;
	if (argc < 2 || qjs.JsValueIsUndefined(argv[1]))
		title = "QJS";
	else
		title = qjs.JsValueToString(ctx, argv[1], "QJS");

	CTestQJSDlg* _this = (CTestQJSDlg*)user_data;

	MessageBoxA(_this->m_hWnd, msg.c_str(), title.c_str(), 0);

	auto item = qjs.NewStringJsValue(ctx, msg.c_str());
	return item;
}

ValueHandle JsPrint(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::stringstream ss;
	for (int i = 0; i < argc; i++)
	{
		ss << qjs.JsValueToString(ctx, argv[i], "");
	}

	CTestQJSDlg* _this = (CTestQJSDlg*)user_data;
	_this->AppendResultText(qjs.Utf8ToUnicode(ss.str().c_str()));
	return qjs.TheJsUndefined();
}

std::string s_str;
ValueHandle JsGetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv)
{
	return qjs.NewStringJsValue(ctx, s_str.c_str());
}
ValueHandle JsSetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv)
{
	if (argc > 0)
		s_str = qjs.JsValueToString(ctx, argv[0], "");
	return qjs.CopyJsValue(ctx, this_val);
}


void CTestQJSDlg::OnBnClickedButton1()
{
	RuntimeHandle rt = qjs.NewRuntime();
	if (rt == NULL)
	{
		AppendResultText(_T("无法创建Runtime"));
		return;
	}
	ContextHandle ctx = qjs.NewContext(rt);
	if (ctx == NULL)
	{
		AppendResultText(_T("无法创建上下文"));
		return;
	}

	ValueHandle alertFunc = qjs.NewFunction(ctx, JsAlert, 2, this);
	bool b = qjs.SetNamedJsValue(ctx, "alert", alertFunc, NULL);

	ValueHandle printFunc = qjs.NewFunction(ctx, JsPrint, -1, this);
	b = qjs.SetNamedJsValue(ctx, "print", printFunc, NULL);

	//ValueHandle bv = qjs.NewBoolJsValue(ctx, true);
	//qjs.SetNamedJsValue(ctx, "bv", bv, NULL);
	//
	//ValueHandle o = qjs.NewObjectJsValue(ctx);
	//qjs.SetNamedJsValue(ctx, "bv", bv, o);
	//qjs.SetNamedJsValue(ctx, "o", o, NULL);
	//	
	//ValueHandle arr = qjs.NewArrayJsValue(ctx);
	//ValueHandle str = qjs.NewStringJsValue(ctx, "mensong");
	//qjs.SetIndexedJsValue(ctx, 10, str, arr);
	//qjs.SetNamedJsValue(ctx, "arr", arr, NULL);
	//b = qjs.JsValueIsArray(ctx, arr);
	//auto jlen = qjs.GetNamedJsValue(ctx, "toString", arr);
	//b = qjs.JsValueIsFunction(ctx, jlen);
	//auto jstrToString = qjs.CallJsFunction(ctx, jlen, NULL, 0, arr);
	//auto strToString = qjs.JsValueToString(ctx, jstrToString, "");
	//qjs.FreeJsValueToStringBuffer(ctx, strToString);

	//auto jint = qjs.NewIntJsValue(ctx, 65536);
	//auto intstr1 = qjs.JsValueToString(ctx, jint, "mensong");
	//auto intstr2 = qjs.JsValueToString(ctx, jint, "mensong");
	//auto intstr3 = qjs.JsValueToString(ctx, jint, "mensong");
	//auto intstr4 = qjs.JsValueToString(ctx, jint, "mensong");
	//auto intstr5 = qjs.JsValueToString(ctx, jint, "mensong");

	//auto getter = qjs.NewFunction(ctx, JsGetter, 0);
	//auto setter = qjs.NewFunction(ctx, JsSetter, 1);
	//b = qjs.DefineGetterSetter(ctx, o, "gs", getter, setter);
	//b = qjs.DefineGetterSetter(ctx, o, "sg", getter, setter);

	//auto g1 = qjs.GetGlobalObject(ctx);
	//auto g2 = qjs.GetGlobalObject(ctx);
	//auto g3 = qjs.GetGlobalObject(ctx);
	//auto g4 = qjs.GetGlobalObject(ctx);

	CString script;
	m_editScript.GetWindowText(script);

	auto result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(script));
	if (!qjs.JsValueIsException(result))
	{
		CString text = qjs.Utf8ToUnicode(qjs.JsValueToString(ctx, result, ""));
		AppendResultText(_T("运行成功：\n") + text);
	}
	else
	{
		ValueHandle exception = qjs.GetJsLastException(ctx);
		CString text = qjs.Utf8ToUnicode(qjs.JsValueToString(ctx, exception, ""));
		AppendResultText(_T("运行错误：\n") + text);
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}
