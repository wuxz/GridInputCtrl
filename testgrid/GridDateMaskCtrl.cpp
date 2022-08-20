// GridDateMaskCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridDateMaskCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridDateMaskCtrl

CGridDateMaskCtrl::CGridDateMaskCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID)
{
	m_pGrid = pGrid;
 	m_nLastChar = 0; 

	// Create window.
	DWORD dwEditStyle = WS_CHILD | dwStyle | ES_AUTOHSCROLL;
	if (!Create(dwEditStyle, CRect(0, 0, 1, 1), m_pGrid, nID))
		return;

	m_nID = nID;
	m_pWnd = this;

    SetFont(&m_pGrid->m_fntCell);
}

CGridDateMaskCtrl::~CGridDateMaskCtrl()
{
}


BEGIN_MESSAGE_MAP(CGridDateMaskCtrl, CDateMaskEdit)
	//{{AFX_MSG_MAP(CGridDateMaskCtrl)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridDateMaskCtrl message handlers

HBRUSH CGridDateMaskCtrl::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	if (m_pBrhBack)
	{
		delete m_pBrhBack;
		m_pBrhBack = NULL;
	}

	m_pBrhBack = new CBrush(m_clrBack);
	pDC->SetTextColor(m_clrFore);
	pDC->SetBkColor(m_clrBack);

	return *m_pBrhBack;
}

void CGridDateMaskCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if (nChar == VK_TAB || nChar == VK_RETURN)
    {
        m_nLastChar = nChar;
        GetParent()->SetFocus();    // This will hide this window
        return;
    }
    if (nChar == VK_ESCAPE) 
    {
		CalcText(m_strInitText);    // Restore original text.
        m_bDirty = FALSE;
        m_nLastChar = 0;
        GetParent()->SetFocus();
        return;
    }
	
	CDateMaskEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CGridDateMaskCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if (nChar == VK_TAB || nChar == VK_RETURN)
    {
        m_nLastChar = nChar;
        GetParent()->SetFocus(); // This will hide this window.
        return;
    }
    else if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
        nChar == VK_DOWN  || nChar == VK_UP   ||
        nChar == VK_RIGHT || nChar == VK_LEFT) &&
        GetKeyState(VK_CONTROL) < 0)
    {
        m_nLastChar = nChar;
        GetParent()->SetFocus();
        return;
    }
	
	CDateMaskEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGridDateMaskCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	DoKillFocus();
}

BOOL CGridDateMaskCtrl::PreTranslateMessage(MSG* pMsg) 
{
    // Catch the Alt key so we don't choke if focus is going to an owner drawn button
    if (pMsg->message == WM_SYSCHAR)
        return TRUE;
	
	return CDateMaskEdit::PreTranslateMessage(pMsg);
}

void CGridDateMaskCtrl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
{
	// Save properties.
	CString strMask = strFormat;
	BOOL bPromptInclude = m_bPromptInclude;

	// Set new properties.
	SetMask(pStyle->strMask);
	SetPromptInclude(pStyle->bPromptInclude);

	// Calc text.
	CalcDisplayText(pStyle->strText, m_bPromptInclude);

	// Draw the cell.
	CGridControl::Draw(pDC, pStyle, bIsCurrentCell);

	// Restore properties.
	SetMask(strMask);
	SetPromptInclude(bPromptInclude);
}

void CGridDateMaskCtrl::GetText(CString &strText)
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
		// If is not dirty, retur original text.
		strText = m_strInitText;
		return;
	}

	// Calc text.
	CalcData();
	GetWindowText(strText);
	CalcDisplayText(strText, m_bPromptInclude, m_nDataType);
}

void CGridDateMaskCtrl::Init(CellStyle *pStyle)
/*
Routine Description:
	Init from saved styles.

Parameters:
	pStyle		The initial styles.

Return Value:
	None.
*/
{
	nCurrentPosition = 0;

	m_bMingGuo = FALSE;
	m_bFormatMingGuo = FALSE;
	nYearFinish = nMonthFinish = nDayFinish = 0;
	m_bTextNull = TRUE;

	strFormat = _T("yyyy/mm/dd");
	year = 0;
	month = 0;
	day = 0;
	m_PrevData.wYear = year;
	m_PrevData.wMonth = month;
	m_PrevData.wDay = day;

	CGridControl::Init(pStyle);

	// If the alignment style is changed, destroy current window and create a new window.
	DWORD dwStyle = GetStyle();
	DWORD dwNewStyle = dwStyle & ~(ES_LEFT | ES_CENTER | ES_RIGHT);

	switch (pStyle->nAlignment)
	{
	case DT_CENTER:
		dwNewStyle |= ES_CENTER;
		break;

	case DT_LEFT:
		dwNewStyle |= ES_LEFT;
		break;

	case DT_RIGHT:
		dwNewStyle |= ES_RIGHT;
		break;
	}

	if (dwStyle != dwNewStyle)
	{
		DestroyWindow();
		if (!Create(dwNewStyle & ~WS_VISIBLE, CRect(0, 0, 1, 1), m_pGrid, m_nID))
			return;

		SetFont(&m_pGrid->m_fntCell);
	}
	
	MoveWindow(pStyle->rect);
	SetMask(pStyle->strMask);
	SetPromptInclude(pStyle->bPromptInclude);
	CalcText(pStyle->strText);

	ShowWindow(SW_SHOW);
	SetFocus();
}

void CGridDateMaskCtrl::OnGridChar(UINT ch)
{
    CString strText;
	GetWindowText(strText);

	switch (ch)
	{
        case VK_LBUTTON: 
        case VK_RETURN:
			return;

        case VK_BACK:
			SetSel((int)_tcslen(strText), -1);
			break;

        case VK_TAB:
        case VK_DOWN: 
        case VK_UP:   
        case VK_RIGHT:
        case VK_LEFT:  
        case VK_NEXT:  
        case VK_PRIOR: 
        case VK_HOME:
        case VK_SPACE:
        case VK_END:
			SetSel(0, -1);
			return;

        default:
		{
			CalcText(_T(""));
			SetSel(0, 0);
		}
    }
    
    SendMessage(WM_CHAR, ch);
}
