// DlgDebugger.cpp: 实现文件
//

#include "pch.h"
#include "afxdialogex.h"
#include "DlgIgnoreList.h"
#include <clocale>

//UTF8 转 Unicode
bool Utf82Unicode(std::wstring& out, const std::string& in, const char* locale = "")
{
	setlocale(LC_ALL, locale);

	int len = MultiByteToWideChar(CP_UTF8, 0, &in[0], in.size(), NULL, 0);
	if (len <= 0)
	{
		//::GetLastError();
		setlocale(LC_ALL, "");
		return false;
	}

	out.resize(len);
	len = MultiByteToWideChar(CP_UTF8, 0, &in[0], in.size(), &out[0], out.size());
	if (len <= 0)
	{
		//::GetLastError();
		setlocale(LC_ALL, "");
		return false;
	}

	setlocale(LC_ALL, "");
	return true;
}

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
			std::wstring usrc;
			size_t usrcLen = 0;
			Utf82Unicode(usrc, m_ignoreListCache[i].c_str());
			int rowId = m_listIgnore.AddString(usrc.c_str());
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
