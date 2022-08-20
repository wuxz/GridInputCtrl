// Grid.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "Grid.h"
#include "GridEditCtrl.h"
#include "GridControl.h"
#include "GridComboCtrl.h"
#include "GridDateMaskCtrl.h"
#include "GridCheckBoxCtrl.h"
#include "GridEditBtnCtrl.h"
#include "GridNumCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMERINTERVAL 50
#define RESIZECAPTURERANGE 3 // When resizing Cols/row, the cursor has to be 
									// within +/-3 pixels of the dividing line for 
									// resizing to be possible

/////////////////////////////////////////////////////////////////////////////
// CGrid

CGrid::CGrid() : m_rtGrid(0, 0, 1, 1)
{
	RegisterWindowClass();

	m_ImageList.Create(MAKEINTRESOURCE(IDB_HEADER), 12, 1, RGB(255, 255, 255));

	m_nMouseMode = MOUSE_NOTHING;

	m_bReadOnly = m_bListMode = m_bAllowAddNew = FALSE;
	m_bAddNewMode = FALSE;
	m_bAllowDelete = TRUE;

	m_bAllowRedraw = TRUE;
	
	m_nLevels = 1;
	m_nRows = 1;
	m_nCols = 1;
	m_nGroups = 0;
	m_nVScrollMax = m_nHScrollMax = 0;
	m_nRow = m_nCol = 0;

	m_nGroupsUsed = m_nColsUsed = m_nRowsUsed = 0;
	m_nRowHeight = m_nLevelHeight = 20;
	m_nHeaderHeight = m_nHeaderLevelHeight = 20;
	m_nFrozenRows = m_nFrozenCols = m_nFrozenGroups = 0;
	m_bAllowColResize = m_bAllowRowResize = m_bAllowGroupResize = TRUE;

	m_bShowRowHeader = m_bShowGroupHeader = m_bShowColHeader = TRUE;
	m_bAllowMoveCol = m_bAllowMoveGroup = TRUE;
	m_bAllowSelectRow = TRUE;

	m_bAlreadyInsideOnSize = FALSE;

	m_nSelectMode = SELECTMODE_NONE;

	m_nTimerID = 0;

	m_pEditCtrl = NULL;
	m_pComboCtrl = m_pComboListCtrl = NULL;
	m_pDateCtrl = NULL;
	m_pCheckBoxCtrl = NULL;
	m_pEditBtnCtrl = NULL;
	m_pButtonCtrl = NULL;
	m_pNumCtrl = NULL;

	m_bIPEdit = FALSE;
	m_bRowDirty = FALSE;

	m_clrFore = GetSysColor(COLOR_WINDOWTEXT);
	m_clrBack = GetSysColor(COLOR_WINDOW); 
	m_clrHeaderFore = GetSysColor(COLOR_WINDOWTEXT);
	m_clrHeaderBack = GetSysColor(COLOR_3DFACE);

	m_nDividerType = DividerTypeBoth;
	m_nDividerStyle = PS_SOLID;
	m_clrDivider = RGB(0, 0, 0);

	m_nDefColWidth = 100;

	m_nMargin = m_nHeaderMargin = 0;
}

CGrid::~CGrid()
{
	// Kill the timer.
	KillTimer();

	// Clear row index array.
	m_arRowIndex.RemoveAll();

	// Clear groups array.
	for (int i = 0; i < m_arGroups.GetSize(); i ++)
		delete m_arGroups[i];

	m_arGroups.RemoveAll();
	
	// Clear cells array.
	for (i = 0; i < m_arCells.GetSize(); i ++)
		delete m_arCells[i];

	m_arCells.RemoveAll();
	
	// Clear cols array.
	for (i = 0; i < m_arCols.GetSize(); i ++)
		delete m_arCols[i];

	m_arCols.RemoveAll();

	// Clear selection array.
	m_arSelection.RemoveAll();

	// Clear font objects.
	m_fntCell.DeleteObject();
	m_fntHeader.DeleteObject();

	if (m_pEditCtrl)
	{
		// Clear inplace edit window.
		m_pEditCtrl->DestroyWindow();
		delete m_pEditCtrl;
	}

	// Clear col objects.
	if (m_pComboCtrl)
	{
		m_pComboCtrl->DestroyWindow();
		delete m_pComboCtrl;
	}

	if (m_pComboListCtrl)
	{
		m_pComboListCtrl->DestroyWindow();
		delete m_pComboListCtrl;
	}

	if (m_pDateCtrl)
	{
		m_pDateCtrl->DestroyWindow();
		delete m_pDateCtrl;
	}

	if (m_pCheckBoxCtrl)
	{
		m_pCheckBoxCtrl->DestroyWindow();
		delete m_pCheckBoxCtrl;
	}

	if (m_pEditBtnCtrl)
	{
		m_pEditBtnCtrl->DestroyWindow();
		delete m_pEditBtnCtrl;
	}

	if (m_pButtonCtrl)
	{
		m_pButtonCtrl->DestroyWindow();
		delete m_pButtonCtrl;
	}
}


BEGIN_MESSAGE_MAP(CGrid, CWnd)
	//{{AFX_MSG_MAP(CGrid)
	ON_WM_PAINT()
	ON_WM_GETDLGCODE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_CAPTURECHANGED()
	ON_WM_CHAR()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGrid message handlers


void CGrid::RegisterWindowClass()
/*
Routine Description:
	Register our own window class.

Parameters:
	None.

Return Value:
	None.
*/
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, BHMGRID_CLASSNAME, &wndcls)))
	{
		// We need to register a new class
		wndcls.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc		= ::DefWindowProc;
		wndcls.cbClsExtra		= wndcls.cbWndExtra = 0;
		wndcls.hInstance		= hInst;
		wndcls.hIcon			= NULL;
		wndcls.hCursor			= AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground	= NULL;
		wndcls.lpszMenuName 	= NULL;
		wndcls.lpszClassName	= BHMGRID_CLASSNAME;

		if (!AfxRegisterClass(&wndcls)) 
			AfxThrowResourceException();
	}
}

void CGrid::OnPaint()
{
	// Should allow redraw.
	if (!m_bAllowRedraw)
		return;

	CPaintDC dc(this); // Device context for painting.
	
	GetClientRect(&m_rtGrid);

	// Call the helper function.
	OnGridDraw(&dc);
}

UINT CGrid::OnGetDlgCode() 
{
	UINT nCode = DLGC_WANTALLKEYS;

	return nCode;
}

BOOL CGrid::RedrawCell(int nRow, int nCol, CDC *pDC)
/*
Routine Description:
	Redraw one cell.

Parameters:
	nRow		The row ordinal.

	nCol		The col ordinal.

	pDC		The DC pointer.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Should allow redraw.
	if (!m_bAllowRedraw)
		return FALSE;

	ASSERT(IsValidCell(nRow, nCol));

	// Should we release DC after drawing?
	BOOL bMustReleaseDC = FALSE;

	// If cell is invisible, do not draw it.
	if (!IsCellVisible(nRow, nCol))
		return FALSE;

	// Get the cell rect.
	CRect rtCell;
	if (!GetCellRect(nRow, nCol, rtCell))
		return FALSE;
	
	if (!pDC)
	{
		// If have no DC, get it by myself.
		pDC = GetDC();
		if (pDC)
		// Should release this DC.
			bMustReleaseDC = TRUE;
		else
			return FALSE;
	}

	if (nCol == 0)
	// This cell is a row header.
		DrawRowHeaderCell(nRow, rtCell, pDC);
	else if (nRow == 0)
	// This cell is a col header.
		DrawColHeaderCell(nRow, nCol, rtCell, pDC);
	else
	// This cell is a content cell.
		DrawCell(nRow, nCol, rtCell, pDC);

	// Release DC.
	if (bMustReleaseDC) 
		ReleaseDC(pDC);

	return TRUE;
}

BOOL CGrid::GetCellRect(int nCellRow, int nCellCol, CRect& rtCell)
/*
Routine Description:
	Get the rect one cell occupies.

Parameters:
	nCellRow		The row ordinal.

	nCellCol		The col ordinal.

	rtCell			The result rect.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(IsValidCell(nCellRow, nCellCol));

	// The topleft point.
	POINT ptGroup, ptStart = m_rtGrid.TopLeft();

	// The level object.
	Level * pLevel;

	// The group object.
	Group * pGroup;

	int nLeftNonFixedGroup, nLeftNonFixedCol, nTopNonFixedRow;
	
	if (IsSingleLevelMode())
	// Have no groups.
	// Get the top left non fixed cell.
		GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
	else
	// Have groups.
	// Get the top left non fixed group.
		GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	if (nCellRow > m_nFrozenRows && nCellRow < nTopNonFixedRow)
	// The cell is invisible.
		return FALSE;

	int nColInGroup;;
	int nGroup;

	if (IsSingleLevelMode())
	{
		// The cell is invisible.
		if (nCellCol > 0 && ((nCellCol > m_nFrozenCols && nCellCol < nLeftNonFixedCol) || !m_arCols[nCellCol - 1]->bVisible))
			return FALSE;
	}
	else
	{
		nColInGroup = nCellCol - 1;
		nGroup;
		
		if (nCellCol > 0)
		{
			nGroup = GetGroupFromCol(nColInGroup);
			if (nGroup == INVALID || (nGroup >= m_nFrozenGroups && nGroup < nLeftNonFixedGroup) || !m_arGroups[nGroup]->bVisible)
			// The group is invisible, so the cell is invisible too.
				return FALSE;
		}
	}

	ptGroup = ptStart;

	if (nCellRow >= nTopNonFixedRow)
	{
		// The cell is non fixed.

		// Add the vertical coordination with the height of fixed rows height.
		nCellRow -= nTopNonFixedRow - 1;
		ptGroup.y += GetFixedRowHeight();
	}
	else if (nCellRow > 0)
		// The cell is fixed.

		// Add the vertical coordination with the height of header row height.
		ptGroup.y += GetVirtualHeaderHeight();

	// Add the vertical coordination with the height of non fixed rows height.
	ptGroup.y += nCellRow ? (nCellRow - 1) * m_nRowHeight : 0;

	if (nCellCol == 0)
	{
		// It is row header cell.
		rtCell.left = ptGroup.x;
		rtCell.right = rtCell.left + (m_bShowRowHeader ? 22 : 0);
		rtCell.top = ptGroup.y;
		rtCell.bottom = rtCell.top + GetRowHeight(nCellRow) + (nCellRow == 0 && m_nGroups && m_bShowGroupHeader ? m_nHeaderLevelHeight : 0);

		return TRUE;
	}
	else if (nCellRow == 0)
		// It is group header or col header cell.
		ptGroup.y += m_nGroups && m_bShowGroupHeader ? m_nHeaderLevelHeight : 0;

	if (!IsSingleLevelMode())
		nCellCol = nColInGroup + 1;

	int i;

	if (IsSingleLevelMode())
	{
		// Have no group.

		if (nCellCol <= m_nFrozenCols)
		{
			// This cell is fixed.

			// Add the width of row header.
			ptGroup.x += (m_bShowRowHeader ? 22 : 0);

			// Calc the group's left border.
			for (i = 0; i < nCellCol; i ++)
				ptGroup.x += m_arCols[i]->bVisible ? m_arCols[i]->nWidth : 0; 
		}
		else
		{
			// This cell is not fixed.

			// Plus fixed col width.
			ptGroup.x += GetFixedColWidth();
			
			for (i = nLeftNonFixedCol; i < nCellCol; i ++)
				ptGroup.x += m_arCols[i - 1]->bVisible ? m_arCols[i - 1]->nWidth : 0; 
		}

		rtCell.left = ptGroup.x;
		rtCell.top = ptGroup.y;
		rtCell.right = rtCell.left + m_arCols[nCellCol - 1]->nWidth;
		rtCell.bottom = rtCell.top + (nCellRow ? m_nLevelHeight : m_nHeaderLevelHeight);

		return TRUE;
	}

	if (nGroup < m_nFrozenGroups)
	{
		ptGroup.x += (m_bShowRowHeader ? 22 : 0);
		for (i = 0; i < nGroup; i ++)
			ptGroup.x += m_arGroups[i]->bVisible ? m_arGroups[i]->nWidth : 0; 
	}
	else
	{
		ptGroup.x += GetFixedColWidth();
		
		for (i = nLeftNonFixedGroup; i < nGroup; i ++)
			ptGroup.x += m_arGroups[i]->bVisible ? m_arGroups[i]->nWidth : 0; 
	}
	
	int nCol = 1;
	
	pGroup = m_arGroups[nGroup];
	
	for (int nLevel = 0; nLevel < m_nLevels; nLevel ++)
	{
		rtCell.left = ptGroup.x;
		rtCell.top = ptGroup.y + nLevel * (nCellRow ? m_nLevelHeight : m_nHeaderLevelHeight);
		rtCell.bottom = rtCell.top + (nCellRow ? m_nLevelHeight : m_nHeaderLevelHeight);
		
		pLevel = pGroup->arLevels[nLevel];
		
		for (int nColIndex = 0; nColIndex < pLevel->nCols; nColIndex ++)
		{
			int nColOrd = GetColFromLevel(nGroup, nLevel, nColIndex);
			
			rtCell.right = rtCell.left + (m_arCols[nColOrd]->bVisible ? m_arCols[nColOrd]->nWidth : 0);
			if (nCol == nCellCol)
				return TRUE;
			
			if (nGroup < m_nFrozenGroups || nGroup >= nLeftNonFixedGroup)
				rtCell.left = rtCell.right;
			
			nCol ++;
		}
	}

	return FALSE;
}

BOOL CGrid::IsCellVisible(int nRow, int nCol)
/*
Routine Description:
	Get if is one cell is visible.

Parameters:
	nRow

	nCol

Return Value:
	None.
*/
{
	if (!IsWindow(m_hWnd))
		return FALSE;

	CRect rtCell;

	if (nRow == 0)
	{
		// It is col colheader.
		if (!m_bShowColHeader)
			return FALSE;
	}

	if (nCol > 0)
	{
		if (!m_arCols[nCol - 1]->bVisible)
			return FALSE;
	}
	else if (!m_bShowRowHeader)
	// It is row header.
		return FALSE;

	// Get the cell rect.
	if (!GetCellRect(nRow, nCol, rtCell))
		return FALSE;

	return (rtCell.left < m_rtGrid.right && rtCell.top < m_rtGrid.bottom);
}

Cell * CGrid::GetCell(int nRow, int nCol)
/*
Routine Description:
	Get pointer of one cell object.

Parameters:
	nRow

	nCol

Return Value:
	The pointer.
*/
{
	ASSERT(IsValidCell(nRow, nCol));

	return m_arCells[m_nCols * (nRow - 1) + nCol];
}

CString CGrid::GetCellText(int nRow, int nCol)
/*
Routine Description:
	Get text of one cell.

Parameters:
	nRow

	nCol

Return Value:
	The text string.
*/
{
	if (nRow > 0 && nCol > 0 && !m_arCols[nCol - 1]->bDirty && nRow <= (m_nRows - (m_bAllowAddNew ? 2 : 1)))
	{
		// This cell is not modified and this row is not pending new row.

		// Load cell textt from helper function.
		CString strText;
		OnLoadCellText(nRow, nCol, strText);
		GetCell(nRow, nCol)->strText = strText;

		return strText;
	}
	else
	// This row is pending new row or this cell is modified, return the current text.
		return GetCell(nRow, nCol)->strText;
}

UINT CGrid::GetCellState(int nRow, int nCol)
/*
Routine Description:
	Return the state of one cell.

Parameters:
	nRow

	nCol

Return Value:
	The state.
*/
{
	return GetCell(nRow, nCol)->nState;
}

BOOL CGrid::IsValidCell(int nRow, int nCol)
/*
Routine Description:
	Decides if the given row/col ordinal is valid.

Parameters:
	nRow

	nCol

Return Value:
	If is valid, return TRUE; Otherwise, return FALSE.
*/
{
	return (nRow >= 0 && nCol >= 0 && nRow < m_nRows && nCol < m_nCols);
}

BOOL CGrid::DrawCell(int nRow, int nCol, CRect rect, CDC *pDC)
/*
Routine Description:
	Draw one cell.

Parameters:
	nRow

	nCol

	rect		The rect to draw in.

	pDC		The DC to draw.

Return Value:
	If succeeded, return TRUE; Otherwiser, return FALSE.
*/
{
	// Should allow redraw.
	if (!m_bAllowRedraw)
		return FALSE;

	Cell * pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	rect.DeflateRect(1, 1, 0, 0);

	// Save DC.
	int nSavedDC = pDC->SaveDC();

	if (!m_bIPEdit || m_nRow != nRow || m_nCol != nCol)
	{
		// It is normal cell.

		// Get the cell properties.
		CellStyle style;
		GetCellStyle(nRow, nCol, style, rect);

		// Get the col control.
		CGridControl * pControl = GetColControl(nRow, nCol);
		ASSERT(pControl);

		// Let the col control to draw itself.
		pControl->SetMargin(m_nMargin);
		pControl->Draw(pDC, &style, m_nRow == nRow && m_nCol == nCol);
	}

	if (pCell->nState & GVIS_FOCUSED && IsWindowVisible())
	{
		// Draw the focus rect.
		pDC->SelectStockObject(BLACK_PEN);
		pDC->SelectStockObject(NULL_BRUSH);
		pDC->Rectangle(rect);
	}

	rect.InflateRect(1, 1, 0, 0);

	// Draw divider lines.

	LOGPEN lgp;
	lgp.lopnColor = m_clrDivider;
	lgp.lopnWidth = CPoint(1, 1);
	lgp.lopnStyle = m_nDividerStyle;

	CPen penDivider;
	
	penDivider.CreatePenIndirect(&lgp);
	
	pDC->SelectObject(&penDivider);
	
	if (m_nDividerType == DividerTypeBoth || m_nDividerType == DividerTypeVertical)
	{
		if (rect.left != (m_bShowRowHeader ? m_rtGrid.left + 22 : m_rtGrid.left))
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom);
		}
		
		pDC->MoveTo(rect.right, rect.top);
		pDC->LineTo(rect.right, rect.bottom);
	}
	
	if (m_nDividerType == DividerTypeBoth || m_nDividerType == DividerTypeHorizontal)
	{
		if (rect.top != GetVirtualHeaderHeight())
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
		}
		
		pDC->MoveTo(rect.left, rect.bottom);
		pDC->LineTo(rect.right, rect.bottom);
	}
	
	// Restore DC.
	pDC->RestoreDC(nSavedDC);

	return TRUE;
}

BOOL CGrid::DrawFixedCell(CRect& rect, CDC *pDC, COLORREF clrBack)
/*
Routine Description:
	Draw 3D fixed cell.

Parameters:
	rect		The cell rect.

	pDC		The DC.

	clrBack		The back color.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	rect.InflateRect(0, 0, 1, 1);

	// Draw 3d rects.

	pDC->Draw3dRect(rect, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_3DDKSHADOW));
	rect.DeflateRect(1, 1);
	pDC->Draw3dRect(rect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
	rect.DeflateRect(1, 1);
	pDC->FillSolidRect(rect, clrBack);

	return TRUE;
}

BOOL CGrid::RedrawGroup(int nRow, int nGroup, CDC *pDC)
/*
Routine Description:
	Redraw one group in one row.

Parameters:
	nRow

	nGroup

	pDC		The DC.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Should allow redraw.
	if (!m_bAllowRedraw)
		return FALSE;

	// If this group block is not visible, return.
	if (!IsGroupVisible(nRow, nGroup))
		return FALSE;

	if (nRow == 0)
	{
		// It is group header.
		CRect rect;
		if (GetGroupHeaderRect(nGroup, rect))
			DrawGroupHeaderCell(nGroup, rect, pDC);
	}

	// Redraw each cell in this group.

	int nColStart = 1;

	for (int i = 0; i < nGroup; i ++)
		nColStart += m_arGroups[i]->nCols;

	Group * pGroup = m_arGroups[i];
	for (int nCol = nColStart; nCol < nColStart + m_arGroups[i]->nCols; nCol ++)
		RedrawCell(nRow, nCol, pDC);

	return TRUE;
}

BOOL CGrid::DrawRowHeaderCell(int nRow, CRect rect, CDC *pDC)
/*
Routine Description:
	Draw row header cell.

Parameters:
	nRow

	rect		The rect to draw in.

	pDC		The DC.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Should allow redraw.
	if (!m_bShowRowHeader)
		return TRUE;

	// Draw 3D rect first.
	if (!DrawFixedCell(rect, pDC, m_clrHeaderBack))
		return FALSE;

	// Draw the picture.
	int nSavedDC = pDC->SaveDC();

	pDC->SetBkMode(TRANSPARENT);

	if (!m_bListMode)
	{
		if (nRow > 0 && m_bAllowAddNew && nRow == m_nRows - 1)
			m_ImageList.Draw(pDC, 2, CPoint(rect.left + (rect.Width() - 12) / 2 + 1, rect.top + (rect.Height() - 12) / 2 + 2), ILD_NORMAL);
		else if (nRow > 0 && nRow == m_nRow)
			m_ImageList.Draw(pDC, m_bIPEdit || m_bRowDirty ? 1 : 0, CPoint(rect.left + (rect.Width() - 12) / 2 + 1, rect.top + (rect.Height() - 12) / 2 + 2), ILD_NORMAL);
	}

	pDC->RestoreDC(nSavedDC);

	return TRUE;
}

BOOL CGrid::DrawColHeaderCell(int nRow, int nCol, CRect rect, CDC *pDC)
/*
Routine Description:
	Draw col header cell.

Parameters:
	nRow

	nCol

	rect		The rect to draw in.

	pDC		The DC.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Should allow redraw.
	if (!m_bShowColHeader)
		return TRUE;

	Col * pCol = m_arCols[nCol - 1];

	if (!pCol)
		return FALSE;

	int nSavedDC = pDC->SaveDC();

	// If the color is default, it means use the color of grid.
	COLORREF clrBack = (pCol->clrHeaderBack == DEFAULTCOLOR ? m_clrHeaderBack : pCol->clrHeaderBack);
	COLORREF clrFore = (pCol->clrHeaderFore == DEFAULTCOLOR ? m_clrHeaderFore : pCol->clrHeaderFore);

	if (IsColSelected(nCol))
	{
		// The col is selected, draw use inverted color.
		pDC->FillSolidRect(rect, (RGB(255, 255, 255) ^ clrBack) & 0xffffff);
		clrFore = (RGB(255, 255, 255) ^ clrFore) & 0xffffff;
	}
	else
	{
		// Draw 3D rect.
		if (!DrawFixedCell(rect, pDC, clrBack))
			return FALSE;
	}

	pDC->SelectObject(&m_fntHeader);
	
	// Draw col caption.

	pDC->SetTextColor(clrFore);
	
	int nAlign = 0;

	if (pCol->nHeaderAlignment == -1)
	// Use header alignment of grid.
		nAlign = m_nHeaderAlignment;
	else
		nAlign = pCol->nHeaderAlignment;

	if (nAlign == DT_LEFT)
		rect.left += m_nHeaderMargin;
	else if (nAlign == DT_RIGHT)
		rect.right -= m_nHeaderMargin;

	pDC->DrawText(pCol->strTitle, rect, DT_VCENTER | DT_SINGLELINE | nAlign);

	pDC->RestoreDC(nSavedDC);

	return TRUE;
}

BOOL CGrid::DrawGroupHeaderCell(int nGroup, CRect rect, CDC *pDC)
/*
Routine Description:
	Draw group header cell.

Parameters:
	nGroup

	rect		The rect to draw in.

	pDC		The DC.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Should allow drawing.
	if (!m_bShowGroupHeader)
		return TRUE;

	Group * pGroup = m_arGroups[nGroup];

	int nSavedDC = pDC->SaveDC();

	COLORREF clrBack = (pGroup->clrBack == DEFAULTCOLOR ? m_clrHeaderBack : pGroup->clrBack);
	COLORREF clrFore = (pGroup->clrFore == DEFAULTCOLOR ? m_clrHeaderFore : pGroup->clrFore);

	if (IsGroupSelected(nGroup + 1))
	{
		// The group is selected, draw use inverted color.
		pDC->FillSolidRect(rect, (RGB(255, 255, 255) ^ clrBack) & 0xffffff);
		clrFore = (RGB(255, 255, 255) ^ clrFore) & 0xffffff;
	}
	else
	{
		// Draw 3D rect.
		if (!DrawFixedCell(rect, pDC, clrBack))
			return FALSE;
	}

	// Draw group caption.
	pDC->SelectObject(&m_fntHeader);
	pDC->SetTextColor(clrFore);

	if (m_nHeaderAlignment == DT_LEFT)
		rect.left += m_nHeaderMargin;
	else if (m_nHeaderAlignment == DT_RIGHT)
		rect.right -= m_nHeaderMargin;

	pDC->DrawText(pGroup->strTitle, rect, m_nHeaderAlignment | DT_VCENTER | DT_SINGLELINE);

	pDC->RestoreDC(nSavedDC);

	return TRUE;
}

BOOL CGrid::GetGroupHeaderRect(int nGroup, CRect &rtCell)
/*
Routine Description:
	Get the rect one group header occupies.

Parameters:
	nGroup

	rtCell		The result rect.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Should be visible group.
	if (nGroup < 0 || nGroup >= m_nGroups || !m_bShowGroupHeader || !m_arGroups[nGroup]->bVisible)
		return FALSE;

	int nTopNonFixedRow, nLeftNonFixedGroup;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	if (nGroup >= m_nFrozenGroups && nGroup < nLeftNonFixedGroup)
		return FALSE;

	CPoint ptLeftTop = m_rtGrid.TopLeft();
	if (nGroup >= m_nFrozenGroups)
		ptLeftTop.x += GetFixedColWidth();
	else
		ptLeftTop.x += (m_bShowRowHeader ? 22 : 0);

	Group * pGroup;

	for (int i = (nGroup >= m_nFrozenGroups ? nLeftNonFixedGroup : 0); i < nGroup; i ++)
	{
		pGroup = m_arGroups[i];
		ptLeftTop.x += pGroup->bVisible ? pGroup->nWidth : 0;
	}

	pGroup = m_arGroups[i];

	rtCell.left = ptLeftTop.x;
	rtCell.top = ptLeftTop.y;
	rtCell.right = rtCell.left + (pGroup->bVisible ? pGroup->nWidth : 0);
	rtCell.bottom = rtCell.top + (pGroup->bVisible ? m_nHeaderLevelHeight : 0);

	return TRUE;
}

BOOL CGrid::InsertGroup(int nGroupIndex)
/*
Routine Description:
	Insert one group.

Parameters:
	nGroupIndex		The position to insert, begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// The group array begins with 0.
	nGroupIndex --;
	if (nGroupIndex == -1)
	// Insert at the end.
		nGroupIndex = m_nGroups;

	if (nGroupIndex < 0 || nGroupIndex > m_nGroups)
		return FALSE;

	Level * pLevel;
	Group * pGroup;
	
	// Create a new group object.
	pGroup = new Group;
		
	// Init this group object.
	pGroup->nCols = 0;
	pGroup->nGroupIndex = ++ m_nGroupsUsed;
	pGroup->strTitle.Format(_T("G%d"), m_nGroupsUsed);
	pGroup->nWidth = m_nDefColWidth;

	CString strName;
	strName.Format("G%d", pGroup->nGroupIndex);
	pGroup->strName = FindUniqueName(strName, pGroup->nGroupIndex);

	// Create and init each level object in the new group object.

	for (int nLevel = 0; nLevel < m_nLevels; nLevel ++)
	{
		pLevel = new Level;
		pGroup->arLevels.Add(pLevel);
	}

	int nColsWidth = 0;
	if (m_nGroups == 0 && m_nCols > 1)
	{
		pLevel = pGroup->arLevels[0];
		pLevel->nCols = m_nCols - 1;

		for (int nCol = 0; nCol < m_nCols - 1; nCol ++)
		{
			if (m_arCols[nCol]->bVisible)
			{
				pLevel->nColsVisible ++;
				nColsWidth += m_arCols[nCol]->nWidth;
			}
		}

		pGroup->nCols = m_nCols - 1;
		
		GetClientRect(m_rtGrid);

		// Col width has limitation.
		nColsWidth = min(nColsWidth, m_rtGrid.Width() - (m_bShowRowHeader ? 22 : 0) - 1);
	}

	// Insert new group object into array.
	m_arGroups.InsertAt(nGroupIndex, pGroup);

	m_nGroups ++;

	if (m_nGroups == 1 && m_nCols > 1)
	{
		// Include all cols exist into this new group if have no group yet.
		CalcHeaderHeight();
		SetGroupWidth(1, nColsWidth);
		SetFrozenCols(0);
	}

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::RemoveGroup(int nGroupIndex)
/*
Routine Description:
	Remove one group.

Parameters:
	nGroupIndex		Begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Index in group array begins with 0.
	nGroupIndex --;

	if (nGroupIndex == -1)
	// Remove last group default.
		nGroupIndex = m_nGroups - 1;

	if (nGroupIndex < 0 || nGroupIndex >= m_nGroups)
		return FALSE;

	if (m_nSelectMode == SELECTMODE_GROUP)
	// If this group is selected, deselect it.
		SelectGroup(nGroupIndex + 1, FALSE);

	Group * pGroup = m_arGroups[nGroupIndex];

	while (pGroup->nCols)
	{
		// Remove every col in this group.
		if (!RemoveColInGroup(nGroupIndex + 1, 1))
			return FALSE;
	}

	// Delete all level objects in this group.
	for (int i = 0; i < m_nLevels; i ++)
		delete pGroup->arLevels[i];

	pGroup->arLevels.RemoveAll();

	// Remove item from group array.
	delete m_arGroups[nGroupIndex];
	m_arGroups.RemoveAt(nGroupIndex);

	m_nGroups --;

	if (m_nGroups == 0)
	{
		// If there is no group at all, the level count should be 1.
		SetLevelCount(1);
	}

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::InsertCol(int nColIndex)
/*
Routine Description:
	Insert a col.

Parameters:
	nColIndex		Begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	// Index in col array begins with 0.
	nColIndex --;

	if (m_nCols >= 255)
		return FALSE;

	if (nColIndex == -1)
	// Insert at the end default.
		nColIndex = m_nCols - 1;
	
	if (nColIndex < 0 || nColIndex >= m_nCols)
		return FALSE;

	if (IsSingleLevelMode())
	{
		// There is no group.

		// Create a new col object.
		Col * pCol = new Col;

		// Init the new col object.
		pCol->nWidth = m_nDefColWidth;
		pCol->nColIndex = ++ m_nColsUsed;
		pCol->strTitle.Format("C%d", m_nColsUsed);

		CString strName;
		strName.Format("C%d", pCol->nColIndex);
		pCol->strName = FindUniqueName(strName, pCol->nColIndex);

		// Insert the new col object into col array.
		m_arCols.InsertAt(nColIndex, pCol);

		m_nCols ++;

		// Add cells for the new col.
		for (int i = 1; i < m_nRows; i ++)
		{
			Cell * pCell = new Cell;
			m_arCells.InsertAt(m_nCols * (i - 1) + nColIndex + 1, pCell);
		}

		if (m_nCol == 0 && m_nRows > 1)
			SetCurrentCell(m_nRow == 0 ? 1 : m_nRow, 1);

		if (m_bAllowRedraw)
		{
			ResetScrollBars();
			Invalidate();
		}
		
		return TRUE;
	}

	// Insert col within one group.

	Group * pGroup;
	
	for (int nGroup = 0; nGroup < m_nGroups; nGroup ++)
	{
		pGroup = m_arGroups[nGroup];
		
		if (pGroup->nCols < nColIndex)
		{
			nColIndex -= pGroup->nCols;
			continue;
		}
		
		return InsertColInGroup(nGroup + 1, nColIndex + 1);
	}

	return FALSE;
}

BOOL CGrid::RemoveCol(int nColIndex)
/*
Routine Description:
	Remove a col.

Parameters:
	nColIndex		Begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nColIndex --;

	if (m_nCols < 2)
	// Can not remove header col.
		return FALSE;

	if (nColIndex == -1)
	// Remove default col default.
		nColIndex = m_nCols - 2;
	
	if (nColIndex < 0 || nColIndex >= m_nCols - 1)
		return FALSE;

	if (m_nSelectMode == SELECTMODE_COL)
	// Deselect this col.
		SelectCol(nColIndex + 1, FALSE);

	if (IsSingleLevelMode())
	{
		// There is no group.
		delete m_arCols[nColIndex];
		m_arCols.RemoveAt(nColIndex);
		
		m_nCols --;
		
		for (int i = 1; i < m_nRows; i ++)
		{
			delete m_arCells[m_nCols * (i - 1) + nColIndex + 1];
			m_arCells.RemoveAt(m_nCols * (i - 1) + nColIndex + 1);
		}
		
		if (m_nCols < 2)
			m_nCol = 0;

		if (m_nCol == nColIndex + 1 && m_nRow > 0)
		{
			if (!Navigate(m_nRow, m_nCol, NV_RIGHT) && !Navigate(m_nRow, m_nCol, NV_LEFT) && !Navigate(m_nRow, 0, NV_RIGHT))
				m_nCol = 0;
		}

		if (m_bAllowRedraw)
		{
			ResetScrollBars();
			Invalidate();
		}

		return TRUE;
	}

	// Remove col within a group.

	int nGroup = GetGroupFromCol(nColIndex);
	if (nGroup == INVALID)
		return FALSE;
	
	return RemoveColInGroup(nGroup + 1, nColIndex + 1);
}

BOOL CGrid::InsertColInGroup(int nGroup, int nColIndex)
/*
Routine Description:
	Insert col with a group.

Parameters:
	nGroup		Begins with 1.

	nColIndex	Begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nGroup --;
	nColIndex --;
	
	if (nGroup < 0 || nGroup >= m_nGroups)
		return FALSE;
	
	Group * pGroup = m_arGroups[nGroup];
	
	if (nColIndex == -1)
	// Insert at the end default.
		nColIndex = pGroup->nCols;
	
	if (nColIndex < 0 || nColIndex > pGroup->nCols)
		return FALSE;
	
	int nColInGroup = nColIndex;
	
	int nLevel = 0, nColInLevel = 0;
	Level * pLevel;

	for (nLevel = 0; nLevel < m_nLevels; nLevel ++)
	{
		// Calc the level ordinal into which insert the new col.
		nColInLevel = 0;
		pLevel = pGroup->arLevels[nLevel];
		
		if (pLevel->nCols < nColIndex)
		{
			nColIndex -= pLevel->nCols;
			continue;
		}

		if (pLevel->nCols >= 255)
			return FALSE;

		// Convert the ordinal in level to global ordinal.
		int nColOrd = GetColFromLevel(nGroup, nLevel, nColIndex);

		// Create new col object.
		Col * pCol = new Col;

		// Init the new col object.
		pCol->nWidth = m_nDefColWidth;
		pCol->nColIndex = ++ m_nColsUsed;
		pCol->strTitle.Format("C%d", m_nColsUsed);

		CString strName;
		strName.Format("C%d", pCol->nColIndex);
		pCol->strName = FindUniqueName(strName, pCol->nColIndex);

		// Insert new col object into array.
		m_arCols.InsertAt(nColOrd, pCol);

		pGroup->nCols ++;
		pLevel->nCols ++;
		pLevel->nColsVisible ++;
		m_nCols ++;

		// Add cells for new col.
		for (int i = 1; i < m_nRows; i ++)
		{
			Cell * pCell = new Cell;
			m_arCells.InsertAt(m_nCols * (i - 1) + nColOrd + 1, pCell);
		}

		// Recalc the width of cols in this level.
		CalcColWidthInLevel(nGroup, nLevel);

		if (m_nCol == 0 && m_nRows > 1)
			SetCurrentCell(m_nRow == 0 ? 1 : m_nRow, 1);

		return TRUE;
	}

	return FALSE;
}

BOOL CGrid::RemoveColInGroup(int nGroup, int nColIndex)
/*
Routine Description:
	Remove col within a group.

Parameters:
	nGroup		Begins with 1.

	nColIndex	Begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nGroup --;
	nColIndex --;

	if (nGroup < 0 || nGroup >= m_nGroups)
		return FALSE;

	Group * pGroup = m_arGroups[nGroup];

	if (nColIndex == -1)
	// Remove the last col default.
		nColIndex = pGroup->nCols - 1;

	if (nColIndex < 0 || nColIndex >= pGroup->nCols)
		return FALSE;

	// Set the width of this col with 0.
	if (!SetColWidthInGroup(nGroup + 1, nColIndex + 1, 0))
		return FALSE;

	int nColOrd = GetColFromGroup(nGroup, nColIndex);

	int nLevel = 0, nColInLevel = 0;
	Level * pLevel;

	nLevel = GetLevelFromCol(nGroup, nColIndex);
	if (nLevel == INVALID)
		return FALSE;

	nColInLevel = 0;
	pLevel = pGroup->arLevels[nLevel];
	
	// Remove col object from array.

	delete m_arCols[nColOrd];
	m_arCols.RemoveAt(nColOrd);

	pGroup->nCols --;
	pLevel->nCols --;
	pLevel->nColsVisible --;

	m_nCols --;

	// Remove cells in this col.	
	for (int i = 1; i < m_nRows; i ++)
	{
		delete m_arCells[m_nCols * (i - 1) + nColOrd + 1];
		m_arCells.RemoveAt(m_nCols * (i - 1) + nColOrd + 1);
	}

	if (m_nCols < 2)
		m_nCol = 0;

	if (m_nCol == nColOrd + 1 && m_nRow > 0)
	{
		// Set the current cell to other cell.
		if (!Navigate(m_nRow, m_nCol, NV_RIGHT) && !Navigate(m_nRow, m_nCol, NV_LEFT) && !Navigate(m_nRow, 0, NV_RIGHT))
			m_nCol = 0;
	}

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::SetLevelCount(int nCount)
/*
Routine Description:
	Set the level count.

Parameters:
	nCount 		The new count.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nCount < 1 || nCount > 255 || (nCount > 1 && m_nGroups == 0))
		return FALSE;

	if (nCount == m_nLevels)
		return TRUE;

	Level * pLevel;

	if (nCount > m_nLevels)
	{
		// Should add levels.
		// Create each new level object in every group.
		for (int nGroup = 0; nGroup < m_nGroups; nGroup ++)
		{
			for (int i = 0; i < nCount - m_nLevels; i ++)
			{
				pLevel = new Level;
				m_arGroups[nGroup]->arLevels.Add(pLevel);
			}
		}
	}
	else
	{
		// Should remove levels.
		for (int nGroup = 0; nGroup < m_nGroups; nGroup ++)
		{
			int nCols = 0, nColsVisible = 0;

			Group * pGroup = m_arGroups[nGroup];
			for (int i = nCount; i < m_nLevels; i ++)
			{
				nCols += pGroup->arLevels[i]->nCols;
				nColsVisible += pGroup->arLevels[i]->nColsVisible;
			}

			pGroup->arLevels[nCount - 1]->nCols += nCols;
			pGroup->arLevels[nCount - 1]->nColsVisible += nColsVisible;

			for (i = nCount; i < m_nLevels; i ++)
			{
				delete pGroup->arLevels[i];
				pGroup->arLevels.RemoveAt(nCount);
			}

			// Recalc the width of cols in this level.
			CalcColWidthInLevel(nGroup, nCount - 1);
		}
	}

	m_nLevels = nCount;

	// Recalc the row height and header height.
	CalcRowHeight();
	CalcHeaderHeight();

	return TRUE;
}

BOOL CGrid::InsertRow(int nRowIndex)
/*
Routine Description:
	Insert a row.

Parameters:
	nRowIndex		Begins with 1.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nRowIndex == 0)
	// Insert at the end default.
		nRowIndex = m_nRows - (m_bAllowAddNew ? 1 : 0);

	if (nRowIndex <= 0 || nRowIndex > m_nRows)
		return FALSE;

	Cell * pCell;

	// Insert new row header cell.
	pCell = new Cell;
	m_arCells.InsertAt(m_nCols * (nRowIndex - 1), pCell);

	int i;

	// Insert cells in this row.
	for (i = 1; i < m_nCols; i ++)
	{
		pCell = new Cell;
//		pCell->strText.Format("%d", m_nCols * nRowIndex + i);

		m_arCells.InsertAt(m_nCols * (nRowIndex - 1) + i, pCell);
	}

	m_nRows ++;
	m_arRowIndex.InsertAt(nRowIndex - 1, ++ m_nRowsUsed);

	if (m_nRow == 0 && m_nCols > 1)
		SetCurrentCell(1, m_nCol == 0 ? 1 : m_nCol);
	else if (nRowIndex <= m_nRow)
		m_nRow ++;

	if (m_nRow >= m_nRows)
		m_nRow = m_nRows - 1;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::RemoveRow(int nRowIndex)
/*
Routine Description:
	Remove a row.

Parameters:
	nRowIndex		Begins with 1.

Return Value:
	None.
*/
{
	if (nRowIndex == 0)
	// Remove the last row default.
		nRowIndex = m_nRows - 1;

	if (nRowIndex < 1 || nRowIndex >= m_nRows)
		return FALSE;

	if (m_nSelectMode == SELECTMODE_ROW)
		SelectRow(nRowIndex, FALSE);

	// Delete cells in this row.
	for (int i = 0; i < m_nCols; i ++)
	{
		delete m_arCells[m_nCols * (nRowIndex - 1)];
		m_arCells.RemoveAt(m_nCols * (nRowIndex - 1));
	}

	m_nRows --;
	m_arRowIndex.RemoveAt(nRowIndex - 1);

	if (m_nRows == 0)
		m_nRow = 0;

	// Change the current cell if is needed.
	if (m_nRow == nRowIndex && m_nCol > 0)
	{
		if (!Navigate(m_nRow, m_nCol, NV_DOWN) && !Navigate(m_nRow, m_nCol, NV_UP) && !Navigate(0, m_nCol, NV_DOWN))
			m_nRow = 0;
	}
	else if (m_nRow > nRowIndex)
		m_nRow --;

	if (m_nRow < 0)
		m_nRow = 0;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::SetGroupCount(int nCount)
/*
Routine Description:
	Set the group count.

Parameters:
	nCount		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nCount < 0 || nCount > 255)
		return FALSE;

	if (nCount == m_nGroups)
		return TRUE;

	BOOL bRedraw = m_bAllowRedraw;
	m_bAllowRedraw = FALSE;

	if (nCount > m_nGroups)
	{
		// Should insert groups.
		while (m_nGroups < nCount)
		{
			// Insert each group.
			if (!InsertGroup())
			{
				m_bAllowRedraw = bRedraw;
				return FALSE;
			}
		}
	}
	else
	{
		// Should remove groups.
		while (m_nGroups > nCount)
		{
			// Remove each group.
			if (!RemoveGroup())
			{
				m_bAllowRedraw = bRedraw;
				return FALSE;
			}
		}
	}

	m_bAllowRedraw = bRedraw;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::SetColCount(int nCount)
/*
Routine Description:
	Set the col count.

Parameters:
	nCount		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nCount ++;

	if (nCount < 1 || nCount > 255)
		return FALSE;

	if (nCount == m_nCols)
		return TRUE;

	BOOL bRedraw = m_bAllowRedraw;
	m_bAllowRedraw = FALSE;

	if (nCount > m_nCols)
	{
		// Should add cols.
		while (m_nCols < nCount)
		{
			// Insert a col.
			if (!InsertCol())
			{
				m_bAllowRedraw = bRedraw;
				return FALSE;
			}
		}
	}
	else
	{
		// Should remove cols.
		while (m_nCols > nCount)
		{
			// Remove a col.
			if (!RemoveCol())
			{
				m_bAllowRedraw = bRedraw;
				return FALSE;
			}
		}
	}

	m_bAllowRedraw = bRedraw;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::SetRowCount(int nCount)
/*
Routine Description:
	Set the row count.

Parameters:
	nCount		The new count.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nCount < 0)
		return FALSE;

	nCount += (m_bAllowAddNew ? 2 : 1);
	if (nCount == m_nRows)
		return TRUE;

	BOOL bRedraw = m_bAllowRedraw;
	m_bAllowRedraw = FALSE;

	if (nCount > m_nRows)
	{
		// Should add rows.
		while (m_nRows < nCount)
		{
			// Add a row.
			if (!InsertRow())
			{
				m_bAllowRedraw = bRedraw;
				return FALSE;
			}
		}
	}
	else
	{
		// Should remove rows.
		while (m_nRows > nCount)
		{
			// Remove a row.
			if (!RemoveRow(m_nRows - (m_bAllowAddNew ? 2 : 1)))
			{
				m_bAllowRedraw = bRedraw;
				return FALSE;
			}
		}
	}

	ASSERT(m_nRows == nCount);

	m_bAllowRedraw = bRedraw;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::RedrawRow(int nRow, CDC *pDC)
/*
Routine Description:
	Redraw a row.

Parameters:
	nRow

	pDC

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE
*/
{
	// Should allow redraw.
	if (!m_bAllowRedraw)
		return FALSE;

	ASSERT(nRow >= 0 && nRow < m_nRows);

	if (nRow == 0 && !m_bShowColHeader && !m_bShowGroupHeader)
		return TRUE;

	// This row should be visible.
	if (!IsRowVisible(nRow))
		return nRow == 0;

	// Should we release the DC after drawing?
	BOOL bMustReleaseDC = FALSE;

	if (!pDC)
	{
		// If have no DC, get it.
		pDC = GetDC();
		if (pDC)
		// Should release the DC.
			bMustReleaseDC = TRUE;
		else
			return FALSE;
	}
	
	// Redraw the row header.
	RedrawCell(nRow, 0, pDC);
	
	if (IsSingleLevelMode())
	{
		// There is no group.

		// Redraw every cell.
		for (int nCol = 1; nCol < m_nCols; nCol ++)
			RedrawCell(nRow, nCol, pDC);
	}
	else
	// There are groups.

	// Redraw every group.
		for (int nGroup = 0; nGroup < m_nGroups; nGroup ++)
			RedrawGroup(nRow, nGroup, pDC);

	if (bMustReleaseDC) 
		ReleaseDC(pDC);

	return TRUE;
}

int CGrid::GetRowCount()
/*
Routine Description:
	Get the contents row count.

Parameters:
	None.

Return Value:
	The count.
*/
{
	return m_nRows - (m_bAllowAddNew ? 2 : 1);
}

int CGrid::GetColCount()
/*
Routine Description:
	Get the contents col count.
Parameters:
	None.

Return Value:
	The count.
*/
{
	return m_nCols - 1;
}

int CGrid::GetGroupCount()
/*
Routine Description:
	Get the group count.

Parameters:
	None.

Return Value:
	The count.
*/
{
	return m_nGroups;
}

int CGrid::GetLevelCount()
/*
Routine Description:
	Get the level count.

Parameters:
	None.

Return Value:
	None.
*/
{
	return m_nLevels;
}

void CGrid::ResetScrollBars()
/*
Routine Description:
	Recalc the range of each scroll bar.

Parameters:
	None.

Return Value:
	None.
*/
{
	if (!::IsWindow(GetSafeHwnd())) 
		return;

	// Have no scroll bar default.
	SetScrollRange(SB_VERT, 0, 0, TRUE);
	SetScrollRange(SB_HORZ, 0, 0, TRUE);

	CRect rect;
	GetWindowRect(rect);
	rect.OffsetRect(-rect.left, -rect.top);

	// Calc the frozen area.
	rect.left += GetFixedColWidth();
	rect.top += GetFixedRowHeight();
	if (rect.left >= rect.right || rect.top >= rect.bottom)
	// Total grid window is filled by frozen cells, so there is no need for scroll bar.
		return;

	// Get the total scrollable area in grid window.
	CRect VisibleRect(GetFixedColWidth(), GetFixedRowHeight(), rect.right, rect.bottom);

	// Get the total non fixed area of grid.
	CRect VirtualRect(GetFixedColWidth(), GetFixedRowHeight(), GetVirtualWidth(), GetVirtualHeight());

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_PAGE;
	
	si.nPage  = VisibleRect.Width();
	SetScrollInfo(SB_HORZ, &si, FALSE); 
	
	si.nPage  = VisibleRect.Height();
	SetScrollInfo(SB_VERT, &si, FALSE); 

	if (VisibleRect.Height() < VirtualRect.Height())
	{
		// The max vertical scroll range.
		m_nVScrollMax = VirtualRect.Height() - 1;

		// If the last row height is larger than scrollable window height, we can only
		// show part of this row.
		if (m_nRowHeight > VisibleRect.Height())
			m_nVScrollMax -= m_nRowHeight - VisibleRect.Height();
	}
	else
		m_nVScrollMax = 0;

	if (VisibleRect.Width() < VirtualRect.Width())
	{
		// The max horizontal scroll range.
		m_nHScrollMax = VirtualRect.Width() - 1;

		// Get the last visible col or group.
		int nLastCol, nLastGroup;
		if (IsSingleLevelMode())
			for (nLastCol = m_nCols - 2; !m_arCols[nLastCol]->bVisible && nLastCol >= 0; nLastCol --);
		else
			for (nLastGroup = m_nGroups - 1; !m_arGroups[nLastGroup]->bVisible && nLastGroup >= 0; nLastGroup --);

		// If the last col/group width is larger than scrollable window width, we can only
		// show part of this col/group.
		int nLastWidth = IsSingleLevelMode() ? m_arCols[nLastCol]->nWidth : GetGroupWidth(nLastGroup + 1);
		if (nLastWidth > VisibleRect.Width())
			m_nHScrollMax -= nLastWidth - VisibleRect.Width();
	}
	else
		m_nHScrollMax = 0;

	ASSERT(m_nVScrollMax < INT_MAX && m_nHScrollMax < INT_MAX); // This should be fine :)

	SetScrollRange(SB_VERT, 0, m_nVScrollMax, TRUE);
	SetScrollRange(SB_HORZ, 0, m_nHScrollMax, TRUE);
}

int CGrid::GetFixedColWidth()
/*
Routine Description:
	Get the total width of all fixed cols.

Parameters:
	None.

Return Value:
	The width.
*/
{
	int nFixedWidth = (m_bShowRowHeader ? 22 : 0);

	if (IsSingleLevelMode())
		for (int i = 0; i < m_nFrozenCols; i ++)
			nFixedWidth += m_arCols[i]->bVisible ? m_arCols[i]->nWidth : 0;
	else
		for (int i = 0; i < m_nFrozenGroups; i ++)
			nFixedWidth += m_arGroups[i]->bVisible ? m_arGroups[i]->nWidth : 0;

	return nFixedWidth;
}

int CGrid::GetFixedRowHeight()
/*
Routine Description:
	Get the total height of all fixed rows.

Parameters:
	None.

Return Value:
	The height.
*/
{
	return GetVirtualHeaderHeight() + m_nFrozenRows * m_nRowHeight;
}

int CGrid::GetVirtualWidth()
/*
Routine Description:
	Get the width of entire grid.

Parameters:
	None.

Return Value:
	The width.
*/
{
	// Add the row header width.
	int nWidth = (m_bShowRowHeader ? 22 : 0);

	if (IsSingleLevelMode())
	// There is no groups, add width of each col.
		for (int i = 0; i < m_nCols - 1; i ++)
			nWidth += m_arCols[i]->bVisible ? m_arCols[i]->nWidth : 0;
	else
	// There are groups, add width of each group.
		for (int i = 0; i < m_nGroups; i ++)
			nWidth += m_arGroups[i]->bVisible ? m_arGroups[i]->nWidth : 0;

	return nWidth;
}

int CGrid::GetVirtualHeight()
/*
Routine Description:
	Get the height of entire grid.

Parameters:
	None.

Return Value:
	The height.
*/
{
	return GetVirtualHeaderHeight() + (m_nRows - 1) * m_nRowHeight;
}

BOOL CGrid::IsGroupVisible(int nRow, int nGroup)
/*
Routine Description:
	Decides if one group block is visible to end user.

Parameters:
	nRow

	nGroup

Return Value:
	The visibility.
*/
{
	if (!IsWindow(m_hWnd) || !m_arGroups[nGroup]->bVisible)
		return FALSE;

	CRect rtGroup;

	// Get the group rect.
	if (!GetGroupRect(nRow, nGroup, rtGroup))
		return FALSE;

	// Compare the result rect with grid rect.
	return (rtGroup.left < m_rtGrid.right && rtGroup.top < m_rtGrid.bottom);
}

BOOL CGrid::GetGroupRect(int nRow, int nGroup, CRect &rtGroup)
/*
Routine Description:
	Get the rect one group block occupies.

Parameters:
	nRow

	nGroup

	rtGroup		The result group.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nRow >= 0 && nRow < m_nRows && nGroup >= 0 && nGroup < m_nGroups);

	rtGroup.left = m_rtGrid.left;
	rtGroup.top = m_rtGrid.top;

	int nTopNonFixedRow, nLeftNonFixedGroup;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	if ((nGroup >= m_nFrozenGroups && nGroup < nLeftNonFixedGroup) || (nRow > m_nFrozenRows &&
		nRow < nTopNonFixedRow))
		return FALSE;

	if (nGroup >= m_nFrozenGroups)
		rtGroup.left += GetFixedColWidth();
	else
		rtGroup.left += (m_bShowRowHeader ? 22 : 0);

	for (int i = nGroup >= m_nFrozenGroups ? nLeftNonFixedGroup : 0; i < nGroup; i ++)
		rtGroup.left += m_arGroups[i]->bVisible ? m_arGroups[i]->nWidth : 0;

	rtGroup.right = rtGroup.left + (m_arGroups[nGroup]->bVisible ? m_arGroups[nGroup]->nWidth : 0);

	if (nRow >= nTopNonFixedRow)
		nRow -= nTopNonFixedRow - 1;

	rtGroup.top += nRow ? (nRow - 1) * m_nRowHeight + GetVirtualHeaderHeight() : 0;
	rtGroup.bottom = rtGroup.top + (nRow ? m_nRowHeight : GetVirtualHeaderHeight());

	return TRUE;
}

BOOL CGrid::SetGroupWidth(int nGroup, int nWidth)
/*
Routine Description:
	Set the width of one group.

Parameters:
	nGroup		Begins with 1.

	nWidth		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nWidth <= 0 || (m_nLevels > 1 && nWidth > m_rtGrid.Width() - (m_bShowRowHeader ? 22 : 0)))
		return FALSE;

	nGroup --;

	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	Group * pGroup = m_arGroups[nGroup];

	if (nWidth < pGroup->nCols)
		return FALSE;

	pGroup->nWidth = nWidth;

	// Recalc cols width in each level.
	for (int nLevel = 0; nLevel < m_nLevels; nLevel ++)
		CalcColWidthInLevel(nGroup, nLevel);

	return TRUE;
}

int CGrid::GetGroupWidth(int nGroup)
/*
Routine Description:
	Get th width of one group.

Parameters:
	nGroup		Begins with 1.

Return Value:
	The width.
*/
{
	nGroup --;

	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	return m_arGroups[nGroup]->nWidth;
}

int CGrid::GetColWidth(int nCol)
/*
Routine Description:
	Get col width.

Parameters:
	nCol		Begins with 1.

Return Value:
	The col width.
*/
{
	nCol --;

	ASSERT(nCol >= 0 && nCol < m_nCols - 1);

	return m_arCols[nCol]->nWidth;
}

int CGrid::GetGroupFromCol(int &nCol)
/*
Routine Description:
	Get the group ordinal from col ordinal.

Parameters:
	nCol

Return Value:
	The group ordinal.
*/
{
	ASSERT(nCol >= 0 && nCol < m_nCols - 1);

	Group * pGroup;
	for (int i = 0; i < m_nGroups; i ++)
	{
		pGroup = m_arGroups[i];

		if (pGroup->nCols > nCol)
			return i;

		nCol -= pGroup->nCols;
	}

	return INVALID;
}

int CGrid::GetLevelFromCol(int nGroup, int &nCol)
/*
Routine Description:
	Get the level ordinal from the ordinal in a group.

Parameters:
	nGroup

	nCol

Return Value:
	The level ordinal.
*/
{
	ASSERT(nGroup >= 0 && nGroup < m_nGroups && nCol >= 0 && nCol < m_nCols - 1);

	Group * pGroup = m_arGroups[nGroup];
	Level * pLevel;

	for (int i = 0; i < m_nLevels; i ++)
	{
		pLevel = pGroup->arLevels[i];

		if (pLevel->nCols > nCol)
			return i;

		nCol -= pLevel->nCols;
	}

	return INVALID;
}

BOOL CGrid::SetColWidth(int nCol, int nWidth)
/*
Routine Description:
	Set width of a col.

Parameters:
	nCol		Begins with 1.

	nWidth		The new width.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nCol --;

	ASSERT(nCol < m_nCols);

	if (nWidth < 0)
		return FALSE;

	if (IsSingleLevelMode())
	{
		// There is no group.
		if (nWidth > m_rtGrid.Width() - (m_bShowRowHeader ? 22 : 0))
			return FALSE;

		m_arCols[nCol]->nWidth = nWidth;


		if (m_bAllowRedraw)
		{
			ResetScrollBars();
			Invalidate();
		}

		return TRUE;
	}

	// There are groups.

	int nGroup;

	nGroup = GetGroupFromCol(nCol);
	if (nGroup == INVALID)
		return FALSE;

	return SetColWidthInGroup(nGroup + 1, nCol + 1, nWidth);
}

BOOL CGrid::SetColWidthInGroup(int nGroup, int nCol, int nWidth)
/*
Routine Description:
	Set col width within a group.

Parameters:
	nGroup		Begins with 1.

	nCol		Begins with 1.

	nWidth		The new width.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nGroup --;
	nCol --;

	if (nWidth < 0)
		return FALSE;

	ASSERT (nGroup >= 0 && nGroup < m_nGroups && nCol >= 0 && nCol < m_nCols - 1);

	int nColOrd = GetColFromGroup(nGroup, nCol);
	Col * pCol = m_arCols[nColOrd];

	int nLevel = GetLevelFromCol(nGroup, nCol);
	if (nLevel == INVALID)
		return FALSE;

	Group * pGroup = m_arGroups[nGroup];
	Level * pLevel = pGroup->arLevels[nLevel];

	if (pCol->nWidth == nWidth)
		return TRUE;

	if (pGroup->nWidth - nWidth < pGroup->nCols)
	// each col width must be 1 at least.
		return FALSE;

	if (pLevel->nColsVisible == 1 && pCol->bVisible)
	// There is only 1 col visible in this level, can not change its width.
		return TRUE;

	if (!pCol->bVisible)
	{
		// This col is invisible, changed its width without doubt.
		pCol->nWidth = nWidth;

		return TRUE;
	}

	int nColsWidth = 0, nColsOldWidth = 0;

	// Calc new width of the cols at right.
	for (int nColInLevel = nCol + 1; nColInLevel < pLevel->nCols; nColInLevel ++)
	{
		int nColOrd = GetColFromLevel(nGroup, nLevel, nColInLevel);
		Col * pCol = m_arCols[nColOrd];

		if (!pCol->bVisible)
			continue;
		
		nColsOldWidth += pCol->nWidth;
//		pCol->nWidth = max(1, (int)(pCol->nWidth * fRatio));
//		nColsWidth += pCol->nWidth;
	}
	
	if (nColsOldWidth == 0)
		return TRUE;

	// The ratio to extend each col.
	float fRatio = (float)(nColsOldWidth - nWidth + pCol->nWidth) / nColsOldWidth;

	// Modify every col at right except the last one.
	for (nColInLevel = nCol + 1; nColInLevel < pLevel->nCols; nColInLevel ++)
	{
		int nColOrd = GetColFromLevel(nGroup, nLevel, nColInLevel);
		Col * pCol = m_arCols[nColOrd];

		if (!pCol->bVisible)
			continue;
		
		// Col width should be 1 at least.
		pCol->nWidth = max(1, (int)(pCol->nWidth * fRatio));
		nColsWidth += pCol->nWidth;
	}
	
	// The width of the last right col is the rest width.
	m_arCols[GetColFromLevel(nGroup, nLevel, nCol)]->nWidth -= nColsWidth - nColsOldWidth;

/*	pGroup->nWidth -= nWidth;
	pCol->bVisible = FALSE;
	pLevel->nColsVisible --;
	CalcColWidthInLevel(nGroup, nLevel);
	pGroup->nWidth += nWidth;
	pCol->nWidth = nWidth;
	pCol->bVisible = TRUE;
	pLevel->nColsVisible ++;
*/
	return TRUE;
}

void CGrid::CalcRowHeight()
/*
Routine Description:
	Recalc the row height.

Parameters:
	None.

Return Value:
	None.
*/
{
	m_nLevelHeight = m_nRowHeight / m_nLevels;
	m_nRowHeight = m_nLevelHeight * m_nLevels;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}
}

void CGrid::CalcHeaderHeight()
/*
Routine Description:
	Recalc header height.

Parameters:
	None.

Return Value:
	None.
*/
{
	m_nHeaderLevelHeight = m_nHeaderHeight / m_nLevels;
	m_nHeaderHeight = m_nHeaderLevelHeight * m_nLevels;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}
}

BOOL CGrid::SetRowHeight(int nHeight)
/*
Routine Description:
	Set the row height.

Parameters:
	nHeight		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	GetClientRect(m_rtGrid);

	if (nHeight < 0 || nHeight > m_rtGrid.Height())
		return FALSE;

	m_nRowHeight = nHeight;
	CalcRowHeight();
	
	return TRUE;
}

BOOL CGrid::SetHeaderHeight(int nHeight)
/*
Routine Description:
	Set header height.

Parameters:
	nHeight		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	GetClientRect(m_rtGrid);

	if (nHeight < 0 || nHeight > m_rtGrid.Height())
		return FALSE;

	m_nHeaderHeight = nHeight;
	CalcHeaderHeight();
	
	return TRUE;
}

void CGrid::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (GetFocus()->GetSafeHwnd() != GetSafeHwnd()) 
		SetFocus();  // Auto-destroy any InPlaceEdit's

	int scrollPos = GetScrollPos32(SB_HORZ);
	
	int nTopNonFixedRow, nLeftNonFixedGroup, nLeftNonFixedCol;

	if (IsSingleLevelMode())
		GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
	else
		GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	
	CRect rect;
	GetClientRect(rect);
	
	switch (nSBCode)
	{
	case SB_LINERIGHT:
		// scroll to right.
		if (scrollPos < m_nHScrollMax)
		{
			int xScroll;
			
			if (IsSingleLevelMode())
			// Scroll by 1 col.
				xScroll = GetColWidth(nLeftNonFixedCol);
			else
			// SCroll by 1 group.
				xScroll = GetGroupWidth(nLeftNonFixedGroup + 1);

			// Update tthe scroll position.
			SetScrollPos32(SB_HORZ, scrollPos + xScroll);
			if (GetScrollPos32(SB_HORZ) == scrollPos)
				break; // didn't work
			
			// Scroll the window left.
			rect.left = GetFixedColWidth() + xScroll + 1;
			ScrollWindow(-xScroll, 0, rect);
			rect.left = rect.right - xScroll;

			// Redraw the right rect.
			InvalidateRect(rect);
		}
		break;
		
	case SB_LINELEFT:
		// Scroll to left.
		if (scrollPos > 0 && ((IsSingleLevelMode() && nLeftNonFixedCol > m_nFrozenCols + 1) || (!IsSingleLevelMode() && nLeftNonFixedGroup > m_nFrozenGroups)))
		{
			int xScroll;
			
			if (IsSingleLevelMode())
			// Scroll by 1 col.
				xScroll = GetColWidth(nLeftNonFixedCol - 1);
			else
			// Scroll by 1 group.
				xScroll = GetGroupWidth(nLeftNonFixedGroup);

			// Update the scroll position.
			SetScrollPos32(SB_HORZ, max(0, scrollPos - xScroll));

			// Scroll the window to right.
			rect.left = GetFixedColWidth() + 1;
			ScrollWindow(xScroll, 0, rect);
			rect.right = rect.left + xScroll;

			// Redraw the left rect.
			InvalidateRect(rect);
		}
		break;
		
	case SB_PAGERIGHT:
		// Scroll to right by 1 page.
		if (scrollPos < m_nHScrollMax)
		{
			// Get the scrollable width.
			rect.left = GetFixedColWidth();
			int offset = rect.Width();
			int pos = min(m_nHScrollMax, scrollPos + offset);

			// Update the scroll position.
			SetScrollPos32(SB_HORZ, pos);

			rect.left = GetFixedColWidth();
			InvalidateRect(rect);
		}
		break;
		
	case SB_PAGELEFT:
		// Scroll to left by 1 page.
		if (scrollPos > 0)
		{
			// Calc the scrollable width.
			rect.left = GetFixedColWidth();
			int offset = -rect.Width();
			int pos = max(0, scrollPos + offset);

			// Update the scroll position.
			SetScrollPos32(SB_HORZ, pos);

			rect.left = GetFixedColWidth();
			InvalidateRect(rect);
		}
		break;
		
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		// Scroll to specified position
		{
			// Update the scroll position.
			SetScrollPos32(SB_HORZ, GetScrollPos32(SB_HORZ, TRUE));

			int nNewTopNonFixedRow, nNewLeftNonFixedGroup, nNewLeftNonFixedCol;
			
			if (IsSingleLevelMode())
			{
				GetTopleftNonFixedCell(nNewTopNonFixedRow, nNewLeftNonFixedCol);

				if (nNewLeftNonFixedCol != nLeftNonFixedCol)
				{
					rect.left = GetFixedColWidth();
					InvalidateRect(rect);
				}
			}
			else
			{
				GetTopleftNonFixedGroup(nNewTopNonFixedRow, nNewLeftNonFixedGroup);

				if (nNewLeftNonFixedGroup != nLeftNonFixedGroup)
				{
					rect.left = GetFixedColWidth();
					InvalidateRect(rect);
				}
			}
		}
		break;
		
	case SB_LEFT:
		if (scrollPos > 0)
		{
			SetScrollPos32(SB_HORZ, 0);
			Invalidate();
		}
		break;
		
	case SB_RIGHT:
		if (scrollPos < m_nHScrollMax)
		{
			SetScrollPos32(SB_HORZ, m_nHScrollMax);
			Invalidate();
		}
		break;
		
		
	default:
		break;
	}
}

void CGrid::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (GetFocus()->GetSafeHwnd() != GetSafeHwnd()) 
		SetFocus();  // Auto-destroy any InPlaceEdit's

	int scrollPos = GetScrollPos32(SB_VERT);
	
	int nTopNonFixedRow, nLeftNonFixedGroup;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	
	CRect rect;
	GetClientRect(rect);
	
	switch (nSBCode)
	{
	case SB_LINEDOWN:
		// Scroll down by 1 row.
		if (scrollPos < m_nVScrollMax)
		{
			int yScroll = m_nRowHeight;

			// UPdate the scroll position.
			SetScrollPos32(SB_VERT, scrollPos + yScroll);
			if (GetScrollPos32(SB_VERT) == scrollPos)
				break; // didn't work
			
			// Scroll window up.
			rect.top = GetFixedRowHeight() + yScroll;
			ScrollWindow(0, -yScroll, rect);

			// Redraw the bottom area.
			rect.top = GetFixedRowHeight();
			InvalidateRect(rect);
		}
		break;
		
	case SB_LINEUP:
		// Scroll up by 1 row.
		if (scrollPos > 0 && nTopNonFixedRow > m_nFrozenRows)
		{
			// Update the scroll position.
			int yScroll = m_nRowHeight;
			SetScrollPos32(SB_VERT, max(0, scrollPos - yScroll));

			// Scroll the window down.
			rect.top = GetFixedRowHeight();
			ScrollWindow(0, yScroll, rect);

			// Redraw the top area.
			rect.bottom = rect.top + yScroll;
			InvalidateRect(rect);
		}
		break;
		
	case SB_PAGEDOWN:
		// Scroll down by 1 page.
		if (scrollPos < m_nVScrollMax)
		{
			// Get the scrollable height.
			rect.top = GetFixedRowHeight();
			scrollPos = min(m_nVScrollMax, scrollPos + rect.Height());

			// Update the scroll position.
			SetScrollPos32(SB_VERT, scrollPos);

			// Redraw the grid.
			InvalidateRect(rect);
		}
		break;
		
	case SB_PAGEUP:
		// Scroll up by 1 page.
		if (scrollPos > 0)
		{
			// Get the scrollable height.
			rect.top = GetFixedRowHeight();
			int offset = -rect.Height();
			int pos = max(0, scrollPos + offset);

			// Update the scroll position.
			SetScrollPos32(SB_VERT, pos);

			// Redraw the grid.
			InvalidateRect(rect);
		}
		break;
		
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		// Scroll to specified position.
		{
			SetScrollPos32(SB_VERT, GetScrollPos32(SB_VERT, TRUE));

			int nNewTopNonFixedRow, nNewLeftNonFixedGroup;
			GetTopleftNonFixedGroup(nNewTopNonFixedRow, nNewLeftNonFixedGroup);

			if (nNewTopNonFixedRow != nTopNonFixedRow)
			{
				rect.top = GetFixedRowHeight();
				InvalidateRect(rect);
			}
		}
		break;
		
	case SB_TOP:
		if (scrollPos > 0)
		{
			SetScrollPos32(SB_VERT, 0);
			Invalidate();
		}
		break;
		
	case SB_BOTTOM:
		if (scrollPos < m_nVScrollMax)
		{
			SetScrollPos32(SB_VERT, m_nVScrollMax);
			Invalidate();
		}
		break;
		
		
	default:
		break;
	}
}

void CGrid::GetTopleftNonFixedCell(int &nRow, int &nCol)
/*
Routine Description:
	Get the top left visible non fixed cell.

Parameters:
	nRow		The result row ordinal.

	nCol		The result col ordinal.

Return Value:
	None.
*/
{
	// The level count should be 1 by calling this function.
	ASSERT(m_nLevels == 1);

	nRow = 1 + m_nFrozenRows;
	nCol = 1 + m_nFrozenCols;

	// Get the scroll position.
	int nVertScroll = GetScrollPos32(SB_VERT), 
		nHorzScroll = GetScrollPos32(SB_HORZ);
	
	// Calc the col ordinal.
	int nRight = 0;
	while (nRight < nHorzScroll && nCol < m_nCols - 1)
		nRight += m_arCols[nCol ++]->bVisible ? GetColWidth(nCol) : 0;
	
	if (nCol == m_nCols - 1)
		nCol = m_nCols - 1;

	// Calc the row ordinal.
	int nTop = 0;
	while (nTop < nVertScroll && nRow < m_nRows - 1)
	{
		nTop += m_nRowHeight;
		nRow ++;
	}
}

void CGrid::GetTopleftNonFixedGroup(int &nRow, int &nGroup)
/*
Routine Description:
	Get the top left visible non fixed group.

Parameters:
	nRow		The result row ordinal.

	nGroup		The result group ordinal.

Return Value:
	None.
*/
{
	nRow = 1 + m_nFrozenRows;
	nGroup = m_nFrozenGroups;

	// Get the scroll position.
	int nVertScroll = GetScrollPos32(SB_VERT), 
		nHorzScroll = GetScrollPos32(SB_HORZ);
	
	// Calc the group ordinal.
	int nRight = 0;
	while (nRight < nHorzScroll && nGroup < m_nGroups)
		nRight += m_arGroups[nGroup ++]->bVisible ? GetGroupWidth(nGroup) : 0;

	if (nGroup == m_nGroups)
		nGroup = m_nGroups - 1;
	
	// Calc the row ordinal.
	int nTop = 0;
	while (nTop < nVertScroll && nRow < m_nRows - 1)
	{
		nTop += m_nRowHeight;
		nRow ++;
	}
}

int CGrid::GetScrollPos32(int nBar, BOOL bGetTrackPos)
/*
Routine Description:
	Get the scroll position.

Parameters:
	nBar		The bar.

	bGetTrackPos	If get the position of the track button.

Return Value:
	None.
*/
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	
	if (bGetTrackPos)
	{
		if (GetScrollInfo(nBar, &si, SIF_TRACKPOS))
			return si.nTrackPos;
	}
	else 
	{
		if (GetScrollInfo(nBar, &si, SIF_POS))
			return si.nPos;
	}
	
	return 0;
}

BOOL CGrid::SetScrollPos32(int nBar, int nPos, BOOL bRedraw)
/*
Routine Description:
	Set the scroll position.

Parameters:
	nBar		The bar.

	nPos		The new position.

	bRedraw		If redraw the window after changing scroll position.

Return Value:
	None.
*/
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_POS;
	si.nPos   = nPos;
	
	if (!SetScrollInfo(nBar, &si, bRedraw))
		return FALSE;

	return TRUE;
}

int CGrid::GetFrozenRows()
/*
Routine Description:
	Get the count of frozen rows.

Parameters:
	None.

Return Value:
	The count.
*/
{
	return m_nFrozenRows;
}

BOOL CGrid::SetFrozenRows(int nRows)
/*
Routine Description:
	Set the count of frozen rows.

Parameters:
	nRows		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nRows < 0 || nRows > m_nRows - 1)
		return FALSE;

	m_nFrozenRows = nRows;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

int CGrid::GetFrozenGroups()
/*
Routine Description:
	Get the count of frozen groups.

Parameters:
	None.

Return Value:
	The count.
*/
{
	return m_nFrozenGroups;
}

BOOL CGrid::SetFrozenGroups(int nGroups)
/*
Routine Description:
	Set the count of frozen groups.

Parameters:
	nGroups		The new count.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nGroups < 0 || nGroups > m_nGroups)
		return FALSE;

	m_nFrozenGroups = nGroups;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

void CGrid::OnSize(UINT nType, int cx, int cy) 
{
	// Avoid reentance.
	if (m_bAlreadyInsideOnSize || !m_bAllowRedraw)
		return;

	if (!::IsWindow(m_hWnd))
		return;

	// Start re-entry blocking
	m_bAlreadyInsideOnSize = TRUE;

	if (m_bIPEdit)
		SetFocus(); // Auto-destroy any InPlaceEdit's

	CWnd::OnSize(nType, cx, cy);
	ResetScrollBars();	  

	// End re-entry blocking
	m_bAlreadyInsideOnSize = FALSE;
}

BOOL CGrid::IsRowVisible(int nRow)
/*
Routine Description:
	Decides if one row is visible.

Parameters:
	nRow

Return Value:
	The visibility.
*/
{
	// Get the scroll position.
	int nVertScroll = GetScrollPos32(SB_VERT);
	
	// The header height.
	int nTop = GetVirtualHeaderHeight();

	if (nRow == 0)
	// It is header row.
		return GetVirtualHeaderHeight();
	else if (nRow <= m_nFrozenRows)
		nTop += nRow * m_nRowHeight;
	else
	{
		nVertScroll += nTop;
		nTop += m_nFrozenRows * m_nRowHeight;
	}

	for (int nRowNow = m_nFrozenRows + 1; nRowNow < nRow; nRowNow ++)
	{
		nTop += m_nRowHeight;
	}

	return (nRow <= m_nFrozenRows && nTop <= m_rtGrid.Height()) || (nRow > m_nFrozenRows && nTop >= nVertScroll && nTop <= nVertScroll + m_rtGrid.bottom);
}

int CGrid::GetRowHeight(int nRow)
/*
Routine Description:
	Get height of one row.
.
Parameters:
	nRow

Return Value:
	The height.
*/
{
	return nRow == 0 ? m_nHeaderHeight: m_nRowHeight;
}

BOOL CGrid::GetColVisible(int nCol)
/*
Routine Description:
	Get if one col is visible.

Parameters:
	nCol

Return Value:
	The visibility.
*/
{
	nCol --;

	ASSERT(nCol >= 0 && nCol < m_nCols - 1);

	return m_arCols[nCol]->bVisible;
}

BOOL CGrid::GetGroupVisible(int nGroup)
/*
Routine Description:
	Get if one group is visible.

Parameters:
	nGroup

Return Value:
	The visibility.
*/
{
	nGroup --;

	ASSERT (nGroup >= 0 && nGroup < m_nGroups);

	return m_arGroups[nGroup]->bVisible;
}

BOOL CGrid::SetGroupVisible(int nGroup, BOOL b)
/*
Routine Description:
	Set the visibility of one group.

Parameters:
	nGroup

	b

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nGroup --;

	ASSERT (nGroup >= 0 && nGroup < m_nGroups);

	Group * pGroup = m_arGroups[nGroup];
	pGroup->bVisible = b;

	int nColStart = GetColFromGroup(nGroup, 0) + 1;
	int nColEnd = GetColFromGroup(nGroup, pGroup->nCols > 0 ? pGroup->nCols - 1 : 0) + 1;

	// Navigate to other cell.
	if (!b && pGroup->nCols > 0 && m_nCol >= nColStart && m_nCol <= nColEnd && m_nRow > 0)
	{
		if (!Navigate(m_nRow, nColEnd, NV_RIGHT) && !Navigate(m_nRow, nColStart, NV_LEFT) && !Navigate(m_nRow, 0, NV_RIGHT))
			m_nCol = 0;
	}
	else if (b && pGroup->nCols > 0 && m_nCol == 0 && m_nRow > 0)
		SetCurrentCell(m_nRow, nColStart);

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::SetColVisibleInGroup(int nGroup, int nCol, int b)
/*
Routine Description:
	Set the visibility of one col in one group.

Parameters:
	nGroup		Begins with 1.

	nCol		Begins with 1.

	b

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nGroup --;
	nCol --;

	int nLevel = GetLevelFromCol(nGroup, nCol);
	if (nLevel == INVALID)
		return FALSE;

	int nColOrd = GetColFromGroup(nGroup, nCol);
	Col * pCol = m_arCols[nColOrd];

	Level * pLevel = m_arGroups[nGroup]->arLevels[nLevel];
	if (pCol->bVisible && !b)
		pLevel->nColsVisible --;
	else if (!pCol->bVisible && b)
		pLevel->nColsVisible ++;

	pCol->bVisible = b;

	int nColWidth = pCol->nWidth;

	// Recalc width of each col in level.
	CalcColWidthInLevel(nGroup, nLevel);
	SetColWidth(nCol + 1, nColWidth);

	// Navigate to other cell.
	if (!b && m_nCol == nColOrd + 1 && m_nRow > 0)
	{
		if (!Navigate(m_nRow, m_nCol, NV_RIGHT) && !Navigate(m_nRow, m_nCol, NV_LEFT) && !Navigate(m_nRow, 0, NV_RIGHT))
			m_nCol = 0;
	}
	else if (b && m_nCol == 0 && m_nRow > 0)
		SetCurrentCell(m_nRow, nColOrd + 1);

	return TRUE;
}

void CGrid::CalcColWidthInLevel(int nGroup, int nLevel)
/*
Routine Description:
	Calc width of every col in one level.

Parameters:
	nGroup

	nLevel

Return Value:
	None.
*/
{
	ASSERT(nGroup >= 0 && nGroup < m_nGroups && nLevel >= 0 && nLevel < m_nLevels);

	Group * pGroup = m_arGroups[nGroup];
	Level * pLevel = pGroup->arLevels[nLevel];
	if (pLevel->nColsVisible == 0)
	// There is no cols visible in this level.
		return;

	int nColsWidth = 0;

	// Calc the total width of each col.
	for (int nCol = 0; nCol < pLevel->nCols; nCol ++)
	{
		int nColOrd = GetColFromLevel(nGroup, nLevel, nCol);
		Col * pCol = m_arCols[nColOrd];
		
		nColsWidth += pCol->bVisible ? pCol->nWidth : 0;
	}

	// The new total width should be equal to that of the group.
	float fRatio = pGroup->nWidth / (float)nColsWidth;

	int nColVisible;

	nColsWidth = 0;

	// Modify width of each col except the last col.
	for (nCol = 0, nColVisible = 0; nColVisible < pLevel->nColsVisible - 1 && nCol < pLevel->nCols - 1; nCol ++)
	{
		int nColOrd = GetColFromLevel(nGroup, nLevel, nCol);
		Col * pCol = m_arCols[nColOrd];

		if (!pCol->bVisible)
			continue;
		
		pCol->nWidth = max(1, (int)(pCol->nWidth * fRatio));
		nColsWidth += pCol->nWidth;
		nColVisible ++;
	}
	
	// Get the last col.
	while (!m_arCols[GetColFromLevel(nGroup, nLevel, nCol)]->bVisible && nCol < pLevel->nCols)
		nCol ++;

	// The width of the last col is the rest width.
	m_arCols[GetColFromLevel(nGroup, nLevel, nCol)]->nWidth = pGroup->nWidth - nColsWidth;

	if (m_bAllowRedraw)
		Invalidate();
}

BOOL CGrid::SetColVisible(int nCol, int b)
/*
Routine Description:
	Set the visibility of one col.

Parameters:
	nCol		Begins with 1.

	b

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	nCol --;

	if (nCol < 0 || nCol >= m_nCols - 1)
		return FALSE;

	if (IsSingleLevelMode())
	{
		// There is no group.

		// Save the new visibility.
		m_arCols[nCol]->bVisible = b;

		// Navigate to other cell.
		if (!b && m_nCol == nCol + 1 && m_nRow > 0)
		{
			if (!Navigate(m_nRow, m_nCol, NV_RIGHT) && !Navigate(m_nRow, m_nCol, NV_LEFT) && !Navigate(m_nRow, 0, NV_RIGHT))
				m_nCol = 0;
			else if (b && m_nCol == 0 && m_nRow > 0)
				SetCurrentCell(m_nRow, nCol + 1);
		}


		if (m_bAllowRedraw)
		{
			ResetScrollBars();
			Invalidate();
		}

		return TRUE;
	}

	// There are groups.
	int nGroup = GetGroupFromCol(nCol);
	if (nGroup == INVALID)
		return FALSE;

	return SetColVisibleInGroup(nGroup + 1, nCol + 1, b);
}

void CGrid::GetCurrentCell(int &nRow, int &nCol)
/*
Routine Description:
	Get th current cell.

Parameters:
	nRow

	nCol

Return Value:
	None.
*/
{
	nRow = m_nRow;
	nCol = m_nCol;
}

BOOL CGrid::SetCurrentCell(int nRow, int nCol)
/*
Routine Description:
	Set the current cell.

Parameters:
	nRow

	nCol

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nRow > 0 && nCol > 0);

	if (!m_bAllowRedraw)
		return FALSE;

	// Terminate inplace editing.
	if (m_bIPEdit)
		SetFocus();
	
	// If can not terminate inplace editing, return.
	if (m_bIPEdit)
		return FALSE;

	if (nRow == m_nRow && nCol == m_nCol)
		return TRUE;

	if (!IsValidCell(nRow, nCol) || !GetColVisible(nCol))
		return FALSE;

	// Reset selection.
	ResetSelection();

	int nRowPrev = m_nRow, nColPrev = m_nCol;

	if (nRow != nRowPrev)
	{
		// The current row will be changed, save all pending modification in current row.
		if (!FlushRecord())
			return FALSE;
	}

	m_nRow = nRow;
	m_nCol = nCol;

	if (IsValidCell(nRowPrev, nColPrev) && nRowPrev > 0 && nColPrev > 0)
	{
		// Update the cell state.
		SetCellState(nRowPrev, nColPrev, GetCellState(nRowPrev, nColPrev) & ~GVIS_FOCUSED);

		// Redraw previous cell.
		RedrawCell(nRowPrev, nColPrev);

		// Redraw current cell.
		RedrawCell(nRowPrev, 0);
	}

	if (nRow > 0 && nCol > 0)
	{
		if (m_bListMode)
		// Selete entire row in list mode.
			SelectRow(nRow);
		else
		{
			// Redraw current cell.
			SetCellState(m_nRow, m_nCol, GetCellState(m_nRow, m_nCol) | GVIS_FOCUSED);
			RedrawCell(m_nRow, m_nCol);
		}

		// Redraw the row header.
		RedrawCell(m_nRow, 0);
	}

	// Ensure current cell is visible.
	EnsureVisible(m_nRow, m_nCol);

	return TRUE;
}

BOOL CGrid::SetCellState(int nRow, int nCol, UINT state)
/*
Routine Description:
	Set the state of one cell.

Parameters:
	nRow

	nCol

	state

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	Cell * pCell = GetCell(nRow, nCol);
	ASSERT(pCell);
	if (!pCell)
		return FALSE;

	pCell->nState = state;

	return TRUE;
}

void CGrid::GetBottomrightNonFixedGroup(int &nRow, int &nGroup)
/*
Routine Description:
	Get the bottom right visible non fixed group.

Parameters:
	nRow

	nGroup

Return Value:
	None.
*/
{
	// Get the top left one.
	GetTopleftNonFixedGroup(nRow, nGroup);

	int nRight = m_arGroups[nGroup]->nWidth;
	int nRightLimit = m_rtGrid.Width() - GetFixedColWidth();

	while (nGroup + 1 < m_nGroups)
	{
		nRight += m_arGroups[nGroup + 1]->bVisible ? GetGroupWidth(nGroup + 2) : 0;
		if (nRight <= nRightLimit)
			nGroup ++;
		else
			break;
	}
	
	int nBottom = m_nRowHeight;
	int nBottomLimit = m_rtGrid.Height() - GetFixedRowHeight();

	while (nRow + 1 < m_nRows)
	{
		nBottom += m_nRowHeight;
		if (nBottom <= nBottomLimit)
			nRow ++;
		else
			break;
	}
}

void CGrid::GetBottomrightNonFixedCell(int &nRow, int &nCol)
/*
Routine Description:
	Get the bottom right visible non fixed cell.

Parameters:
	nRow

	nCol

Return Value:
	None.
*/
{
	// Get the top left one.
	GetTopleftNonFixedCell(nRow, nCol);

	int nRight = m_arCols[nCol - 1]->nWidth;
	int nRightLimit = m_rtGrid.Width() - GetFixedColWidth();

	while (nCol + 1 <= m_nCols - 1)
	{
		nRight += m_arCols[nCol]->bVisible ? GetColWidth(nCol + 1) : 0;
		if (nRight <= nRightLimit)
			nCol ++;
		else
			break;
	}
	
	int nBottom = m_nRowHeight;
	int nBottomLimit = m_rtGrid.Height() - GetFixedRowHeight();

	while (nRow + 1 < m_nRows)
	{
		nBottom += m_nRowHeight;
		if (nBottom <= nBottomLimit)
			nRow ++;
		else
			break;
	}
}

BOOL CGrid::GetCellFromPt(CPoint pt, int &nRow, int &nCol)
/*
Routine Description:
	Get the underlying cell from one point.

Parameters:
	pt

	nRow

	nCol

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (!m_rtGrid.PtInRect(pt))
		return FALSE;

	int nHeaderHeight = GetVirtualHeaderHeight();
	if (pt.y - m_rtGrid.top <= nHeaderHeight)
		nRow = 0;
	else
	{
		int nRowsHold = (pt.y - m_rtGrid.top - nHeaderHeight) / m_nRowHeight + 1;
		int nTopNonFixedRow, nLeftNonFixedGroup, nLeftNonFixedCol;

		if (IsSingleLevelMode())
			GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
		else
			GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

		if (nRowsHold <= m_nFrozenRows)
			nRow = nRowsHold;
		else
			nRow = nRowsHold + (nTopNonFixedRow - m_nFrozenRows) - 1;
	}

	if (nRow >= m_nRows)
		return FALSE;

	CRect rtCell;
	int nGroup;

	if (m_bShowRowHeader && pt.x - m_rtGrid.left <= 22)
	{
		nCol = 0;
			
		return TRUE;
	}

	if (nRow == 0 && m_nGroups && m_bShowGroupHeader && pt.y - m_rtGrid.top < m_nHeaderLevelHeight)
	{
		for (nGroup = 0; nGroup < m_nGroups; nGroup ++)
		{
			if (GetGroupHeaderRect(nGroup, rtCell) && rtCell.PtInRect(pt))
			{
				nCol = -1;
				nRow = nGroup;

				return TRUE;
			}
		}

		return FALSE;
	}

	for (nCol = 1; nCol < m_nCols; nCol ++)
	{
		if (GetCellRect(nRow, nCol, rtCell) && rtCell.PtInRect(pt))
			return TRUE;
	}

	return FALSE;
}

void CGrid::OnLButtonDown(UINT nFlags, CPoint point) 
{
	HWND hOldFocusWnd = ::GetFocus();

	// Save this point.
	m_LeftClickDownPoint = point;

	// Get the underlying cell.
	if (!GetCellFromPt(point, m_nLeftClickDownRow, m_nLeftClickDownCol))
		return;
	
	// Get the underlying group.
	int nColCopy = m_nLeftClickDownCol - 1;
	if (nColCopy >= 0)
		m_nLeftClickDownGroup = GetGroupFromCol(nColCopy);
	else if (nColCopy == -2)
		m_nLeftClickDownGroup = m_nLeftClickDownRow;

	if (!IsValidCell(m_nLeftClickDownRow, m_nLeftClickDownCol) && (m_nLeftClickDownGroup < 0 || m_nLeftClickDownGroup >= m_nGroups))
		return;
	
	SetFocus(); // Auto-destroy any InPlaceEdit's

	if (m_nLeftClickDownRow == m_nRow && m_nLeftClickDownCol == m_nCol && m_nRow > 0 && m_nCol > 0)
	{
		// Click on current cell again.
		// Prepare to edit.
		m_nMouseMode = MOUSE_PREPARE_EDIT;
		return;
	}
	else if (m_nMouseMode != MOUSE_OVER_COL_DIVIDE &&
		m_nMouseMode != MOUSE_OVER_ROW_DIVIDE && m_nMouseMode != MOUSE_OVER_GROUP_DIVIDE)
	{
		// If is inplace editing, ignore it.
		if (m_bIPEdit)
			return;

		// Clicked in one cell.
		OnLButtonClickedCell(m_nLeftClickDownRow, m_nLeftClickDownCol);
	}
	
	// Capture the mouse.
	SetCapture();
	
	if (m_nMouseMode == MOUSE_NOTHING)
	{
		if (m_bAllowGroupResize && MouseOverGroupResizeArea(point))
		{
			// Mouse is in the group resize area.
			if (m_nMouseMode != MOUSE_OVER_GROUP_DIVIDE)
			{
				// Start tracking mouse.
				if (!OnStartTracking(0, m_nLeftClickDownCol, TRACKGROUPWIDTH))
					return;

				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
				m_nMouseMode = MOUSE_OVER_GROUP_DIVIDE;
			}
		}
		else if (m_bAllowColResize && MouseOverColResizeArea(point))
		{
			// Mouse is in the col resize area.
			if (m_nMouseMode != MOUSE_OVER_COL_DIVIDE)
			{
				// Start tracking mouse.
				if (!OnStartTracking(0, m_nLeftClickDownCol, TRACKCOLWIDTH))
					return;

				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
				m_nMouseMode = MOUSE_OVER_COL_DIVIDE;
			}
		}
		else if (m_bAllowRowResize && MouseOverRowResizeArea(point))
		{
			// Mouse is in the col resize area.
			if (m_nMouseMode != MOUSE_OVER_ROW_DIVIDE)
			{
				/ Start tracking mouse.
				if (!OnStartTracking(0, m_nLeftClickDownCol, TRACKROWHEIGHT))
					return;

				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
				m_nMouseMode = MOUSE_OVER_ROW_DIVIDE;
			}
		}
	}
	
	if (m_nMouseMode == MOUSE_OVER_GROUP_DIVIDE) // sizing group
	{
		// Begin to resize group width.
		m_nMouseMode = MOUSE_SIZING_GROUP;
		CRect rtGroup;
		if (!GetGroupRect(0, m_nLeftClickDownGroup, rtGroup))
			return;
		
		// Draw a inverted line.
		CRect rect;
		GetClientRect(rect);
		CRect invertedRect(point.x, rect.top, point.x + 2, rect.bottom);
		
		CDC* pDC = GetDC();
		if (pDC)
		{
			pDC->InvertRect(&invertedRect);
			ReleaseDC(pDC);
		}
		
		if (point.x - rtGroup.left <= RESIZECAPTURERANGE) // clicked right of border
			if (!GetGroupRect(0, --m_nLeftClickDownGroup, rtGroup))
				return;
			
		rect.left = rtGroup.left;
		ClientToScreen(rect);
		ClipCursor(rect);
	}
	else if (m_nMouseMode == MOUSE_OVER_COL_DIVIDE) // sizing Col
	{
		// Begin to resize col width.

		m_nMouseMode = MOUSE_SIZING_COL;
		CRect rtCell;
		if (!GetCellRect(0, m_nLeftClickDownCol, rtCell))
			return;

		// Draw a inverted line.		
		CRect rect;
		GetClientRect(rect);
		CRect invertedRect(point.x, rect.top, point.x + 2, rect.bottom);
		
		CDC* pDC = GetDC();
		if (pDC)
		{
			pDC->InvertRect(&invertedRect);
			ReleaseDC(pDC);
		}
		
		if (point.x - rtCell.left <= RESIZECAPTURERANGE) // clicked right of border
			if (!GetCellRect(0, --m_nLeftClickDownCol, rtCell))
				return;
			
		rect.left = rtCell.left;
		ClientToScreen(rect);
		ClipCursor(rect);
	}
	else if (m_nMouseMode == MOUSE_OVER_ROW_DIVIDE) // sizing row
	{
		// Begin to resize row height.
		m_nMouseMode = MOUSE_SIZING_ROW;
		CRect rtCell;
		if (!GetCellRect(m_nLeftClickDownRow, m_nLeftClickDownCol, rtCell))
			return;
		
		// Draw a inverted line.
		CRect rect;
		GetClientRect(rect);
		CRect invertedRect(rect.left, point.y, rect.right, point.y + 2);
		
		CDC* pDC = GetDC();
		if (pDC)
		{
			pDC->InvertRect(&invertedRect);
			ReleaseDC(pDC);
		}
		
		if (point.y - rtCell.top <= RESIZECAPTURERANGE) // clicked below border
			if (!GetCellRect(--m_nLeftClickDownRow, 0, rtCell))
				return;
			
		rect.top = rtCell.top;
		ClientToScreen(rect);
		ClipCursor(rect);
	}
	else // not sizing or editing -- selecting
	{	
		if (m_nLeftClickDownCol == 0 && m_nLeftClickDownRow > 0)
			OnRowHeaderClick(m_nLeftClickDownRow);
		else if (m_nLeftClickDownCol < 0 && m_nLeftClickDownRow >= 0)
			OnGroupHeaderClick(m_nLeftClickDownRow);
		if (m_nLeftClickDownRow == 0 && m_nLeftClickDownCol > 0)
			OnColHeaderClick(m_nLeftClickDownCol);
	}

	m_LastMousePoint = point;
}

void CGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_nRow == 0 || m_nCol == 0)
		return;

	switch(nChar)
	{
	case VK_ESCAPE:
		CancelRecord();
		break;

	case VK_DELETE:
		if (m_nSelectMode == SELECTMODE_ROW && m_arSelection.GetSize() > 0)
			DeleteRecord(m_arSelection[0]);
		else
			OnEditCell(m_nRow, m_nCol, VK_DELETE);

		break;

	case VK_TAB:
		Navigate(m_nRow, m_nCol, NV_TAB);
		
		break;

	case VK_LEFT:
		Navigate(m_nRow, m_nCol, NV_LEFT);
		
		break;

	case VK_RIGHT:
		Navigate(m_nRow, m_nCol, NV_RIGHT);
		
		break;

	case VK_UP:
		Navigate(m_nRow, m_nCol, NV_UP);
		
		break;

	case VK_DOWN:
		Navigate(m_nRow, m_nCol, NV_DOWN);
		
		break;

	case VK_PRIOR:
		Navigate(m_nRow, m_nCol, NV_PRIOR);
		
		break;

	case VK_NEXT:
		Navigate(m_nRow, m_nCol, NV_NEXT);
		
		break;

	case VK_HOME:
		Navigate(m_nRow, m_nCol, NV_HOME);
		
		break;

	case VK_END:
		Navigate(m_nRow, m_nCol, NV_END);
		
		break;

	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

int CGrid::Navigate(int nStartRow, int nStartCol, int nDirection)
/*
Routine Description:
	Change current cell position.

Parameters:
	nStartRow		The start row ordinal.

	nStartCol		The start col ordinal.

	nDirection		The navigation direction.

Return Value:
	If navigate horizontal, return the result row ordinal; Otherwise, return the result col
	ordinal.
*/
{
	if (m_nRows < 2 || m_nCols < 2)
		return 0;

	SCROLLINFO si;
	int nPageSize = 0;
	if (GetScrollInfo(SB_VERT, &si, SIF_PAGE))
		nPageSize = si.nPage;

	int nTopNonFixedRow, nLeftNonFixedGroup, nBottomNonFixedRow, nRightNonFixedGroup, nLeftNonFixedCol, nRightNonFixedCol;
	
	if (IsSingleLevelMode())
	{
		GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
		GetBottomrightNonFixedCell(nBottomNonFixedRow, nRightNonFixedCol);
	}
	else
	{
		GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
		GetBottomrightNonFixedGroup(nBottomNonFixedRow, nRightNonFixedGroup);
	}

	int nBottomVisibleRow = nBottomNonFixedRow;
	CRect rtCell;
	if (nBottomVisibleRow > m_nFrozenRows)
	{
		int nHeight = GetFixedRowHeight() + (nBottomVisibleRow - nTopNonFixedRow) * m_nRowHeight;
		if (nHeight < m_rtGrid.Height())
			nBottomVisibleRow ++;
	}

	int nRow = nStartRow, nCol = nStartCol, nGroup;
	BOOL bFound = FALSE;
	int nColToFind;

	switch (nDirection)
	{
	case NV_TAB:
		
		if (IsSHIFTPressed())
		{
			if (Navigate(nStartRow, nStartCol, NV_LEFT))
				return 1;

			if ((nStartCol = Navigate(nStartRow, m_nCols, NV_LEFT)) == 0)
				return 0;
		
			if ((nStartRow = Navigate(nStartRow, nStartCol, NV_UP)) == 0)
				return Navigate(m_nRows, nStartCol, NV_UP);
		}
		else
		{
			if (Navigate(nStartRow, nStartCol, NV_RIGHT))
				return 1;
			
			if ((nStartCol = Navigate(nStartRow, 0, NV_RIGHT)) == 0)
				return 0;
			
			if ((nStartRow = Navigate(nStartRow, nStartCol, NV_DOWN)) == 0)
				return Navigate(0, nStartCol, NV_DOWN);
		}

		break;

	case NV_LEFT:
		nCol = nCol - 1;
		
		while(nCol > 0 && !bFound)
		{
			if (!GetColVisible(nCol))
			{
				nCol --;
				continue;
			}

			if (!IsSingleLevelMode())
			{
				nColToFind = nCol - 1;
				nGroup = GetGroupFromCol(nColToFind);
				bFound = nGroup != INVALID && m_arGroups[nGroup]->bVisible;
				if (!bFound)
					nCol --;
			}
			else
				bFound = TRUE;
		}

		if (!bFound)
			return 0;

		if (nRow > 0)
			SetCurrentCell(nRow, nCol);

		return nCol;
		
		break;

	case NV_RIGHT:
		nCol = nCol + 1;
		
		while(nCol < m_nCols && !bFound)
		{
			if (!GetColVisible(nCol))
			{
				nCol ++;
				continue;
			}

			if (!IsSingleLevelMode())
			{
				nColToFind = nCol - 1;
				nGroup = GetGroupFromCol(nColToFind);
				bFound = nGroup != INVALID && m_arGroups[nGroup]->bVisible;
				if (!bFound)
					nCol ++;
			}
			else
				bFound = TRUE;
		}

		if (!bFound)
			return 0;

		if (nRow > 0)
			SetCurrentCell(nRow, nCol);

		return nCol;
		
		break;

	case NV_UP:
		if (nRow <= 1)
			return 0;

		nRow = nRow - 1;

		if (nCol > 0)
			SetCurrentCell(nRow, nCol);

		return nRow;

		break;

	case NV_DOWN:
		if (nRow >= m_nRows - 1)
			return FALSE;

		nRow = nRow + 1;
		
		if (nCol > 0)
			SetCurrentCell(nRow, nCol);

		return nRow;

		break;

	case NV_PRIOR:
		if (nRow <= 1)
			return 0;

		if (m_nVScrollMax == 0)
			nRow = 1;
		else
			nRow = max(1, nRow - nPageSize / m_nRowHeight);

		if (nCol > 0)
			SetCurrentCell(nRow, nCol);

		return nRow;

		break;

	case NV_NEXT:
		if (nRow >= m_nRows)
			return 0;

		if (m_nVScrollMax == 0)
			nRow = m_nRows - 1;
		else
			nRow = min(m_nRows - 1, nRow + nPageSize / m_nRowHeight);

		if (nCol > 0)
			SetCurrentCell(nRow, nCol);

		return nRow;

		break;

	case NV_HOME:
		return Navigate(nStartRow, 0, NV_RIGHT);
		break;

	case NV_END:
		return Navigate(nStartRow, m_nCols, NV_LEFT);
		break;
	}

	return 0;
}

int CGrid::GetColFromLevel(int nGroup, int nLevel, int nCol)
/*
Routine Description:
	Get the global col ordinal from col ordinal in a level.

Parameters:
	nGroup

	nLevel

	nCol

Return Value:
	The result col ordinal.
*/
{
	ASSERT(nGroup >= 0 && nGroup < m_nGroups && nLevel >= 0 && nLevel < m_nLevels && nCol >= 0);

	int nRetCol = 0;
	for (int i = 0; i < nGroup; i ++)
		nRetCol += m_arGroups[i]->nCols;

	Group * pGroup = m_arGroups[nGroup];
	for (i = 0; i < nLevel; i ++)
		nRetCol += pGroup->arLevels[i]->nCols;

	nRetCol += nCol;

	return nRetCol;
}

int CGrid::GetColFromGroup(int nGroup, int nCol)
/*
Routine Description:
	Get the global col ordinal from that in a group.

Parameters:
	nGroup

	nCol.

Return Value:
	The result ordinal.
*/
{
	ASSERT(nGroup >= 0 && nGroup < m_nGroups && nCol >= 0);

	int nRetCol = 0;
	for (int i = 0; i < nGroup; i ++)
		nRetCol += m_arGroups[i]->nCols;

	nRetCol += nCol;

	return nRetCol;
}

int CGrid::GetFrozenCols()
/*
Routine Description:
	Get the count of frozen cols.

Parameters:
	None.

Return Value:
	The count.
*/
{
	return m_nFrozenCols;
}

BOOL CGrid::SetFrozenCols(int nCols)
/*
Routine Description:
	Set the count of frozen cols.

Parameters:
	nCols		The new value.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nCols < 0 || nCols > m_nCols - 1)
		return FALSE;

	m_nFrozenCols = nCols;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

void CGrid::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect(rect);
	
	// If outside client area, return (unless we are drag n dropping)
	if (m_nMouseMode != MOUSE_DRAGGING && !rect.PtInRect(point))
		return;

	// If the left mouse button is up, then test to see if row/Col sizing is imminent
	if (!(nFlags & MK_LBUTTON)) 
//		|| (m_nMouseMode == MOUSE_NOTHING && (nFlags & MK_LBUTTON)))
	{
		if (m_bAllowGroupResize && MouseOverGroupResizeArea(point))
		{
			if (m_nMouseMode != MOUSE_OVER_GROUP_DIVIDE)
			{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
				m_nMouseMode = MOUSE_OVER_GROUP_DIVIDE;
			}
		}
		else if (m_bAllowColResize && MouseOverColResizeArea(point))
		{
			if (m_nMouseMode != MOUSE_OVER_COL_DIVIDE)
			{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
				m_nMouseMode = MOUSE_OVER_COL_DIVIDE;
			}
		}
		else if (m_bAllowRowResize && MouseOverRowResizeArea(point))
		{
			if (m_nMouseMode != MOUSE_OVER_ROW_DIVIDE)
			{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
				m_nMouseMode = MOUSE_OVER_ROW_DIVIDE;
			}
		}
		else if (m_nMouseMode != MOUSE_NOTHING)
		{
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
			m_nMouseMode = MOUSE_NOTHING;
		}
		
		m_LastMousePoint = point;
		return;
	}
	
	if (!IsValidCell(m_nLeftClickDownRow, m_nLeftClickDownCol) && (m_nLeftClickDownGroup < 0 || m_nLeftClickDownGroup >= m_nGroups))
	{
		m_LastMousePoint = point;
		return;
	}
	
	// If the left mouse button is down, then process appropriately
	if (nFlags & MK_LBUTTON) 
	{
		switch(m_nMouseMode)
		{
		case MOUSE_SIZING_GROUP:
		{
			// Now is resizing group width.
			CDC* pDC = GetDC();
			if (!pDC)
				break;
			
			// Clear old border.
			CRect oldInvertedRect(m_LastMousePoint.x, rect.top, 
				m_LastMousePoint.x + 2, rect.bottom);
			pDC->InvertRect(&oldInvertedRect);

			// Invert new border.
			CRect newInvertedRect(point.x, rect.top, 
				point.x + 2, rect.bottom);
			pDC->InvertRect(&newInvertedRect);
			ReleaseDC(pDC);
		}
		
		break;
			
		case MOUSE_SIZING_COL:
		{
			// Now is resizing col width.
			CDC* pDC = GetDC();
			if (!pDC)
				break;
			
			// Clear old border.
			CRect oldInvertedRect(m_LastMousePoint.x, rect.top, 
				m_LastMousePoint.x + 2, rect.bottom);
			pDC->InvertRect(&oldInvertedRect);

			// Invert new border.
			CRect newInvertedRect(point.x, rect.top, 
				point.x + 2, rect.bottom);
			pDC->InvertRect(&newInvertedRect);
			ReleaseDC(pDC);
		}

		break;
			
		case MOUSE_SIZING_ROW:
		{
			// Now is resizing row height.

			CDC* pDC = GetDC();
			if (!pDC)
				break;

			// Clear old border.			
			CRect oldInvertedRect(rect.left, m_LastMousePoint.y, 
				rect.right, m_LastMousePoint.y + 2);
			pDC->InvertRect(&oldInvertedRect);

			// Invert new border.
			CRect newInvertedRect(rect.left, point.y, 
				rect.right, point.y + 2);
			pDC->InvertRect(&newInvertedRect);
			ReleaseDC(pDC);
		}
			
		break;

		case MOUSE_PREPARE_DRAG:
		{
			// Begin draging.
			m_nMouseMode = MOUSE_DRAGGING;
		}

		break;

		case MOUSE_DRAGGING:
		{
			// Now is dragging.
			SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG));
		}

		break;
		}
	}

	m_LastMousePoint = point;
}

BOOL CGrid::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT)
	{
		switch (m_nMouseMode) 
		{
			case MOUSE_OVER_COL_DIVIDE:
			case MOUSE_OVER_GROUP_DIVIDE:
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
				break;

			case MOUSE_OVER_ROW_DIVIDE:
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
				break;

			case MOUSE_DRAGGING:
				SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_DRAG));
				break;

			default:
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		}

		return TRUE;
	}
	
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CGrid::MouseOverRowResizeArea(CPoint &point)
/*
Routine Description:
	Decides if one point is in the row resize area.

Parameters:
	point

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	if (point.x >= (m_bShowRowHeader ? 22 : 0))
		return FALSE;
	
	int nRow, nCol;
	if (!GetCellFromPt(point, nRow, nCol) || nCol < 0)
		return FALSE;

	CPoint start;
	CRect rtCell;
	if (!GetCellRect(nRow, nCol, rtCell)) 
		return FALSE;
	
	int endy = rtCell.top + (nRow == 0 ? GetVirtualHeaderHeight() : m_nRowHeight);
	
	if ((point.y - rtCell.top <= RESIZECAPTURERANGE && nRow != 0) || 
		endy - point.y <= RESIZECAPTURERANGE)
		return TRUE;
	else
		return FALSE;
}

BOOL CGrid::MouseOverColResizeArea(CPoint &point)
/*
Routine Description:
	Decides if one point is in the col resize area.

Parameters:
	point

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	// The point should in the header area.
	if (point.y >= GetVirtualHeaderHeight())
		return FALSE;
	
	int nRow, nCol;
	if (!GetCellFromPt(point, nRow, nCol) || nCol < 1)
		return FALSE;

	CRect rtCell;
	if (!GetCellRect(nRow, nCol, rtCell)) 
		return FALSE;
	
	int endx = rtCell.left + GetColWidth(nCol);
	
	if (endx - point.x <= RESIZECAPTURERANGE && m_arCols[nCol - 1]->bAllowSizing)
		return TRUE;
	else
		return FALSE;
}

void CGrid::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);
	
	KillTimer();
	ClipCursor(NULL);

	// m_nMouseMode == MOUSE_PREPARE_EDIT only if user clicked down on current cell
	// and then didn't move mouse before clicking up (releasing button)
	if (m_nMouseMode == MOUSE_PREPARE_EDIT)	
	{
		OnEditCell(m_nRow, m_nCol, VK_LBUTTON);
	}
	// m_MouseMode == MOUSE_PREPARE_DRAG only if user clicked down on a selected cell
	// and then didn't move mouse before clicking up (releasing button)
	else if (m_nMouseMode == MOUSE_PREPARE_DRAG) 
	{
		ResetSelection();
	}
	else if (m_nMouseMode == MOUSE_SIZING_GROUP)
	{
		// End of changing group width.

		// Clear old border.
		CRect rect;
		GetClientRect(rect);
		CRect invertedRect(m_LastMousePoint.x, rect.top, m_LastMousePoint.x + 2, rect.bottom);
		
		CDC* pDC = GetDC();
		if (pDC)
		{
			pDC->InvertRect(&invertedRect);
			ReleaseDC(pDC);
		}
		
		if (m_LeftClickDownPoint != point)
		{   
			CRect rtGroup;
			if (!GetGroupRect(m_nLeftClickDownRow, m_nLeftClickDownGroup, rtGroup))
				goto exit;

			// Set new group width.
			StoreGroupWidth(m_nLeftClickDownGroup, point.x - rtGroup.left);
			ResetScrollBars();
			Invalidate();
		}
	}
	else if (m_nMouseMode == MOUSE_SIZING_COL)
	{
		// End of changing col width.

		// Clear old border.
		CRect rect;
		GetClientRect(rect);
		CRect invertedRect(m_LastMousePoint.x, rect.top, m_LastMousePoint.x + 2, rect.bottom);
		
		CDC* pDC = GetDC();
		if (pDC)
		{
			pDC->InvertRect(&invertedRect);
			ReleaseDC(pDC);
		}
		
		if (m_LeftClickDownPoint != point) 
		{   
			CRect rtCell;
			if (!GetCellRect(m_nLeftClickDownRow, m_nLeftClickDownCol, rtCell))
				goto exit;

			// Set new col width.
			StoreColWidth(m_nLeftClickDownCol, point.x - rtCell.left);
			ResetScrollBars();
			Invalidate();
		}
	}
	else if (m_nMouseMode == MOUSE_SIZING_ROW)
	{
		// End of chaning row height.

		// Clear the old border.
		CRect rect;
		GetClientRect(rect);
		CRect invertedRect(rect.left, m_LastMousePoint.y, rect.right, m_LastMousePoint.y + 2);
		
		CDC* pDC = GetDC();
		if (pDC)
		{
			pDC->InvertRect(&invertedRect);
			ReleaseDC(pDC);
		}
		
		if (m_LeftClickDownPoint != point) 
		{
			CRect rtCell;
			if (!GetCellRect(m_nLeftClickDownRow, m_nLeftClickDownCol, rtCell))
				goto exit;
			
			// Store new row height.
			if (m_nLeftClickDownRow == 0)
				StoreHeaderHeight(point.y - rtCell.top - (m_bShowGroupHeader ? m_nHeaderLevelHeight : 0));
			else
				StoreRowHeight(point.y - rtCell.top);

			ResetScrollBars();
			Invalidate();
		}
	}
	else if (m_nMouseMode == MOUSE_DRAGGING)
	{
		// End of draging.

		int nRowDrop, nColDrop, nGroupDrop, nLevelDrop, nLevelFrom, nColFrom, nGroupFrom;
		int nLeftNonFixedGroup, nLeftNonFixedCol, nTopNonFixedRow;
		
		if (IsSingleLevelMode())
		{
			// It is moving col.
			GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);

			if (GetCellFromPt(point, nRowDrop, nColDrop))
			{
				if (nColDrop < nLeftNonFixedCol)
					goto exit;
			}
			else
			{
				int nRightNonFixedCol;
				GetBottomrightNonFixedCell(nTopNonFixedRow, nRightNonFixedCol);

				CRect rtCell;
				if (!GetCellRect(nTopNonFixedRow, nRightNonFixedCol, rtCell))
					goto exit;

				if (point.x < m_rtGrid.right && point.x > rtCell.right)
					nColDrop = m_nCols;
				else
					goto exit;
			}
			
			// Move the col to new position.
			MoveCol(m_nLeftClickDownCol, nColDrop);
		}
		else
		{
			// If we are moving col?
			BOOL bColMode = (m_nSelectMode == SELECTMODE_COL);

			GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

			CRect rtGroup;

			for (nGroupDrop = (bColMode ? 0 : nLeftNonFixedGroup); nGroupDrop < m_nGroups; nGroupDrop ++)
			{
				if (GetGroupRect(nTopNonFixedRow, nGroupDrop, rtGroup) && point.x >= rtGroup.left && point.x <= rtGroup.right)
					break;
			}

			if (nGroupDrop >= m_nGroups)
			{
				int nRightNonFixedGroup;
				GetBottomrightNonFixedGroup(nTopNonFixedRow, nRightNonFixedGroup);

				if (!GetGroupRect(nTopNonFixedRow, nRightNonFixedGroup, rtGroup))
					goto exit;

				if (point.x < m_rtGrid.right && point.x > rtGroup.right)
					nGroupDrop = m_nGroups;
				else
					goto exit;
			}

			if (!bColMode)
			{
				if (nGroupDrop == m_nLeftClickDownRow)
					goto exit;

				// Move group to new position.
				MoveGroup(m_nLeftClickDownRow, nGroupDrop);
			}
			else
			{
				nColFrom = m_nLeftClickDownCol - 1;
				nGroupFrom = GetGroupFromCol(nColFrom);
				if (nGroupDrop < nGroupFrom || nGroupFrom == INVALID)
					goto exit;

				nLevelFrom = GetLevelFromCol(nGroupFrom, nColFrom);

				nRowDrop = -1, nColDrop == 0;
				GetCellFromPt(point, nRowDrop, nColDrop);
				if (nRowDrop == -1 || nColDrop <= 0)
					goto exit;

				if (nRowDrop != 0)
					goto exit;

				nLevelDrop = (m_nGroups && m_bShowGroupHeader ? point.y - m_nHeaderLevelHeight : point.y) / m_nHeaderLevelHeight;
				nLevelDrop = min(m_nLevels - 1, nLevelDrop);

				Level * pLevel = m_arGroups[nGroupFrom]->arLevels[nLevelDrop];

				if (pLevel->nColsVisible == 0 || nGroupDrop > nGroupFrom)
					nColDrop = pLevel->nCols;
				else
				{
					if (!GetCellFromPt(point, nRowDrop, nColDrop))
						goto exit;

					nColDrop --;
					nGroupDrop = GetGroupFromCol(nColDrop);
					nLevelDrop = GetLevelFromCol(nGroupDrop, nColDrop);
				}

				// Move col to new position.
				MoveColInGroup(nGroupFrom, nLevelFrom, nLevelDrop, nColFrom, nColDrop);
			}
		}

		Invalidate();
	}

exit:

	if (GetCapture()->GetSafeHwnd() == GetSafeHwnd())
	{
		// Release mouse capturing.
		ReleaseCapture();
	}
	
	m_nMouseMode = MOUSE_NOTHING;
	
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

BOOL CGrid::MouseOverGroupResizeArea(CPoint &point)
/*
Routine Description:
	Decides if one point is in the group resize area.

Parameters:
	point

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	// Should have groups and the point should in the header area to return TRUE.
	if (IsSingleLevelMode() || point.y >= GetVirtualHeaderHeight())
		return FALSE;
	
	int nRow, nCol;
	if (!GetCellFromPt(point, nRow, nCol) || (nCol < 1 && nCol != -1))
		return FALSE;

	int nGroup;

	if (nCol == -1)
		nGroup = nRow;
	else
	{
		nCol --;
		nGroup = GetGroupFromCol(nCol);
		if (nGroup == INVALID)
			return FALSE;
	}

	CRect rtGroup;
	if (!GetGroupRect(nCol == -1 ? 0 : nRow, nGroup, rtGroup))
		return FALSE;
	
	int endx = rtGroup.left + GetGroupWidth(nGroup + 1);
	
	if (endx - point.x <= RESIZECAPTURERANGE && m_arGroups[nGroup]->bAllowSizing)
		return TRUE;
	else
		return FALSE;
}

void CGrid::SelectRow(int nRow, BOOL bSelect)
/*
Routine Description:
	Select a row or not.

Parameters:
	nRow

	bSelect		The selection state.

Return Value:
	None.
*/
{
	ASSERT(nRow > 0 && nRow < m_nRows);

	if (m_nSelectMode != SELECTMODE_NONE && m_nSelectMode != SELECTMODE_ROW)
	{
		// Reset selection if there is cols or groups selected before selecting a row.
		if (bSelect)
			ResetSelection();
		else
			return;
	}

	int i = IsRowSelected(nRow);

	if ((bSelect && i) || (!bSelect && !i))
		return;

	if (bSelect)
	{
		// Select this row.
		m_arSelection.Add(nRow);

		// Update the select mode.
		m_nSelectMode = SELECTMODE_ROW;
	}
	else
	{
		// Deselect this row.
		m_arSelection.RemoveAt(i - 1);

		// Update the select mode.
		if (m_arSelection.GetSize() == 0)
			m_nSelectMode = SELECTMODE_NONE;
	}

	// Redraw row.
	if (bSelect)
	{
		for (i = 1; i < m_nCols; i ++)
		{
			SetCellState(nRow, i, GetCellState(nRow, i) | GVIS_SELECTED);
			RedrawCell(nRow, i);
		}
	}
	else
	{
		for (i = 1; i < m_nCols; i ++)
		{
			SetCellState(nRow, i, GetCellState(nRow, i) & ~GVIS_SELECTED);
			RedrawCell(nRow, i);
		}
	}
}

void CGrid::SelectCol(int nCol, BOOL bSelect)
/*
Routine Description:
	Select a col or not.

Parameters:
	nCol

	bSelect		The selection state.

Return Value:
	None.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	if (m_nSelectMode != SELECTMODE_NONE && m_nSelectMode != SELECTMODE_COL)
	{
		// Reset selection if there is rows or groups selected before selecting a col.
		if (bSelect)
			ResetSelection();
		else
			return;
	}

	int i = IsColSelected(nCol);

	if ((bSelect && i) || (!bSelect && !i))
		return;

	if (bSelect)
	{
		// Select this col.
		ResetSelection();
		m_arSelection.Add(nCol);

		// Update the select mode.
		m_nSelectMode = SELECTMODE_COL;
	}
	else
	{
		// Deselect this col.
		m_arSelection.RemoveAt(i - 1);

		// Update the select mode.
		if (m_arSelection.GetSize() == 0)
			m_nSelectMode = SELECTMODE_NONE;
	}

	if (bSelect)
	{
		for (i = 1; i < m_nRows; i ++)
			SetCellState(i, nCol, GetCellState(i, nCol) | GVIS_SELECTED);
	}
	else
	{
		for (i = 1; i < m_nRows; i ++)
			SetCellState(i, nCol, GetCellState(i, nCol) & ~GVIS_SELECTED);
	}

	RedrawCol(nCol);
}

void CGrid::SelectGroup(int nGroup, BOOL bSelect)
/*
Routine Description:
	Select a group or not.

Parameters:
	nGroup

	bSelect		The selection state.

Return Value:
	None.
*/
{
	nGroup --;
	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	if (m_nSelectMode != SELECTMODE_NONE && m_nSelectMode != SELECTMODE_GROUP)
	{
		// Reset selection if there is rows or cols selected before selecting a group.
		if (bSelect)
			ResetSelection();
		else
			return;
	}

	int i = IsGroupSelected(nGroup + 1);

	if ((bSelect && i) || (!bSelect && !i))
		return;

	if (bSelect)
	{
		// Select this group.
		ResetSelection();
		m_arSelection.Add(nGroup);

		// Update the selection mode.
		m_nSelectMode = SELECTMODE_GROUP;
	}
	else
	{
		// Deselect this group.
		m_arSelection.RemoveAt(i - 1);

		// Update the selection mode.
		if (m_arSelection.GetSize() == 0)
			m_nSelectMode = SELECTMODE_NONE;
	}

	Group * pGroup = m_arGroups[nGroup];
	if (pGroup->nCols == 0)
		return;

	int nColStart = GetColFromGroup(nGroup, 0) + 1;
	int nCol;

	if (bSelect)
	{
		for (nCol = nColStart; nCol < nColStart + pGroup->nCols; nCol ++)
		{
			for (i = 1; i < m_nRows; i ++)
				SetCellState(i, nCol, GetCellState(i, nCol) | GVIS_SELECTED);
		
//			RedrawCol(nCol);
		}
	}
	else
	{
		for (nCol = nColStart; nCol < nColStart + pGroup->nCols; nCol ++)
		{
			for (i = 1; i < m_nRows; i ++)
				SetCellState(i, nCol, GetCellState(i, nCol) & ~GVIS_SELECTED);

//			RedrawCol(nCol);
		}
	}

	RedrawGroup(nGroup + 1);
}

void CGrid::ResetSelection()
/*
Routine Description:
	Deselect all.

Parameters:
	None.

Return Value:
	None.
*/
{
	if (m_nSelectMode == SELECTMODE_NONE || m_arSelection.GetSize() == 0)
		return;

	KillTimer();

	CArray<int, int> arSelectionCopy;
	arSelectionCopy.Append(m_arSelection);
	int i;

	switch (m_nSelectMode)
	{
	case SELECTMODE_ROW:
		for (i = 0; i < arSelectionCopy.GetSize(); i ++)
			SelectRow(arSelectionCopy[i], FALSE);

		break;

	case SELECTMODE_COL:
		for (i = 0; i < arSelectionCopy.GetSize(); i ++)
			SelectCol(arSelectionCopy[i], FALSE);

		break;

	case SELECTMODE_GROUP:
		for (i = 0; i < arSelectionCopy.GetSize(); i ++)
			SelectGroup(arSelectionCopy[i] + 1, FALSE);

		break;
	}

	arSelectionCopy.RemoveAll();

	m_nSelectMode = SELECTMODE_NONE;
}

int CGrid::IsRowSelected(int nRow)
/*
Routine Description:
	Decides if one row is selected.

Parameters:
	nRow

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nRow > 0 && nRow < m_nRows);

	if (m_nSelectMode != SELECTMODE_ROW)
		return 0;

	for (int i = 0; i < m_arSelection.GetSize(); i ++)
	{
		if (m_arSelection[i] == nRow)
			return i + 1;
	}

	return 0;
}

int CGrid::IsColSelected(int nCol)
/*
Routine Description:
	Decides if one col is selected.

Parameters:
	nRow

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	if (m_nSelectMode != SELECTMODE_COL)
		return 0;

	for (int i = 0; i < m_arSelection.GetSize(); i ++)
	{
		if (m_arSelection[i] == nCol)
			return i + 1;
	}

	return 0;
}

int CGrid::IsGroupSelected(int nGroup)
/*
Routine Description:
	Decides if one group is selected.

Parameters:
	nRow

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	nGroup --;

	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	if (m_nSelectMode != SELECTMODE_GROUP)
		return 0;

	for (int i = 0; i < m_arSelection.GetSize(); i ++)
	{
		if (m_arSelection[i] == nGroup)
			return i + 1;
	}

	return 0;
}

void CGrid::OnRowHeaderClick(int nRow)
/*
Routine Description:
	One row header is clicked.

Parameters:
	nRow

Return Value:
	None.
*/
{
	// Toggle the selection of this row.
	if (m_bAllowSelectRow)
		SelectRow(nRow, !IsRowSelected(nRow));
}

void CGrid::OnColHeaderClick(int nCol)
/*
Routine Description:
	One col header is clicked.

Parameters:
	nCol

Return Value:
	None.
*/
{
	if (IsColSelected(nCol))
	{
		// Prepare to drag this col.
		if (IsSingleLevelMode())
			m_nTimerID = SetTimer(WM_LBUTTONDOWN, TIMERINTERVAL, 0);

		m_nMouseMode = MOUSE_PREPARE_DRAG;
	}
	else
	{
		// Toggle the selection of this col.

		if (!m_bAllowMoveCol || (IsSingleLevelMode() && nCol <= m_nFrozenCols))
			return;

		SelectCol(nCol);
	}
}

void CGrid::OnGroupHeaderClick(int nGroup)
/*
Routine Description:
	One row header is clicked.

Parameters:
	nRow

Return Value:
	None.
*/
{
	if (IsGroupSelected(nGroup + 1))
	{
		// Prepare to drag this group.
		m_nTimerID = SetTimer(WM_LBUTTONDOWN, TIMERINTERVAL, 0);
		m_nMouseMode = MOUSE_PREPARE_DRAG;
	}
	else if (m_bAllowMoveGroup && nGroup >= m_nFrozenGroups)
		// Toggle the selection of this row.
		SelectGroup(nGroup + 1);
}

void CGrid::OnTimer(UINT nIDEvent) 
{
	// If the mouse cursor is out of grid window and if we are draging somthing, scroll the
	// grid each time.
	ASSERT(nIDEvent == WM_LBUTTONDOWN);
	if (nIDEvent != WM_LBUTTONDOWN)
		return;

	CPoint pt;

	if (!GetCursorPos(&pt))
		return;

	ScreenToClient(&pt);

	CRect rect;
	GetClientRect(rect);

	int nFixedColWidth = GetFixedColWidth();

	if (pt.x > rect.right)
	{
		SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);

		if (pt.y < rect.top)	
			pt.y = rect.top;
		if (pt.y > rect.bottom) 
			pt.y = rect.bottom;
		pt.x = rect.right;
//		OnSelecting(GetCellFromPt(pt));
	}
	else if (pt.x < nFixedColWidth)
	{
		SendMessage(WM_HSCROLL, SB_LINELEFT, 0);

		if (pt.y < rect.top)	
			pt.y = rect.top;
		if (pt.y > rect.bottom)
			pt.y = rect.bottom;
		pt.x = nFixedColWidth + 1;
//		OnSelecting(GetCellFromPt(pt));
	}
}

void CGrid::OnCaptureChanged(CWnd *pWnd) 
{
	if (pWnd->GetSafeHwnd() == GetSafeHwnd()) return;

	KillTimer();

	// Kill drag and drop if active
	if (m_nMouseMode == MOUSE_DRAGGING)
		m_nMouseMode = MOUSE_NOTHING;
}

// kill timer if active
void CGrid::KillTimer()
/*
Routine Description:
	Kill the timer.

Parameters:
	None.

Return Value:
	None.
*/
{
	if (m_nTimerID != 0)
	{
		CWnd::KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}
}

BOOL CGrid::MoveCol(int nColFrom, int nColTo)
/*
Routine Description:
	Move one col to new position.

Parameters:
	nColFrom

	nColTo

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
//	ASSERT(IsSingleLevelMode() && nColFrom > m_nFrozenCols && nColFrom < m_nCols && nColTo > m_nFrozenCols && nColTo <= m_nCols);

	if (nColFrom == nColTo)
		return FALSE;

//	ASSERT(m_nSelectMode == SELECTMODE_COL && m_arSelection.GetSize() == 1 && m_arSelection[0] == nColFrom);

	// Make a copy of cells array.
	CArray<Cell *, Cell *> arCellsCopy;
	CArray<Col *, Col *> arColsCopy;

	arCellsCopy.Append(m_arCells);
	arColsCopy.Append(m_arCols);

	if (nColFrom > nColTo)
	{
		// Move from right to left.

		// Move cells in array.
		m_arSelection[0] = nColTo;

		if (m_nCol == nColFrom)
			m_nCol = nColTo;
		else if (m_nCol >= nColTo && m_nCol < nColFrom)
			m_nCol ++;

		arColsCopy[nColTo - 1] = m_arCols[nColFrom - 1];

		for (int i = nColTo; i < nColFrom; i ++)
			arColsCopy[i] = m_arCols[i - 1];

		for (int nRow = 1; nRow < m_nRows; nRow ++)
		{
			arCellsCopy[m_nCols * (nRow - 1) + nColTo] = m_arCells[m_nCols * (nRow - 1) + nColFrom];

			for (i = nColTo + 1; i <= nColFrom; i ++)
				arCellsCopy[m_nCols * (nRow - 1) + i] = m_arCells[m_nCols * (nRow - 1) + i - 1];
		}
	}
	else
	{
		// Move from left to right.

		// Move cells in array.
		m_arSelection[0] = nColTo - 1;

		if (m_nCol == nColFrom)
			m_nCol = nColTo - 1;
		else if (m_nCol > nColFrom && m_nCol < nColTo)
			m_nCol --;

		arColsCopy[nColTo - 2] = m_arCols[nColFrom - 1];

		for (int i = nColFrom; i < nColTo - 1; i ++)
			arColsCopy[i - 1] = m_arCols[i];

		for (int nRow = 1; nRow < m_nRows; nRow ++)
		{
			arCellsCopy[m_nCols * (nRow - 1) + nColTo - 1] = m_arCells[m_nCols * (nRow - 1) + nColFrom];

			for (i = nColFrom; i < nColTo - 1; i ++)
				arCellsCopy[m_nCols * (nRow - 1) + i] = m_arCells[m_nCols * (nRow - 1) + i + 1];
		}
	}
	

	// Reallocate memory and copy data back.
	m_arCols.RemoveAll();
	m_arCols.Append(arColsCopy);
	m_arCells.RemoveAll();
	m_arCells.Append(arCellsCopy);
	arColsCopy.RemoveAll();
	arCellsCopy.RemoveAll();

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::MoveGroup(int nGroupFrom, int nGroupTo)
/*
Routine Description:
	Move a group to new position.

Parameters:
	nGroupFrom

	nGroupTo

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nGroupFrom >= 0 && nGroupTo >= 0 && nGroupFrom < m_nGroups && nGroupTo <= m_nGroups);

	if (nGroupFrom == nGroupTo)
		return FALSE;

	CArray<Group *, Group *> arGroupsCopy;
	arGroupsCopy.Append(m_arGroups);

	int nColFromOrd = GetColFromGroup(nGroupFrom, 0) + 1;
	int nColToOrd = nGroupTo == m_nGroups ? m_nCols : GetColFromGroup(nGroupTo, 0) + 1;
	int nColsToMove = m_arGroups[nGroupFrom]->nCols;

	int nLeftNonFixedGroup, nTopNonFixedRow;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	if (nGroupFrom > nGroupTo)
	{
		for (int i = 0; i < nColsToMove; i ++)
			MoveCol(nColFromOrd + i, nColToOrd + i);

		m_arSelection[0] = nGroupTo;

		arGroupsCopy[nGroupTo] = m_arGroups[nGroupFrom];

		for (i = nGroupTo + 1; i <= nGroupFrom; i ++)
			arGroupsCopy[i] = m_arGroups[i - 1];
	}
	else
	{
		for (int i = 0; i < nColsToMove; i ++)
			MoveCol(nColFromOrd, nColToOrd);

		m_arSelection[0] = nGroupTo - 1;

		arGroupsCopy[nGroupTo - 1] = m_arGroups[nGroupFrom];

		for (i = nGroupFrom; i < nGroupTo - 1; i ++)
			arGroupsCopy[i] = m_arGroups[i + 1];

		if (nLeftNonFixedGroup > nGroupFrom && nLeftNonFixedGroup <= nGroupTo)
		{
			nLeftNonFixedGroup --;
			int nNewHPos = 0;

			for (int i = 0; i < nLeftNonFixedGroup; i ++)
				nNewHPos += m_arGroups[nLeftNonFixedGroup]->bVisible ? m_arGroups[nLeftNonFixedGroup]->nWidth : 0;

			SetScrollPos32(SB_HORZ, nNewHPos);
		}
	}

	m_arGroups.RemoveAll();
	m_arGroups.Append(arGroupsCopy);
	arGroupsCopy.RemoveAll();

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}

	return TRUE;
}

BOOL CGrid::MoveColInGroup(int nGroup, int nLevelFrom, int nLevelTo, int nColFrom, int nColTo)
/*
Routine Description:
	Move col within a group.

Parameters:
	nGroup

	nLevelFrom

	nLevelTo

	nColFrom

	nColTo

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nLevelFrom == nLevelTo && nColFrom == nColTo)
		return FALSE;

	Group * pGroup = m_arGroups[nGroup];
	Level * pLevelFrom = pGroup->arLevels[nLevelFrom];
	Level * pLevelTo = pGroup->arLevels[nLevelTo];

	int nColFromOrd = GetColFromLevel(nGroup, nLevelFrom, nColFrom) + 1;
	int nColToOrd = GetColFromLevel(nGroup, nLevelTo, nColTo) + 1;

	MoveCol(nColFromOrd, nColToOrd);

	if (nLevelFrom != nLevelTo)
	{
		// Move col between levels.
		pLevelFrom->nCols --;
		pLevelFrom->nColsVisible --;
		pLevelTo->nCols ++;
		pLevelTo->nColsVisible ++;

		// Recalc col width in levels.
		CalcColWidthInLevel(nGroup, nLevelFrom);
		CalcColWidthInLevel(nGroup, nLevelTo);
	}

	if (m_bAllowRedraw)
		RedrawGroup(nGroup + 1);

	return TRUE;
}

BOOL CGrid::GetShowRowHeader()
/*
Routine Description:
	Get the permission to show row header.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bShowRowHeader;
}

void CGrid::SetShowRowHeader(BOOL b)
/*
Routine Description:
	Set the permission to show row header.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bShowRowHeader = b;

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}
}

BOOL CGrid::GetShowColHeader()
/*
Routine Description:
	Get the permission to show col header.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bShowColHeader;
}

void CGrid::SetShowColHeader(BOOL b)
/*
Routine Description:
	Set the permission to show col header.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bShowColHeader = b;

	CalcHeaderHeight();
}

BOOL CGrid::GetShowGroupHeader()
/*
Routine Description:
	Get the permission to show group header.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bShowGroupHeader;
}

void CGrid::SetShowGroupHeader(BOOL b)
/*
Routine Description:
	Set the permission to show group header.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bShowGroupHeader = b;

	CalcHeaderHeight();
}

BOOL CGrid::GetAllowColResize()
/*
Routine Description:
	Get the permission to resize col width.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowColResize;
}

void CGrid::SetAllowColResize(BOOL b)
/*
Routine Description:
	Set the permission to resize col width.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowColResize = b;
}

BOOL CGrid::GetAllowRowResize()
/*
Routine Description:
	Get the permission to resize row height.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowRowResize;
}

void CGrid::SetAllowRowResize(BOOL b)
/*
Routine Description:
	Set the permission to resize row height.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowRowResize = b;
}

BOOL CGrid::GetAllowGroupResize()
/*
Routine Description:
	Get the permission to resize group width.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowGroupResize;
}

void CGrid::SetAllowGroupResize(BOOL b)
/*
Routine Description:
	Set the permission to group width.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowGroupResize = b;
}

BOOL CGrid::GetAllowMoveCol()
/*
Routine Description:
	Get the permission to move col.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowMoveCol;
}

void CGrid::SetAllowMoveCol(BOOL b)
/*
Routine Description:
	Set the permission to move col.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowMoveCol = b;
}

BOOL CGrid::GetAllowMoveGroup()
/*
Routine Description:
	Get the permission to move group.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowMoveGroup;
}

void CGrid::SetAllowMoveGroup(BOOL b)
/*
Routine Description:
	Set the permission to move group.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowMoveGroup = b;
}

int CGrid::GetSelectedRows(CArray<int, int> &arRows)
{
	if (m_nSelectMode != SELECTMODE_ROW)
		return 0;

	arRows.RemoveAll();
	arRows.Append(m_arSelection);

	return arRows.GetSize();
}

BOOL CGrid::GetAllowSelectRow()
/*
Routine Description:
	Get the permission to select a row

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowSelectRow;
}

void CGrid::SetAllowSelectRow(BOOL b)
/*
Routine Description:
	Set the permission to select a row.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowSelectRow = b;
}

void CGrid::SetCellFont(LOGFONT *pLogFont)
{
	m_fntCell.DeleteObject();
	m_fntCell.CreateFontIndirect(pLogFont);

	CDC * pDC = GetDC();

	CFont * pFntOld = pDC->SelectObject(&m_fntCell);
	SIZE sz = pDC->GetOutputTextExtent(_T(" "));
	m_nMargin = sz.cx;
	pDC->SelectObject(pFntOld);

	ReleaseDC(pDC);

	SetRowHeight(pLogFont->lfHeight + 4);
}

void CGrid::PreSubclassWindow() 
{
	CWnd::PreSubclassWindow();

	Init();

	ResetScrollBars();   
}

void CGrid::SetHeaderFont(LOGFONT *pLogFont)
{
	m_fntHeader.DeleteObject();
	m_fntHeader.CreateFontIndirect(pLogFont);

	CDC * pDC = GetDC();

	CFont * pFntOld = pDC->SelectObject(&m_fntHeader);
	SIZE sz = pDC->GetOutputTextExtent(_T(" "));
	m_nHeaderMargin = sz.cx;
	pDC->SelectObject(pFntOld);

	ReleaseDC(pDC);

	SetHeaderHeight(pLogFont->lfHeight + 4);
}

void CGrid::OnEditCell(int nRow, int nCol, UINT nChar)
{
	CRect rtCell;
	if (IsColReadOnly(nCol) || !GetCellRect(nRow, nCol, rtCell))
		return;

	if (m_bAllowAddNew && nRow == m_nRows - 1 && !AddNew())
		return;

	if (!GetCellRect(nRow, nCol, rtCell))
		return;

	rtCell.DeflateRect(2, 2, 1, 1);
	
	CellStyle style;
	GetCellStyle(nRow, nCol, style, rtCell);

	CGridControl * pControl = GetColControl(nRow, nCol);
	ASSERT(pControl);
	pControl->Init(&style);
	m_bIPEdit = TRUE;
	pControl->OnGridChar(nChar);

	RedrawCell(m_nRow, 0);
}


BOOL CGrid::OnEndEditCell()
{
	// In case OnEndInPlaceEdit called as window is being destroyed
	if (!IsWindow(GetSafeHwnd()))
		return TRUE;

	CGridControl * pControl = GetColControl(m_nRow, m_nCol);
	ASSERT(pControl);

	CString strCurrent, strNew;

	strCurrent = GetCellText(m_nRow, m_nCol);
	pControl->GetText(strNew);
	if (strCurrent != strNew)
	{
		if (!StoreCellValue(m_nRow, m_nCol, strNew, strCurrent))
			return FALSE;

		SetCellText(m_nRow, m_nCol, strNew);
	}

	pControl->m_pWnd->ShowWindow(SW_HIDE);
	m_bIPEdit = FALSE;

	if (!m_bRowDirty)
		CancelRecord();
	else
	{
		RedrawCell(m_nRow, m_nCol);
		RedrawCell(m_nRow, 0);
	}

	return TRUE;
}

BOOL CGrid::SetCellText(int nRow, int nCol, CString strText)
{
	SetFocus();

	Cell * pCell = GetCell(nRow, nCol);
	if (!pCell)
		return FALSE;

	if (m_bAllowAddNew && nRow == m_nRows - 1 && !AddNew())
		return FALSE;

	if (nRow > 0 && nCol > 0)
	{
		if (m_bRowDirty && nRow != m_nRow && nRow > 0)
		{
			if (!FlushRecord())
				return FALSE;
		}

		Col * pCol = m_arCols[nCol - 1];
		if (!pCol->bDirty)
		{
			pCol->bDirty = TRUE;
			pCol->strSaved = GetCellText(nRow, nCol);
		}
		
		if (!m_bRowDirty)
		{
			m_bRowDirty = TRUE;
			RedrawCell(nRow, 0);
		}
	}

	pCell->strText = strText;

	if (m_bAllowRedraw)
		RedrawCell(nRow, nCol);

	return TRUE;
}

void CGrid::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!IsCTRLPressed() && m_nMouseMode == MOUSE_NOTHING && m_nRow > 0 && m_nCol > 0)
	{
		if (nChar != VK_TAB && nChar != VK_RETURN && nChar != VK_ESCAPE)
			OnEditCell(m_nRow, m_nCol, nChar);
	}
	
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CGrid::GetCellStyle(int nRow, int nCol, CellStyle &style, CRect rect)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	Cell * pCell = GetCell(nRow, nCol);
	if (!pCell)
		return;

	Col * pCol = m_arCols[nCol - 1];

	style.nRow = nRow;
	style.nCol = nCol;
	style.strText = GetCellText(nRow, nCol);
	style.nState = pCell->nState;
	style.rect = rect;
	style.strChoiceList = pCol->strChoiceList;
	style.bPromptInclude = pCol->bPromptInclude;
	style.strMask = pCol->strMask;
	style.nMaxLength = pCol->nMaxLength;
	style.clrBack = pCol->clrBack;
	style.clrFore = pCol->clrFore;
	style.nCase = pCol->nCase;
	style.nAlignment = pCol->nAlignment;
	style.nDataType = pCol->nDataType;
	style.strPromptChar = pCol->strPromptChar;

	if (style.clrBack == DEFAULTCOLOR)
		style.clrBack = m_clrBack;
	
	if (style.clrFore == DEFAULTCOLOR)
		style.clrFore = m_clrFore;

	if (style.nAlignment == -1)
		style.nAlignment = m_nAlignment;

	if (pCell->nState & GVIS_SELECTED)
	{
		style.clrBack = (RGB(255, 255, 255) ^ style.clrBack) & 0xffffff;
		style.clrFore = (RGB(255, 255, 255) ^ style.clrFore) & 0xffffff;
	}
}

CGridControl * CGrid::GetColControl(int nRow, int nCol)
{
	ASSERT(nRow > 0 && nCol > 0 && nRow < m_nRows && nCol < m_nCols);

	switch (m_arCols[nCol - 1]->nStyle)
	{
	case COLSTYLE_EDIT:
	{
		if (!m_pEditCtrl)
			m_pEditCtrl = new CGridEditCtrl(this, ES_CENTER, IDC_INPLACE_EDIT);

		return m_pEditCtrl;
	}

	break;

	case COLSTYLE_COMBOBOX:
	{
		if (!m_pComboCtrl)
			m_pComboCtrl = new CGridComboCtrl(this, CBS_DROPDOWN, IDC_INPLACE_EDIT);

		return m_pComboCtrl;
	}

	break;

	case COLSTYLE_COMBOBOX_DROPLIST:
	{
		if (!m_pComboListCtrl)
			m_pComboListCtrl = new CGridComboCtrl(this, CBS_DROPDOWNLIST, IDC_INPLACE_EDIT);

		return m_pComboListCtrl;
	}

	break;

	case COLSTYLE_DATEMASK:
	{
		if (!m_pDateCtrl)
			m_pDateCtrl = new CGridDateMaskCtrl(this, ES_CENTER, IDC_INPLACE_EDIT);

		return m_pDateCtrl;
	}

	break;

	case COLSTYLE_CHECKBOX:
	{
		if (!m_pCheckBoxCtrl)
			m_pCheckBoxCtrl = new CGridCheckBoxCtrl(this, NULL, IDC_INPLACE_EDIT);

		return m_pCheckBoxCtrl;
	}

	break;
	
	case COLSTYLE_EDITBUTTON:
	{
		if (!m_pEditBtnCtrl)
			m_pEditBtnCtrl = new CGridEditBtnCtrl(this, NULL, IDC_INPLACE_EDIT);

		return m_pEditBtnCtrl;
	}

	break;
	
	case COLSTYLE_BUTTON:
	{
		if (!m_pButtonCtrl)
			m_pButtonCtrl = new CGridEditBtnCtrl(this, NULL, IDC_INPLACE_EDIT + 1);

		return m_pButtonCtrl;
	}

	break;

	case COLSTYLE_NUM:
	{
		if (!m_pNumCtrl)
			m_pNumCtrl = new CGridNumCtrl(this, ES_CENTER, IDC_INPLACE_EDIT);

		return m_pNumCtrl;
	}

	break;

	default:
		return NULL;
	}
}

void CGrid::SetColControl(int nCol, int nStyle)
{
	ASSERT (nStyle >= 0 && nStyle < 8 && nCol > 0 && nCol < m_nCols);

	SetFocus();

	Col * pCol = m_arCols[nCol - 1];
	if (pCol->nStyle == nStyle)
		return;

	pCol->nStyle = nStyle;

	RedrawCol(nCol);
}

void CGrid::RedrawCol(int nCol)
{
	if (!m_bAllowRedraw)
		return;

	ASSERT(nCol > 0 && nCol < m_nCols);

	int nTopNonFixedRow, nLeftNonFixedGroup;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	for (int nRow = 0; nRow < m_nRows; nRow ++)
	{
		if (nRow > m_nFrozenRows && nRow < nTopNonFixedRow)
			continue;

		if (IsRowVisible(nRow))
			RedrawCell(nRow, nCol);
		else
			break;
	}
}

void CGrid::SetColChoiceList(int nCol, CString strList)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	Col * pCol = m_arCols[nCol - 1];
	pCol->strChoiceList = strList;
}

void CGrid::SetColPromptInclude(int nCol, BOOL b)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->bPromptInclude = b;

	RedrawCol(nCol);
}

void CGrid::SetForeColor(COLORREF clr)
{
	m_clrFore = clr;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetBackColor(COLORREF clr)
{
	m_clrBack = clr;
}

void CGrid::SetColMaxLength(int nCol, int nLength)
{
	ASSERT(nCol > 0 && nCol < m_nCols && nLength > 0 && nLength <= 255);

	m_arCols[nCol - 1]->nMaxLength = nLength;

	RedrawCol(nCol);
}

void CGrid::SetColMask(int nCol, CString strMask)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->strMask = strMask;

	RedrawCol(nCol);
}

void CGrid::SetGroupBackColor(int nGroup, COLORREF clr)
{
	nGroup --;

	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	m_arGroups[nGroup]->clrBack = clr;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetGroupForeColor(int nGroup, COLORREF clr)
{
	nGroup --;

	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	m_arGroups[nGroup]->clrFore = clr;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetColBackColor(int nCol, COLORREF clr)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->clrBack = clr;

	RedrawCol(nCol);
}

void CGrid::SetColForeColor(int nCol, COLORREF clr)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->clrFore = clr;

	RedrawCol(nCol);
}

void CGrid::SetColHeaderForeColor(int nCol, COLORREF clr)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->clrHeaderFore = clr;

	RedrawCell(0, nCol);
}

void CGrid::SetColHeaderBackColor(int nCol, COLORREF clr)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->clrHeaderBack = clr;

	RedrawCell(0, nCol);
}

void CGrid::RedrawGroup(int nGroup)
{
	if (!m_bAllowRedraw)
		return;

	nGroup --;

	ASSERT(nGroup >= 0 && nGroup < m_nGroups);

	Group * pGroup = m_arGroups[nGroup];

	CRect rect;
	if (GetGroupHeaderRect(nGroup, rect))
	{
		CDC * pDC = GetDC();
		DrawGroupHeaderCell(nGroup, rect, pDC);
		ReleaseDC(pDC);
	}

	for (int i = 0; i < pGroup->nCols; i ++)
		RedrawCol(GetColFromGroup(nGroup, i + 1));
}

BOOL CGrid::GetReadOnly()
{
	return m_bReadOnly;
}

void CGrid::SetReadOnly(BOOL b)
{
	if (b)
	{
		SetAllowDelete(FALSE);
		SetAllowAddNew(FALSE);
		SetFocus();
	}

	m_bReadOnly = b;
}

BOOL CGrid::StoreCellValue(int nRow, int nCol, CString strNewValue, CString strOldValue)
{
	return !IsColReadOnly(nCol);
}

void CGrid::SetColReadOnly(int nCol, BOOL b)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	if (b && m_nCol == nCol)
		SetFocus();

	m_arCols[nCol - 1]->bReadOnly = b;
}

void CGrid::SetColCase(int nCol, int nCase)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->nCase = nCase;

	RedrawCol(nCol);
}

void CGrid::SetAlignment(int nAlign)
{
	m_nAlignment = nAlign;
}

void CGrid::SetHeaderAlignment(int nAlign)
{
	m_nHeaderAlignment = nAlign;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetColAlignment(int nCol, int nAlign)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->nAlignment = nAlign;

	RedrawCol(nCol);
}

void CGrid::SetColHeaderAlignment(int nCol, int nAlign)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->nHeaderAlignment = nAlign;

	RedrawCell(0, nCol);
}

void CGrid::SetColDataType(int nCol, int nType)
{
	ASSERT(nCol > 0 && nCol < m_nCols && nType >= 0 && nType <= 7);

	m_arCols[nCol - 1]->nDataType = nType;

	RedrawCol(nCol);
}

BOOL CGrid::IsColReadOnly(int nCol)
{
	return m_bReadOnly || m_arCols[nCol - 1]->bReadOnly;
}

int CGrid::GetRowIndex(int nRow)
{
	ASSERT(nRow > 0 && nRow < m_nRows);

	return m_arRowIndex[nRow - 1];
}

int CGrid::GetColIndex(int nCol)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	return m_arCols[nCol - 1]->nColIndex;
}

int CGrid::GetGroupIndex(int nGroup)
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	return m_arGroups[nGroup - 1]->nGroupIndex;
}

void CGrid::SetColAllowSizing(int nCol, BOOL b)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->bAllowSizing = b;
}

void CGrid::SetGroupAllowSizing(int nGroup, BOOL b)
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	m_arGroups[nGroup - 1]->bAllowSizing = b;
}

void CGrid::SetDividerType(int nType)
{
	m_nDividerType = nType;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetDividerStyle(int nStyle)
{
	m_nDividerStyle = nStyle;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetDividerColor(COLORREF clr)
{
	m_clrDivider = clr;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetDefColWidth(int nWidth)
{
	if (nWidth <= 0)
		return;

	m_nDefColWidth = nWidth;
}

void CGrid::CancelRecord()
{
	if (m_bIPEdit)
		GetFocus()->SendMessage(WM_KEYDOWN, VK_ESCAPE, 1);

	if (m_bRowDirty)
	{
		for (int i = 0; i < m_nCols - 1; i ++)
		{
			Col * pCol = m_arCols[i];
			if (pCol->bDirty)
			{
				m_arCols[i]->bDirty = FALSE;
				GetCell(m_nRow, i + 1)->strText = m_arCols[i]->strSaved;
			}
		}

		m_bRowDirty = FALSE;
		RedrawRow(m_nRow);
	}

	if (m_bAddNewMode)
	{
		RemoveRow();
		m_bAddNewMode = FALSE;
		ResetScrollBars();
		Invalidate();
	}
	else
		RedrawCell(m_nRow, 0);
}

int CGrid::GetColFromIndex(int nIndex)
{
	for (int i = 1; i < m_nCols; i ++)
		if (m_arCols[i - 1]->nColIndex == nIndex)
			return i;

	return INVALID;
}

int CGrid::GetRowFromIndex(int nIndex)
{
	for (int i = 1; i < m_nRows; i ++)
		if (m_arRowIndex[i - 1] == nIndex)
			return i;

	return INVALID;
}

int CGrid::GetGroupFromIndex(int nIndex)
{
	for (int i = 0; i < m_nGroups; i ++)
		if (m_arGroups[i]->nGroupIndex == nIndex)
			return i + 1;

	return INVALID;
}

COLORREF CGrid::GetBackColor()
{
	return m_clrBack;
}

COLORREF CGrid::GetHeaderForeColor()
{
	return m_clrHeaderFore;
}

COLORREF CGrid::GetHeaderBackColor()
{
	return m_clrHeaderBack;
}

void CGrid::SetHeaderForeColor(COLORREF clr)
{
	m_clrHeaderFore = clr;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetHeaderBackColor(COLORREF clr)
{
	m_clrHeaderBack = clr;

	if (m_bAllowRedraw)
		Invalidate();
}

void CGrid::SetListMode(BOOL b)
{
	if (b && !m_bListMode)
	{
		m_bListMode = b;
		SetReadOnly(TRUE);
		
		SetAllowRowResize(FALSE);
		SetAllowColResize(FALSE);
		SetAllowGroupResize(FALSE);
		SetAllowMoveCol(FALSE);
		SetAllowMoveGroup(FALSE);

		ResetSelection();
		if (m_nRow > 0)
			SelectRow(m_nRow);
	}

	m_bListMode = b;
}

void CGrid::OnLButtonClickedCell(int nRow, int nCol)
{
	if (nRow > 0 && nCol > 0)
		SetCurrentCell(nRow, nCol);
}

BOOL CGrid::IsColDirty(int nCol)
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	return m_arCols[nCol - 1]->bDirty;
}

void CGrid::OnClickedCellButton(int nRow, int nCol)
{
}

BOOL CGrid::OnStartTracking(int nRow, int nCol, int nTrackingMode)
{
	return TRUE;
}

void CGrid::StoreGroupWidth(int nGroup, int nWidth)
{
	SetGroupWidth(nGroup + 1, nWidth);
}

void CGrid::StoreColWidth(int nCol, int nWidth)
{
	SetColWidth(nCol, nWidth);
}

void CGrid::SetGridRect(RECT *pRect)
{
	m_rtGrid.CopyRect(pRect);
}

void CGrid::OnGridDraw(CDC *pDC)
{
#ifndef _DEBUG

	CDC memdc;
	CBitmap membmp, * pBmpOld;

	CRect rtCopy = m_rtGrid;
	m_rtGrid.OffsetRect(- m_rtGrid.left, - m_rtGrid.top);

	memdc.CreateCompatibleDC(pDC);
	membmp.CreateCompatibleBitmap(pDC, m_rtGrid.Width(), m_rtGrid.Height());
	pBmpOld = memdc.SelectObject(&membmp);
	memdc.FillSolidRect(m_rtGrid, GetSysColor(COLOR_3DSHADOW));

	int nTopNonFixedRow, nLeftNonFixedGroup;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	memdc.FillSolidRect(m_rtGrid, GetSysColor(COLOR_3DSHADOW));

	for (int nRow = 0; nRow < m_nRows; nRow ++)
	{
		if (nRow > m_nFrozenRows && nRow < nTopNonFixedRow)
			continue;

		if (!RedrawRow(nRow, &memdc))
			break;
	}

	m_rtGrid = rtCopy;
	pDC->BitBlt(m_rtGrid.left, m_rtGrid.top, m_rtGrid.Width(), m_rtGrid.Height(), &memdc, 0, 0, SRCCOPY);
	memdc.SelectObject(pBmpOld);

#else

	int nTopNonFixedRow, nLeftNonFixedGroup;
	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	pDC->FillSolidRect(m_rtGrid, GetSysColor(COLOR_3DSHADOW));

	for (int nRow = 0; nRow < m_nRows; nRow ++)
	{
		if (nRow > m_nFrozenRows && nRow < nTopNonFixedRow)
			continue;

		if (!RedrawRow(nRow, pDC))
			break;
	}

#endif
}

int CGrid::GetVisibleCols()
{
	if (!IsSingleLevelMode())
		return 0;

	int nLeftNonFixedCol, nTopNonFixedRow;

	GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);

	int nCols = 0, nFrozenWidth = 0;
	int nVisibleWidth = m_rtGrid.Width() - (m_bShowRowHeader ? 22 : 0);

	for (int nCol = 1; nCol <= m_nFrozenCols; nCol ++)
	{
		if (nCol > m_nFrozenCols && nCol < nLeftNonFixedCol)
			continue;

		if (!m_arCols[nCol - 1]->bVisible)
			continue;

		nCols ++;
		nFrozenWidth += m_arCols[nCol - 1]->nWidth;
		if (nFrozenWidth >= nVisibleWidth)
			return nCols;
	}

	return nCols;
}

int CGrid::GetVisibleRows()
{
	int nVisibleHeight;

	nVisibleHeight = m_rtGrid.Height() - GetVirtualHeaderHeight();

	return ((int)(nVisibleHeight / m_nRowHeight - 0.0001)) + 1;
}

int CGrid::GetVisibleGroups()
{
	if (IsSingleLevelMode())
		return 0;

	int nLeftNonFixedGroup, nTopNonFixedRow, nRightNonFixedGroup, nBottomNonFixedRow;

	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	GetBottomrightNonFixedGroup(nBottomNonFixedRow, nRightNonFixedGroup);

	int nGroups = 0, nFrozenWidth = 0;
	int nVisibleWidth = m_rtGrid.Width() - (m_bShowRowHeader ? 22 : 0);

	for (int nGroup = 0; nGroup < m_nFrozenGroups; nGroup ++)
	{
		if (!m_arGroups[nGroup]->bVisible)
			continue;

		nGroups ++;
		nFrozenWidth += m_arGroups[nGroup]->nWidth;
		if (nFrozenWidth >= nVisibleWidth)
			return nGroups;
	}

	for (nGroup = nLeftNonFixedGroup; nGroup < m_nGroups && nGroup < nRightNonFixedGroup; nGroup ++)
	{
		if (!m_arGroups[nGroup]->bVisible)
			continue;

		nGroups ++;
		nFrozenWidth += m_arGroups[nGroup]->nWidth;
		if (nFrozenWidth >= nVisibleWidth)
			return nGroups;
	}

	return nGroups;
}

int CGrid::GetLeftCol()
{
	if (!IsSingleLevelMode())
		return 0;

	if (m_nFrozenCols)
		return m_nCols > 1 ? 1 : 0;

	int nLeftNonFixedCol, nTopNonFixedRow;

	GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);

	return nLeftNonFixedCol;
}

int CGrid::GetLeftGroup()
{
	if (IsSingleLevelMode())
		return 0;

	if (m_nFrozenGroups)
		return m_nGroups ? 1 : 0;

	int nLeftNonFixedGroup, nTopNonFixedRow;

	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	return nLeftNonFixedGroup + 1;
}

void CGrid::SetLeftCol(int nCol)
{
	if (nCol <= 0 || nCol >= m_nCols)
		return;

	if (!IsSingleLevelMode() || m_nFrozenCols || m_arCols[nCol - 1]->bVisible)
		return;

	int nLeftNonFixedCol, nTopNonFixedRow;
	int nXPos = GetScrollPos32(SB_HORZ);

	GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);

	while (nXPos < m_nHScrollMax && nCol > nLeftNonFixedCol)
	{
		SendMessage(WM_HSCROLL, SB_LINERIGHT);
		nXPos = GetScrollPos32(SB_HORZ);
		GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
	}

	while (nXPos > 0 && nCol < nLeftNonFixedCol)
	{
		SendMessage(WM_HSCROLL, SB_LINELEFT);
		nXPos = GetScrollPos32(SB_HORZ);
		GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
	}
}

void CGrid::SetLeftGroup(int nGroup)
{
	if (nGroup <= 0 || nGroup > m_nGroups)
		return;

	if (IsSingleLevelMode() || m_nFrozenGroups || m_arGroups[nGroup - 1]->bVisible)
		return;

	int nLeftNonFixedGroup, nTopNonFixedRow;
	int nXPos = GetScrollPos32(SB_HORZ);

	GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);

	while (nXPos < m_nHScrollMax && nGroup > nLeftNonFixedGroup)
	{
		SendMessage(WM_HSCROLL, SB_LINERIGHT);
		nXPos = GetScrollPos32(SB_HORZ);
		GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	}

	while (nXPos > 0 && nGroup < nLeftNonFixedGroup)
	{
		SendMessage(WM_HSCROLL, SB_LINELEFT);
		nXPos = GetScrollPos32(SB_HORZ);
		GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
	}
}

int CGrid::GetTopRow()
{
	if (m_nFrozenRows)
		return m_nRows > 1 ? 1 : 0;

	int nYPos = GetScrollPos32(SB_VERT);

	return nYPos / m_nRowHeight + 1;
}

void CGrid::SetTopRow(int nRow)
{
	if (m_nFrozenRows || nRow <= 0 || nRow >= m_nRows)
		return;

	SetScrollPos32(SB_VERT, m_nRowHeight * nRow - 1);
	Invalidate();
}

BOOL CGrid::GetAllowAddNew()
/*
Routine Description:
	Get the permission to add new row manually.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowAddNew;
}

void CGrid::SetAllowAddNew(BOOL b)
/*
Routine Description:
	Set the permission to add new row manually.

Parameters:
	b

Return Value:
	None.
*/
{
	if (m_bReadOnly)
		return;

	if (m_bAllowAddNew && !b)
	{
		if (m_bAddNewMode)
			CancelRecord();

		RemoveRow();
	}
	else if (!m_bAllowAddNew && b)
	{
		InsertRow(m_nRows);
		RedrawCell(m_nRows - 1, 0);
	}

	m_bAllowAddNew = b;
}

BOOL CGrid::AddNew()
{
	if (!m_bAllowAddNew)
		return FALSE;

	InsertRow(m_nRows);
	RedrawRow(m_nRows - 1);
	EnsureVisible(m_nRow, m_nCol);
	m_bAddNewMode = TRUE;

	return TRUE;
}

BOOL CGrid::FlushRecord()
{
	SetFocus();
	if (m_bIPEdit)
		return FALSE;

	m_bRowDirty = FALSE;
	m_bAddNewMode = FALSE;

	for (int i = 0; i < m_nCols - 1; i ++)
		m_arCols[i]->bDirty = FALSE;

	return TRUE;
}

void CGrid::OnLoadCellText(int nRow, int nCol, CString &strText)
{
	strText = GetCell(nRow, nCol)->strText;
}

BOOL CGrid::DeleteRecord(int nRow)
{
	if (m_bReadOnly || !m_bAllowDelete)
		return FALSE;

	RemoveRow(nRow);

	return TRUE;
}

BOOL CGrid::GetAllowDelete()
/*
Routine Description:
	Get the permission to delete row manually.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return m_bAllowDelete;
}

void CGrid::SetAllowDelete(BOOL b)
/*
Routine Description:
	Set the permission to delete row manually.

Parameters:
	b

Return Value:
	None.
*/
{
	if (m_bReadOnly)
		return;

	m_bAllowDelete = b;
}

void CGrid::LockUpdate(BOOL b)
/*
Routine Description:
	Set the permission to redraw.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bAllowRedraw = !b;
}

BOOL CGrid::IsLockUpdate()
/*
Routine Description:
	Get the permission to redraw.

Parameters:
	None.

Return Value:
	The permission.
*/
{
	return !m_bAllowRedraw;
}

int CGrid::OnGetRecordCount()
/*
Routine Description:
	Get the record count in grid.

Parameters:
	None.

Return Value:
	The count.
*/
{
	// Exclude the extra row and pending new row.
	return m_nRows - (m_bAllowAddNew ? 2 : 1) - (m_bAddNewMode ? 1 : 0);
}

void CGrid::Create(DWORD style, const RECT &rect, CWnd *pParent, UINT id)
{
	CWnd::Create(BHMGRID_CLASSNAME, NULL, style, rect, pParent, id);
}

BOOL CGrid::IsAddRow()
/*
Routine Description:
	Get if has pending new row.

Parameters:
	None.

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	return m_bAddNewMode;
}

BOOL CGrid::IsRecordDirty()
/*
Routine Description:
	Get if has pending modification in current row.

Parameters:
	None.

Return Value:
	If is, return TRUE; Otherwise, return FALSE;
*/
{
	return m_bRowDirty;
}

BOOL CGrid::IsCurrentCell(int nRow, int nCol)
/*
Routine Description:
	Get the given cell is current cell.

Parameters:
	nRow

	nCol

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	return m_nRow == nRow && m_nCol == nCol;
}

int CGrid::GetAlignment()
/*
Routine Description:
	Get the text alignment

Parameters:
	None.

Return Value:
	The alignment.
*/
{
	return m_nAlignment;
}

int CGrid::GetHeaderAlignment()
/*
Routine Description:
	Get the header text alignment.

Parameters:
	None.

Return Value:
	The alignment.
*/
{
	return m_nHeaderAlignment;
}

void CGrid::SetColPromptChar(int nCol, CString strChar)
/*
Routine Description:
	Set the prompt char of one col.

Parameters:
	nCol

	strChar		The new value.

Return Value:
	None.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	m_arCols[nCol - 1]->strPromptChar = strChar;

	RedrawCol(nCol);
}

int CGrid::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Init();
	
	return 0;
}

void CGrid::Init()
/*
Routine Description:
	Init some grid properties.

Parameters:
	None.

Return Value:
	None.
*/
{
	NONCLIENTMETRICS ncm;
	LOGFONT lfCell, lfHeader;
	m_nAlignment = m_nHeaderAlignment = DT_LEFT;

	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0));
	memcpy(&lfCell, &(ncm.lfMessageFont), sizeof(LOGFONT));
	memcpy(&lfHeader, &(ncm.lfMessageFont), sizeof(LOGFONT));
	SetCellFont(&lfCell);
	SetHeaderFont(&lfHeader);

	SetAllowAddNew(TRUE);
}

BOOL CGrid::MoveTo(int nRow)
/*
Routine Description:
	Set one row as current row.

Parameters:
	nRow

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	if (nRow <= 0 || nRow >= m_nRows || m_nCols <= 1)
		return FALSE;

	int row, col;
	GetCurrentCell(row, col);
	if (col == 0 && m_nCols > 1)
		return Navigate(nRow, 0, NV_RIGHT);

	return SetCurrentCell(nRow, col);
}

void CGrid::EnsureVisible(int nRowDest, int nColDest)
/*
Routine Description:
	Ensure one cell is visible.

Parameters:
	nRowDest

	nColDest

Return Value:
	None.
*/
{
	int nGroup, nColCopy = nColDest - 1;
	nGroup = GetGroupFromCol(nColCopy);

	int nTopNonFixedRow, nBottomNonFixedRow, nLeftNonFixedGroup, nRightNonFixedGroup, nLeftNonFixedCol, nRightNonFixedCol;
	
	if (IsSingleLevelMode())
	{
		GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
		GetBottomrightNonFixedCell(nBottomNonFixedRow, nRightNonFixedCol);
	}
	else
	{
		GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
		GetBottomrightNonFixedGroup(nBottomNonFixedRow, nRightNonFixedGroup);
	}

	if ((nRowDest > m_nFrozenRows && nRowDest < nTopNonFixedRow) || nRowDest > nBottomNonFixedRow ||
		(IsSingleLevelMode() && ((nColDest > m_nFrozenCols && nColDest < nLeftNonFixedCol) || nColDest > nRightNonFixedCol)) || (!IsSingleLevelMode() && ((nGroup >= m_nFrozenGroups && nGroup < nLeftNonFixedGroup) || nGroup > nRightNonFixedGroup)))
	{
		// The cell is invisible.
		int nVertPos = GetScrollPos32(SB_VERT);

		SetRedraw(FALSE);

		// Calc the rows to scroll.
		if (nRowDest > m_nFrozenRows)
		{
			if (nRowDest < nTopNonFixedRow)
			{
				while (nRowDest < nTopNonFixedRow -- && nVertPos > 0)
					nVertPos -= m_nRowHeight;
			}
			else if (nRowDest > nBottomNonFixedRow)
			{
				while (nRowDest > nBottomNonFixedRow ++ && nVertPos < m_nVScrollMax)
					nVertPos += m_nRowHeight;
			}

			nVertPos = min(m_nVScrollMax, nVertPos);
			nVertPos = max(0, nVertPos);

			// Scroll the grid.
			SetScrollPos32(SB_VERT, nVertPos, FALSE);
		}

		if (IsSingleLevelMode() && nColDest > m_nFrozenCols)
		{
			// Calc the cols to scroll.
			while (nColDest < nLeftNonFixedCol && GetScrollPos32(SB_HORZ) > 0)
			{
				SendMessage(WM_HSCROLL, SB_LINELEFT);
				GetTopleftNonFixedCell(nTopNonFixedRow, nLeftNonFixedCol);
			}
			
			while (nColDest > nRightNonFixedCol && GetScrollPos32(SB_HORZ) < m_nHScrollMax - 1)
			{
				SendMessage(WM_HSCROLL, SB_LINERIGHT);
				GetBottomrightNonFixedCell(nBottomNonFixedRow, nRightNonFixedCol);
			}
		}
		else if (!IsSingleLevelMode() && nGroup >= m_nFrozenGroups)
		{
			// Calc the group sto scroll.
			while (nGroup < nLeftNonFixedGroup && GetScrollPos32(SB_HORZ) > 0)
			{
				SendMessage(WM_HSCROLL, SB_LINELEFT);
				GetTopleftNonFixedGroup(nTopNonFixedRow, nLeftNonFixedGroup);
			}
			
			while (nGroup > nRightNonFixedGroup && GetScrollPos32(SB_HORZ) < m_nHScrollMax - 1)
			{
				SendMessage(WM_HSCROLL, SB_LINERIGHT);
				GetBottomrightNonFixedGroup(nBottomNonFixedRow, nRightNonFixedGroup);
			}
		}

		SetRedraw(TRUE);
		Invalidate();
	}
}

void CGrid::OnDrawCellText(CDC * pDC, CellStyle *pStyle)
/*
Routine Description:
	Draw text in cell.

Parameters:
	pDC

	pStyle		The styles of cell.

Return Value:
	None.
*/
{
	DrawCellBackGround(pDC, pStyle);
}

void CGrid::DrawCellBackGround(CDC *pDC, CellStyle *pStyle)
/*
Routine Description:
	Draw the background of one cell.

Parameters:
	pDC

	pStyle

Return Value:
	None.
*/
{
	pStyle->rect.right ++;
	pStyle->rect.bottom ++; // FillRect doesn't draw RHS or bottom

	pDC->FillSolidRect(pStyle->rect, pStyle->clrBack);
	
	pStyle->rect.right --;
	pStyle->rect.bottom --;
}

void CGrid::LoadLayout(CGroupArray * pGroups, CColArray * pCols, CCellArray * pCells)
/*
Routine Description:
	Load layout from external data.

Parameters:
	pGroups		The group array.

	pCols		The col array.

	pCells		The cell array.

Return Value:
	None.
*/
{
	ASSERT(pGroups && pCols);
	
	BOOL bAllowAddNew = m_bAllowAddNew;
	SetAllowAddNew(FALSE);

	// Clear contents.
	CancelRecord();
	SetRowCount(0);
	SetColCount(0);
	SetGroupCount(0);

	// The group count.
	m_nGroups = pGroups->GetSize();
	
	// The col count.
	m_nCols = pCols->GetSize() + 1;

	// The level count.
	m_nLevels = 1;
	m_nGroupsUsed = 0;
	m_nColsUsed = 0;

	for (int i = 0; i < m_nGroups; i ++)
	{
		// Copy each group object.

		Group * pGroup = pGroups->ElementAt(i);
		Group * pGroupNew = new Group;
		CopyGroup(pGroup, pGroupNew);
		pGroupNew->nGroupIndex = ++ m_nGroupsUsed;

		m_nLevels = pGroup->arLevels.GetSize();

		for (int j = 0; j < m_nLevels; j ++)
		{
			// Copy each level object.
			Level * pLevel = pGroup->arLevels[j];
			Level * pLevelNew = new Level;

			pLevelNew->nCols = pLevel->nCols;
			pLevelNew->nColsVisible = pLevel->nColsVisible;

			pGroupNew->arLevels.Add(pLevelNew);
		}

		m_arGroups.Add(pGroupNew);
	}

	// Copy each col object.
	for (i = 1; i < m_nCols; i ++)
	{
		Col * pCol = pCols->ElementAt(i - 1);
		Col * pColNew = new Col;

		CopyCol(pCol, pColNew);
		pColNew->nColIndex = ++ m_nColsUsed;

		m_arCols.Add(pColNew);
	}

	// Copy each cell object.
	if (pCells)
	{
		SetRowCount(pCells->GetSize() / m_nCols);

		for (i = 1; i < m_nRows; i ++)
			for (int j = 1; j < m_nCols; j ++)
				GetCell(i, j)->strText = pCells->ElementAt((i - 1) * m_nCols + j)->strText;
	}

	SetAllowAddNew(bAllowAddNew);

	// Calc the layout.
	CalcRowHeight();
	CalcHeaderHeight();

	if (m_bAllowRedraw)
	{
		ResetScrollBars();
		Invalidate();
	}
}

void CGrid::SaveLayout(CGroupArray * pGroups, CColArray * pCols, CCellArray * pCells)
/*
Routine Description:
	Save layout to external data.

Parameters:
	pGroups		The group array.

	pCols		The col array.

	pCells		The cell array.

Return Value:
	None.
*/
{
	ASSERT(pGroups && pCols && pCells);
	
	// Clear given buffer first.

	BOOL bAllowAddNew = m_bAllowAddNew;
	SetAllowAddNew(FALSE);

	for (int i = 0; i < pGroups->GetSize(); i ++)
		delete pGroups->ElementAt(i);

	pGroups->RemoveAll();

	for (i = 0; i < pCols->GetSize(); i ++)
		delete pCols->ElementAt(i);

	pCols->RemoveAll();

	for (i = 0; i < pCells->GetSize(); i ++)
		delete pCells->ElementAt(i);

	pCells->RemoveAll();

	// Save group array.
	for (i = 0; i < m_nGroups; i ++)
	{
		Group * pGroup = m_arGroups[i];
		Group * pGroupNew = new Group;

		CopyGroup(pGroup, pGroupNew);

		for (int j = 0; j < m_nLevels; j ++)
		{
			// Save level array.
			Level * pLevel = pGroup->arLevels[j];
			Level * pLevelNew = new Level;

			pLevelNew->nCols = pLevel->nCols;
			pLevelNew->nColsVisible = pLevel->nColsVisible;

			pGroupNew->arLevels.Add(pLevelNew);
		}

		pGroups->Add(pGroupNew);
	}

	// Save col array.
	for (i = 0; i < m_nCols - 1; i ++)
	{
		Col * pCol = m_arCols[i];
		Col * pColNew = new Col;

		CopyCol(pCol, pColNew);

		pCols->Add(pColNew);
	}

	// Save cell array.
	for (i = 0; i < m_arCells.GetSize(); i ++)
	{
		Cell * pCell = m_arCells[i];
		Cell * pCellNew = new Cell;

		pCellNew->strText = pCell->strText;

		pCells->Add(pCellNew);
	}

	SetAllowAddNew(bAllowAddNew);
}

void CGrid::CopyCol(Col *pCol, Col *pColNew)
/*
Routine Description:
	Clone a col.

Parameters:
	pCol		The source col.

	pColNew		The result col.

Return Value:
	None.
*/
{
	pColNew->bAllowSizing = pCol->bAllowSizing;
	pColNew->bPromptInclude = pCol->bPromptInclude;
	pColNew->bReadOnly = pCol->bReadOnly;
	pColNew->bVisible = pCol->bVisible;
	pColNew->clrBack = pCol->clrBack;
	pColNew->clrFore = pCol->clrFore;
	pColNew->clrHeaderBack = pCol->clrHeaderBack;
	pColNew->clrHeaderFore = pCol->clrHeaderFore;
	pColNew->nAlignment = pCol->nAlignment;
	pColNew->nCase = pCol->nCase;
	pColNew->nColIndex = pCol->nColIndex;
	pColNew->nDataType = pCol->nDataType;
	pColNew->nHeaderAlignment = pCol->nHeaderAlignment;
	pColNew->nMaxLength = pCol->nMaxLength;
	pColNew->nStyle = pCol->nStyle;
	pColNew->nWidth = pCol->nWidth;
	pColNew->strMask = pCol->strMask;
	pColNew->strPromptChar = pCol->strPromptChar;
	pColNew->strTitle = pCol->strTitle;
	pColNew->strName = pCol->strName;
	pColNew->arUserAttrib.RemoveAll();
	pColNew->arUserAttrib.Append(pCol->arUserAttrib);
	pColNew->strChoiceList = pCol->strChoiceList;
}

void CGrid::CopyGroup(Group *pGroup, Group *pGroupNew)
/*
Routine Description:
	Clone a group.

Parameters:
	pGroup		The source group.

	pGroupNew	The result group.

Return Value:
	None.
*/
{
	pGroupNew->bAllowSizing = pGroup->bAllowSizing;
	pGroupNew->bVisible = pGroup->bVisible;
	pGroupNew->clrBack = pGroup->clrBack;
	pGroupNew->clrFore = pGroup->clrFore;
	pGroupNew->nCols = pGroup->nCols;
	pGroupNew->nGroupIndex = pGroup->nGroupIndex;
	pGroupNew->nWidth = pGroup->nWidth;
	pGroupNew->strTitle = pGroup->strTitle;
	pGroupNew->strName = pGroup->strName;
}

void CGrid::SetColTitle(int nCol, CString strText)
/*
Routine Description:
	Set title of one col.

Parameters:
	nCol

	strText		The new title.

Return Value:
	None.
*/
{
	nCol --;

	if (nCol < 0 || nCol >= m_nCols - 1)
		return;

	m_arCols[nCol]->strTitle = strText;

	RedrawCell(0, nCol + 1);
}

CStringArray & CGrid::GetColUserAttribRef(int nCol)
/*
Routine Description:
	Get the user defined attribute array in one col.

Parameters:
	nCol

Return Value:
	The result array.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->arUserAttrib;
}

int CGrid::GetColAlignment(int nCol)
/*
Routine Description:
	Get the text alignmnet of one col.

Parameters:
	nCol

Return Value:
	The alignment.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->nAlignment;
}

void CGrid::CloneColProp(int nCol, Col *pColNew)
/*
Routine Description:
	Clone a col object.

Parameters:
	nCol

	pColNew		The result col object.

Return Value:
	None.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	Col * pCol = m_arCols[nCol];

	CopyCol(pCol, pColNew);
}

int CGrid::GetGroupColCount(int nGroup)
/*
Routine Description:
	Get the col count in a group.

Parameters:
	nGroup

Return Value:
	The count.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	return m_arGroups[nGroup - 1]->nCols;
}

CString CGrid::GetGroupTitle(int nGroup)
/*
Routine Description:
	Get the title of one group.

Parameters:
	nGroup

Return Value:
	The title.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	return m_arGroups[nGroup]->strTitle;
}

void CGrid::SetGroupTitle(int nGroup, CString strTitle)
/*
Routine Description:
	Set the title of one group.

Parameters:
	nGroup

	strTitle		The new title.

Return Value:
	None.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	m_arGroups[nGroup]->strTitle = strTitle;

	if (m_bAllowRedraw)
		Invalidate();
}

COLORREF CGrid::GetGroupBackColor(int nGroup)
/*
Routine Description:
	Get the back color of one group.

Parameters:
	nGroup

Return Value:
	The color.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	return m_arGroups[nGroup]->clrBack;
}

COLORREF CGrid::GetGroupForeColor(int nGroup)
/*
Routine Description:
	Get the fore color of one group.

Parameters:
	nGroup

Return Value:
	The color.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	return m_arGroups[nGroup]->clrFore;
}

BOOL CGrid::GetGroupAllowSizing(int nGroup)
/*
Routine Description:
	Get the permission to resize one group width.

Parameters:
	nGroup

Return Value:
	The permission.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	return m_arGroups[nGroup]->bAllowSizing;
}

CString CGrid::FindUniqueName(CString &strRecm, int nOrdinal)
/*
Routine Description:
	Find unique name.

Parameters:
	strRecm		The prefered string

	nOrdinal	The start digit used in name.

Return Value:
	The result string.
*/
{
	for (int i = 0; i < m_nCols - 1; i ++)
	{
		if (!strRecm.CompareNoCase(m_arCols[i]->strName))
		{
			strRecm.Format("%s%d", strRecm, ++ nOrdinal);
			return FindUniqueName(strRecm, nOrdinal);
		}
	}

	return strRecm;
}

CString CGrid::GetColName(int nCol)
/*
Routine Description:
	Get the name of one col.

Parameters:
	nCol

Return Value:
	The string.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->strName;
}

BOOL CGrid::SetColName(int nCol, CString strName)
/*
Routine Description:
	Set the name of one col.

Parameters:
	nCol

	strName		The new name.

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	if (!IsColNameUnique(strName))
		return FALSE;

	m_arCols[nCol]->strName = strName;

	return TRUE;
}

int CGrid::GetColFromName(CString strName)
/*
Routine Description:
	Get the col ordinal from col name.

Parameters:
	strName		The name to search.

Return Value:
	The result ordinal.
*/
{
	// Compare the given name with each col name.
	for (int i = 0; i < m_nCols - 1; i ++)
	{
		if (!m_arCols[i]->strName.CompareNoCase(strName))
			return i + 1;
	}

	return INVALID;
}

CString CGrid::GetGroupName(int nGroup)
/*
Routine Description:
	Get the name of one group.

Parameters:
	nGroup

Return Value:
	The name.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	return m_arGroups[nGroup]->strName;
}

BOOL CGrid::SetGroupName(int nGroup, CString strName)
/*
Routine Description:
	Set the name of one group.

Parameters:
	nGroup

	strName

Return Value:
	If succeeded, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nGroup > 0 && nGroup <= m_nGroups);

	nGroup --;

	if (!IsGroupNameUnique(strName))
		return FALSE;

	m_arGroups[nGroup]->strName = strName;

	return TRUE;
}

int CGrid::GetGroupFromName(CString strName)
/*
Routine Description:
	Get the col ordinal from group name.

Parameters:
	strName		The name to search.

Return Value:
	The result ordinal.
*/
{
	// Compare the given name with each group name.
{
	for (int i = 0; i < m_nGroups; i ++)
	{
		if (!m_arGroups[i]->strName.CompareNoCase(strName))
			return i + 1;
	}

	return INVALID;
}

BOOL CGrid::IsColNameUnique(CString strName)
/*
Routine Description:
	Get if the given col name is unique.

Parameters:
	strName.

Return Value:
	If is, return TRUE; Otherwise, return FALSE
*/
{
	for (int i = 0; i < m_nCols - 1; i ++)
	{
		if (!m_arCols[i]->strName.CompareNoCase(strName))
			return FALSE;
	}

	return TRUE;
}

BOOL CGrid::IsGroupNameUnique(CString strName)
/*
Routine Description:
	Get if the given group name is unique.

Parameters:
	strName.

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	for (int i = 0; i < m_nGroups; i ++)
	{
		if (!m_arGroups[i]->strName.CompareNoCase(strName))
			return FALSE;
	}

	return TRUE;
}

COLORREF CGrid::GetForeColor()
/*
Routine Description:
	Get the fore color.

Parameters:
	None.

Return Value:
	The color.
*/
{
	return m_clrFore;
}

CString CGrid::GetColTitle(int nCol)
/*
Routine Description:
	Get the title of one col.

Parameters:
	nCol

Return Value:
	The title.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->strTitle;
}

COLORREF CGrid::GetColForeColor(int nCol)
/*
Routine Description:
	Get the fore color of one col.

Parameters:
	None.

Return Value:
	The color.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->clrFore;
}

COLORREF CGrid::GetColBackColor(int nCol)
/*
Routine Description:
	Get the back color of one col.

Parameters:
	nCol

Return Value:
	The color.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->clrBack;
}

int CGrid::GetColCase(int nCol)
/*
Routine Description:
	Get the text case of one col.

Parameters:
	nCol

Return Value:
	The text case.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->nCase;
}

int CGrid::GetColMaxLength(int nCol)
/*
Routine Description:
	Get the max text length of one col.

Parameters:
	nCol

Return Value:
	The limitation.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->nMaxLength;
}

BOOL CGrid::GetColAllowSizing(int nCol)
/*
Routine Description:
	Get the permission to resize the width of one col.

Parameters:
	nCol		Begins with 1.

Return Value:
	The permission.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->bAllowSizing;
}

COLORREF CGrid::GetColHeaderForeColor(int nCol)
/*
Routine Description:
	Get the header fore color of one col.

Parameters:
	nCol

Return Value:
	The color.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->clrHeaderFore;
}

COLORREF CGrid::GetColHeaderBackColor(int nCol)
/*
Routine Description:
	Get the header back color of one col.

Parameters:
	nCol

Return Value:
	The color.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->clrHeaderBack;
}

BOOL CGrid::GetColReadOnly(int nCol)
/*
Routine Description:
	Get if one col is read only.

Parameters:
	nCol

Return Value:
	If is, return TRUE; Otherwise, return FALSE.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->bReadOnly;
}

int CGrid::GetColDataType(int nCol)
/*
Routine Description:
	Get the type of the underlying data in one col.

Parameters:
	nCol

Return Value:
	The data type.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->nDataType;
}

int CGrid::GetColControl(int nCol)
/*
Routine Description:
	Get the style of one col.

Parameters:
	nCol

Return Value:
	The style.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->nStyle;
}

int CGrid::GetColHeaderAlignment(int nCol)
/*
Routine Description:
	Get the text alignment in one col's header.

Parameters:
	nCol

Return Value:
	The alignment.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->nHeaderAlignment;
}

CString CGrid::GetColMask(int nCol)
/*
Routine Description:
	Get the input mask of one col.

Parameters:
	nCol

Return Value:
	The mask.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->strMask;
}

CString CGrid::GetColPromptChar(int nCol)
/*
Routine Description:
	Get the prompt char of one col.

Parameters:
	nCol

Return Value:
	The string.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->strPromptChar;
}

BOOL CGrid::GetColPromptInclude(int nCol)
/*
Routine Description:
	Get the permission to include prompt char in text in one col.

Parameters:
	nCol		Begins with 1.

Return Value:
	The permission.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->bPromptInclude;
}

CString CGrid::GetColChoiceList(int nCol)
/*
Routine Description:
	Get the choice list string of one col.

Parameters:
	nCol

Return Value:
	The string.
*/
{
	ASSERT(nCol > 0 && nCol < m_nCols);

	nCol --;

	return m_arCols[nCol]->strChoiceList;
}

int CGrid::GetLevelHeight()
/*
Routine Description:
	Get the height of each level.

Parameters:
	None.

Return Value:
	The height.
*/
{
	return m_nHeaderLevelHeight;
}

void CGrid::StoreRowHeight(int nHeight)
/*
Routine Description:
	Save the row height.

Parameters:
	nHeight

Return Value:
	None.
*/
{
	SetRowHeight(nHeight);
}

void CGrid::StoreHeaderHeight(int nHeight)
/*
Routine Description:
	Save the header height.

Parameters:
	nHeight

Return Value:
	None.
*/
{
	SetHeaderHeight(nHeight);
}

int CGrid::GetVirtualHeaderHeight()
/*
Routine Description:
	Get the height of header row.

Parameters:
	None.

Return Value:
	The height.
*/
{
	return (m_bShowColHeader ? m_nHeaderHeight : 0) + (m_bShowGroupHeader && m_nGroups ? m_nHeaderLevelHeight : 0);
}
