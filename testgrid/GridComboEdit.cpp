// GridComboEdit.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "GridComboEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridComboEdit

CGridComboEdit::CGridComboEdit()
{
}

CGridComboEdit::~CGridComboEdit()
{
}


BEGIN_MESSAGE_MAP(CGridComboEdit, CMaskEdit)
	//{{AFX_MSG_MAP(CGridComboEdit)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridComboEdit message handlers

BOOL CGridComboEdit::PreTranslateMessage(MSG* pMsg) 
{
	// Make sure that the keystrokes continue to the appropriate handlers
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		::TranslateMessage(pMsg);
		::DispatchMessage(pMsg);
		return TRUE;
	}	

	// Catch the Alt key so we don't choke if focus is going to an owner drawn button
	if (pMsg->message == WM_SYSCHAR)
		return TRUE;

	return CEdit::PreTranslateMessage(pMsg);
}

void CGridComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		 nChar == VK_DOWN  || nChar == VK_UP   ||
		 nChar == VK_RIGHT || nChar == VK_LEFT) &&
		(GetKeyState(VK_CONTROL) < 0 && GetDlgCtrlID() == IDC_GRIDCOMBOEDIT))
	{
		// Get the owner combo box window.
		CWnd* pOwner = GetOwner();
		if (pOwner)
		// Let the owner window process this message.
			pOwner->SendMessage(WM_KEYDOWN, nChar, nRepCnt + (((DWORD)nFlags)<<16));

		return;
	}

	CMaskEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGridComboEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE) 
	{
		// Get the owner combo box window.
		CWnd* pOwner = GetOwner();
		if (pOwner)
		// Let the owner window process this message.
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (((DWORD)nFlags)<<16));

		return;
	}

	if (nChar == VK_TAB || nChar == VK_RETURN)
	{
		// Get the owner combo box window.
		CWnd* pOwner = GetOwner();

		// Let the owner window process this message.
		if (pOwner)
			pOwner->SendMessage(WM_KEYUP, nChar, nRepCnt + (((DWORD)nFlags)<<16));
		
		return;
	}
	
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CGridComboEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	CWnd * pOwner = GetOwner();
	if (pOwner->IsWindowVisible())
	// Let the owner window process this message.
		pOwner->SendMessage(WM_KILLFOCUS, (WPARAM)pNewWnd->GetSafeHwnd(), 0);
}

