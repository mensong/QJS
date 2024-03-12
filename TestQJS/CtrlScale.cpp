#include "CtrlScale.h"

std::map<HWND, CCtrlScale*> CCtrlScale::ms_scaleManagers;

bool CCtrlScale::HasAnchorType(DWORD flags, CCtrlScale::AnchorType rectType)
{
	return (flags & rectType) != 0;
}

CCtrlScale::CCtrlScale(void)
	: m_pParentWnd(NULL)
	, m_originalWinProc(NULL)
{
}

CCtrlScale::CCtrlScale(HWND pParentWnd)
	: m_pParentWnd(pParentWnd)
	, m_originalWinProc(NULL)
{
	Init(pParentWnd);
}

CCtrlScale::~CCtrlScale(void)
{
	removeScaleManager();

	m_ctrlRect.clear();
}

BOOL CCtrlScale::Init(HWND pParentWnd)
{
	if (!pParentWnd)
		return FALSE;

	m_pParentWnd = pParentWnd;
	::GetClientRect(m_pParentWnd, &m_rectWin);//获取对话框的大小

	removeScaleManager();

	m_originalWinProc = (WNDPROC)::SetWindowLongPtr(m_pParentWnd, GWLP_WNDPROC, (LONG_PTR)ParentWinProc);
	if (!m_originalWinProc)
		return FALSE;
	ms_scaleManagers[pParentWnd] = this;

	return TRUE;
}

void CCtrlScale::Reset()
{
	m_ctrlRect.clear();
	m_ctrlAnchorType.clear();
	m_ctrlExclude.clear();
}

void CCtrlScale::Scale(int cx, int cy)
{
	if (!m_pParentWnd)
		return;

	//判定是否为最小状态，是的话就不进行重绘
	if (cx == 0 || cy == 0)
		return;

	//获取控件得到大小以及原大小方便进行比例调整
	HWND pWnd = NULL;
	int nID = 0;
	//列出所有控件
	HWND hChild = ::GetWindow(m_pParentWnd, GW_CHILD);
	while (hChild)
	{
		nID = ::GetDlgCtrlID(hChild);//获得控件ID
		if (m_ctrlExclude.find(nID) != m_ctrlExclude.end())
		{
			hChild = ::GetWindow(hChild, GW_HWNDNEXT);
			continue;
		}

		pWnd = ::GetDlgItem(m_pParentWnd, nID);//获取控件指针
		if (pWnd)
		{
			RECT rect;//获取当前控件的大小
			::GetWindowRect(pWnd, &rect);
			POINT pt;
			pt.x = rect.left;
			pt.y = rect.top;
			::ScreenToClient(m_pParentWnd, &pt);//将控件大小转换为在窗口的区域坐标
			rect.left = pt.x;
			rect.top = pt.y;
			pt.x = rect.right;
			pt.y = rect.bottom;
			::ScreenToClient(m_pParentWnd, &pt);//将控件大小转换为在窗口的区域坐标
			rect.right = pt.x;
			rect.bottom = pt.y;
			//防止控件失真
			::InvalidateRect(m_pParentWnd, &rect, TRUE);
			
			//获得AnchorType
			DWORD anchorType = AnchorType::ProportionalScale;
			std::map<int, DWORD>::iterator itFinder = m_ctrlAnchorType.find(nID);
			if (itFinder != m_ctrlAnchorType.end())
			{
				anchorType = itFinder->second;
			}

			CTRLRECT cr;
			std::map<int, CTRLRECT>::iterator it = m_ctrlRect.find(nID);
			if (it != m_ctrlRect.end())
			{//如果保存的有该控件与窗口比值，直接乘上当前窗口大小
				cr = it->second;
			}
			else
			{//没有找到控件的比值，则加入
				LONG winWidth = m_rectWin.right - m_rectWin.left;
				LONG winHeight = m_rectWin.bottom - m_rectWin.top;
				cr.dScale[0] = (double)rect.left / winWidth;//注意类型转换，不然保存成long型就直接为0了
				cr.dScale[1] = (double)rect.right / winWidth;
				cr.dScale[2] = (double)rect.top / winHeight;
				cr.dScale[3] = (double)rect.bottom / winHeight;
								
				//设置Anchor
				if (HasAnchorType(anchorType, AnchorType::AnchorLeftToWinRight))
					cr.offsetAnchor[AnchorType::AnchorLeftToWinRight] = winWidth - rect.left;

				if (HasAnchorType(anchorType, AnchorType::AnchorRightToWinRight))
					cr.offsetAnchor[AnchorType::AnchorRightToWinRight] = winWidth - rect.right;
				else if (HasAnchorType(anchorType, AnchorType::AnchorRightToWinLeft))
					cr.offsetAnchor[AnchorType::AnchorRightToWinLeft] = rect.right;

				if (HasAnchorType(anchorType, AnchorType::AnchorTopToWinBottom))
					cr.offsetAnchor[AnchorType::AnchorTopToWinBottom] = winHeight - rect.top;

				if (HasAnchorType(anchorType, AnchorType::AnchorBottomToWinBottom))
					cr.offsetAnchor[AnchorType::AnchorBottomToWinBottom] = winHeight - rect.bottom;

				m_ctrlRect.insert(std::make_pair(nID, cr));
			}

			//调整控件
			if (HasAnchorType(anchorType, AnchorType::AnchorLeftToWinLeft))
				;//StaticLeft
			else if (HasAnchorType(anchorType, AnchorType::AnchorLeftToWinRight))
				rect.left = cx - cr.offsetAnchor[AnchorType::AnchorLeftToWinRight];
			else
				rect.left = cr.dScale[0] * cx;//ProportionalScale

			if (HasAnchorType(anchorType, AnchorType::AnchorRightToWinRight))
				rect.right = cx - cr.offsetAnchor[AnchorType::AnchorRightToWinRight];
			else if (HasAnchorType(anchorType, AnchorType::AnchorRightToWinLeft))
				rect.right = cr.offsetAnchor[AnchorType::AnchorRightToWinLeft];
			else
				rect.right = cr.dScale[1] * cx;//ProportionalScale

			if (HasAnchorType(anchorType, AnchorType::AnchorTopToWinTop))
				;//StaticTop
			else if (HasAnchorType(anchorType, AnchorType::AnchorTopToWinBottom))
				rect.top = cy - cr.offsetAnchor[AnchorType::AnchorTopToWinBottom];
			else
				rect.top = cr.dScale[2] * cy;//ProportionalScale

			if (HasAnchorType(anchorType, AnchorType::AnchorBottomToWinBottom))
				rect.bottom = cy - cr.offsetAnchor[AnchorType::AnchorBottomToWinBottom];
			else if (HasAnchorType(anchorType, AnchorType::AnchorBottomToWinTop))
				;
			else
				rect.bottom = cr.dScale[3] * cy;//ProportionalScale


			//if (pWnd->IsKindOf(RUNTIME_CLASS(CComboBox)))
			//{
			//	//解决ComboBox缩放后,无法下拉的问题
			//	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height() + 200);
			//}
			//else
			{
				LONG ctrlWidth = rect.right - rect.left;
				LONG ctrlHeight = rect.bottom - rect.top;
				::MoveWindow(pWnd, rect.left, rect.top, ctrlWidth, ctrlHeight, TRUE);//设置控件大小
			}
		}
		hChild = ::GetWindow(hChild, GW_HWNDNEXT);
	}
	::GetClientRect(m_pParentWnd, &m_rectWin);//获取变化后的窗口大小
}

void CCtrlScale::SetAnchor(int id, DWORD/*CCtrlScale::AnchorType*/ rectType)
{
	m_ctrlAnchorType[id] |= rectType;
}

void CCtrlScale::AddExclude(int ctrlId)
{
	m_ctrlExclude.insert(ctrlId);
}

void CCtrlScale::RemoveExclude(int ctrlId)
{
	m_ctrlExclude.erase(ctrlId);
}

void CCtrlScale::removeScaleManager()
{
	if (m_pParentWnd)
	{
		std::map<HWND, CCtrlScale*>::iterator itFinder = ms_scaleManagers.find(m_pParentWnd);
		if (itFinder != ms_scaleManagers.end())
		{
			CCtrlScale* scale = itFinder->second;
			::SetWindowLongPtr(m_pParentWnd, GWLP_WNDPROC, (LONG_PTR)scale->m_originalWinProc);
			ms_scaleManagers.erase(m_pParentWnd);
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
