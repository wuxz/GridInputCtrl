#if !defined(AFX_GRIDEDITBTNCTRL_H__69BCD6C0_677E_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDEDITBTNCTRL_H__69BCD6C0_677E_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridEditBtnCtrl.h : header file
//

#include "gridcontrol.h"
#include "gridcomboedit.h"

/////////////////////////////////////////////////////////////////////////////
// CGridEditBtnCtrl window

class CGridEditBtnCtrl : public CWnd, public CGridControl
{
// Construction
public:
	CGridEditBtnCtrl(CGrid * pGrid, DWORD dwStyle, UINT nID);

// Attributes
public:

protected:
	// The edit box.
	CGridComboEdit m_wndEdit;
	
	// The button.
	CRect m_rectButton;
	CButton m_wndButton;
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridEditBtnCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell);
	virtual void GetText(CString &strText);
	virtual void OnGridChar(UINT ch);
	virtual void Init(CellStyle * pStyle);
	virtual ~CGridEditBtnCtrl();

	// Generated message map functions
protected:
	void CalcButtonRect(CRect rt);
	//{{AFX_MSG(CGridEditBtnCtrl)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClickButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDEDITBTNCTRL_H__69BCD6C0_677E_11D3_A7FE_0080C8763FA4__INCLUDED_)
