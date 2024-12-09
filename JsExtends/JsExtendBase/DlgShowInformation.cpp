// DlgShowInformation.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendBase.h"
#include "afxdialogex.h"
#include "DlgShowInformation.h"

bool DlgShowInformation::ms_noShowAgain = false;

// DlgShowInformation 对话框

IMPLEMENT_DYNAMIC(DlgShowInformation, CDialogEx)

DlgShowInformation::DlgShowInformation(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_INFORMATION, pParent)
	, m_title(_T("Information"))
{

}

DlgShowInformation::~DlgShowInformation()
{
}

void DlgShowInformation::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editMsg);
	DDX_Control(pDX, IDC_CHECK1, m_chkNoShow);
}

BOOL DlgShowInformation::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	m_msg.Replace(_T("\r\n"), _T("\n"));
	m_msg.Replace(_T("\n"), _T("\r\n"));

	SetWindowText(m_title);
	m_editMsg.SetWindowText(m_msg);

	m_scale.SetAnchor(m_editMsg.m_hWnd, 
		CCtrlScale::AnchorTopToWinTop |
		CCtrlScale::AnchorLeftToWinLeft | 
		CCtrlScale::AnchorRightToWinRight | 
		CCtrlScale::AnchorBottomToWinBottom);
	m_scale.SetAnchor(m_chkNoShow.m_hWnd,
		CCtrlScale::AnchorTopToWinBottom |
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight |
		CCtrlScale::AnchorBottomToWinBottom);
	m_scale.Init(m_hWnd);

	return ret;
}


BEGIN_MESSAGE_MAP(DlgShowInformation, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK1, &DlgShowInformation::OnBnClickedCheck1)
END_MESSAGE_MAP()


// DlgShowInformation 消息处理程序


void DlgShowInformation::OnBnClickedCheck1()
{
	ms_noShowAgain = m_chkNoShow.GetCheck();
}
