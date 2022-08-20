#if !defined(AFX_GRIDNUMCTRL_H__627D0AC0_74DE_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDNUMCTRL_H__627D0AC0_74DE_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridNumCtrl.h : header file
//

#include "gridcontrol.h"
#include "NumEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CGridNumCtrl window

class CGridNumCtrl : public CNumEdit, public CGridControl
{
// Construction
public:
	CGridNumCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridNumCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	// Set the mask string.
	void SetMask(CString strMask);

	// Process char message from grid.
	void OnGridChar(UINT ch);

	// Init control styles.
	void Init(CellStyle *pStyle);

	// Draw a cell.
	void Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell);

	// Get the current text.
	void GetText(CString & strText);
	virtual ~CGridNumCtrl();

protected:
	// The mask string.
	CString m_strMask;

	// Generated message map functions
protected:
	// Calc length of each part from mask.
	void CalcMask();

	//{{AFX_MSG(CGridNumCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDNUMCTRL_H__627D0AC0_74DE_11D3_A7FE_0080C8763FA4__INCLUDED_)
