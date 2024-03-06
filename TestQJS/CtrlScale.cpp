#include "pch.h"
#include "CtrlScale.h"

std::map<HWND, CCtrlScale*> CCtrlScale::ms_scaleManagers;

CCtrlScale::CCtrlScale(void)
	: m_pParentWnd(NULL)
	, m_originalWinProc(NULL)
{
}

CCtrlScale::CCtrlScale(CWnd* pParentWnd)
	: m_pParentWnd(pParentWnd)
	, m_originalWinProc(NULL)
{
	Init(pParentWnd);
}

CCtrlScale::~CCtrlScale(void)
{
	removeScaleManager();

	m_vecCtrl.clear();
}

BOOL CCtrlScale::Init(CWnd* pParentWnd)
{
	if (!pParentWnd)
		return FALSE;

	m_pParentWnd = pParentWnd;
	m_pParentWnd->GetClientRect(&m_rect); //获取对话框的大小

	removeScaleManager();

	m_originalWinProc = (WNDPROC)::SetWindowLongPtr(m_pParentWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)ParentWinProc);
	if (!m_originalWinProc)
		return FALSE;
	ms_scaleManagers[pParentWnd->m_hWnd] = this;

	return TRUE;
}

void CCtrlScale::Scale(int cx, int cy)
{
	if (!m_pParentWnd)
		return;

	//判定是否为最小状态，是的话就不进行重绘
	if (cx == 0 || cy == 0)
		return;

	//获取控件得到大小以及原大小方便进行比例调整
	CWnd* pWnd;
	int nID = 0;
	//列出所有控件
	HWND hChild = ::GetWindow(m_pParentWnd->m_hWnd, GW_CHILD);
	while (hChild)
	{
		nID = ::GetDlgCtrlID(hChild);//获得控件ID
		pWnd = m_pParentWnd->GetDlgItem(nID);//获取控件指针
		if (pWnd)
		{
			CRect rect;//获取当前控件的大小
			pWnd->GetWindowRect(&rect);
			m_pParentWnd->ScreenToClient(&rect);//将控件大小转换为在窗口的区域坐标
			//防止控件失真
			m_pParentWnd->InvalidateRect(rect);

			CTRLRECT cr;
			cr.nId = nID;
			std::vector<CTRLRECT>::iterator it = find_if(m_vecCtrl.begin(), m_vecCtrl.end(), cr);
			if (it != m_vecCtrl.end())
			{//如果保存的有该控件与窗口比值，直接乘上当前窗口大小
				DWORD rectType = RectType::ProportionalScale;
				std::map<int, DWORD>::iterator itFinder = m_ctrlRectType.find(nID);
				if (itFinder != m_ctrlRectType.end())
				{
					rectType = itFinder->second;
				}

				if ((rectType & RectType::StaticLeft) == 0)
					rect.left = it->dScale[0] * cx;

				if ((rectType & RectType::AnchorRight) != 0)
					rect.right = cx - it->rightOffset;//AnchorRight
				else if ((rectType & RectType::StaticRight) == 0)
					rect.right = it->dScale[1] * cx;//ProportionalScale
				//else rect.right = rect.right; //StaticRight

				if ((rectType & RectType::StaticTop) == 0)
					rect.top = it->dScale[2] * cy;

				if ((rectType & RectType::AnchorBottom) != 0)
					rect.bottom = cy - it->bottomOffset;//AnchorBottom
				else if ((rectType & RectType::StaticBottom) == 0)
					rect.bottom = it->dScale[3] * cy;//ProportionalScale
				//else rect.bottom = rect.bottom; //StaticBottom
			}
			else
			{//没有找到控件的比值，则加入
				cr.dScale[0] = (double)rect.left / m_rect.Width();//注意类型转换，不然保存成long型就直接为0了
				cr.dScale[1] = (double)rect.right / m_rect.Width();
				cr.dScale[2] = (double)rect.top / m_rect.Height();
				cr.dScale[3] = (double)rect.bottom / m_rect.Height();

				DWORD rectType = RectType::ProportionalScale;
				std::map<int, DWORD>::iterator itFinder = m_ctrlRectType.find(nID);
				if (itFinder != m_ctrlRectType.end())
				{
					rectType = itFinder->second;
				}

				//调整控件大小
				if ((rectType & RectType::StaticLeft) == 0)
					rect.left = cr.dScale[0] * cx;

				if ((rectType & RectType::AnchorRight) != 0)
				{//AnchorRight
					CRect rectWin;
					m_pParentWnd->GetClientRect(rectWin);
					cr.rightOffset = rectWin.Width() - rect.right;
					rect.right = cx - cr.rightOffset;
				}
				else if ((rectType & RectType::StaticRight) == 0)
					rect.right = cr.dScale[1] * cx;//ProportionalScale
				//else rect.right = rect.right;//StaticRight

				if ((rectType & RectType::StaticTop) == 0)
					rect.top = cr.dScale[2] * cy;

				if ((rectType & RectType::AnchorBottom) != 0)
				{//AnchorBottom
					CRect rectWin;
					m_pParentWnd->GetClientRect(rectWin);
					cr.bottomOffset = rectWin.Height() - rect.bottom;
					rect.bottom = cy - cr.bottomOffset;
				}
				else if ((rectType & RectType::StaticBottom) == 0)
					rect.bottom = cr.dScale[3] * cy;//ProportionalScale
				//else rect.bottom = rect.bottom;//StaticBottom

				m_vecCtrl.push_back(cr);
			}

			if (pWnd->IsKindOf(RUNTIME_CLASS(CComboBox)))
			{
				//解决ComboBox缩放后,无法下拉的问题
				pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height() + 200);
			}
			else
			{
				pWnd->MoveWindow(rect);//设置控件大小
			}
		}
		hChild = ::GetWindow(hChild, GW_HWNDNEXT);
	}
	m_pParentWnd->GetClientRect(&m_rect);//获取变化后的窗口大小
}

void CCtrlScale::SetRectType(int id, CCtrlScale::RectType rectType)
{
	m_ctrlRectType[id] |= rectType;
}

void CCtrlScale::removeScaleManager()
{
	if (m_pParentWnd)
	{
		std::map<HWND, CCtrlScale*>::iterator itFinder = ms_scaleManagers.find(m_pParentWnd->m_hWnd);
		if (itFinder != ms_scaleManagers.end())
		{
			CCtrlScale* scale = itFinder->second;
			::SetWindowLongPtr(m_pParentWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)scale->m_originalWinProc);
			ms_scaleManagers.erase(m_pParentWnd->m_hWnd);
		}
	}
}

LRESULT CCtrlScale::ParentWinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	std::map<HWND, CCtrlScale*>::iterator itFinder = ms_scaleManagers.find(hWnd);
	if (itFinder == ms_scaleManagers.end())
		return 0;
	WNDPROC oldWinProc = NULL;
	CCtrlScale* scale = itFinder->second;

	if (Msg == WM_SIZE)
	{
		int nWidth = LOWORD(lParam);
		int nHeight = HIWORD(lParam);
		scale->Scale(nWidth, nHeight);
	}

	return CallWindowProc(scale->m_originalWinProc, hWnd, Msg, wParam, lParam);
}
