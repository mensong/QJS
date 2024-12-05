/** 
 * @file   	ListCtrlCustom.h
 * @brief  	带下拉框的列表控件
 * ------------------------------------------------------------
 * 版本历史       注释                日期
 * ------------------------------------------------------------
 * @version v1.0  初始版本            7/8/2014
 * ------------------------------------------------------------
 * Note:    
 */
			
#pragma once
#include <afxcmn.h>

class CListCtrlCustom;

//控件调整回调函数
//返回TRUE:表示已经在LPFUNC_SETTING_RECT做了调整处理；
//返回FALSE:表示根据inOutRcGrid并采用默认的处理方法（填满的方式）
typedef BOOL (CALLBACK *LPFUNC_SETTING_RECT)(CListCtrlCustom *pListCtrl, CWnd *pCtrl, CRect &inOutRcGrid);

/** 
 * @class  	SDCtrlComboboxListctrl
 * @brief  	带下拉框的列表控件
 * ------------------------------------------------------------
 * 版本历史       注释                日期
 * ------------------------------------------------------------
 * @version v1.0  初始版本            7/8/2014
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
	 * @brief     	获取某行某列的附加控件
	 * @param       nRow - 行号
	 * @param     	nCol - 列号
	 * @retval    	NULL:此行列没有附加控件
	 * @note      	
	 */
	CWnd *GetCtrl(int nRow, int nCol);
	/**
	 * @brief     	获得某行某列的文本
	 * @param       nRow - 行号
	 * @param     	nCol - 列号
	 * @retval    	
	 * @note        如果该行列有额外控件则获得此控件的文本，如果没有额外控件则获取该行列的文本
	 */
	CString GetText(int nRow, int nCol);
	/** 
	 * @brief     	设置格子内容
	 * @param       nItem - 行
	 * @param     	nSubItem - 列
	 * @param     	pExCtrl - 附加控件（自动删除）
	 * @retval    	TRUE:成功；FALSE: 失败
	 * @note      	
	 */
	BOOL SetItemEx(int nItem, int nSubItem, CWnd *pExCtrl);
	/**
	 * @brief     	设置行高
	 * @param       nHeight - 行高
	 * @retval    	TRUE:成功；FALSE: 失败
	 * @note        
	 */
	BOOL SetRowHeight(int nHeight);

	/**
	 * @brief     	获得当前鼠标下的行列号
	 * @param       row - 返回行号
	 * @param       col - 返回列号
	 * @retval    	TRUE:成功；FALSE: 失败
	 * @note  
	*/
	BOOL GetItemMouseAbove(int& row, int& col);

public:
	//设置附加控件调整回调函数
	void RegOnSettingRect(LPFUNC_SETTING_RECT pfnResizeExCtrl)
		{ m_pfnResizeExCtrl = pfnResizeExCtrl; }

protected:
	//自定义控件的位置调整
	void _updateExCtrlsPos();
	//清除所有自定义控件
	void _clearExCtrls();
	//获得listctrl格子的范围
	//为什么不直接使用GetSubItemRect呢，那是因为GetSubItemRect对于
	//第一列获取的时候，获取到的宽度会是整行的宽度，所以这里需要修正
	BOOL _getGridRect(int nRow, int nCol, CRect &rect);

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnLvnEndScroll(NMHDR *pNMHDR, LRESULT *pResult);//无论横还是竖的滚动，都会进这个函数
	DECLARE_MESSAGE_MAP()

protected:
	//附加控件列表
	typedef CArray<CWnd *> COLUMN_INFO;
	typedef COLUMN_INFO *LP_COLUMN_INFO;
	typedef CArray<LP_COLUMN_INFO> ROW_EX_CTRL_INFO;
	ROW_EX_CTRL_INFO m_arrExCtrls;

	//调整控件回调函数
	LPFUNC_SETTING_RECT m_pfnResizeExCtrl;
	//用于设置行高的图标列表
	CImageList m_lstImgForRowHeight;

};

