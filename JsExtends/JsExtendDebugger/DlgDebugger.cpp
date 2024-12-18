// DlgDebugger.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendDebugger.h"
#include "afxdialogex.h"
#include "DlgDebugger.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"

// DlgDebugger 对话框

IMPLEMENT_DYNAMIC(DlgDebugger, CDialogEx)

DlgDebugger::DlgDebugger(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEBUGGER, pParent)
	, m_debugMode(true)
	, m_singleStepExecution(false)
	, m_continue(false)
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

void DlgDebugger::DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data)
{
	DlgDebugger* _this = (DlgDebugger*)user_data;
	if (!_this)
		return;

	if (_this->m_debugMode													//是否开启调试
		&& (
			_this->m_breakPoints.find(line_no) != _this->m_breakPoints.end()	//断点
			|| _this->m_singleStepExecution										//单步
			)
		)
	{
		_this->ShowWindow(SW_SHOW);

		//设置源码
		ValueHandle backtrace = qjs.GetDebuggerBacktrace(ctx, pc);
		std::wstring funcName;
		if (qjs.JsValueIsArray(backtrace))
		{
			ValueHandle jframe = qjs.GetIndexedJsValue(ctx, 0, backtrace);
			ValueHandle jfilename = qjs.GetNamedJsValue(ctx, "filename", jframe);
			std::string src = qjs.JsValueToStdString(ctx, jfilename);
			src = pystring::replace(src, "\r\n", "\n");
			src = pystring::replace(src, "\n", "\r\n");
			CString usrc = qjs.Utf8ToUnicode(ctx, src.c_str());

			//添加行号
			CString usrc_line_no;
			CStringArray lines;
			_this->SplitCString(usrc, _T("\r\n"), lines);
			for (size_t i = 0; i < lines.GetSize(); i++)
			{
				CString a;
				a.Format(_T("%d%s\t\t %s\r\n"),
					(int)(i + 1), 
					(line_no == (i + 1) ? _T("->") : _T("")), 
					lines[i].GetString());
				usrc_line_no += a;
			}

			_this->m_editSrc.SetWindowText(usrc_line_no);

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
			_this->AppendResultText(_T("======================="), true);

			txt.Format(_T("(DEBUG)运行到行号(%d)，已暂停……"), line_no);

			//选中调试行
			int start = _this->m_editSrc.LineIndex(line_no - 1);
			int end = -1;
			if (-1 != start)
			{
				end = start + _this->m_editSrc.LineLength(start);
				_this->m_editSrc.SetFocus();
				_this->m_editSrc.SetSel(start, end);
			}
		}
		_this->AppendResultText(txt, true);

		_this->AppendResultText(_T("(DEBUG)FunctionName:"), true);
		_this->AppendResultText(funcName.c_str(), false);

		int stack = qjs.GetDebuggerStackDepth(ctx);
		_this->AppendResultText(_T("(DEBUG)StackDepth:"), true);
		_this->AppendResultText(std::to_wstring(stack).c_str(), false);

		ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
		_this->AppendResultText(_T("(DEBUG)LocalVariables:"), true);
		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, localVars), false);

		ValueHandle closureVars = qjs.GetDebuggerClosureVariables(ctx, 0);
		_this->AppendResultText(_T("(DEBUG)ClosureVariables:"), true);
		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, closureVars), false);

		//_this->AppendResultText(_T("(DEBUG)Backtrace:"), true);
		//_this->AppendResultText(ctx, qjs.JsonStringify(ctx, backtrace), false);

		//重置标识
		_this->m_singleStepExecution = false;
		_this->m_continue = false;

		_this->EnbaleDebugOperations(TRUE);		
		while (!_this->m_continue)
		{
			if (!DoEvent(_this, ctx))
				break;
		}
		_this->EnbaleDebugOperations(FALSE);
	}
	else
	{
		MSG msg;
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::DispatchMessage(&msg);
			::TranslateMessage(&msg);
		}

		//_this->m_lastBreak = false;
	}
}

bool DlgDebugger::DoEvent(DlgDebugger* dlg, ContextHandle ctx)
{
	bool ret = true;
	MSG msg;
	if (::PeekMessage(&msg, dlg->m_hWnd, 0, 0, PM_REMOVE))
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
	m_scale.Init(m_hWnd);

	EnbaleDebugOperations(FALSE);

	return ret;
}

BEGIN_MESSAGE_MAP(DlgDebugger, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_STEP, &DlgDebugger::OnBnClickedButtonStep)
	ON_BN_CLICKED(IDC_BUTTON_CONTINUE, &DlgDebugger::OnBnClickedButtonContinue)
	ON_WM_CLOSE()
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
	ShowWindow(SW_HIDE);
	//CDialogEx::OnClose();
}
