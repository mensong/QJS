// DlgDebugger.cpp: 实现文件
//

#include "pch.h"
#include "afxdialogex.h"
#include "DlgIgnoreList.h"
#include "../pystring/pystring.h"
#include "../pystring/pywstring.h"
#include "../StringConvert/StringConvert.h"

// DlgIgnoreList 对话框

IMPLEMENT_DYNAMIC(DlgIgnoreList, CDialogEx)

DlgIgnoreList::DlgIgnoreList(std::set<std::string>* ignoredSrc, CWnd* pParent /*=nullptr*/)
	: CDialogEx(DlgIgnoreList::IDD, pParent)
	, m_ignoredSrc(ignoredSrc)
{

}

DlgIgnoreList::~DlgIgnoreList()
{
}

void DlgIgnoreList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_IGNORED, m_listIgnore);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_btnRemove);
}

BOOL DlgIgnoreList::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	if (m_ignoredSrc)
	{
		for (auto it = m_ignoredSrc->begin(); it != m_ignoredSrc->end(); ++it)
		{
			m_ignoreListCache.push_back(*it);			
		}

		for (size_t i = 0; i < m_ignoreListCache.size(); i++)
		{
			wchar_t* usrc = NULL;
			size_t usrcLen = 0;
			StringConvert::Ins().Utf82Unicode(&usrc, &usrcLen, m_ignoreListCache[i].c_str(), m_ignoreListCache[i].size(), "");
			if (!usrc)
				continue;
			int rowId = m_listIgnore.AddString(usrc);
			m_listIgnore.SetItemData(rowId, i);
		}
	}

	return ret;
}

BEGIN_MESSAGE_MAP(DlgIgnoreList, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &DlgIgnoreList::OnBnClickedButtonRemove)
END_MESSAGE_MAP()


// DlgIgnoreList 消息处理程序



void DlgIgnoreList::OnBnClickedButtonRemove()
{
	int rowId = m_listIgnore.GetCurSel();
	if (rowId < 0)
		return;

	int srcId = m_listIgnore.GetItemData(rowId);
	m_listIgnore.DeleteString(rowId);
	
	if (m_ignoreListCache.size() <= rowId)
		return;
	m_ignoredSrc->erase(m_ignoreListCache[rowId]);
}
