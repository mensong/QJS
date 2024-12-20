#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "../../3RD/CtrlScale/CtrlScale.h"

// DlgDebugOptions 对话框

class DlgDebugOptions : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDebugOptions)

public:
	DlgDebugOptions(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgDebugOptions();

// 对话框数据
	enum { IDD = IDD_DIALOG_DEBUG_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CButton m_chkShowLocalVars;
	CButton m_chkShowFuncName;
	CButton m_chkShowStackDepth;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit m_editAutoDebugScript;
	CButton m_btnOK;
	CButton m_btnCancel;
	CCtrlScale m_scale;
};
