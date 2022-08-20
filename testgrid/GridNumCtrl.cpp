// GridNumCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridNumCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridNumCtrl

CGridNumCtrl::CGridNumCtrl(CGrid *pGrid, DWORD dwStyle, UINT nID)
{
	// Default length.
	m_nIntegralDigits = 5;
	m_nDecimalDigits = 2;

	m_pGrid = pGrid;
 	m_nLastChar = 0; 

	// Create window.
	DWORD dwEditStyle = WS_CHILD | dwStyle | ES_AUTOVSCROLL;
	if (!Create(dwEditStyle, CRect(0, 0, 1, 1), m_pGrid, nID))
		return;

	m_nID = nID;
	m_pWnd = this;

    SetFont(&m_pGrid->m_fntCell);
}

CGridNumCtrl::~CGridNumCtrl()
{
}


BEGIN_MESSAGE_MAP(CGridNumCtrl, CNumEdit)
	//{{AFX_MSG_MAP(CGridNumCtrl)
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridNumCtrl message handlers

void CGridNumCtrl::GetText(CString &strText)
/*
Routine Description:
	Get current text.

Parameters:
	strText		The result text.

Return Value:
	None.
*/
{
	if (!m_bDirty)
	{
		// If is not dirty, retur the original text.
		strText = m_strInitText;
		return;
	}

	// Calc text.
	GetWindowText(strText);
	CalcDisplayText(strText, m_bPromptInclude);
	strText.Remove(_T(' '));
}

void CGridNumCtrl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
/*
Routine Description:
	Draw cell.

Parameters:
	pDC

	pStyle		The initial styles.

	bIsCurrentCell	If this cell has focus.

Return Value:
	None.
*/
{
	// Save current properties.
	CString strMask = m_strMask;
	BOOL bPromptInclude = m_bPromptInclude;

	// Use new properties.
	SetPromptInclude(pStyle->bPromptInclude);
	SetMask(pStyle->strMask);

	// Calc text.
	CalcDisplayText(pStyle->strText, m_bPromptInclude);
	pStyle->strText.Remove(_T(' '));

	// Restore old properties.
	SetPromptInclude(bPromptInclude);
	SetMask(strMask);

	// Call base class to draw.
	CGridControl::Draw(pDC, pStyle, bIsCurrentCell);
}

void CGridNumCtrl::Init(CellStyle *pStyle)
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
	// one.

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
	
	SetPromptInclude(pStyle->bPromptInclude);
	SetMask(pStyle->strMask);

	MoveWindow(pStyle->rect);
	CalcText(pStyle->strText);

	ShowWindow(SW_SHOW);
	SetFocus();
}

void CGridNumCtrl::OnGridChar(UINT ch)
/*
Routine Description:
	Process char message from grid.

Parameters:
	ch		The char value.

Return Value:
	None.
*/
{
    CString strText;
	GetWindowText(strText);

	switch (ch)
	{
        case VK_LBUTTON: 
        case VK_RETURN:
			return;

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
			MoveRight();
			return;

		case VK_DELETE:
			CalcText(_T(""));
			MoveRight();
			SendMessage(WM_KEYDOWN, ch, 1);
			return;

		default:
		{
			CalcText(_T(""));
			SetSel(0, 0);
			MoveRight();
		}
    }
    
	// Let edit box process this message.
    SendMessage(WM_CHAR, ch);
}

void CGridNumCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_TAB || nChar == VK_RETURN)
	{
		m_nLastChar = nChar;
		GetParent()->SetFocus();	// This will hide this window
		return;
	}
	if (nChar == VK_ESCAPE) 
	{
		CalcText(m_strInitText);	// Restore original text.
		m_bDirty = FALSE;
		m_nLastChar = 0;
		GetParent()->SetFocus();
		return;
	}

	CNumEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CGridNumCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	DoKillFocus();
}

void CGridNumCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
        GetParent()->SetFocus();
        return;
    }
	
	CNumEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

HBRUSH CGridNumCtrl::CtlColor(CDC* pDC, UINT nCtlColor) 
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

void CGridNumCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);

	// Skip delimiters and spaces.	
	MoveRight();
}

void CGridNumCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CEdit::OnLButtonDown(nFlags, point);
	
	// Skip delimiters and spaces.
	MoveRight();
}

UINT CGridNumCtrl::OnGetDlgCode() 
{
    return DLGC_WANTALLKEYS;
}

void CGridNumCtrl::SetMask(CString strMask)
/*
Routine Description:
	Set the input mask.

Parameters:
	strMask		The new value.

Return Value:
	None.
*/
{
	// Save new valud.
	m_strMask = strMask;

	// Calc mask.
	CalcMask();
	
	if (IsWindowVisible())
	{
		CalcText();
		MoveRight();
	}

	// Calc the length of each part.
	m_nIntegralLen = (m_nIntegralDigits ? m_nIntegralDigits : 1) + (m_bPromptInclude ? (m_nIntegralDigits - 1) / 3 : 0);
	m_nDecimalLen = m_nDecimalDigits;
}

void CGridNumCtrl::CalcMask()
/*
Routine Description:
	Calc text format from mask string.

Parameters:
	None.

Return Value:
	None.
*/
{
	if (m_strMask.GetLength() != 5)
	// The length sould like this:"i10d5".
		return;

	m_strMask.MakeLower();

	// Validate the mask.
	if (m_strMask[0] != _T('i') || m_strMask[3] != _T('d'))
		return;

	if (m_strMask[1] < _T('0') || m_strMask[1] > _T('9') || 
		m_strMask[2] < _T('0') || m_strMask[2] > _T('9') || 
		m_strMask[4] < _T('0') || m_strMask[4] > _T('9'))
		return;

	// Calc length of every part.
	m_nIntegralDigits = (m_strMask[1] - _T('0')) * 10 + m_strMask[2] - _T('0');
	m_nDecimalDigits = m_strMask[4] - _T('0');
}
