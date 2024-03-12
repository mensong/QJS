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
	::GetClientRect(m_pParentWnd, &m_rectWin);//��ȡ�Ի���Ĵ�С

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

	//�ж��Ƿ�Ϊ��С״̬���ǵĻ��Ͳ������ػ�
	if (cx == 0 || cy == 0)
		return;

	//��ȡ�ؼ��õ���С�Լ�ԭ��С������б�������
	HWND pWnd = NULL;
	int nID = 0;
	//�г����пؼ�
	HWND hChild = ::GetWindow(m_pParentWnd, GW_CHILD);
	while (hChild)
	{
		nID = ::GetDlgCtrlID(hChild);//��ÿؼ�ID
		if (m_ctrlExclude.find(nID) != m_ctrlExclude.end())
		{
			hChild = ::GetWindow(hChild, GW_HWNDNEXT);
			continue;
		}

		pWnd = ::GetDlgItem(m_pParentWnd, nID);//��ȡ�ؼ�ָ��
		if (pWnd)
		{
			RECT rect;//��ȡ��ǰ�ؼ��Ĵ�С
			::GetWindowRect(pWnd, &rect);
			POINT pt;
			pt.x = rect.left;
			pt.y = rect.top;
			::ScreenToClient(m_pParentWnd, &pt);//���ؼ���Сת��Ϊ�ڴ��ڵ���������
			rect.left = pt.x;
			rect.top = pt.y;
			pt.x = rect.right;
			pt.y = rect.bottom;
			::ScreenToClient(m_pParentWnd, &pt);//���ؼ���Сת��Ϊ�ڴ��ڵ���������
			rect.right = pt.x;
			rect.bottom = pt.y;
			//��ֹ�ؼ�ʧ��
			::InvalidateRect(m_pParentWnd, &rect, TRUE);
			
			//���AnchorType
			DWORD anchorType = AnchorType::ProportionalScale;
			std::map<int, DWORD>::iterator itFinder = m_ctrlAnchorType.find(nID);
			if (itFinder != m_ctrlAnchorType.end())
			{
				anchorType = itFinder->second;
			}

			CTRLRECT cr;
			std::map<int, CTRLRECT>::iterator it = m_ctrlRect.find(nID);
			if (it != m_ctrlRect.end())
			{//���������иÿؼ��봰�ڱ�ֵ��ֱ�ӳ��ϵ�ǰ���ڴ�С
				cr = it->second;
			}
			else
			{//û���ҵ��ؼ��ı�ֵ�������
				LONG winWidth = m_rectWin.right - m_rectWin.left;
				LONG winHeight = m_rectWin.bottom - m_rectWin.top;
				cr.dScale[0] = (double)rect.left / winWidth;//ע������ת������Ȼ�����long�;�ֱ��Ϊ0��
				cr.dScale[1] = (double)rect.right / winWidth;
				cr.dScale[2] = (double)rect.top / winHeight;
				cr.dScale[3] = (double)rect.bottom / winHeight;
								
				//����Anchor
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

			//�����ؼ�
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
			//	//���ComboBox���ź�,�޷�����������
			//	pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height() + 200);
			//}
			//else
			{
				LONG ctrlWidth = rect.right - rect.left;
				LONG ctrlHeight = rect.bottom - rect.top;
				::MoveWindow(pWnd, rect.left, rect.top, ctrlWidth, ctrlHeight, TRUE);//���ÿؼ���С
			}
		}
		hChild = ::GetWindow(hChild, GW_HWNDNEXT);
	}
	::GetClientRect(m_pParentWnd, &m_rectWin);//��ȡ�仯��Ĵ��ڴ�С
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
