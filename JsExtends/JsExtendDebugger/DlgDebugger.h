#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include <set>
#include "QJS.h"
#include "../../3RD/CtrlScale/CtrlScale.h"

// DlgDebugger 对话框

class DlgDebugger 
	: public CDialogEx
{
	DECLARE_DYNAMIC(DlgDebugger)

public:
	DlgDebugger(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgDebugger();

	static void DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data);
	static bool DoEvent(DlgDebugger* dlg, ContextHandle ctx);

	std::set<int> m_breakPoints;
	bool m_debugMode;
	bool m_singleStepExecution;
	bool m_continue;

	void AppendResultText(const wchar_t* msg, bool newLine);
	void AppendResultText(ContextHandle ctx, const ValueHandle& msg, bool newLine);
	void AppendResultText(ContextHandle ctx, const char* msg, bool newLine);

	void QuitDebug();

// 对话框数据
	enum { IDD = IDD_DIALOG_DEBUGGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

protected:
	void RefreshBreakPoints();
	void SplitCString(const CString& _cstr, const CString& _flag, CStringArray& _resultArray);
	void EnbaleDebugOperations(BOOL enable);

public:
	CEdit m_editSrc;
	CEdit m_editBreakpoints;
	CButton m_btnStep;
	CButton m_btnContinue;
	CEdit m_editDebuggerExpr;
	CEdit m_editOutput;
	CCtrlScale m_scale;
	afx_msg void OnBnClickedButtonStep();
	afx_msg void OnBnClickedButtonContinue();
	
};
