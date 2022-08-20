#if !defined(AFX_GRIDCOMBOCTRL_H__8F27C8A1_6465_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDCOMBOCTRL_H__8F27C8A1_6465_11D3_A7FE_0080C8763FA4__INCLUDED_

#include "GridComboEdit.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridComboCtrl.h : header file
//

#include "gridcontrol.h"

/////////////////////////////////////////////////////////////////////////////
// CGridComboCtrl window

class CGridComboCtrl : public CComboBox, public CGridControl
{
// Construction
public:
	CGridComboCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID);

// Attributes
public:

protected:
	// The edit box part.
	CGridComboEdit m_wndEdit;

	// The rect for the button.
	CRect m_rectButton;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridComboCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnGridChar(UINT ch);
	virtual void GetText(CString &strText);
	virtual void Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell);
	virtual void Init(CellStyle * pStyle);
	virtual ~CGridComboCtrl();

	// Generated message map functions
protected:
	// Calc the rect the button occupies.
	void CalcButtonRect(CRect rt);

	// Decides if this is a combo list.
	BOOL IsDropList();

	//{{AFX_MSG(CGridComboCtrl)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCOMBOCTRL_H__8F27C8A1_6465_11D3_A7FE_0080C8763FA4__INCLUDED_)
