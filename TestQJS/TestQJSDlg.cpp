
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
#include "CDlgExtendsList.h"



std::string ReadText(const char* path)
{
	FILE* f = NULL;
	long sz;

	if (!path)
	{
		return "";
	}

	std::string sRet;

	if (fopen_s(&f, path, "rb") != 0)
	{
		return "";
	}

	do
	{
		if (fseek(f, 0, SEEK_END) < 0)
		{
			break;
		}

		sz = ftell(f);
		if (sz < 0)
		{
			break;
		}

		if (fseek(f, 0, SEEK_SET) < 0)
		{
			break;
		}

		sRet.resize((size_t)sz + 1, '\0');

		if ((size_t)fread(const_cast<char*>(sRet.c_str()), 1, (size_t)sz, f) != (size_t)sz)
		{
			sRet = "";
			break;
		}
	} while (0);

	fclose(f);

	return sRet;
}

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
	AppendResultText(ctx, sz, newLine);
	qjs.FreeJsValueToStringBuffer(ctx, sz);
}

void CTestQJSDlg::AppendResultText(ContextHandle ctx, const char* msg, bool newLine)
{
	CString text = qjs.Utf8ToUnicode(ctx, msg);
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
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_LOAD_FROM_FILE, &CTestQJSDlg::OnBnClickedBtnLoadFromFile)
	ON_BN_CLICKED(IDC_BUTTON5, &CTestQJSDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BTN_CLEAR_SRC, &CTestQJSDlg::OnBnClickedBtnClearSrc)
	ON_BN_CLICKED(IDC_BTN_LOAD_EXTEND, &CTestQJSDlg::OnBnClickedBtnLoadExtend)
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

	m_scale.SetAnchor(m_editScript.m_hWnd, CCtrlScale::AnchorLeftToWinLeft | CCtrlScale::AnchorTopToWinTop | CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(m_editResult.m_hWnd, CCtrlScale::AnchorLeftToWinLeft | CCtrlScale::AnchorBottomToWinBottom | CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(GetDlgItem(IDC_STATIC_DEBUG)->m_hWnd, CCtrlScale::AnchorLeftToWinLeft | CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(GetDlgItem(IDC_STATIC1)->m_hWnd, CCtrlScale::AnchorLeftToWinLeft | CCtrlScale::AnchorRightToWinLeft);
	m_scale.SetAnchor(GetDlgItem(IDC_STATIC2)->m_hWnd, CCtrlScale::AnchorLeftToWinLeft | CCtrlScale::AnchorRightToWinLeft);
	m_scale.SetAnchor(GetDlgItem(IDC_BTN_CLEAR_SRC)->m_hWnd, CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(GetDlgItem(IDC_BTN_LOAD_FROM_FILE)->m_hWnd, CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(GetDlgItem(IDC_BUTTON5)->m_hWnd, CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(m_editBreakpointsList.m_hWnd, CCtrlScale::AnchorLeftToWinLeft);
	m_scale.SetAnchor(m_editTestScript.m_hWnd, CCtrlScale::AnchorLeftToWinLeft | CCtrlScale::AnchorRightToWinRight);
	m_scale.Init(GetSafeHwnd());

	CDlgExtendsList::ExtInfo baseExt;
	baseExt.parentName = "";//放在globalobject里
	baseExt.extFilePath = "JsExtendBase.dll";
	m_extends.push_back(baseExt);

	m_editScript.SetWindowText(_T("var a = \"hello world\";\r\nalert(a);"));

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
	_this->AppendResultText(qjs.Utf8ToUnicode(ctx, ss.str().c_str()));
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

		int stack = qjs.GetDebuggerStackDepth(ctx);
		_this->AppendResultText(_T("(DEBUG)StackDepth:"), true);
		_this->AppendResultText(std::to_wstring(stack).c_str(), false);
		
		ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
		_this->AppendResultText(_T("(DEBUG)LocalVariables:"), true);
		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, localVars, qjs.TheJsUndefined(), qjs.TheJsUndefined()), false);

		ValueHandle closureVars = qjs.GetDebuggerClosureVariables(ctx, 0);
		_this->AppendResultText(_T("(DEBUG)ClosureVariables:"), true);
		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, closureVars, qjs.TheJsUndefined(), qjs.TheJsUndefined()), false);

		ValueHandle backtrace = qjs.GetDebuggerBacktrace(ctx, pc);
		_this->AppendResultText(_T("(DEBUG)Backtrace:"), true);
		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, backtrace, qjs.TheJsUndefined(), qjs.TheJsUndefined()), false);

		_this->m_lastBreak = true;
		_this->m_singleStepExecution = false;
		_this->m_debugNext = false;

		MSG msg;
		while (!_this->m_debugNext)
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
#pragma region 执行临时脚本
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
								ValueHandle res = qjs.RunScript(ctx, qjs.UnicodeToUtf8(ctx, script.GetString()), qjs.TheJsNull(), "");
								if (!qjs.JsValueIsException(res))
								{
									_this->AppendResultText(_T("(DEBUG)") + script + _T(":"), true);
									_this->AppendResultText(ctx, res, false);
								}
								else
								{
									ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
									_this->AppendResultText(_T("(DEBUG Exception)") + script + _T(":"), true);
									_this->AppendResultText(ctx, exception, false);
								}

							}
							continue;
						}
						break;
					}
				}
#pragma endregion

				if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
				{
					_this->m_debugNext = true;
					_this->m_onDebugMode = false;
					break;
				}

				::DispatchMessage(&msg);
				::TranslateMessage(&msg);
			}
		}
	}
	else
	{
		MSG msg;
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::DispatchMessage(&msg);
			::TranslateMessage(&msg);
		}

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

	strResult.Add(strSrc);
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
		m_debugNext = false;
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
		m_editBreakpointsList.EnableWindow(FALSE);
	}


	//ValueHandle alertFunc = qjs.NewFunction(ctx, JsAlert, 2, this);
	//bool b = qjs.SetNamedJsValue(ctx, "alert", alertFunc, qjs.TheJsNull());

	//ValueHandle printFunc = qjs.NewFunction(ctx, JsPrint, -1, this);
	//b = qjs.SetNamedJsValue(ctx, "print", printFunc, qjs.TheJsNull());

	//auto WScript = qjs.NewObjectJsValue(ctx);
	//qjs.SetNamedJsValue(ctx, "WScript", WScript, qjs.TheJsNull());
	//qjs.SetNamedJsValue(ctx, "Echo", printFunc, WScript);

	//auto console = qjs.NewObjectJsValue(ctx);
	//qjs.SetNamedJsValue(ctx, "console", console, qjs.TheJsNull());
	//qjs.SetNamedJsValue(ctx, "log", printFunc, console);

	//b = qjs.SetNamedJsValue(ctx, "telemetryLog", printFunc, qjs.TheJsNull());
	
	//加载插件
	for (size_t i = 0; i < m_extends.size(); i++)
	{
		ValueHandle parent;
		if (!m_extends[i].parentName.IsEmpty())
		{
			parent = qjs.NewObjectJsValue(ctx);
			std::string sName = qjs.UnicodeToUtf8(ctx, m_extends[i].parentName.GetString());
			if (!qjs.SetNamedJsValue(ctx, sName.c_str(), parent, qjs.GetGlobalObject(ctx)))
			{
				AppendResultText(_T("加载插件错误:") + m_extends[i].parentName + _T(" 名无效"));
				continue;
			}
		}
		else
		{
			parent = qjs.GetGlobalObject(ctx);
		}

		std::string file = CW2A(m_extends[i].extFilePath);
		int extId = qjs.LoadExtend(ctx, file.c_str(), parent, NULL);
		if (extId > 0)
		{
			CString objName;
			if (m_extends[i].parentName.IsEmpty())
				objName = _T("GlobalObject");
			else
				objName = m_extends[i].parentName;
			AppendResultText(_T("加载插件:") + m_extends[i].extFilePath + _T(" into ") + objName + _T(""));
		}
		else
		{
			AppendResultText(_T("加载插件错误:") + m_extends[i].extFilePath);
		}
	}

	CString script;
	m_editScript.GetWindowText(script);

	DWORD t1 = ::GetTickCount();
	auto result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(ctx, script), qjs.TheJsNull(), m_curFilename.c_str());
	DWORD st = ::GetTickCount() - t1;

	if (!qjs.JsValueIsException(result))
	{
		AppendResultText(_T("运行成功："), true);
		AppendResultText(ctx, result, false);
	}
	else
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		AppendResultText(_T("运行错误："), true);
		AppendResultText(ctx, exception, false);
	}
	CString timetext;
	timetext.Format(_T("耗时:%u毫秒"), st);
	AppendResultText(timetext, true);

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);

	m_btnRun.EnableWindow(TRUE);
	m_btnContinue.EnableWindow(FALSE);
	m_btnSingleStep.EnableWindow(FALSE);
	m_editScript.SetReadOnly(FALSE);
	m_editBreakpointsList.EnableWindow(TRUE);
}


void CTestQJSDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_debugNext = true;
	m_singleStepExecution = false;
}


void CTestQJSDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_debugNext = true;
	m_singleStepExecution = true;
}


void CTestQJSDlg::OnBnClickedCheck1()
{
	m_onDebugMode = m_chkIsDebug.GetCheck() == TRUE;
}


void CTestQJSDlg::OnClose()
{
	//当前正在调试时关闭窗口，则关闭调试
	m_debugNext = true;

	CDialogEx::OnClose();
}


void CTestQJSDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

std::string UnicodeToAnsi(const wchar_t* wideByteRet)
{
	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_ACP, 0, wideByteRet, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_ACP, 0, wideByteRet, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	std::string Ret_UnicodeToAnsi;
	Ret_UnicodeToAnsi.resize(nLenOfMultiCharStr + 1, 0);
	strcpy_s(&Ret_UnicodeToAnsi[0], Ret_UnicodeToAnsi.size(), pMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return Ret_UnicodeToAnsi.c_str();
}

void CTestQJSDlg::OnBnClickedBtnLoadFromFile()
{
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		L"*.js|*.js|*.txt|*.txt|All files(*.*)|*.*||");
	fdlg.m_ofn.lpstrTitle = L"加载JS文件";
	fdlg.m_ofn.lpstrInitialDir = L".\\";
	if (fdlg.DoModal() == IDOK)
	{
		CString strFileName = fdlg.GetPathName();

		std::string strContent = ReadText(CW2A(strFileName));

		RuntimeHandle rt = qjs.NewRuntime();
		ContextHandle ctx = qjs.NewContext(rt);

		CString sContent = qjs.Utf8ToUnicode(ctx, strContent.c_str());
		m_editScript.SetWindowText(sContent); // 将文件内容显示到文本编辑框中

		qjs.FreeContext(ctx);
		qjs.FreeRuntime(rt);

		m_curFilename = UnicodeToAnsi(strFileName.GetString());
	}
}


void CTestQJSDlg::OnBnClickedButton5()
{
	m_editResult.SetWindowText(_T(""));
}


void CTestQJSDlg::OnBnClickedBtnClearSrc()
{
	m_editScript.SetWindowText(_T(""));
	m_curFilename = "";
}


void CTestQJSDlg::OnBnClickedBtnLoadExtend()
{
	CDlgExtendsList dlgExtendList;
	dlgExtendList.m_extents = m_extends;
	if (dlgExtendList.DoModal() == IDOK)
	{
		m_extends = dlgExtendList.m_extents;
	}
}
