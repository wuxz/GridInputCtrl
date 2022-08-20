// GridEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridEditCtrl

CGridEditCtrl::CGridEditCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID)
{
	m_pGrid = pGrid;
 	m_nLastChar = 0; 

	// Create the window.
	DWORD dwEditStyle = WS_CHILD | dwStyle | ES_MULTILINE | ES_AUTOVSCROLL;
//	DWORD dwEditStyle = WS_CHILD | dwStyle | ES_AUTOVSCROLL;
	if (!Create(dwEditStyle, CRect(0, 0, 1, 1), m_pGrid, nID))
		return;

	m_nID = nID;
	m_pWnd = this;

    SetFont(&m_pGrid->m_fntCell);
}

CGridEditCtrl::~CGridEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CGridEditCtrl, CMaskEdit)
	//{{AFX_MSG_MAP(CGridEditCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridEditCtrl message handlers
void CGridEditCtrl::OnGridChar(UINT ch)
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

void CGridEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	DoKillFocus();
}

void CGridEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if (nChar == VK_TAB || nChar == VK_RETURN)
    {
        m_nLastChar = nChar;
        GetParent()->SetFocus(); // This will hide this window
        return;
    }
    else if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
        nChar == VK_DOWN  || nChar == VK_UP   ||
        nChar == VK_RIGHT || nChar == VK_LEFT) &&
        GetKeyState(VK_CONTROL) < 0)
    {
        m_nLastChar = nChar;
        GetParent()->SetFocus(); // Hide this window.
        return;
    }
	
	CMaskEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGridEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_TAB || nChar == VK_RETURN)
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();	// This will destroy this window
		return;
	}
	if (nChar == VK_ESCAPE) 
	{
		CalcText(m_strInitText);	// Restore original text
		m_bDirty = FALSE;
		m_nLastChar = 0;
		GetParent()->SetFocus();
		return;
	}

	CMaskEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CGridEditCtrl::PreTranslateMessage(MSG* pMsg) 
{
    // Catch the Alt key so we don't choke if focus is going to an owner drawn button
    if (pMsg->message == WM_SYSCHAR)
        return TRUE;
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		OnChar(VK_ESCAPE, 1, 1);
	
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}


void CGridEditCtrl::Init(CellStyle *pStyle)
/*
Routine Description:
	Init from saved styles.

Parameters:
	pStyle		The initial styles.

Return Value:
	None.
*/
{
	// Init base class.
	CGridControl::Init(pStyle);

	// If the new text alignment is not the same, destroy current window and create a new
	// window.
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
	SetMaxLength(pStyle->nMaxLength);
	SetPromptInclude(pStyle->bPromptInclude);
	SetPromptChar(pStyle->strPromptChar);
	CalcText(pStyle->strText);

	ShowWindow(SW_SHOW);
	SetFocus();
}

void CGridEditCtrl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
/*
Routine Description:
	Draw cell.

Parameters:
	pDC

	pStyle

	bIsCurrentCell		If this cell has focus?

Return Value:
	None.
*/
{
	// Save current properties.
	CString strMask = m_strMask;
	int nMaxLength = m_nMaxLength;
	BOOL bPromptInclude = m_bPromptInclude;
	CString strPromptChar = m_strPromptChar;

	// Use new properties.
	SetMask(pStyle->strMask);
	SetMaxLength(pStyle->nMaxLength);
	SetPromptInclude(pStyle->bPromptInclude);
	SetPromptChar(pStyle->strPromptChar);

	// Calc text.
	CalcDisplayText(pStyle->strText, m_bPromptInclude);

	// Draw cell.
	CGridControl::Draw(pDC, pStyle, bIsCurrentCell);

	// Restore old properties.
	SetMask(strMask);
	SetMaxLength(nMaxLength);
	SetPromptInclude(bPromptInclude);
	SetPromptChar(strPromptChar);
}

void CGridEditCtrl::GetText(CString &strText)
/*
Routine Description:
	Get current text.

Parameters:
	strText		The result text

Return Value:
	None.
*/
{
	if (!m_bDirty)
	{
		// If is not dirty, return original text.
		strText = m_strInitText;
		return;
	}

	// Calc text.
	GetWindowText(strText);
	CalcDisplayText(strText, m_bPromptInclude, m_nDataType);
}

HBRUSH CGridEditCtrl::CtlColor(CDC* pDC, UINT nCtlColor) 
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
