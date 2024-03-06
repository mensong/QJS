#pragma once
#include <vector>
#include <map>

typedef struct TAG_CTRLRECT
{
public:
    int nId;//控件ID
    double dScale[4];//缩放值

    LONG rightOffset;
    LONG bottomOffset;

    TAG_CTRLRECT()
    {
        nId = 0;//未定义
        dScale[0] = 0;
        dScale[1] = 0;
        dScale[2] = 0;
        dScale[3] = 0;

        rightOffset = 0;
        bottomOffset = 0;
    }
    TAG_CTRLRECT(const TAG_CTRLRECT& cr)
    {
        *this = cr;
    }
    bool operator () (const TAG_CTRLRECT& cr)
    {
        return (cr.nId == nId);
    }
} CTRLRECT;

class CCtrlScale
{
public:
    typedef enum RectType
    {
		ProportionalScale = 0b0,    //按比例缩放(默认)

		StaticLeft = 0b1,      //控件左不动
		StaticRight = 0b10,     //控件右不动
		StaticTop = 0b100,    //控件顶不动
		StaticBottom = 0b1000,   //控件底不动

		AnchorRight = 0b10000,  //控件右相对于窗口右的距离不变
		AnchorBottom = 0b100000, //控件底相对于窗口底的距离不变
    } RectType;

public:
    CCtrlScale(void);
    CCtrlScale(HWND pParentWnd);
    virtual ~CCtrlScale(void);

    void SetRectType(int ctrlId, CCtrlScale::RectType rectType);
    BOOL Init(HWND pParentWnd);
    
protected:
    static std::map<HWND, CCtrlScale*> ms_scaleManagers;
    void removeScaleManager();

    WNDPROC m_originalWinProc;
    static LRESULT CALLBACK ParentWinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    void Scale(int cx, int cy);

private:
    HWND m_pParentWnd;
    RECT m_rectWin;
    std::vector<CTRLRECT> m_vecCtrl;//保存控件缩放信息

    std::map<int, DWORD> m_ctrlRectType;
};