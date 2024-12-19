#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include <set>
#include <string>
#include "../../3RD/CtrlScale/CtrlScale.h"

// DlgIgnoreList 对话框

class DlgIgnoreList
	: public CDialogEx
{
	DECLARE_DYNAMIC(DlgIgnoreList)

public:
	DlgIgnoreList(std::set<std::string>* ignoredSrc, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~DlgIgnoreList();

	std::set<std::string>* m_ignoredSrc;
	std::vector<std::string> m_ignoreListCache;

	// 对话框数据
	enum { IDD = IDD_DIALOG_IGNORE_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CListBox m_listIgnore;
	CButton m_btnRemove;
	afx_msg void OnBnClickedButtonRemove();
};
