#pragma once
#include <Windows.h>
#include <vector>
#include <map>
#include <set>

typedef struct TAG_CTRLRECT
{
public:
    double dScale[4];//缩放值

    std::map<DWORD/*AnchorType*/, LONG> offsetAnchor;

    TAG_CTRLRECT()
    {
        dScale[0] = 0;
        dScale[1] = 0;
        dScale[2] = 0;
        dScale[3] = 0;
    }
    TAG_CTRLRECT(const TAG_CTRLRECT& cr)
    {
        *this = cr;
    }
} CTRLRECT;

class CCtrlScale
{
public:
    typedef enum AnchorType
    {
		ProportionalScale = 0b0,    //按比例缩放(默认)

        AnchorLeftToWinLeft     = 0b1,       //控件左相对于窗口左的距离不变
        AnchorLeftToWinRight    = 0b10,      //控件左相对于窗口右的距离不变
        AnchorRightToWinLeft    = 0b100,     //控件右相对于窗口左的距离不变
        AnchorRightToWinRight   = 0b1000,    //控件右相对于窗口右的距离不变
        AnchorTopToWinTop       = 0b10000,   //控件顶相对于窗口顶的距离不变
        AnchorTopToWinBottom    = 0b100000,  //控件顶相对于窗口底的距离不变
        AnchorBottomToWinTop    = 0b1000000, //控件底相对于窗口顶的距离不变
        AnchorBottomToWinBottom = 0b10000000,//控件底相对于窗口底的距离不变
    } AnchorType;

    bool HasAnchorType(DWORD/*CCtrlScale::AnchorType*/ flags, CCtrlScale::AnchorType rectType);

public:
    CCtrlScale(void);
    CCtrlScale(HWND pParentWnd);
    virtual ~CCtrlScale(void);

    void SetAnchor(int ctrlId, DWORD/*CCtrlScale::AnchorType*/ rectType);
    void AddExclude(int ctrlId);
    void RemoveExclude(int ctrlId);
    BOOL Init(HWND pParentWnd);
    
    void Reset();

protected:
    static std::map<HWND, CCtrlScale*> ms_scaleManagers;
    void removeScaleManager();

    WNDPROC m_originalWinProc;
    static LRESULT CALLBACK ParentWinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    void Scale(int cx, int cy);

private:
    HWND m_pParentWnd;
    RECT m_rectWin;

    std::map<int, CTRLRECT> m_ctrlRect;//保存控件缩放信息
    std::map<int, DWORD> m_ctrlAnchorType;
    std::set<int> m_ctrlExclude;
};