// DlgDebugOptions.cpp: 实现文件
//

#include "pch.h"
#include "JsExtendDebugger.h"
#include "afxdialogex.h"
#include "DlgDebugOptions.h"
#include "DlgDebugger.h"


std::string UnicodeToUtf8(const wchar_t* wideByteRet)
{
	std::string Ret_UnicodeToUtf8;
	char* pMultiCharStr; //定义返回的多字符指针
	int nLenOfMultiCharStr; //保存多字符个数，注意不是字节数
	//获取多字符的个数
	nLenOfMultiCharStr = WideCharToMultiByte(CP_UTF8, 0, wideByteRet, -1, NULL, 0, NULL, NULL);
	//获得多字符指针
	pMultiCharStr = (char*)(HeapAlloc(GetProcessHeap(), 0, nLenOfMultiCharStr * sizeof(char)));
	WideCharToMultiByte(CP_UTF8, 0, wideByteRet, -1, pMultiCharStr, nLenOfMultiCharStr, NULL, NULL);
	//返回
	Ret_UnicodeToUtf8.resize(nLenOfMultiCharStr + 1, 0);
	strcpy_s(&Ret_UnicodeToUtf8[0], Ret_UnicodeToUtf8.size(), pMultiCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pMultiCharStr);
	return Ret_UnicodeToUtf8.c_str();
}

std::wstring Utf8ToUnicode(const char* utf8ByteStr)
{
	std::wstring Ret_Utf8ToUnicode;
	wchar_t* pWideCharStr; //定义返回的宽字符指针
	int nLenOfWideCharStr; //保存宽字符个数，注意不是字节数
	//获取宽字符的个数
	nLenOfWideCharStr = MultiByteToWideChar(CP_UTF8, 0, utf8ByteStr, -1, NULL, 0);
	//获得宽字符指针
	pWideCharStr = (wchar_t*)(HeapAlloc(GetProcessHeap(), 0, nLenOfWideCharStr * sizeof(wchar_t)));
	MultiByteToWideChar(CP_UTF8, 0, utf8ByteStr, -1, pWideCharStr, nLenOfWideCharStr);
	//返回
	Ret_Utf8ToUnicode.resize(nLenOfWideCharStr + 1, 0);
	wcscpy_s(&Ret_Utf8ToUnicode[0], Ret_Utf8ToUnicode.size(), pWideCharStr);
	//销毁内存中的字符串
	HeapFree(GetProcessHeap(), 0, pWideCharStr);
	return Ret_Utf8ToUnicode.c_str();
}


// DlgDebugOptions 对话框

IMPLEMENT_DYNAMIC(DlgDebugOptions, CDialogEx)

DlgDebugOptions::DlgDebugOptions(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEBUG_OPTIONS, pParent)
{

}

DlgDebugOptions::~DlgDebugOptions()
{
}

void DlgDebugOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SHOW_DEBUG_OPTIONS, m_chkShowLocalVars);
	DDX_Control(pDX, IDC_CHECK_SHOW_FUNC_NAME, m_chkShowFuncName);
	DDX_Control(pDX, IDC_CHECK_SHOW_STACKDEPTH, m_chkShowStackDepth);
	DDX_Control(pDX, IDC_EDIT_AUTO_DEBUG_SCRIPT, m_editAutoDebugScript);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BOOL DlgDebugOptions::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	m_scale.SetAnchor(m_editAutoDebugScript.m_hWnd,
		CCtrlScale::AnchorTopToWinTop | 
		CCtrlScale::AnchorBottomToWinBottom | 
		CCtrlScale::AnchorLeftToWinLeft |
		CCtrlScale::AnchorLeftToWinRight
	);
	m_scale.SetAnchor(m_btnOK.m_hWnd, 
		CCtrlScale::AnchorTopToWinBottom |
		CCtrlScale::AnchorBottomToWinBottom |
		CCtrlScale::AnchorLeftToWinRight | 
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.SetAnchor(m_btnCancel.m_hWnd,
		CCtrlScale::AnchorTopToWinBottom |
		CCtrlScale::AnchorBottomToWinBottom |
		CCtrlScale::AnchorLeftToWinRight |
		CCtrlScale::AnchorRightToWinRight
	);
	m_scale.AddExclude(m_chkShowFuncName.m_hWnd);
	m_scale.AddExclude(m_chkShowLocalVars.m_hWnd);
	m_scale.AddExclude(m_chkShowStackDepth.m_hWnd);
	m_scale.AddExclude(GetDlgItem(IDC_STATIC_AUTO_DEBUG_SCRIPT_TIP)->m_hWnd);
	m_scale.Init(m_hWnd);

	m_chkShowLocalVars.SetCheck(DlgDebugger::ms_debugShowLocalVars);
	m_chkShowFuncName.SetCheck(DlgDebugger::ms_debugShowFuncName);
	m_chkShowStackDepth.SetCheck(DlgDebugger::ms_debugShowStackDepth);
	m_editAutoDebugScript.SetWindowText(Utf8ToUnicode(DlgDebugger::ms_debugAutoScript.c_str()).c_str());

	return ret;
}


BEGIN_MESSAGE_MAP(DlgDebugOptions, CDialogEx)
	ON_BN_CLICKED(IDOK, &DlgDebugOptions::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &DlgDebugOptions::OnBnClickedCancel)
END_MESSAGE_MAP()


// DlgDebugOptions 消息处理程序


void DlgDebugOptions::OnBnClickedOk()
{
	DlgDebugger::ms_debugShowLocalVars = m_chkShowLocalVars.GetCheck();
	DlgDebugger::ms_debugShowFuncName = m_chkShowFuncName.GetCheck();
	DlgDebugger::ms_debugShowStackDepth = m_chkShowStackDepth.GetCheck();

	CString script;
	m_editAutoDebugScript.GetWindowText(script);
	DlgDebugger::ms_debugAutoScript = UnicodeToUtf8(script.GetString());

	CDialogEx::OnOK();
}


void DlgDebugOptions::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
