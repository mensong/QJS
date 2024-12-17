// DlgDebugger.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendDebugger.h"
#include "afxdialogex.h"
#include "DlgDebugger.h"


// DlgDebugger 对话框

IMPLEMENT_DYNAMIC(DlgDebugger, CDialogEx)

DlgDebugger::DlgDebugger(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEBUGGER, pParent)
{

}

DlgDebugger::~DlgDebugger()
{
}

void DlgDebugger::DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data)
{
	DlgDebugger* _this = (DlgDebugger*)user_data;
	if (!_this)
		return;
//
//	if (_this->m_onDebugMode													//是否开启调试
//		&& (
//			_this->m_breakPoints.find(line_no) != _this->m_breakPoints.end()	//断点
//			|| (_this->m_lastBreak && _this->m_singleStepExecution)				//单步
//			)
//		)
//	{
//		CString txt;
//		if (line_no == 0)
//			txt = _T("(DEBUG)入口点，已暂停……");
//		else
//		{
//			txt.Format(_T("(DEBUG)运行到行号(%d)，已暂停……"), line_no);
//			int start = _this->m_editScript.LineIndex(line_no - 1);
//			int end = -1;
//			if (-1 != start)
//			{
//				end = start + _this->m_editScript.LineLength(start);
//				_this->m_editScript.SetFocus();
//				_this->m_editScript.SetSel(start, end);
//			}
//		}
//		_this->AppendResultText(txt, true);
//
//		int stack = qjs.GetDebuggerStackDepth(ctx);
//		_this->AppendResultText(_T("(DEBUG)StackDepth:"), true);
//		_this->AppendResultText(std::to_wstring(stack).c_str(), false);
//
//		ValueHandle localVars = qjs.GetDebuggerLocalVariables(ctx, 0);
//		_this->AppendResultText(_T("(DEBUG)LocalVariables:"), true);
//		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, localVars), false);
//
//		ValueHandle closureVars = qjs.GetDebuggerClosureVariables(ctx, 0);
//		_this->AppendResultText(_T("(DEBUG)ClosureVariables:"), true);
//		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, closureVars), false);
//
//		ValueHandle backtrace = qjs.GetDebuggerBacktrace(ctx, pc);
//		_this->AppendResultText(_T("(DEBUG)Backtrace:"), true);
//		_this->AppendResultText(ctx, qjs.JsonStringify(ctx, backtrace), false);
//
//		_this->m_lastBreak = true;
//		_this->m_singleStepExecution = false;
//		_this->m_debugNext = false;
//
//		MSG msg;
//		while (!_this->m_debugNext)
//		{
//			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//			{
//#pragma region 执行临时脚本
//				if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)
//				{
//					switch (GetFocus()->GetDlgCtrlID())
//					{
//					case IDC_EDIT3:
//						if (ctx)
//						{
//							CString script;
//							_this->m_editTestScript.GetWindowText(script);
//							_this->m_editTestScript.SetSel(0, script.GetLength());
//							if (!script.IsEmpty())
//							{
//								ValueHandle res = qjs.RunScript(ctx, qjs.UnicodeToUtf8(ctx, script.GetString()), qjs.TheJsNull(), "");
//								if (!qjs.JsValueIsException(res))
//								{
//									_this->AppendResultText(_T("(DEBUG)") + script + _T(":"), true);
//									_this->AppendResultText(ctx, res, false);
//								}
//								else
//								{
//									ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
//									_this->AppendResultText(_T("(DEBUG Exception)") + script + _T(":"), true);
//									_this->AppendResultText(ctx, exception, false);
//								}
//
//							}
//							continue;
//						}
//						break;
//					}
//				}
//#pragma endregion
//
//				if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
//				{
//					_this->m_debugNext = true;
//					_this->m_onDebugMode = false;
//					break;
//				}
//
//				::DispatchMessage(&msg);
//				::TranslateMessage(&msg);
//			}
//		}
//	}
//	else
//	{
//		MSG msg;
//		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//		{
//			::DispatchMessage(&msg);
//			::TranslateMessage(&msg);
//		}
//
//		_this->m_lastBreak = false;
//	}
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


BEGIN_MESSAGE_MAP(DlgDebugger, CDialogEx)
END_MESSAGE_MAP()


// DlgDebugger 消息处理程序
