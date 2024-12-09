// DlgInputBox.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendBase.h"
#include "afxdialogex.h"
#include "DlgInputBox.h"


// DlgInputBox 对话框

IMPLEMENT_DYNAMIC(DlgInputBox, CDialogEx)

DlgInputBox::DlgInputBox(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_INPUTBOX, pParent)
	, m_title(_T("Input"))
	, m_tip(_T("请输入:"))
{

}

DlgInputBox::~DlgInputBox()
{
}

void DlgInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TIP, m_editTip);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_editInput);
}

BOOL DlgInputBox::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	m_tip.Replace(_T("\r\n"), _T("\n"));
	m_tip.Replace(_T("\n"), _T("\r\n"));
	m_content.Replace(_T("\r\n"), _T("\n"));
	m_content.Replace(_T("\n"), _T("\r\n"));

	SetWindowText(m_title);
	m_editTip.SetWindowText(m_tip);
	m_editInput.SetWindowText(m_content);

	m_scale.SetAnchor(m_editTip.m_hWnd,
		CCtrlScale::AnchorTopToWinTop |
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(m_editInput.m_hWnd,
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorRightToWinRight |
		CCtrlScale::AnchorBottomToWinBottom);
	m_scale.SetAnchor(GetDlgItem(IDOK)->m_hWnd,
		CCtrlScale::AnchorTopToWinTop |
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight);
	m_scale.SetAnchor(GetDlgItem(IDCANCEL)->m_hWnd,
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight);
	m_scale.Init(m_hWnd);

	return ret;
}


BEGIN_MESSAGE_MAP(DlgInputBox, CDialogEx)
	ON_BN_CLICKED(IDOK, &DlgInputBox::OnBnClickedOk)
END_MESSAGE_MAP()


// DlgInputBox 消息处理程序


void DlgInputBox::OnBnClickedOk()
{
	m_editInput.GetWindowText(m_content);
	CDialogEx::OnOK();
}
