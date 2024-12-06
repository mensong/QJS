// CDlgExtendsList.cpp: 实现文件
//

#include "pch.h"
#include "TestQJS.h"
#include "afxdialogex.h"
#include "CDlgExtendsList.h"
#include "CItemFile.h"


// CDlgExtendsList 对话框

IMPLEMENT_DYNAMIC(CDlgExtendsList, CDialogEx)

CDlgExtendsList::CDlgExtendsList(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_EXTENDS_LIST, pParent)
{

}

CDlgExtendsList::~CDlgExtendsList()
{
}

void CDlgExtendsList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EXTENDS, m_listExtends);
}

BOOL CDlgExtendsList::OnInitDialog()
{
	BOOL res = CDialogEx::OnInitDialog();

	m_scale.Init(m_hWnd);

	m_listExtends.InsertColumn(0, _T("父变量名"), 0, 150);
	m_listExtends.InsertColumn(1, _T("插件文件"), 0, 350);

	for (size_t i = 0; i < m_extents.size(); i++)
	{
		AddExt(m_extents[i].parentName, m_extents[i].extFilePath);
	}

	return res;
}


BEGIN_MESSAGE_MAP(CDlgExtendsList, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgExtendsList::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDlgExtendsList::OnBnClickedBtnAdd)
END_MESSAGE_MAP()


// CDlgExtendsList 消息处理程序


void CDlgExtendsList::OnBnClickedOk()
{
	m_extents.clear();

	int len = m_listExtends.GetItemCount();
	for (int i = 0; i < len; i++)
	{
		ExtInfo extInfo;

		CEdit* edit = (CEdit*)m_listExtends.GetCtrl(i, 0);
		edit->GetWindowText(extInfo.parentName);

		CItemFile* file = (CItemFile*)m_listExtends.GetCtrl(i, 1);
		file->m_editFile.GetWindowText(extInfo.extFilePath);

		if (!extInfo.extFilePath.IsEmpty())
			m_extents.push_back(extInfo);
	}

	CDialogEx::OnOK();
}

void CDlgExtendsList::AddExt(const CString& parentName, const CString& extFile)
{
	int rowId = m_listExtends.InsertItem(m_listExtends.GetItemCount(), _T(""));

	CEdit* pEdit = new CEdit();
	pEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER, RECT(), &m_listExtends, m_listExtends.GetDlgCtrlID() + rowId);
	pEdit->SetWindowText(parentName);
	m_listExtends.SetItemEx(rowId, 0, pEdit);

	CItemFile* item = new CItemFile;
	item->Create(CItemFile::IDD, &m_listExtends);
	item->m_editFile.SetWindowText(extFile);
	CRect rect;
	item->GetWindowRect(rect);
	m_listExtends.SetRowHeight(rect.Height());
	m_listExtends.SetItemEx(rowId, 1, item);
}


void CDlgExtendsList::OnBnClickedBtnAdd()
{
	AddExt(_T(""), _T(""));
}
