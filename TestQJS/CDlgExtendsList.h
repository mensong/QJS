#pragma once
#include "afxdialogex.h"
#include "ListCtrlCustom.h"
#include <string>
#include <vector>
#include "CtrlScale.h"


// CDlgExtendsList 对话框

class CDlgExtendsList : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgExtendsList)

public:
	CDlgExtendsList(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDlgExtendsList();

	struct ExtInfo
	{
		CString parentName;
		CString extFilePath;
	};
	std::vector<ExtInfo> m_extents;

// 对话框数据
	enum { IDD = IDD_DLG_EXTENDS_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CListCtrlCustom m_listExtends;
	CCtrlScale m_scale;

	afx_msg void OnBnClickedOk();
	void AddExt(const CString& parentName, const CString& extFile);
	afx_msg void OnBnClickedBtnAdd();
};
