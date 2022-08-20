// GridEditBtnCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridEditBtnCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridEditBtnCtrl

CGridEditBtnCtrl::CGridEditBtnCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID)
{
	m_pGrid = pGrid;
 	m_nLastChar = 0;
	m_nID = nID;

	if (!Create(NULL, NULL, WS_CHILD, CRect(0, 0, 1, 1), m_pGrid, nID))
		return;

	if (m_nID == IDC_INPLACE_EDIT)
	{
		// It's edit button control.

		// Create edit box.
		m_wndEdit.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, CRect(0, 0, 1, 1), this, 1);

		// Create button.
		m_wndButton.Create(_T("..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(0, 0, 1, 1), this, 2);
	}
	else
	// It's button control.
	// Create button.
		m_wndButton.Create(NULL, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(0, 0, 1, 1), this, 2);

	m_pWnd = this;

	SetFocus();

	if (m_nID == IDC_INPLACE_EDIT)
	// It's edit button control.
		m_wndEdit.SetFont(&m_pGrid->m_fntCell);

	m_wndButton.SetFont(&m_pGrid->m_fntCell);
}

CGridEditBtnCtrl::~CGridEditBtnCtrl()
{
}


BEGIN_MESSAGE_MAP(CGridEditBtnCtrl, CWnd)
	//{{AFX_MSG_MAP(CGridEditBtnCtrl)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_KILLFOCUS()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(2, OnClickButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGridEditBtnCtrl message handlers

void CGridEditBtnCtrl::OnGridChar(UINT ch)
{
	if (m_nID != IDC_INPLACE_EDIT)
	{
		m_pGrid->SetFocus();

		if (ch == VK_LBUTTON)
			OnClickButton();

		return;
	}

	CString strText;
	m_wndEdit.GetWindowText(strText);

	switch (ch)
	{
		case VK_LBUTTON:
		{
			CPoint pt;
			if (!GetCursorPos(&pt))
				return;

			ScreenToClient(&pt);
			if (m_rectButton.PtInRect(pt))
			// Let the button process this clicking message.
				OnClickButton();

			return;
		}

		case VK_RETURN:
			return;

		case VK_BACK:
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
			m_wndEdit.SetSel(0, -1);

			return;

		default:
		{
			m_wndEdit.CalcText(_T(""));
			m_wndEdit.SetSel(0, 0);
		}
	}
	
	m_wndEdit.SendMessage(WM_CHAR, ch);
}

void CGridEditBtnCtrl::GetText(CString &strText)
/*
Routine Description:
	Get the current text.

Parameters:
	strText		The result text.

Return Value:
	None.
*/
{
	if (m_nID != IDC_INPLACE_EDIT)
	{
		// It's button control.
		// Return the text on the button.
		m_wndButton.GetWindowText(strText);
		
		return;
	}
	else if (!m_bDirty)
	{
		// It's not dirty, retur original text.
		strText = m_strInitText;
		
		return;
	}

	// Calc text comes from edit box.
	m_wndEdit.GetWindowText(strText);
	m_wndEdit.CalcDisplayText(strText, m_wndEdit.m_bPromptInclude, m_nDataType);
}

void CGridEditBtnCtrl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
/*
Routine Description:
	Draw cell.

Parameters:
	pDC

	pStyle

	bIsCurrentCell		If this cell has focus.

Return Value:
	None.
*/
{
	if (bIsCurrentCell)
	{
		// Has focus, draw the button and text.

		// Calc the button rect.
		CalcButtonRect(pStyle->rect);
		pStyle->rect.right -= 15;
		
		// Draw the button.
		COLORREF clrBk = pDC->GetBkColor();
		
		pDC->Draw3dRect(&m_rectButton, GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_3DDKSHADOW));
		m_rectButton.DeflateRect(1, 1);
		pDC->Draw3dRect(&m_rectButton, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
		m_rectButton.DeflateRect(1, 1);
		pDC->FillSolidRect(&m_rectButton, GetSysColor(COLOR_3DFACE));
		m_rectButton.InflateRect(2, 2);
		
		if (m_nID == IDC_INPLACE_EDIT)
		{
			CRect r = m_rectButton;
			r.top += 2;
			r.bottom -= 2;
			
			// Now draw the bitmap centered on the face
			CDC memDC;
			if(memDC.CreateCompatibleDC(pDC))
			{
				// Draw the triangle picture on button.
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
		}
		else
		{
			// Draw text only.
			pDC->SelectObject(&m_pGrid->m_fntCell);
			pDC->DrawText(pStyle->strText, m_rectButton, pStyle->nAlignment | DT_SINGLELINE | DT_VCENTER);
		}

		pDC->SetBkColor(clrBk);
	}

	if (m_nID == IDC_INPLACE_EDIT)
	{
		// It's edit button control. Calc text from input mask.

		// Save properties.
		CString strMask = m_wndEdit.m_strMask;
		int nMaxLength = m_wndEdit.m_nMaxLength;
		BOOL bPromptInclude = m_wndEdit.m_bPromptInclude;
		CString strPromptChar = m_wndEdit.m_strPromptChar;

		// Set new properties.
		m_wndEdit.SetMask(pStyle->strMask);
		m_wndEdit.SetMaxLength(pStyle->nMaxLength);
		m_wndEdit.SetPromptInclude(pStyle->bPromptInclude);
		m_wndEdit.SetPromptChar(pStyle->strPromptChar);

		// Calc text.
		m_wndEdit.CalcDisplayText(pStyle->strText, m_wndEdit.m_bPromptInclude);

		// Draw the cell.
		CGridControl::Draw(pDC, pStyle, bIsCurrentCell);

		// Restore properties.
		m_wndEdit.SetMask(strMask);
		m_wndEdit.SetMaxLength(nMaxLength);
		m_wndEdit.SetPromptInclude(bPromptInclude);
		m_wndEdit.SetPromptChar(strPromptChar);
	}
}

void CGridEditBtnCtrl::Init(CellStyle *pStyle)
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

	if (m_nID != IDC_INPLACE_EDIT)
	// If is button, it will not be dirty.
		m_bDirty = FALSE;

	MoveWindow(pStyle->rect);

	CRect rect = pStyle->rect;
	rect.OffsetRect(-rect.left, -rect.top);
	CalcButtonRect(rect);
	rect.right -= m_rectButton.Width();

	if (m_nID == IDC_INPLACE_EDIT)
		m_wndEdit.MoveWindow(rect);

	m_wndButton.MoveWindow(m_rectButton);

	if (m_nID == IDC_INPLACE_EDIT)
	{
		m_wndEdit.SetMask(pStyle->strMask);
		m_wndEdit.CalcText(pStyle->strText);
		m_wndEdit.SetMaxLength(pStyle->nMaxLength);
	}

	ShowWindow(SW_SHOW);
	
	m_wndButton.ShowWindow(SW_SHOW);
	
	if (m_nID == IDC_INPLACE_EDIT)
	{
		m_wndEdit.ShowWindow(SW_SHOW);
		m_wndEdit.SetFocus();
	}
	else
	{
		m_wndButton.SetWindowText(pStyle->strText);
		SetFocus();
	}
}

void CGridEditBtnCtrl::CalcButtonRect(CRect rt)
/*
Routine Description:
	Calc the rect the butto occupies.

Parameters:
	rt		The entire control window rect.

Return Value:
	None.
*/
{
	m_rectButton = rt;
	
	if (m_nID == IDC_INPLACE_EDIT)
	// The height is 15 in edit button cell.
		m_rectButton.left = m_rectButton.right - 15;
}

void CGridEditBtnCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		nChar == VK_DOWN || nChar == VK_UP ||
		nChar == VK_RIGHT || nChar == VK_LEFT) &&
		(GetKeyState(VK_CONTROL) < 0 || m_nID != IDC_INPLACE_EDIT))
	{
		m_nLastChar = nChar;
		m_pGrid->SetFocus();	// Hide this window.

		return;
	}
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGridEditBtnCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE) 
	{
		m_bDirty = FALSE;
		
		if (m_nID == IDC_INPLACE_EDIT)
			m_wndEdit.CalcText(m_strInitText); // Restore original text.
	}

	if (nChar == VK_TAB || nChar == VK_RETURN || nChar == VK_ESCAPE)
	{
		m_nLastChar = (nChar == VK_ESCAPE ? 0 : nChar);
		GetParent()->SetFocus(); // This will hide this window.
		
		return;
	}
	
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CGridEditBtnCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	if (GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_wndEdit.GetSafeHwnd() == pNewWnd->GetSafeHwnd())
		return;

	if (pNewWnd->GetSafeHwnd() == m_wndButton.m_hWnd)
	{
//		m_wndEdit.SetFocus();
		return;
	}

	// Only end editing on change of focus if we're using the CBS_DROPDOWNLIST style
	if (m_pGrid->m_bIPEdit)
		DoKillFocus();
}

HBRUSH CGridEditBtnCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

void CGridEditBtnCtrl::OnClickButton()
/*
Routine Description:
	The button is clicked.

Parameters:
	None.

Return Value:
	None.
*/
{
	m_pGrid->OnClickedCellButton(m_pGrid->m_nRow, m_pGrid->m_nCol);
}
