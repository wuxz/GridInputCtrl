// GridCheckBoxCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridCheckBoxCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridCheckBoxCtrl

CGridCheckBoxCtrl::CGridCheckBoxCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID)
{
	// Save the grid pointer.
	m_pGrid = pGrid;

	// The last char.
 	m_nLastChar = 0; 

	// Create window.
	DWORD dwNewStyle = WS_CHILD | dwStyle;
	if (!Create(NULL, NULL, dwNewStyle, CRect(0, 0, 1, 1), m_pGrid, nID))
		return;

	// Save ID.
	m_nID = nID;

	m_pWnd = this;
}

CGridCheckBoxCtrl::~CGridCheckBoxCtrl()
{
}


BEGIN_MESSAGE_MAP(CGridCheckBoxCtrl, CWnd)
	//{{AFX_MSG_MAP(CGridCheckBoxCtrl)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGridCheckBoxCtrl message handlers

BOOL CGridCheckBoxCtrl::PreTranslateMessage(MSG* pMsg) 
{
    // Catch the Alt key so we don't choke if focus is going to an owner drawn button
    if (pMsg->message == WM_SYSCHAR)
        return TRUE;
	
	return CWnd::PreTranslateMessage(pMsg);
}

void CGridCheckBoxCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_TAB:
	case VK_RETURN:
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();	// This will hide this window
		return;
	}

	break;

	case VK_ESCAPE:
	{
		// restore previous text
		m_bDirty = FALSE;
		m_nLastChar = 0;
		GetParent()->SetFocus();
		return;
	}
	
	break;

	case VK_SPACE:
	{
		// Toggle the checked state.
		m_bChecked = !m_bChecked;

		// Redraw.
		Invalidate();
	}

	break;
	}
	
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CGridCheckBoxCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_TAB || nChar == VK_RETURN)
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus(); // This will hide this window
		return;
	}
	else if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		nChar == VK_DOWN  || nChar == VK_UP ||
		nChar == VK_RIGHT || nChar == VK_LEFT))
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();
		return;
	}
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGridCheckBoxCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	// Save data.
	DoKillFocus();
}

void CGridCheckBoxCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);

	// Toggle the checked state.
	OnChar(VK_SPACE, 1, 1);
}

UINT CGridCheckBoxCtrl::OnGetDlgCode() 
{
	return DLGC_WANTALLKEYS;
}

void CGridCheckBoxCtrl::GetText(CString &strText)
/*
Routine Description:
	Get the current text.

Parameters:
	strText		The result text.

Return Value:
	None.
*/
{
	if (!m_bDirty)
	{
		// If the state is not modified, return the previous text.
		strText = m_strInitText;
		return;
	}

	// Calc new text.
	CalcText(strText);
}

void CGridCheckBoxCtrl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
/*
Routine Description:
	Draw the control.

Parameters:
	pDC

	pStyle		The styles used to draw.

	bIsCurrentCell	Is this cell is the current cell.

Return Value:
	None.
*/
{
	// Draw the background.
	DrawBackGround(pDC, pStyle, bIsCurrentCell);

	pStyle->rect.DeflateRect(1, 1);

	// Calc the rect to draw check mark.

	POINT pt = pStyle->rect.CenterPoint();
	CRect rect = pStyle->rect;

	switch (pStyle->nAlignment)
	{
	case DT_CENTER:
		rect.left = pt.x - 6;
	case DT_LEFT:
		rect.right = rect.left + 12;
		break;

	case DT_RIGHT:
		rect.left = rect.right - 12;
	}

	rect.top = pt.y - 6;
	rect.bottom = pt.y + 6;

	pStyle->rect.left = max(rect.left, pStyle->rect.left);
	pStyle->rect.right = min(rect.right, pStyle->rect.right);
	pStyle->rect.top = max(rect.top, pStyle->rect.top);
	pStyle->rect.bottom = min(rect.bottom, pStyle->rect.bottom);

	// Draw the check box rect.
	CPen pen(PS_SOLID, 1, pStyle->clrFore);
	pDC->SelectObject(&pen);
	pDC->SelectStockObject(NULL_BRUSH);
	pDC->Rectangle(pStyle->rect);

	CString strText;
	CalcText(strText, pStyle->strText);
	if (strText[0] != '0')
	{
		// The state is checked, draw the cross.
		pDC->MoveTo(pStyle->rect.left, pStyle->rect.top);
		pDC->LineTo(pStyle->rect.right, pStyle->rect.bottom);
		pDC->MoveTo(pStyle->rect.right - 1, pStyle->rect.top);
		pDC->LineTo(pStyle->rect.left - 1, pStyle->rect.bottom);
	}
}

void CGridCheckBoxCtrl::Init(CellStyle *pStyle)
/*
Routine Description:
	Init cell from styles.

Parameters:
	pStyle		The initial styles.

Return Value:
	None.
*/
{
	// Init the base class.
	CGridControl::Init(pStyle);

	// Move the window position.
	MoveWindow(pStyle->rect);

	// Calc the check state from text style.
	CString strText;
	CalcText(strText, pStyle->strText);
	m_bChecked = atoi(strText);

	// Show window.
	ShowWindow(SW_SHOW);
	SetFocus();
}

void CGridCheckBoxCtrl::OnGridChar(UINT ch)
/*
Routine Description:
	One char message activates this cell.

Parameters:
	ch		The char.

Return Value:
	None.
*/
{
	if (ch == VK_LBUTTON || ch == VK_SPACE)
		OnChar(VK_SPACE, 1, 1);
}

void CGridCheckBoxCtrl::CalcText(CString &strText, LPCTSTR lpszValue)
/*
Routine Description:
	Calc the current text from state.

Parameters:
	strText		The result text.

	lpszValue	The current text.

Return Value:
	None.
*/
{
	BOOL bChecked = lpszValue ? atoi(lpszValue) : m_bChecked;

	strText = bChecked ? _T("1") : _T("0");
}

void CGridCheckBoxCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Save styles before calc text.	
	CellStyle style;
	style.clrBack = m_clrBack;
	style.clrFore = m_clrFore;

	// Calc text.
	CalcText(style.strText);

	// Restore original styles.
	style.nAlignment = m_nAlignment;
	GetClientRect(&style.rect);
	
	// Draw.
	Draw(&dc, &style, TRUE);
}
