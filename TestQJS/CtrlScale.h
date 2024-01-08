#pragma once
#include <vector>

typedef struct TAG_CTRLRECT
{
public:
    int nId;//控件ID
    double dScale[4];//缩放值
    TAG_CTRLRECT()
    {
        nId = 0;//未定义
        dScale[0] = 0;
        dScale[1] = 0;
        dScale[2] = 0;
        dScale[3] = 0;
    }
    TAG_CTRLRECT(const TAG_CTRLRECT& cr)
    {
        *this = cr;
    }
    bool operator () (const TAG_CTRLRECT& cr) {
        return (cr.nId == nId);
    }
} CTRLRECT;

class CCtrlScale
{
public:
    CCtrlScale(void);
    CCtrlScale(CWnd* pParentWnd);
    virtual ~CCtrlScale(void);

    void Init(CWnd* pParentWnd);
    void Scale(int cx, int cy);

private:
    CWnd* m_pParentWnd;
    CRect m_rect;
    std::vector<CTRLRECT> m_vecCtrl;//保存控件缩放信息
};