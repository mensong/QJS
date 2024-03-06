#include "pch.h"
#include "CtrlScale.h"

std::map<HWND, CCtrlScale*> CCtrlScale::ms_scaleManagers;

CCtrlScale::CCtrlScale(void) 
	: m_pParentWnd(NULL)
    , m_oldWinProc(NULL)
{
}

CCtrlScale::CCtrlScale(CWnd* pParentWnd) 
    : m_pParentWnd(pParentWnd)
    , m_oldWinProc(NULL)
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
	m_pParentWnd->GetClientRect(&m_rect); //��ȡ�Ի���Ĵ�С

	removeScaleManager();

	m_oldWinProc = (WNDPROC)::SetWindowLongPtr(m_pParentWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)ParentWinProc);
	if (!m_oldWinProc)
		return FALSE;
	ms_scaleManagers[pParentWnd->m_hWnd] = this;

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
    CWnd* pWnd;
    int nID = 0;
    //�г����пؼ�
    HWND hChild = ::GetWindow(m_pParentWnd->m_hWnd, GW_CHILD);
    while (hChild) 
    {
        nID = ::GetDlgCtrlID(hChild);//��ÿؼ�ID
        pWnd = m_pParentWnd->GetDlgItem(nID);//��ȡ�ؼ�ָ��
        if (pWnd) 
        {
            CRect rect;//��ȡ��ǰ�ؼ��Ĵ�С
            pWnd->GetWindowRect(&rect);
            m_pParentWnd->ScreenToClient(&rect);//���ؼ���Сת��Ϊ�ڴ��ڵ���������
            //��ֹ�ؼ�ʧ��
            m_pParentWnd->InvalidateRect(rect);

            CTRLRECT cr;
            cr.nId = nID;
            std::vector<CTRLRECT>::iterator it = find_if(m_vecCtrl.begin(), m_vecCtrl.end(), cr);
            if (it != m_vecCtrl.end()) 
            {//���������иÿؼ��봰�ڱ�ֵ��ֱ�ӳ��ϵ�ǰ���ڴ�С
                rect.left = it->dScale[0] * cx;
                rect.right = it->dScale[1] * cx;
                rect.top = it->dScale[2] * cy;
                rect.bottom = it->dScale[3] * cy;
            }
            else 
            {//û���ҵ��ؼ��ı�ֵ�������
                cr.dScale[0] = (double)rect.left / m_rect.Width();//ע������ת������Ȼ�����long�;�ֱ��Ϊ0��
                cr.dScale[1] = (double)rect.right / m_rect.Width();
                cr.dScale[2] = (double)rect.top / m_rect.Height();
                cr.dScale[3] = (double)rect.bottom / m_rect.Height();
                m_vecCtrl.push_back(cr);
                //�����ؼ���С
				rect.left = cr.dScale[0] * cx;
                rect.right = cr.dScale[1] * cx;
                rect.top = cr.dScale[2] * cy;
                rect.bottom = cr.dScale[3] * cy;
            }

            if (pWnd->IsKindOf(RUNTIME_CLASS(CComboBox))) 
            {
                //���ComboBox���ź�,�޷�����������
                pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height() + 200);
            }
            else 
            {
                pWnd->MoveWindow(rect);//���ÿؼ���С
            }
        }
        hChild = ::GetWindow(hChild, GW_HWNDNEXT);
    }
    m_pParentWnd->GetClientRect(&m_rect);//��ȡ�仯��Ĵ��ڴ�С
}

void CCtrlScale::removeScaleManager()
{
    if (m_pParentWnd)
    {
        std::map<HWND, CCtrlScale*>::iterator itFinder = ms_scaleManagers.find(m_pParentWnd->m_hWnd);
        if (itFinder != ms_scaleManagers.end())
        {
            CCtrlScale* scale = itFinder->second;
            ::SetWindowLongPtr(m_pParentWnd->m_hWnd, GWLP_WNDPROC, (LONG_PTR)scale->m_oldWinProc);
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

	return CallWindowProc(scale->m_oldWinProc, hWnd, Msg, wParam, lParam);
}
