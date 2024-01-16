
// TestQJSDlg.h: 头文件
//

#pragma once
#include <set>
#include <string>
#include "../QJS/QJS.h"
#include "CtrlScale.h"

// CTestQJSDlg 对话框
class CTestQJSDlg : public CDialogEx
{
// 构造
public:
	CTestQJSDlg(CWnd* pParent = nullptr);	// 标准构造函数

	void AppendResultText(const wchar_t* msg, bool newLine = true);
	void AppendResultText(ContextHandle ctx, const ValueHandle& msg, bool newLine = true);
	void AppendResultText(const char* msg, bool newLine = true);

	bool m_onDebugMode;
	std::set<int> m_breakPoints;
	bool m_debugNext;
	bool m_singleStepExecution;
	bool m_lastBreak;
	static void DebuggerLineCallback(ContextHandle ctx, uint32_t line_no, const uint8_t* pc, void* user_data);

	std::string m_curFilename;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTQJS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
		
// 实现
protected:
	HICON m_hIcon;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK() { }
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();

	CEdit m_editScript;
	CEdit m_editResult;
	CEdit m_editBreakpointsList;
	CEdit m_editTestScript;
	CButton m_chkIsDebug;
	CButton m_btnRun;
	CButton m_btnContinue;
	CButton m_btnSingleStep;
	CCtrlScale m_scale;

	afx_msg void OnBnClickedCheck1();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnLoadFromFile();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedBtnClearSrc();
};
