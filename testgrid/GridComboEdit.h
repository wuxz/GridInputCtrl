#if !defined(AFX_GRIDCOMBOEDIT_H__8F27C8A2_6465_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDCOMBOEDIT_H__8F27C8A2_6465_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridComboEdit.h : header file
//

#include "maskedit.h"

/////////////////////////////////////////////////////////////////////////////
// CGridComboEdit window

class CGridComboEdit : public CMaskEdit
{
// Construction
public:
	CGridComboEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridComboEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGridComboEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridComboEdit)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	friend class CGridComboCtrl;
	friend class CGridEditBtnCtrl;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCOMBOEDIT_H__8F27C8A2_6465_11D3_A7FE_0080C8763FA4__INCLUDED_)
