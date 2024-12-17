#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "QJS.h"

// DlgDebugger 对话框

class DlgDebugger : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDebugger)

public:
	DlgDebugger(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgDebugger();

	static void DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data);

// 对话框数据
	enum { IDD = IDD_DIALOG_DEBUGGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CEdit m_editSrc;
	CEdit m_editBreakpoints;
	CButton m_btnStep;
	CButton m_btnContinue;
	CEdit m_editDebuggerExpr;
	CEdit m_editOutput;
};
