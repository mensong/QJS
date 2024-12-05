// CItemFile.cpp: 实现文件
//

#include "pch.h"
#include "TestQJS.h"
#include "afxdialogex.h"
#include "CItemFile.h"
#include <string>
#include "ListCtrlCustom.h"


// CItemFile 对话框

IMPLEMENT_DYNAMIC(CItemFile, CDialogEx)

CItemFile::CItemFile(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ITEM_FILE, pParent)
{

}

CItemFile::~CItemFile()
{
}

void CItemFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILE, m_editFile);
}


BEGIN_MESSAGE_MAP(CItemFile, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SELECT_FILE, &CItemFile::OnBnClickedBtnSelectFile)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CItemFile::OnBnClickedBtnRemove)
END_MESSAGE_MAP()


// CItemFile 消息处理程序


void CItemFile::OnBnClickedBtnSelectFile()
{
	CFileDialog fdlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"*.dll|*.dll|All files(*.*)|*.*||");
	fdlg.m_ofn.lpstrTitle = L"选择插件文件";
	fdlg.m_ofn.lpstrInitialDir = L".\\";
	if (fdlg.DoModal() == IDOK)
	{
		CString strFileName = fdlg.GetPathName();
		m_editFile.SetWindowText(strFileName);
	}
}


void CItemFile::OnBnClickedBtnRemove()
{
	CListCtrlCustom* listCtrl = (CListCtrlCustom*)GetParent();
	int row, col;
	if (listCtrl->GetItemMouseAbove(row, col))
	{
		listCtrl->DeleteItem(row);
	}
}
