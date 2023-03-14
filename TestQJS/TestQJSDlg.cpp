
// TestQJSDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TestQJS.h"
#include "TestQJSDlg.h"
#include "afxdialogex.h"
#include "../QJS/QJS.h"

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

ValueHandle JsAlert(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv)
{
	std::string msg;
	if (argc > 0)
		msg = qjs.JsValueToString(ctx, argv[0], "");

	std::string title;
	if (argc < 2 || qjs.JsValueIsUndefined(argv[1]))
		title = "QJS";
	else
		title = qjs.JsValueToString(ctx, argv[1], "QJS");

	MessageBoxA(NULL, msg.c_str(), title.c_str(), 0);

	return qjs.CreateStringJsValue(ctx, msg.c_str());
}

std::string s_str;
ValueHandle JsGetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv)
{
	return qjs.CreateStringJsValue(ctx, s_str.c_str());
}
ValueHandle JsSetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv)
{
	if (argc > 0)
		s_str = qjs.JsValueToString(ctx, argv[0], "");
	return qjs.TheJsUndefined();
}


void CTestQJSDlg::OnBnClickedButton1()
{
	RuntimeHandle rt = qjs.CreateRuntime();
	if (rt == NULL)
	{
		AppendResultText(_T("无法创建Runtime"));
		return;
	}
	ContextHandle ctx = qjs.CreateContext(rt);
	if (ctx == NULL)
	{
		AppendResultText(_T("无法创建上下文"));
		return;
	}

	ValueHandle alertFunc = qjs.CreateFunction(ctx, JsAlert, 2);
	bool b = qjs.SetNamedJsValue(ctx, "alert", alertFunc, NULL);

	ValueHandle bv = qjs.CreateBoolJsValue(ctx, true);
	qjs.SetNamedJsValue(ctx, "bv", bv, NULL);
	
	ValueHandle o = qjs.CreateObjectJsValue(ctx);
	qjs.SetNamedJsValue(ctx, "bv", bv, o);
	qjs.SetNamedJsValue(ctx, "o", o, NULL);
		
	ValueHandle arr = qjs.CreateArrayJsValue(ctx);
	ValueHandle str = qjs.CreateStringJsValue(ctx, "mensong");
	qjs.SetIndexedJsValue(ctx, 0, str, arr);
	qjs.SetNamedJsValue(ctx, "arr", arr, NULL);
	b = qjs.JsValueIsArray(ctx, arr);

	auto getter = qjs.CreateFunction(ctx, JsGetter, 0);
	auto setter = qjs.CreateFunction(ctx, JsSetter, 1);
	b = qjs.DefineGetterSetter(ctx, o, "gs", getter, setter);
	b = qjs.DefineGetterSetter(ctx, o, "sg", getter, setter);

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
