#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "..\3RD\CtrlScale\CtrlScale.h"

// DlgShowInformation 对话框

class DlgShowInformation : public CDialogEx
{
	DECLARE_DYNAMIC(DlgShowInformation)

public:
	DlgShowInformation(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgShowInformation();

	CString m_title;
	CString m_msg;

	static bool ms_noShowAgain;

// 对话框数据
	enum { IDD = IDD_DIALOG_INFORMATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CEdit m_editMsg;
	CButton m_chkNoShow;
	CCtrlScale m_scale;
	afx_msg void OnBnClickedCheck1();
};
