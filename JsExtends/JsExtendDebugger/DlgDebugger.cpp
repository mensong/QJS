﻿// DlgDebugger.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendDebugger.h"
#include "afxdialogex.h"
#include "DlgDebugger.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "DlgIgnoreList.h"

// DlgDebugger 对话框

IMPLEMENT_DYNAMIC(DlgDebugger, CDialogEx)

DlgDebugger::DlgDebugger(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEBUGGER, pParent)
	, m_debugMode(true)
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
	CStringArray arrBPList;
	SplitCString(strBPList, _T(",; /.*&`~@#$%^()-=_+[]\\{}|'\"<>?"), arrBPList);
	for (size_t i = 0; i < arrBPList.GetSize(); i++)
	{
		CString text = arrBPList.GetAt(i);
		if (!text.IsEmpty())
			m_breakPoints.insert(_ttoi(text.GetString()));
	}
}

void DlgDebugger::SplitCString(const CString& _cstr, const CString& _flag, CStringArray& _resultArray)
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

void DlgDebugger::EnbaleDebugOperations(BOOL enable)
{
	m_btnStep.EnableWindow(enable);
	m_btnContinue.EnableWindow(enable);
}

void DlgDebugger::DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data, const char* src)
{
	if (this->m_debugMode													//是否开启调试
		&& (
			this->m_breakPoints.find(line_no) != this->m_breakPoints.end()	//断点
			|| this->m_singleStepExecution									//单步
			|| this->m_debuggerChanged										//debugger是从其它debugger切换回来的，目的是为了
			)
		)
	{
		//设置源码
		ValueHandle backtrace = qjs.GetDebuggerBacktrace(ctx, pc);
		std::wstring funcName;
		if (qjs.JsValueIsArray(backtrace))
		{
			ValueHandle jframe = qjs.GetIndexedJsValue(ctx, 0, backtrace);

			this->m_curSrc = src;
			std::string src = pystring::replace(this->m_curSrc, "\r\n", "\n");
			src = pystring::replace(src, "\n", "\r\n");
			CString usrc = qjs.Utf8ToUnicode(ctx, src.c_str());

			//添加行号
			CString usrc_line_no;
			CStringArray lines;
			this->SplitCString(usrc, _T("\r\n"), lines);
			for (size_t i = 0; i < lines.GetSize(); i++)
			{
				CString a;
				a.Format(_T("%d%s\t\t %s\r\n"),
					(int)(i + 1), 
					(line_no == (i + 1) ? _T("->") : _T("")), 
					lines[i].GetString());
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

		this->AppendResultText(_T("(DEBUG)FunctionName:"), true);
		this->AppendResultText(funcName.c_str(), false);

		int stack = qjs.GetDebuggerStackDepth(ctx);
		this->AppendResultText(_T("(DEBUG)StackDepth:"), true);
		this->AppendResultText(std::to_wstring(stack).c_str(), false);

		ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
		this->AppendResultText(_T("(DEBUG)LocalVariables:"), true);
		this->AppendResultText(ctx, qjs.JsonStringify(ctx, localVars), false);

		ValueHandle closureVars = qjs.GetDebuggerClosureVariables(ctx, 0);
		this->AppendResultText(_T("(DEBUG)ClosureVariables:"), true);
		this->AppendResultText(ctx, qjs.JsonStringify(ctx, closureVars), false);

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
	m_debugMode = false;
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
