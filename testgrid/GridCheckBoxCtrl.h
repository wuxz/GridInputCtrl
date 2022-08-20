#if !defined(AFX_GridCheckBoxCtrl_H__352724A3_669E_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GridCheckBoxCtrl_H__352724A3_669E_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridCheckBoxCtrl.h : header file
//

#include "gridcontrol.h"

/////////////////////////////////////////////////////////////////////////////
// CGridCheckBoxCtrl window

class CGridCheckBoxCtrl : public CWnd, public CGridControl
{
// Construction
public:
	CGridCheckBoxCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID);

// Attributes
public:

protected:
	// The checked state.
	BOOL m_bChecked;
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridCheckBoxCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	// Process char message from grid.
	virtual void OnGridChar(UINT ch);

	// Init cell from styles.
	virtual void Init(CellStyle *pStyle);

	// Draw the cell.
	virtual void Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell);

	// Get current text.
	virtual void GetText(CString & strText);
	virtual ~CGridCheckBoxCtrl();

	// Generated message map functions
protected:
	// Calc current value.
	void CalcText(CString & strText, LPCTSTR lpszValue = NULL);

	//{{AFX_MSG(CGridCheckBoxCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GridCheckBoxCtrl_H__352724A3_669E_11D3_A7FE_0080C8763FA4__INCLUDED_)
