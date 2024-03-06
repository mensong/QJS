#pragma once
#include <vector>
#include <map>

typedef struct TAG_CTRLRECT
{
public:
    int nId;//�ؼ�ID
    double dScale[4];//����ֵ

    LONG rightOffset;
    LONG bottomOffset;

    TAG_CTRLRECT()
    {
        nId = 0;//δ����
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
		ProportionalScale = 0b0,    //����������(Ĭ��)

		StaticLeft = 0b1,      //�ؼ��󲻶�
		StaticRight = 0b10,     //�ؼ��Ҳ���
		StaticTop = 0b100,    //�ؼ�������
		StaticBottom = 0b1000,   //�ؼ��ײ���

		AnchorRight = 0b10000,  //�ؼ�������ڴ����ҵľ��벻��
		AnchorBottom = 0b100000, //�ؼ�������ڴ��ڵ׵ľ��벻��
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
    std::vector<CTRLRECT> m_vecCtrl;//����ؼ�������Ϣ

    std::map<int, DWORD> m_ctrlRectType;
};