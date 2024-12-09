#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "..\3RD\CtrlScale\CtrlScale.h"

// DlgInputBox 对话框

class DlgInputBox 
	: public CDialogEx
{
	DECLARE_DYNAMIC(DlgInputBox)

public:
	DlgInputBox(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgInputBox();

	CString m_title;
	CString m_tip;
	CString m_content;

// 对话框数据
	enum { IDD = IDD_DIALOG_INPUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CEdit m_editTip;
	CEdit m_editInput;
	CCtrlScale m_scale;
	afx_msg void OnBnClickedOk();
};
