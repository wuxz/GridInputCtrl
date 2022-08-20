#if !defined(AFX_GRIDEDITCTRL_H__8F27C8A0_6465_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDEDITCTRL_H__8F27C8A0_6465_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridEditCtrl.h : header file
//

#include "maskedit.h"
#include "gridcontrol.h"

/////////////////////////////////////////////////////////////////////////////
// CGridEditCtrl window

class CGridEditCtrl : public CMaskEdit, public CGridControl
{
// Construction
public:
	CGridEditCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID);

// Attributes
public:

protected:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridEditCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	// Get current text.
	virtual void GetText(CString & strText);

	// Draw cell.
	virtual void Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell);

	// Init control from saved styles.
	virtual void Init(CellStyle *pStyle);

	// Process char message from grid.
	virtual void OnGridChar(UINT ch);

	virtual ~CGridEditCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridEditCtrl)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDEDITCTRL_H__8F27C8A0_6465_11D3_A7FE_0080C8763FA4__INCLUDED_)
