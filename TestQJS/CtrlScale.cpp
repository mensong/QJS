#include "pch.h"
#include "CtrlScale.h"


CCtrlScale::CCtrlScale(void) :m_pParentWnd(NULL)
{
}

CCtrlScale::CCtrlScale(CWnd* pParentWnd) :m_pParentWnd(pParentWnd)
{
    Init(pParentWnd);
}

CCtrlScale::~CCtrlScale(void)
{

    m_vecCtrl.clear();
}

void CCtrlScale::Init(CWnd* pParentWnd)
{
    if (pParentWnd) {
        m_pParentWnd = pParentWnd;
        m_pParentWnd->GetClientRect(&m_rect); //获取对话框的大小
    }
}

void CCtrlScale::Scale(int cx, int cy)
{
    if (!m_pParentWnd) {
        return;
    }

    //判定是否为最小状态，是的话就不进行重绘
    if (cx == 0 || cy == 0) {
        return;
    }

    //获取控件得到大小以及原大小方便进行比例调整
    CWnd* pWnd;
    int nID = 0;
    //列出所有控件
    HWND hChild = ::GetWindow(m_pParentWnd->m_hWnd, GW_CHILD);
    while (hChild) {
        nID = ::GetDlgCtrlID(hChild);//获得控件ID
        pWnd = m_pParentWnd->GetDlgItem(nID);//获取控件指针
        if (pWnd) {
            CRect rect;//获取当前控件的大小
            pWnd->GetWindowRect(&rect);
            m_pParentWnd->ScreenToClient(&rect);//将控件大小转换为在窗口的区域坐标
            //防止控件失真
            m_pParentWnd->InvalidateRect(rect);

            CTRLRECT cr;
            cr.nId = nID;
            std::vector<CTRLRECT>::iterator it = find_if(m_vecCtrl.begin(), m_vecCtrl.end(), cr);
            if (it != m_vecCtrl.end()) {//如果保存的有该控件与窗口比值，直接乘上当前窗口大小
                rect.left = it->dScale[0] * cx;
                rect.right = it->dScale[1] * cx;
                rect.top = it->dScale[2] * cy;
                rect.bottom = it->dScale[3] * cy;
            }
            else {//没有找到控件的比值，则加入
                cr.dScale[0] = (double)rect.left / m_rect.Width();//注意类型转换，不然保存成long型就直接为0了
                cr.dScale[1] = (double)rect.right / m_rect.Width();
                cr.dScale[2] = (double)rect.top / m_rect.Height();
                cr.dScale[3] = (double)rect.bottom / m_rect.Height();
                m_vecCtrl.push_back(cr);
                //调整控件大小
				rect.left = cr.dScale[0] * cx;
                rect.right = cr.dScale[1] * cx;
                rect.top = cr.dScale[2] * cy;
                rect.bottom = cr.dScale[3] * cy;
            }

            if (pWnd->IsKindOf(RUNTIME_CLASS(CComboBox))) {
                //解决ComboBox缩放后,无法下拉的问题
                pWnd->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height() + 200);
            }
            else {
                pWnd->MoveWindow(rect);//设置控件大小
            }
        }
        hChild = ::GetWindow(hChild, GW_HWNDNEXT);
    }
    m_pParentWnd->GetClientRect(&m_rect);//获取变化后的窗口大小
}