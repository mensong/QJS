﻿
// TestQJSDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TestQJS.h"
#include "TestQJSDlg.h"
#include "afxdialogex.h"
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
	DDX_Control(pDX, IDC_EDIT_BREAKPOITS, m_editBreakpointsList);
	DDX_Control(pDX, IDC_CHECK1, m_chkIsDebug);
	DDX_Control(pDX, IDC_EDIT3, m_editTestScript);
	DDX_Control(pDX, IDC_BUTTON1, m_btnRun);
	DDX_Control(pDX, IDC_BUTTON2, m_btnContinue);
	DDX_Control(pDX, IDC_BUTTON3, m_btnSingleStep);
}

void CTestQJSDlg::AppendResultText(const wchar_t* msg, bool newLine)
{
	CString txt;
	m_editResult.GetWindowText(txt);
	if (newLine && !txt.IsEmpty())
		txt += _T("\r\n");
	txt += msg;
	m_editResult.SetWindowText(txt);

	m_editResult.LineScroll(m_editResult.GetLineCount());
}

void CTestQJSDlg::AppendResultText(ContextHandle ctx, const ValueHandle& msg, bool newLine)
{
	const char* sz = qjs.JsValueToString(ctx, msg);
	AppendResultText(sz, newLine);
	qjs.FreeJsValueToStringBuffer(ctx, sz);
}

void CTestQJSDlg::AppendResultText(const char* msg, bool newLine)
{
	CString text = qjs.Utf8ToUnicode(msg);
	AppendResultText(text, newLine);
}

BEGIN_MESSAGE_MAP(CTestQJSDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CTestQJSDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTestQJSDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CTestQJSDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_CHECK1, &CTestQJSDlg::OnBnClickedCheck1)
	ON_WM_CLOSE()
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
	m_editScript.SetLimitText(UINT_MAX);
	m_btnRun.EnableWindow(TRUE);
	m_btnContinue.EnableWindow(FALSE);
	m_btnSingleStep.EnableWindow(FALSE);

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

BOOL CTestQJSDlg::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}

ValueHandle JsAlert(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::string msg;
	if (argc > 0)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[0]);
		if (sz)
			msg = sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	std::string title;
	if (argc < 2 || qjs.JsValueIsUndefined(argv[1]))
		title = "QJS";
	else
	{
		const char* sz = qjs.JsValueToString(ctx, argv[1]);
		if (sz)
			title = sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

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
		const char* sz = qjs.JsValueToString(ctx, argv[i]);
		if (sz)
			ss << sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	CTestQJSDlg* _this = (CTestQJSDlg*)user_data;
	_this->AppendResultText(qjs.Utf8ToUnicode(ss.str().c_str()));
	return qjs.NewStringJsValue(ctx, ss.str().c_str());
}

std::string s_str = "test_getter_setter_value";
ValueHandle JsGetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	return qjs.NewStringJsValue(ctx, s_str.c_str());
}
ValueHandle JsSetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc > 0)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[0]);
		if (sz)
			s_str = std::string("test_getter_setter_value:") + sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}
	return this_val;
}

ValueHandle JsTestThrow(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	auto throwWhat = qjs.NewStringJsValue(ctx, "throw test");
	auto ex = qjs.NewThrowJsValue(ctx, throwWhat);
	qjs.FreeValueHandle(ctx, throwWhat);
	return ex;
}

void CTestQJSDlg::DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data)
{
	//MSG msg;
	//::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	//::DispatchMessage(&msg);
	//::TranslateMessage(&msg);


	CTestQJSDlg* _this = (CTestQJSDlg*)user_data;
	if (!_this)
		return;

	if (_this->m_onDebugMode													//是否开启调试
		&& (
			_this->m_breakPoints.find(line_no) != _this->m_breakPoints.end()	//断点
			|| (_this->m_lastBreak && _this->m_singleStepExecution)				//单步
			)
		)
	{
		CString txt;
		if (line_no == 0)
			txt = _T("(DEBUG)入口点，已暂停……");
		else
		{
			txt.Format(_T("(DEBUG)运行到行号(%d)，已暂停……"), line_no);
			int start = _this->m_editScript.LineIndex(line_no - 1);
			int end = -1;
			if (-1 != start)
			{
				end = start + _this->m_editScript.LineLength(start);
				_this->m_editScript.SetFocus();
				_this->m_editScript.SetSel(start, end);
			}
		}
		_this->AppendResultText(txt, true);

		//int stack = qjs.GetDebuggerStackDepth(ctx);
		ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
		_this->AppendResultText(_T("(DEBUG)局部变量:"), true);
		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, localVars), false);
		qjs.FreeValueHandle(ctx, localVars);

		_this->m_lastBreak = true;
		_this->m_singleStepExecution = false;
		_this->m_debugContinue = false;
		while (!_this->m_debugContinue)
		{
			MSG msg;
			while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{


				//处理临时脚本
				if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)
				{
					switch (GetFocus()->GetDlgCtrlID())
					{
					case IDC_EDIT3:
						if (ctx)
						{
							CString script;
							_this->m_editTestScript.GetWindowText(script);
							_this->m_editTestScript.SetSel(0, script.GetLength());
							if (!script.IsEmpty())
							{
								ValueHandle res = qjs.RunScript(ctx, qjs.UnicodeToUtf8(script.GetString()), NULL);
								if (!qjs.JsValueIsException(res))
								{
									_this->AppendResultText(_T("(DEBUG)") + script + _T(":"), true);
									_this->AppendResultText(ctx, res, false);
								}
								else
								{
									ValueHandle exception = qjs.GetJsLastException(ctx);
									_this->AppendResultText(_T("(DEBUG Exception)") + script + _T(":"), true);
									_this->AppendResultText(ctx, exception, false);
									qjs.FreeValueHandle(ctx, exception);
								}
								qjs.FreeValueHandle(ctx, res);

							}
							continue;
						}
						break;
					}
				}
				//结束处理临时脚本

				::DispatchMessage(&msg);
				::TranslateMessage(&msg);
			}
		}
	}
	else
	{
		_this->m_lastBreak = false;
	}

}

void SplitCString(const CString& _cstr, const CString& _flag, CStringArray& _resultArray)
{
	CString strSrc(_cstr);

	CStringArray& strResult = _resultArray;
	CString strLeft = _T("");

	int nPos = strSrc.FindOneOf(_flag);
	while (0 <= nPos)
	{
		strLeft = strSrc.Left(nPos);
		if (!strLeft.IsEmpty())
		{
			strResult.Add(strLeft);
		}
		strSrc = strSrc.Right(strSrc.GetLength() - nPos - 1);
		nPos = strSrc.FindOneOf(_flag);
	}

	if (!strSrc.IsEmpty()) {
		strResult.Add(strSrc);
	}
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

	m_btnRun.EnableWindow(FALSE);

	m_onDebugMode = m_chkIsDebug.GetCheck() == TRUE;
	if (m_onDebugMode)
	{
		m_debugContinue = false;
		m_singleStepExecution = false;
		m_lastBreak = false;
		qjs.SetDebuggerMode(ctx, true);
		qjs.SetDebuggerLineCallback(ctx, DebuggerLineCallback, this);

		//生成断点
		m_breakPoints.clear();
		// 入口点断点
		m_breakPoints.insert(0);
		CString strBPList;
		m_editBreakpointsList.GetWindowText(strBPList);
		CStringArray arrBPList;
		SplitCString(strBPList, _T(",; /.*&`~@#$%^()-=_+[]\\{}|'\"<>?"), arrBPList);
		for (size_t i = 0; i < arrBPList.GetSize(); i++)
		{
			CString text = arrBPList.GetAt(i);
			if (!text.IsEmpty())
				m_breakPoints.insert(_ttoi(text.GetString()));
		}

		m_btnContinue.EnableWindow(TRUE);
		m_btnSingleStep.EnableWindow(TRUE);
		m_editScript.SetReadOnly(TRUE);
	}


	ValueHandle alertFunc = qjs.NewFunction(ctx, JsAlert, 2, this);
	bool b = qjs.SetNamedJsValue(ctx, "alert", alertFunc, NULL);
	qjs.FreeValueHandle(ctx, alertFunc);

	ValueHandle printFunc = qjs.NewFunction(ctx, JsPrint, -1, this);
	b = qjs.SetNamedJsValue(ctx, "print", printFunc, NULL);	

	auto WScript = qjs.NewObjectJsValue(ctx);
	qjs.SetNamedJsValue(ctx, "WScript", WScript, NULL);
	qjs.SetNamedJsValue(ctx, "Echo", printFunc, WScript);
	qjs.FreeValueHandle(ctx, WScript);

	auto console = qjs.NewObjectJsValue(ctx);
	qjs.SetNamedJsValue(ctx, "console", console, NULL);
	qjs.SetNamedJsValue(ctx, "log", printFunc, console);
	qjs.FreeValueHandle(ctx, console);

	b = qjs.SetNamedJsValue(ctx, "telemetryLog", printFunc, NULL);
	qjs.FreeValueHandle(ctx, printFunc);

#if 0

	{
		auto argv = qjs.NewStringJsValue(ctx, "mensong,");
		ValueHandle argvs[] = { argv, argv, argv };
		auto ret1 = qjs.CallJsFunction(ctx, printFunc, argvs, 3, NULL);
		qjs.FreeValueHandle(ctx, ret1);
		qjs.FreeValueHandle(ctx, argv);
	}

	{
		ValueHandle o = qjs.NewIntJsValue(ctx, 123);
		qjs.SetObjectUserData(o, (void*)123);
		void* pu = qjs.GetObjectUserData(o);
		int iu = (int)pu;
	}

	{
		ValueHandle bv = qjs.NewBoolJsValue(ctx, true);
		qjs.SetNamedJsValue(ctx, "bv", bv, NULL);

		ValueHandle o = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "bv", bv, o);
		qjs.SetNamedJsValue(ctx, "o", o, NULL);

		qjs.FreeValueHandle(ctx, bv);
		qjs.FreeValueHandle(ctx, o);
	}

	{
		ValueHandle arr = qjs.NewArrayJsValue(ctx);
		ValueHandle str = qjs.NewStringJsValue(ctx, "mensong");
		qjs.SetIndexedJsValue(ctx, 10, str, arr);//设置id 10的元素为字符串mensong
		qjs.SetIndexedJsValue(ctx, 2, str, arr);
		qjs.DeleteIndexedJsValue(ctx, 2, arr);
		qjs.SetNamedJsValue(ctx, "arr", arr, NULL);

		auto item10 = qjs.GetIndexedJsValue(ctx, 10, arr);
		const char* sz = qjs.JsValueToString(ctx, item10);
		qjs.FreeValueHandle(ctx, item10);
		qjs.FreeJsValueToStringBuffer(ctx, sz);

		auto item5 = qjs.GetIndexedJsValue(ctx, 5, arr);
		const char* sz5 = qjs.JsValueToString(ctx, item5);
		qjs.FreeValueHandle(ctx, item5);
		qjs.FreeJsValueToStringBuffer(ctx, sz5);

		auto arrLen = qjs.GetLength(ctx, arr);
		for (size_t i = 0; i < arrLen; i++)
		{
			ValueHandle v = qjs.GetIndexedJsValue(ctx, i, arr);
			//...
			qjs.FreeValueHandle(ctx, v);
		}

		bool b = qjs.JsValueIsArray(ctx, arr);
		auto jToString = qjs.GetNamedJsValue(ctx, "toString", arr);
		b = qjs.JsValueIsFunction(ctx, jToString);
		if (b)
		{
			auto jstrToString = qjs.CallJsFunction(ctx, jToString, NULL, 0, arr);
			auto strToString = qjs.JsValueToString(ctx, jstrToString);
			qjs.FreeValueHandle(ctx, jstrToString);
			qjs.FreeJsValueToStringBuffer(ctx, strToString);
		}
		qjs.FreeValueHandle(ctx, jToString);

		ValueHandle jlen = qjs.GetNamedJsValue(ctx, "length", arr);
		arrLen = qjs.JsValueToInt(ctx, jlen, 0);
		qjs.FreeValueHandle(ctx, jlen);

		qjs.FreeValueHandle(ctx, arr);
		qjs.FreeValueHandle(ctx, str);
	}

	{
		auto jint = qjs.NewIntJsValue(ctx, 65536);
		auto intstr1 = qjs.JsValueToString(ctx, jint);
		qjs.FreeValueHandle(ctx, jint);
		qjs.FreeJsValueToStringBuffer(ctx, intstr1);
	}

	{
		//ValueHandle test_getter_setter = qjs.NewObjectJsValue(ctx);
		//qjs.SetNamedJsValue(ctx, "test_getter_setter", test_getter_setter, NULL);
		//auto getter = qjs.NewFunction(ctx, JsGetter, 0, NULL);
		//auto setter = qjs.NewFunction(ctx, JsSetter, 1, NULL);
		//qjs.DefineGetterSetter(ctx, test_getter_setter, "gs", getter, setter);
		//qjs.FreeValueHandle(ctx, getter);
		//qjs.FreeValueHandle(ctx, setter);
		//qjs.FreeValueHandle(ctx, test_getter_setter);

		//auto getter2 = qjs.NewFunction(ctx, JsGetter, 0, NULL);
		//auto setter2 = qjs.NewFunction(ctx, JsSetter, 1, NULL);
		//auto globalObj = qjs.GetGlobalObject(ctx);
		//bool b = qjs.DefineGetterSetter(ctx, globalObj, "gs2", getter2, setter2);
		//qjs.FreeValueHandle(ctx, getter2);
		//qjs.FreeValueHandle(ctx, setter2);
		//qjs.FreeValueHandle(ctx, globalObj);
	}

	{
		auto g1 = qjs.GetGlobalObject(ctx);
		auto g2 = qjs.GetGlobalObject(ctx);
		auto g3 = qjs.GetGlobalObject(ctx);
		auto g4 = qjs.GetGlobalObject(ctx);
		qjs.FreeValueHandle(ctx, g1);
		qjs.FreeValueHandle(ctx, g2);
		qjs.FreeValueHandle(ctx, g3);
		qjs.FreeValueHandle(ctx, g4);
	}

	{
		auto date = qjs.NewDateJsValue(ctx, 1679044555000);
		uint64_t ts = qjs.JsValueToTimestamp(ctx, date);
		qjs.SetNamedJsValue(ctx, "mydate", date, NULL);
		qjs.FreeValueHandle(ctx, date); 
	}

	{
		auto o = qjs.NewObjectJsValue(ctx);
		auto prop = qjs.NewIntJsValue(ctx, 123);
		qjs.SetNamedJsValue(ctx, "a", prop, o);

		auto jstr1 = qjs.JsonStringify(ctx, o);
		qjs.FreeValueHandle(ctx, o);
		auto ostr1 = qjs.JsValueToString(ctx, jstr1);
		qjs.FreeValueHandle(ctx, jstr1);
		auto o2 = qjs.JsonParse(ctx, ostr1);
		qjs.FreeJsValueToStringBuffer(ctx, ostr1);
		auto jstr2 = qjs.JsonStringify(ctx, o2);
		auto ostr2 = qjs.JsValueToString(ctx, jstr2);
		qjs.FreeValueHandle(ctx, jstr2);
		qjs.FreeValueHandle(ctx, o2);
		qjs.FreeJsValueToStringBuffer(ctx, ostr2);

		auto jsonobj = qjs.JsonParse(ctx, "[{\"a\":123}, {\"a\":456}]");
		auto jsonobjstr = qjs.JsonStringify(ctx, jsonobj);
		const char* str = qjs.JsValueToString(ctx, jsonobjstr);
		qjs.FreeJsValueToStringBuffer(ctx, str);
		qjs.FreeValueHandle(ctx, jsonobj);
		qjs.FreeValueHandle(ctx, jsonobjstr);
	}

	{
		auto arrLenTest = qjs.RunScript(ctx, "[{\"a\":123}, {\"a\":456}]", NULL);
		auto arrTestLen = qjs.GetLength(ctx, arrLenTest);
		auto v0 = qjs.GetIndexedJsValue(ctx, 0, arrLenTest);
		auto a0 = qjs.GetNamedJsValue(ctx, "a", v0);
		int ia0 = qjs.JsValueToInt(ctx, a0, 0);//ia0==123
		qjs.FreeValueHandle(ctx, arrLenTest);
		qjs.FreeValueHandle(ctx, v0);
		qjs.FreeValueHandle(ctx, a0);
	}
	
	{
		const uint32_t qjsc_qjsc_test_size = 79;
		const uint8_t qjsc_qjsc_test[79] = {
		 0x02, 0x03, 0x0a, 0x61, 0x6c, 0x65, 0x72, 0x74,
		 0x2a, 0x49, 0x20, 0x61, 0x6d, 0x20, 0x63, 0x6f,
		 0x6d, 0x69, 0x6e, 0x67, 0x20, 0x66, 0x72, 0x6f,
		 0x6d, 0x20, 0x62, 0x69, 0x6e, 0x2e, 0x18, 0x71,
		 0x6a, 0x73, 0x63, 0x2d, 0x74, 0x65, 0x73, 0x74,
		 0x2e, 0x6a, 0x73, 0x0e, 0x00, 0x06, 0x00, 0xa6,
		 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x0d,
		 0x01, 0xa8, 0x01, 0x00, 0x00, 0x00, 0x38, 0xe3,
		 0x00, 0x00, 0x00, 0x04, 0xe4, 0x00, 0x00, 0x00,
		 0xf0, 0xce, 0x28, 0xca, 0x03, 0x01, 0x00,
		};

		ValueHandle binRes = qjs.RunBinary(ctx, qjsc_qjsc_test, qjsc_qjsc_test_size);
		if (qjs.JsValueIsException(binRes))
		{
			ValueHandle exception = qjs.GetJsLastException(ctx);
			AppendResultText(_T("运行错误："), true);
			AppendResultText(ctx, exception, false);
			qjs.FreeValueHandle(ctx, exception);
		}
		else
		{
			AppendResultText(ctx, binRes, false);
		}
		qjs.FreeValueHandle(ctx, binRes);
	}

	{
		auto testThrowFunc = qjs.NewFunction(ctx, JsTestThrow, 0, NULL);
		qjs.SetNamedJsValue(ctx, "TestThrow", testThrowFunc, NULL);
		qjs.FreeValueHandle(ctx, testThrowFunc);
		qjs.RunScript(ctx, "try{ TestThrow(); } catch (e){ print(e) }", NULL);
	}

	{
		auto jNull = qjs.TheJsNull();
		qjs.FreeValueHandle(ctx, jNull);
		auto jException = qjs.TheJsException();
		qjs.FreeValueHandle(ctx, jException);
		auto jFalse = qjs.TheJsFalse();
		qjs.FreeValueHandle(ctx, jFalse);
		auto jTrue = qjs.TheJsTrue();
		qjs.FreeValueHandle(ctx, jTrue);
		auto jUndefined = qjs.TheJsUndefined();
		qjs.FreeValueHandle(ctx, jUndefined);
	}

#endif


	CString script;
	m_editScript.GetWindowText(script);

	DWORD t1 = ::GetTickCount();
	auto result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(script), NULL);
	DWORD st = ::GetTickCount() - t1;

	if (!qjs.JsValueIsException(result))
	{
		AppendResultText(_T("运行成功："), true);
		AppendResultText(ctx, result, false);
	}
	else
	{
		ValueHandle exception = qjs.GetJsLastException(ctx);
		AppendResultText(_T("运行错误："), true);
		AppendResultText(ctx, exception, false);
		qjs.FreeValueHandle(ctx, exception);
	}
	CString timetext;
	timetext.Format(_T("耗时:%u毫秒"), st);
	AppendResultText(timetext, true);

	qjs.FreeValueHandle(ctx, result);

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);

	m_btnRun.EnableWindow(TRUE);
	m_btnContinue.EnableWindow(FALSE);
	m_btnSingleStep.EnableWindow(FALSE);
	m_editScript.SetReadOnly(FALSE);
}


void CTestQJSDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_debugContinue = true;
	m_singleStepExecution = false;
}


void CTestQJSDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_debugContinue = true;
	m_singleStepExecution = true;
}


void CTestQJSDlg::OnBnClickedCheck1()
{
	m_onDebugMode = m_chkIsDebug.GetCheck() == TRUE;
}


void CTestQJSDlg::OnClose()
{
	//当前正在调试时关闭窗口，则关闭调试
	m_debugContinue = true;

	CDialogEx::OnClose();
}
