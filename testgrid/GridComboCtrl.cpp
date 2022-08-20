// GridComboCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridComboCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridComboCtrl

CGridComboCtrl::CGridComboCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID)
{
	// Save init states.
	m_pGrid = pGrid;
 	m_nLastChar = 0; 

	// Create a combo box window.
 	DWORD dwComboStyle = WS_CHILD | WS_VSCROLL | CBS_AUTOHSCROLL | dwStyle;
	if (!Create(dwComboStyle, CRect(0, 0, 1, 1), m_pGrid, nID))
		return;

	if (!IsDropList())
	// It is combo box, subclass the edit box.
		m_wndEdit.SubclassDlgItem(IDC_GRIDCOMBOEDIT, this);

	m_pWnd = this;

	SetFocus();

	// Save the font.
	SetFont(&m_pGrid->m_fntCell);
}

CGridComboCtrl::~CGridComboCtrl()
{
}


BEGIN_MESSAGE_MAP(CGridComboCtrl, CComboBox)
	//{{AFX_MSG_MAP(CGridComboCtrl)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridComboCtrl message handlers

void CGridComboCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);

	// If the focus is obtained from the edit box, ignore it.
	if (GetSafeHwnd() == pNewWnd->GetSafeHwnd())
		return;

	// Only end editing on change of focus if we're using the CBS_DROPDOWNLIST style.
	if (m_pGrid->m_bIPEdit)
		DoKillFocus();
}

void CGridComboCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		nChar == VK_DOWN || nChar == VK_UP ||
		nChar == VK_RIGHT || nChar == VK_LEFT) &&
		GetKeyState(VK_CONTROL) < 0)
	{
		// Save the last char.
		m_nLastChar = nChar;

		// Hide this window.
		m_pGrid->SetFocus();

		return;
	}
	
	CComboBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGridComboCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE) 
	{
		m_bDirty = FALSE;
		
		if (!IsDropList())
			m_wndEdit.CalcText(m_strInitText); // Restore previous text.
		else
		// Select current string.
			SelectString(-1, m_strInitText);
	}

	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		m_nLastChar = (nChar == VK_ESCAPE ? 0 : nChar);
		GetParent()->SetFocus(); // This will hide this window.
		
		return;
	}
	
	CComboBox::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CGridComboCtrl::Init(CellStyle *pStyle)
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

	pStyle->rect.bottom -= 6;
	int nHeight = pStyle->rect.Height();
	pStyle->rect.bottom = pStyle->rect.bottom + 8 * nHeight + ::GetSystemMetrics(SM_CYHSCROLL);

	ResetContent();
	// Add the strings
	for (int i = 0; i < m_arItems.GetSize(); i++) 
		AddString(m_arItems[i]);

	// Resize the edit window and the drop down window
	MoveWindow(pStyle->rect);

	SetItemHeight(-1, nHeight);
	SetHorizontalExtent(0); // no horz scrolling

	if (IsDropList())
	// Is combo list window.
		SelectString(-1, m_strInitText);
	else
	{
		// Is combo box, init the input mask.
		m_wndEdit.SetMask(pStyle->strMask);
		m_wndEdit.CalcText(pStyle->strText);
		m_wndEdit.SetMaxLength(pStyle->nMaxLength);
	}

	ShowWindow(SW_SHOW);
	SetFocus();
}

void CGridComboCtrl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
/*
Routine Description:
	Draw the cell.

Parameters:
	pDC

	pStyle

	bIsCurrentCell		If this is the current cell.

Return Value:
	None.
*/
{
	if (bIsCurrentCell)
	{
		// Is current cell, simulate the combo box's aspect.

		// Calc the rect to draw the button.
		CalcButtonRect(pStyle->rect);

		// Calc the rect to draw text.
		pStyle->rect.right -= 15;

		// Draw the button.
	
		COLORREF clrBk = pDC->GetBkColor();

		pDC->Draw3dRect(&m_rectButton, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DDKSHADOW));
		m_rectButton.DeflateRect(1, 1);
		pDC->Draw3dRect(&m_rectButton, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
		m_rectButton.DeflateRect(1, 1);
		pDC->FillSolidRect(&m_rectButton, GetSysColor(COLOR_3DFACE));
		m_rectButton.InflateRect(2, 2);
		
		CRect r = m_rectButton;
		r.top += 2;
		r.bottom -= 2;
		
		// Now draw the bitmap centered on the face
		CDC memDC;
		if(memDC.CreateCompatibleDC(pDC))
		{
			// Draw the triangle picture on the button.

			CBitmap bmp;
			bmp.LoadBitmap(IDB_BITMAP_DOWNARROW);
			
			BITMAP bm;
			VERIFY(bmp.GetObject(sizeof(bm), &bm) == sizeof(bm));
			
			int nLeft = max(0, (r.Width() - bm.bmWidth)/2) + r.left;
			int nTop  = max(0, (r.Height() - bm.bmHeight)/2) + r.top;
			
			//		if(bPressed)
			// The button is currently pressed, so offset bitmap to give
			// impression of movement.
			//			nLeft++, nTop++;
			
			CBitmap *pOldBmp = memDC.SelectObject(&bmp);
			pDC->BitBlt(nLeft, nTop, min(bm.bmWidth, r.Width()-1), min(bm.bmHeight, r.Height()-1), &memDC, 0, 0, SRCCOPY);
			memDC.SelectObject(pOldBmp);
		}

		pDC->SetBkColor(clrBk);
	}

	if (!IsDropList())
	{
		// Save current properties before calc display text.
		CString strMask = m_wndEdit.m_strMask;
		int nMaxLength = m_wndEdit.m_nMaxLength;
		BOOL bPromptInclude = m_wndEdit.m_bPromptInclude;
		CString strPromptChar = m_wndEdit.m_strPromptChar;

		// Set properties as given.
		m_wndEdit.SetMask(pStyle->strMask);
		m_wndEdit.SetMaxLength(pStyle->nMaxLength);
		m_wndEdit.SetPromptInclude(pStyle->bPromptInclude);
		m_wndEdit.SetPromptChar(pStyle->strPromptChar);

		// Calc display text.
		m_wndEdit.CalcDisplayText(pStyle->strText, m_wndEdit.m_bPromptInclude);

		// Restore original properties.
		m_wndEdit.SetMask(strMask);
		m_wndEdit.SetMaxLength(nMaxLength);
		m_wndEdit.SetPromptInclude(bPromptInclude);
		m_wndEdit.SetPromptChar(strPromptChar);
	}

	// Draw the text.
	CGridControl::Draw(pDC, pStyle, bIsCurrentCell);
}

void CGridComboCtrl::GetText(CString &strText)
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
		// If have not been modified, return original text.
		strText = m_strInitText;
		return;
	}

	// Get current text.
	GetWindowText(strText);
	if (!IsDropList())
	// Calc the display text if have input mask.
		m_wndEdit.CalcDisplayText(strText, m_wndEdit.m_bPromptInclude, m_nDataType);
}

BOOL CGridComboCtrl::IsDropList()
/*
Routine Description:
	Decides if this is drop list window.

Parameters:
	None.

Return Value:
	If is, return TRUE; Otherwiser, return FALSE.
*/
{
	return (GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST;
}

void CGridComboCtrl::CalcButtonRect(CRect rt)
/*
Routine Description:
	Calc the rect to draw the button.

Parameters:
	rt		The entire window rect.

Return Value:
	None.
*/
{
	// The width of this button is 15.
	m_rectButton = rt;
	m_rectButton.left = m_rectButton.right - 15;
}

void CGridComboCtrl::OnGridChar(UINT ch)
/*
Routine Description:
	Process char message from the grid.

Parameters:
	ch		The message value.

Return Value:
	None.
*/
{
    CString strText;
	GetWindowText(strText);

	switch (ch)
	{
        case VK_LBUTTON:
		{
			CPoint pt;
			if (!GetCursorPos(&pt))
				return;

			// Let combo box window process this click message.
			ScreenToClient(&pt);
			SendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
			SendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));

			return;
		}

        case VK_RETURN:
			return;

        case VK_BACK:
			if (!IsDropList())
			// Delete text.
				m_wndEdit.SetSel((int)_tcslen(strText), -1);

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
			if (!IsDropList())
				m_wndEdit.SetSel(0, -1);

			return;

        default:
		{
			if (!IsDropList())
			{
				// Init the text.
				m_wndEdit.CalcText(_T(""));
				m_wndEdit.SetSel(0, 0);
			}
		}
    }
    
    SendMessage(WM_CHAR, ch);
}

HBRUSH CGridComboCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// Delete old brush.
	if (m_pBrhBack)
	{
		delete m_pBrhBack;
		m_pBrhBack = NULL;
	}

	// Create new brush.
	m_pBrhBack = new CBrush(m_clrBack);

	// Set colors.
	pDC->SetTextColor(m_clrFore);
	pDC->SetBkColor(m_clrBack);

	return *m_pBrhBack;
}
