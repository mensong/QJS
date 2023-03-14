
// TestQJSDlg.h: 头文件
//

#pragma once


// CTestQJSDlg 对话框
class CTestQJSDlg : public CDialogEx
{
// 构造
public:
	CTestQJSDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTQJS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void AppendResultText(const CString& msg, bool endl = true);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editScript;
	CEdit m_editResult;
	afx_msg void OnBnClickedButton1();
};
