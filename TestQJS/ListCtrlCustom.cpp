#include "pch.h"
#include "ListCtrlCustom.h"

IMPLEMENT_DYNAMIC(CListCtrlCustom, CListCtrl)

CListCtrlCustom::CListCtrlCustom(void)
	: m_pfnResizeExCtrl(NULL)
{
}

CListCtrlCustom::~CListCtrlCustom(void)
{
	_clearExCtrls();
	m_lstImgForRowHeight.DeleteImageList();
}

BEGIN_MESSAGE_MAP(CListCtrlCustom, CListCtrl)
	//{{AFX_MSG_MAP(SDCtrlComboboxListctrl)
	ON_NOTIFY_REFLECT(LVN_ENDSCROLL, &CListCtrlCustom::OnLvnEndScroll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

LRESULT CListCtrlCustom::DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT lRet = CListCtrl::DefWindowProc(message, wParam, lParam);
	LVITEM *pItem = (LVITEM *)lParam;

	switch (message)
	{
	case LVM_INSERTITEM:
		{
			int nCurRow = (int)lRet;		//当前行号
			LP_COLUMN_INFO pArrColumnCreatedCtrls = new COLUMN_INFO;
			int nColumnCount = GetHeaderCtrl()->GetItemCount();
			for (int i=0; i<nColumnCount; ++i)
			{
				pArrColumnCreatedCtrls->Add(NULL);
			}
			m_arrExCtrls.InsertAt(nCurRow, pArrColumnCreatedCtrls);//如果此行没有自定义控件的话，则插入一个NULL
			if (NULL!=pArrColumnCreatedCtrls)
			{
				_updateExCtrlsPos();
			}
		}
		break;

	case LVM_DELETEITEM:
		{
			int nCurRow = (int)wParam;		//当前行号

			LP_COLUMN_INFO pColInfo = m_arrExCtrls[nCurRow];
			if (NULL==pColInfo)
			{
				break;
			}
			int nCtrls = (int)pColInfo->GetCount();
			for (int i=0; i<nCtrls; ++i)
			{
				CWnd *pCtrl = pColInfo->GetAt(i);
				if (NULL==pCtrl)
				{
					continue;
				}

				pCtrl->DestroyWindow();
				delete (pCtrl);
			}
			delete pColInfo;
			m_arrExCtrls.RemoveAt(nCurRow);

			_updateExCtrlsPos();
		}
		break;

	case LVM_DELETEALLITEMS:
		{
			_clearExCtrls();
		}
		break;

	case LVM_INSERTCOLUMN:
		{
			int nInsertColumn = (int)wParam;
			int nRowCount = (int)m_arrExCtrls.GetCount();
			for (int i=0; i<nRowCount; ++i)
			{
				LP_COLUMN_INFO pColInfo = m_arrExCtrls[i];
				if (NULL == pColInfo)
				{
					continue;
				}

				pColInfo->InsertAt(nInsertColumn, (CWnd *)NULL);
			}

			_updateExCtrlsPos();
		}
		break;

	case LVM_DELETECOLUMN:
		{
			int nRemoveColumn = (int)wParam;
			int nRowCount = (int)m_arrExCtrls.GetCount();
			for (int i=0; i<nRowCount; ++i)
			{
				LP_COLUMN_INFO pColInfo = m_arrExCtrls[i];
				if (NULL == pColInfo)
				{
					continue;
				}
				if (nRemoveColumn < pColInfo->GetCount())
				{
					//删除列的时候，如果有自定控件则需要删除自定义控件
					CWnd *pCtrl = pColInfo->GetAt(nRemoveColumn);
					if (NULL!=pCtrl)
					{
						pCtrl->DestroyWindow();
						delete (pCtrl);
					}
					pColInfo->RemoveAt(nRemoveColumn);
				}
			}

			_updateExCtrlsPos();
		}
		break;
	}

	return lRet;
}

BOOL CListCtrlCustom::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	NMHDR *pNMHDR = (NMHDR *)lParam;
	switch (pNMHDR->code)
	{
		//拖动和双击都会引起表头宽度的变化
	case HDN_BEGINTRACK:	//开始拖动列头
		{
			//隐藏掉全部控件
			int nRows = (int)m_arrExCtrls.GetCount();
			for (int i=0; i<nRows; ++i)
			{
				LP_COLUMN_INFO pColInfo = m_arrExCtrls[i];
				if (NULL==pColInfo)
				{
					continue;
				}
				int nCtrls = (int)pColInfo->GetCount();
				for (int i=0; i<nCtrls; ++i)
				{
					CWnd *pCtrl = pColInfo->GetAt(i);
					if (NULL==pCtrl)
					{
						continue;
					}

					pCtrl->ShowWindow(SW_HIDE);
				}
			}
		}
		break;
	case HDN_ENDTRACK:			//拖动列头
		{
			_updateExCtrlsPos();
			RedrawWindow();//防止拖动列（列由大拖到小）的时候，控件不及时刷新
		}
		break;
	case HDN_DIVIDERDBLCLICK:	//双击列头
		{
			//禁止双击表头分割线调整列的宽度
			*pResult = TRUE;
			return TRUE;
		}
		break;
	}

	return CListCtrl::OnNotify(wParam, lParam, pResult);
}

void CListCtrlCustom::OnLvnEndScroll( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMLVSCROLL pStateChanged = reinterpret_cast<LPNMLVSCROLL>(pNMHDR);
	_updateExCtrlsPos();
	*pResult = 0;
}

void CListCtrlCustom::_updateExCtrlsPos()
{
	int nTopRowIndex = GetTopIndex();
	int nRowCount = GetItemCount();

	//隐藏掉全部控件
	int nRows = (int)m_arrExCtrls.GetCount();
	for (int i=0; i<nRows; ++i)
	{
		LP_COLUMN_INFO pColInfo = m_arrExCtrls[i];
		if (NULL==pColInfo)
		{
			continue;
		}
		int nCtrls = (int)pColInfo->GetCount();
		for (int i=0; i<nCtrls; ++i)
		{
			CWnd *pCtrl = pColInfo->GetAt(i);
			if (NULL==pCtrl)
			{
				continue;
			}

			pCtrl->ShowWindow(SW_HIDE);
		}
	}

	//获得listctrl的范围
	CRect rectWin;
	GetWindowRect(&rectWin);
	ScreenToClient(&rectWin);

	//如果控件的位置在listctrl的外面，则不需要显视和调整位置
	//重新调整位置
	//判断高度是否退出
	bool bExit = false;
	int nRowMove=nTopRowIndex;
	for (; nRowMove<nRowCount; ++nRowMove)
	{
		//重新匹配
		LP_COLUMN_INFO pColInfo = m_arrExCtrls[nRowMove];
		if (NULL==pColInfo)
		{
			continue;
		}
		int nCtrls = (int)pColInfo->GetCount();
		for (int nColumn=0; nColumn<nCtrls; ++nColumn)
		{
			CWnd *pCtrl = pColInfo->GetAt(nColumn);
			if (NULL==pCtrl)
			{
				continue;
			}

			CRect rectGrid;
			//GetSubItemRect(nRowMove, nColumn, LVIR_BOUNDS, rectGrid);
			_getGridRect(nRowMove, nColumn, rectGrid);
			if (rectGrid.top>rectWin.bottom)
			{
				bExit = true;
				break;
			}

			pCtrl->ShowWindow(SW_SHOW);
			if (NULL==m_pfnResizeExCtrl 
				|| m_pfnResizeExCtrl(this, pCtrl, rectGrid) == FALSE)
			{//填满
				pCtrl->MoveWindow(&rectGrid);	
			}
		}

		if (bExit)
		{
			break;
		}
	}

	//RedrawWindow();
}

BOOL CListCtrlCustom::Create( _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ UINT nID )
{
	BOOL bRet = CListCtrl::Create(dwStyle, rect, pParentWnd, nID);

	//强制使用report方式
	LONG lStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	lStyle |= LVS_REPORT;
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, lStyle);

	return bRet;
}

BOOL CListCtrlCustom::CreateEx( _In_ DWORD dwExStyle, _In_ DWORD dwStyle, _In_ const RECT& rect, _In_ CWnd* pParentWnd, _In_ UINT nID )
{
	BOOL bRet = CListCtrl::CreateEx(dwExStyle, dwStyle, rect, pParentWnd, nID);

	//强制使用report方式
	LONG lStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	lStyle |= LVS_REPORT;
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, lStyle);

	return bRet;
}

void CListCtrlCustom::_clearExCtrls()
{
	int nRows = (int)m_arrExCtrls.GetCount();
	for (int i=0; i<nRows; ++i)
	{
		LP_COLUMN_INFO pColInfo = m_arrExCtrls[i];
		if (NULL==pColInfo)
		{
			continue;
		}
		int nCtrls = (int)pColInfo->GetCount();
		for (int i=0; i<nCtrls; ++i)
		{
			CWnd *pCtrl = pColInfo->GetAt(i);
			if (NULL==pCtrl)
			{
				continue;
			}

			pCtrl->DestroyWindow();
			delete (pCtrl);
		}
		delete pColInfo;
	}
	m_arrExCtrls.RemoveAll();
}

/** 
* @brief     	获取某行谋列的附加控件
* @param        nRow - 行号
* @param     	nCol - 列号
* @retval    	NULL:此行列没有附加控件
* @note      	
*/
CWnd * CListCtrlCustom::GetCtrl( int nRow, int nCol )
{
	LP_COLUMN_INFO pColumnInfo = m_arrExCtrls[nRow];
	if (NULL == pColumnInfo)
	{
		return NULL;
	}

	if (nCol < pColumnInfo->GetCount())
	{
		return pColumnInfo->GetAt(nCol);
	}

	return NULL;
}

BOOL CListCtrlCustom::_getGridRect( int nRow, int nCol, CRect &rect )
{
	BOOL bRet = GetSubItemRect(nRow, nCol, LVIR_BOUNDS, rect);
	//对于第一列，需要特殊处理
	if (TRUE==bRet &&  0==nCol)
	{
		int nWidth0 = GetColumnWidth(0);
		rect.right = rect.left + nWidth0;
	}
	return bRet;
}

CString CListCtrlCustom::GetText( int nRow, int nCol )
{
	//如果存在附加控件则返回附加控件的文本，如果没有附加控件则直接返回格子的值
	CWnd *pCtrl = GetCtrl(nRow, nCol);
	if (NULL == pCtrl)
	{
		return GetItemText(nRow, nCol);
	}

	CString sText;
	pCtrl->GetWindowText(sText);
	return sText;
}

/** 
* @brief     	设置格子内容
* @param        nItem - 行
* @param     	nSubItem - 列
* @param     	pExCtrl - 附加控件（自动删除）
* @retval    	TRUE:成功；FALSE: 失败
* @note      	
*/
BOOL CListCtrlCustom::SetItemEx( int nItem, int nSubItem, CWnd *pExCtrl )
{
	//获得列的数量
	int nColumnCount = GetHeaderCtrl()->GetItemCount();
	if (nItem >= GetItemCount()			//如果行不够
		|| nSubItem >= nColumnCount		//如果列不够
		)
	{
		return FALSE;
	}

	//为每个格子申请仓库
	LP_COLUMN_INFO &pColumnInfo = m_arrExCtrls[nItem];
	if (NULL == pColumnInfo)
	{
		pColumnInfo = new COLUMN_INFO;
		for (int i=0; i<nColumnCount; ++i)
		{
			pColumnInfo->Add(NULL);	
		}
	}
	else
	{//如果原来的格子里已经有控件，则需要把原来的控件删除了
		if (pColumnInfo->GetCount() > nSubItem)
		{
			CWnd *pCtrlOld = pColumnInfo->GetAt(nSubItem);
			if (NULL != pCtrlOld)
			{
				delete pCtrlOld;
			}
		}
	}

	//设置符加控件到仓库
	pColumnInfo->SetAt(nSubItem, pExCtrl);

	//设置父窗口为list control
	pExCtrl->SetParent(this);

	_updateExCtrlsPos();

	return TRUE;
}

BOOL CListCtrlCustom::SetRowHeight( int nHeight )
{
	m_lstImgForRowHeight.DeleteImageList();
	m_lstImgForRowHeight.Create(1, nHeight, ILC_COLOR32, 1, 0);
	if (SetImageList(&m_lstImgForRowHeight, LVSIL_SMALL) == NULL)
	{
		return FALSE;
	}

	_updateExCtrlsPos();

	//目的是让滚动条出来（粗糙的方法）
	//DeleteItem(InsertItem(0, _T("")));
	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		ShowWindow(SW_SHOW);
	}

	return TRUE;
}

BOOL CListCtrlCustom::GetItemMouseAbove(int& row, int& col)
{
	row = -1;
	col = -1;

	LVHITTESTINFO hitTestInfo;
	BOOL b = ::GetCursorPos(&hitTestInfo.pt); // 获取当前鼠标位置
	if (b == FALSE)
		return FALSE;

	ScreenToClient(&hitTestInfo.pt); // 转换到列表控件的客户区坐标
	//int rowId = HitTest(&hitTestInfo);
	int rowId = SubItemHitTest(&hitTestInfo);
	if (rowId == -1)
		return FALSE;

	row = hitTestInfo.iItem;
	col = hitTestInfo.iSubItem;
	return TRUE;
}

