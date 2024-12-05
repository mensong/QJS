#pragma once
#include "afxdialogex.h"


// CItemFile 对话框

class CItemFile : public CDialogEx
{
	DECLARE_DYNAMIC(CItemFile)

public:
	CItemFile(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CItemFile();

// 对话框数据
	enum { IDD = IDD_ITEM_FILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CEdit m_editFile;
	afx_msg void OnBnClickedBtnSelectFile();
	afx_msg void OnBnClickedBtnRemove();
};
