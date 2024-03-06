#include "pch.h"
#include "CtrlScale.h"

std::map<HWND, CCtrlScale*> CCtrlScale::ms_scaleManagers;

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

	m_vecCtrl.clear();
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

			CTRLRECT cr;
			cr.nId = nID;
			std::vector<CTRLRECT>::iterator it = find_if(m_vecCtrl.begin(), m_vecCtrl.end(), cr);
			if (it != m_vecCtrl.end())
			{//���������иÿؼ��봰�ڱ�ֵ��ֱ�ӳ��ϵ�ǰ���ڴ�С
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
			{//û���ҵ��ؼ��ı�ֵ�������
				LONG winWidth = m_rectWin.right - m_rectWin.left;
				LONG winHeight = m_rectWin.bottom - m_rectWin.top;
				cr.dScale[0] = (double)rect.left / winWidth;//ע������ת������Ȼ�����long�;�ֱ��Ϊ0��
				cr.dScale[1] = (double)rect.right / winWidth;
				cr.dScale[2] = (double)rect.top / winHeight;
				cr.dScale[3] = (double)rect.bottom / winHeight;

				DWORD rectType = RectType::ProportionalScale;
				std::map<int, DWORD>::iterator itFinder = m_ctrlRectType.find(nID);
				if (itFinder != m_ctrlRectType.end())
				{
					rectType = itFinder->second;
				}

				//�����ؼ���С
				if ((rectType & RectType::StaticLeft) == 0)
					rect.left = cr.dScale[0] * cx;

				if ((rectType & RectType::AnchorRight) != 0)
				{//AnchorRight
					cr.rightOffset = winWidth - rect.right;
					rect.right = cx - cr.rightOffset;
				}
				else if ((rectType & RectType::StaticRight) == 0)
					rect.right = cr.dScale[1] * cx;//ProportionalScale
				//else rect.right = rect.right;//StaticRight

				if ((rectType & RectType::StaticTop) == 0)
					rect.top = cr.dScale[2] * cy;

				if ((rectType & RectType::AnchorBottom) != 0)
				{//AnchorBottom
					cr.bottomOffset = winHeight - rect.bottom;
					rect.bottom = cy - cr.bottomOffset;
				}
				else if ((rectType & RectType::StaticBottom) == 0)
					rect.bottom = cr.dScale[3] * cy;//ProportionalScale
				//else rect.bottom = rect.bottom;//StaticBottom

				m_vecCtrl.push_back(cr);
			}

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

void CCtrlScale::SetRectType(int id, CCtrlScale::RectType rectType)
{
	m_ctrlRectType[id] |= rectType;
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
