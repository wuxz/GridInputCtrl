#if !defined(AFX_GRIDDATEMASKCTRL_H__352724A1_669E_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDDATEMASKCTRL_H__352724A1_669E_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridDateMaskCtrl.h : header file
//

#include "gridcontrol.h"
#include "datemaskedit.h"

/////////////////////////////////////////////////////////////////////////////
// CGridDateMaskCtrl window

class CGridDateMaskCtrl : public CDateMaskEdit, public CGridControl
{
// Construction
public:
	CGridDateMaskCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridDateMaskCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnGridChar(UINT ch);
	virtual void Init(CellStyle *pStyle);
	virtual void GetText(CString &strText);
	virtual void Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell);
	virtual ~CGridDateMaskCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridDateMaskCtrl)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDDATEMASKCTRL_H__352724A1_669E_11D3_A7FE_0080C8763FA4__INCLUDED_)
