// DlgDebugger.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendDebugger.h"
#include "afxdialogex.h"
#include "DlgDebugger.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "DlgIgnoreList.h"
#include "DlgDebugOptions.h"

bool DlgDebugger::ms_debugShowLocalVars = true;
bool DlgDebugger::ms_debugShowFuncName = false;
bool DlgDebugger::ms_debugShowStackDepth = false;
std::string DlgDebugger::ms_debugAutoScript;

// DlgDebugger 对话框

IMPLEMENT_DYNAMIC(DlgDebugger, CDialogEx)

DlgDebugger::DlgDebugger(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEBUGGER, pParent)
	, m_singleStepExecution(false)
	, m_continue(false)
	, m_ignoredSrc(NULL)
	, m_debuggerChanged(false)
{
	m_breakPoints.insert(0);
}

DlgDebugger::~DlgDebugger()
{
}


void DlgDebugger::AppendResultText(const wchar_t* msg, bool newLine)
{
	CString txt;
	m_editOutput.GetWindowText(txt);
	if (newLine && !txt.IsEmpty())
		txt += _T("\r\n");
	txt += msg;
	m_editOutput.SetWindowText(txt);

	m_editOutput.LineScroll(m_editOutput.GetLineCount());
}

void DlgDebugger::AppendResultText(ContextHandle ctx, const ValueHandle& msg, bool newLine)
{
	const char* sz = qjs.JsValueToString(ctx, msg);
	AppendResultText(ctx, sz, newLine);
	qjs.FreeJsValueToStringBuffer(ctx, sz);
}

void DlgDebugger::AppendResultText(ContextHandle ctx, const char* msg, bool newLine)
{
	CString text = qjs.Utf8ToUnicode(ctx, msg);
	text.Replace(_T("\r\n"), _T("\n"));
	text.Replace(_T("\n"), _T("\r\n"));
	AppendResultText(text, newLine);
}

void DlgDebugger::OnDebuggerChanged()
{
	m_debuggerChanged = true;
}

void DlgDebugger::RefreshBreakPoints()
{
	m_breakPoints.clear();
	// 入口点断点
	m_breakPoints.insert(0);
	CString strBPList;
	m_editBreakpoints.GetWindowText(strBPList);
	std::vector<std::wstring> arrBPList;
	pywstring::split(strBPList.GetString(), arrBPList, L",");
	for (size_t i = 0; i < arrBPList.size(); i++)
	{
		if (!arrBPList[i].empty())
			m_breakPoints.insert(_ttoi(arrBPList[i].c_str()));
	}
}

void DlgDebugger::EnbaleDebugOperations(BOOL enable)
{
	m_btnStep.EnableWindow(enable);
	m_btnContinue.EnableWindow(enable);
}

void DlgDebugger::DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data, const char* src)
{
	if (this->m_breakPoints.find(line_no) != this->m_breakPoints.end()	//断点
		|| this->m_singleStepExecution									//单步
		|| this->m_debuggerChanged										//debugger是从其它debugger切换回来的，目的是为了
	)
	{
		//设置源码
		ValueHandle backtrace = qjs.GetDebuggerBacktrace(ctx, pc);
		std::wstring funcName;
		if (qjs.JsValueIsArray(backtrace))
		{
			ValueHandle jframe = qjs.GetIndexedJsValue(ctx, 0, backtrace);

			this->m_curSrc = src;
			std::wstring usrc = qjs.Utf8ToUnicode(ctx, src);

			//添加行号
			CString usrc_line_no;
			std::vector<std::wstring> lines;
			pywstring::splitlines(usrc, lines);
			for (size_t i = 0; i < lines.size(); i++)
			{
				CString a;
				a.Format(_T("%d%s\t\t %s\r\n"),
					(int)(i + 1), 
					(line_no == (i + 1) ? _T("->") : _T("")), 
					lines[i].c_str());
				usrc_line_no += a;
			}

			this->m_editSrc.SetWindowText(usrc_line_no);

			ValueHandle jname = qjs.GetNamedJsValue(ctx, "name", jframe);
			funcName = qjs.Utf8ToUnicode(ctx, qjs.JsValueToString(ctx, jname));
		}

		CString txt;
		if (line_no == 0)
		{
			txt = _T("(DEBUG)入口点，已暂停……");
		}
		else
		{
			this->AppendResultText(_T("======================="), true);

			txt.Format(_T("(DEBUG)运行到行号(%d)，已暂停……"), line_no);

			//选中调试行
			int start = this->m_editSrc.LineIndex(line_no - 1);
			int end = -1;
			if (-1 != start)
			{
				end = start + this->m_editSrc.LineLength(start);
				this->m_editSrc.SetFocus();
				this->m_editSrc.SetSel(start, end);
			}
		}
		this->AppendResultText(txt, true);

		if (this->ms_debugShowFuncName)
		{
			this->AppendResultText(_T("(DEBUG)FunctionName:"), true);
			this->AppendResultText(funcName.c_str(), false);
		}

		if (this->ms_debugShowStackDepth)
		{
			int stack = qjs.GetDebuggerStackDepth(ctx);
			this->AppendResultText(_T("(DEBUG)StackDepth:"), true);
			this->AppendResultText(std::to_wstring(stack).c_str(), false);
		}

		if (this->ms_debugShowLocalVars)
		{
#if 1
			ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
			this->AppendResultText(_T("(DEBUG)LocalVariables:"), true);
			ValueHandle gap = qjs.NewIntJsValue(ctx, 4);
			this->AppendResultText(ctx, qjs.JsonStringify(ctx, localVars, qjs.TheJsUndefined(), gap), false);
			qjs.FreeValueHandle(&gap);
#else
			ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
			ValueHandle jkeys = qjs.GetObjectPropertyKeys(ctx, localVars, false, true);
			int64_t keyCount = qjs.GetLength(ctx, jkeys);
			for (size_t i = 0; i < keyCount; i++)
			{
				ValueHandle jkey = qjs.GetIndexedJsValue(ctx, i, jkeys);
				std::string key = qjs.JsValueToStdString(ctx, jkey);
				ValueHandle jval = qjs.GetNamedJsValue(ctx, key.c_str(), localVars);
				std::string val = qjs.JsValueToStdString(ctx, jval);
				CString sKey = qjs.Utf8ToUnicode(ctx, key.c_str());
				CString sVal = qjs.Utf8ToUnicode(ctx, val.c_str());
				this->AppendResultText(_T("(DEBUG)") + sKey + _T("="), true);
				this->AppendResultText(sVal, false);
			}
#endif
		}

		if (!pystring::iscempty(ms_debugAutoScript))
		{
			ValueHandle res = qjs.RunScript(ctx, ms_debugAutoScript.c_str(), qjs.TheJsNull(), "");
			if (!qjs.JsValueIsException(res))
			{
				this->AppendResultText(_T("(DEBUG)AutoScript:"), true);
				this->AppendResultText(ctx, res, false);
			}
			else
			{
				ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
				this->AppendResultText(_T("(DEBUG Exception)AutoScript:"), true);
				this->AppendResultText(ctx, exception, false);
			}
		}

		//ValueHandle closureVars = qjs.GetDebuggerClosureVariables(ctx, 0);
		//this->AppendResultText(_T("(DEBUG)ClosureVariables:"), true);
		//this->AppendResultText(ctx, qjs.JsonStringify(ctx, closureVars), false);

		//this->AppendResultText(_T("(DEBUG)Backtrace:"), true);
		//this->AppendResultText(ctx, qjs.JsonStringify(ctx, backtrace), false);

		this->m_singleStepExecution = false;
		this->m_continue = false;
		this->m_debuggerChanged = false;

		this->EnbaleDebugOperations(TRUE);		
		while (!this->m_continue)
		{
			if (!DoEvent(this, ctx))
				break;
		}
		this->EnbaleDebugOperations(FALSE);
	}
	else
	{
		MSG msg;
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::DispatchMessage(&msg);
			::TranslateMessage(&msg);
		}

		//this->m_lastBreak = false;
	}
}

bool DlgDebugger::DoEvent(DlgDebugger* dlg, ContextHandle ctx)
{
	bool ret = true;
	MSG msg;
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
					dlg->m_editDebuggerExpr.GetWindowText(script);
					dlg->m_editDebuggerExpr.SetSel(0, script.GetLength());
					if (!script.IsEmpty())
					{
						ValueHandle res = qjs.RunScript(ctx, qjs.UnicodeToUtf8(ctx, script.GetString()), qjs.TheJsNull(), "");
						if (!qjs.JsValueIsException(res))
						{
							dlg->AppendResultText(_T("(DEBUG)") + script + _T(":"), true);
							dlg->AppendResultText(ctx, res, false);
						}
						else
						{
							ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
							dlg->AppendResultText(_T("(DEBUG Exception)") + script + _T(":"), true);
							dlg->AppendResultText(ctx, exception, false);
						}

					}
				}
				break;
			}
		}
#pragma endregion

#pragma region 快捷键
		else if (msg.message == WM_KEYDOWN)
		{
			if (msg.wParam == VK_F8)
				dlg->OnBnClickedButtonStep();
			else if (msg.wParam == VK_F5)
				dlg->OnBnClickedButtonContinue();

		}
#pragma endregion


		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
		{
			dlg->QuitDebug();
			ret = false;
		}

		::DispatchMessage(&msg);
		::TranslateMessage(&msg);
	}

	return true;
}

void DlgDebugger::QuitDebug()
{
	m_continue = true;
	m_editSrc.SetWindowText(_T(""));
	m_editOutput.SetWindowText(_T(""));
	m_chkIgnoreThisSrc.SetCheck(FALSE);

	ShowWindow(SW_HIDE);
}

void DlgDebugger::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SRC, m_editSrc);
	DDX_Control(pDX, IDC_EDIT_BREAKPOITS, m_editBreakpoints);
	DDX_Control(pDX, IDC_BUTTON_STEP, m_btnStep);
	DDX_Control(pDX, IDC_BUTTON_CONTINUE, m_btnContinue);
	DDX_Control(pDX, IDC_EDIT_DEBUGGER_EXPR, m_editDebuggerExpr);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_editOutput);
	DDX_Control(pDX, IDC_CHECK_IGNORE_THIS, m_chkIgnoreThisSrc);
	DDX_Control(pDX, IDC_BUTTON_SHOW_IGNORE, m_btnShowIngoreList);
	DDX_Control(pDX, IDC_BUTTON_DEBUG_OPTIONS, m_btnDebugOptions);
}

BOOL DlgDebugger::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}

BOOL DlgDebugger::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	m_scale.SetAnchor(m_editSrc.m_hWnd, 
		CCtrlScale::AnchorTopToWinTop | 
		CCtrlScale::AnchorLeftToWinLeft | 
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.SetAnchor(m_editOutput.m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinRight | 
		CCtrlScale::AnchorBottomToWinBottom
	);
	m_scale.SetAnchor(m_btnStep.m_hWnd,
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.SetAnchor(m_btnContinue.m_hWnd,
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.SetAnchor(m_editBreakpoints.m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.SetAnchor(m_editDebuggerExpr.m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.SetAnchor(GetDlgItem(IDC_STATIC_BREAKPOINTS)->m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinLeft
	);
	m_scale.SetAnchor(GetDlgItem(IDC_STATIC_EXPR)->m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinLeft
	);
	m_scale.SetAnchor(m_chkIgnoreThisSrc.m_hWnd,
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight |
		CCtrlScale::AnchorTopToWinTop |
		CCtrlScale::AnchorBottomToWinTop
	);
	m_scale.SetAnchor(m_btnShowIngoreList.m_hWnd,
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight |
		CCtrlScale::AnchorTopToWinTop |
		CCtrlScale::AnchorBottomToWinTop
	);
	m_scale.SetAnchor(m_btnDebugOptions.m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinLeft |
		CCtrlScale::AnchorTopToWinBottom |
		CCtrlScale::AnchorBottomToWinBottom
	);
	m_scale.Init(m_hWnd);

	EnbaleDebugOperations(FALSE);

	return ret;
}

BEGIN_MESSAGE_MAP(DlgDebugger, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_STEP, &DlgDebugger::OnBnClickedButtonStep)
	ON_BN_CLICKED(IDC_BUTTON_CONTINUE, &DlgDebugger::OnBnClickedButtonContinue)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SHOW_IGNORE, &DlgDebugger::OnBnClickedButtonShowIgnore)
	ON_BN_CLICKED(IDC_CHECK_IGNORE_THIS, &DlgDebugger::OnBnClickedCheckIgnoreThis)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG_OPTIONS, &DlgDebugger::OnBnClickedButtonDebugOptions)
END_MESSAGE_MAP()


// DlgDebugger 消息处理程序

void DlgDebugger::OnBnClickedButtonStep()
{
	RefreshBreakPoints();

	m_singleStepExecution = true;
	m_continue = true;
}


void DlgDebugger::OnBnClickedButtonContinue()
{
	RefreshBreakPoints();

	m_singleStepExecution = false;
	m_continue = true;
}


void DlgDebugger::OnClose()
{
	QuitDebug();
	//CDialogEx::OnClose();
}


void DlgDebugger::OnBnClickedButtonShowIgnore()
{
	if (!m_ignoredSrc)
		return;

	DlgIgnoreList dlg(m_ignoredSrc);
	dlg.DoModal();

	if (m_ignoredSrc->find(m_curSrc) == m_ignoredSrc->end())
		m_chkIgnoreThisSrc.SetCheck(FALSE);
}


void DlgDebugger::OnBnClickedCheckIgnoreThis()
{
	if (!m_ignoredSrc)
		return;

	if (m_chkIgnoreThisSrc.GetCheck())
		m_ignoredSrc->insert(m_curSrc);
	else
		m_ignoredSrc->erase(m_curSrc);
}


void DlgDebugger::OnBnClickedButtonDebugOptions()
{
	DlgDebugOptions dlg(this);
	dlg.DoModal();
}
