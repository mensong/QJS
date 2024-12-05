/** 
 * @file   	ListCtrlCustom.h
 * @brief  	����������б�ؼ�
 * ------------------------------------------------------------
 * �汾��ʷ       ע��                ����
 * ------------------------------------------------------------
 * @version v1.0  ��ʼ�汾            7/8/2014
 * ------------------------------------------------------------
 * Note:    
 */
			
#pragma once
#include <afxcmn.h>

class CListCtrlCustom;

//�ؼ������ص�����
//����TRUE:��ʾ�Ѿ���LPFUNC_SETTING_RECT���˵�������
//����FALSE:��ʾ����inOutRcGrid������Ĭ�ϵĴ������������ķ�ʽ��
typedef BOOL (CALLBACK *LPFUNC_SETTING_RECT)(CListCtrlCustom *pListCtrl, CWnd *pCtrl, CRect &inOutRcGrid);

/** 
 * @class  	SDCtrlComboboxListctrl
 * @brief  	����������б�ؼ�
 * ------------------------------------------------------------
 * �汾��ʷ       ע��                ����
 * ------------------------------------------------------------
 * @version v1.0  ��ʼ�汾            7/8/2014
 * ------------------------------------------------------------
 * Note:   	
 */	
class CListCtrlCustom :
	public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlCustom)

public:
	CListCtrlCustom(void);
	virtual ~CListCtrlCustom(void);

	// Generic creator
	BOOL Create(_In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ UINT nID);
	// Generic creator allowing extended style bits
	BOOL CreateEx(_In_ DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect,
		_In_ CWnd* pParentWnd, _In_ UINT nID);

public:
	/** 
	 * @brief     	��ȡĳ��ĳ�еĸ��ӿؼ�
	 * @param       nRow - �к�
	 * @param     	nCol - �к�
	 * @retval    	NULL:������û�и��ӿؼ�
	 * @note      	
	 */
	CWnd *GetCtrl(int nRow, int nCol);
	/**
	 * @brief     	���ĳ��ĳ�е��ı�
	 * @param       nRow - �к�
	 * @param     	nCol - �к�
	 * @retval    	
	 * @note        ����������ж���ؼ����ô˿ؼ����ı������û�ж���ؼ����ȡ�����е��ı�
	 */
	CString GetText(int nRow, int nCol);
	/** 
	 * @brief     	���ø�������
	 * @param       nItem - ��
	 * @param     	nSubItem - ��
	 * @param     	pExCtrl - ���ӿؼ����Զ�ɾ����
	 * @retval    	TRUE:�ɹ���FALSE: ʧ��
	 * @note      	
	 */
	BOOL SetItemEx(int nItem, int nSubItem, CWnd *pExCtrl);
	/**
	 * @brief     	�����и�
	 * @param       nHeight - �и�
	 * @retval    	TRUE:�ɹ���FALSE: ʧ��
	 * @note        
	 */
	BOOL SetRowHeight(int nHeight);

	/**
	 * @brief     	��õ�ǰ����µ����к�
	 * @param       row - �����к�
	 * @param       col - �����к�
	 * @retval    	TRUE:�ɹ���FALSE: ʧ��
	 * @note  
	*/
	BOOL GetItemMouseAbove(int& row, int& col);

public:
	//���ø��ӿؼ������ص�����
	void RegOnSettingRect(LPFUNC_SETTING_RECT pfnResizeExCtrl)
		{ m_pfnResizeExCtrl = pfnResizeExCtrl; }

protected:
	//�Զ���ؼ���λ�õ���
	void _updateExCtrlsPos();
	//��������Զ���ؼ�
	void _clearExCtrls();
	//���listctrl���ӵķ�Χ
	//Ϊʲô��ֱ��ʹ��GetSubItemRect�أ�������ΪGetSubItemRect����
	//��һ�л�ȡ��ʱ�򣬻�ȡ���Ŀ�Ȼ������еĿ�ȣ�����������Ҫ����
	BOOL _getGridRect(int nRow, int nCol, CRect &rect);

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnLvnEndScroll(NMHDR *pNMHDR, LRESULT *pResult);//���ۺỹ�����Ĺ�����������������
	DECLARE_MESSAGE_MAP()

protected:
	//���ӿؼ��б�
	typedef CArray<CWnd *> COLUMN_INFO;
	typedef COLUMN_INFO *LP_COLUMN_INFO;
	typedef CArray<LP_COLUMN_INFO> ROW_EX_CTRL_INFO;
	ROW_EX_CTRL_INFO m_arrExCtrls;

	//�����ؼ��ص�����
	LPFUNC_SETTING_RECT m_pfnResizeExCtrl;
	//���������иߵ�ͼ���б�
	CImageList m_lstImgForRowHeight;

};

