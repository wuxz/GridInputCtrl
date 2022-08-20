// testgridDlg.h : header file
//

#if !defined(AFX_TESTGRIDDLG_H__761FA606_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_TESTGRIDDLG_H__761FA606_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTestgridDlg dialog
#include "grid.h"

class CTestgridDlg : public CDialog
{
// Construction
public:
	CTestgridDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestgridDlg)
	enum { IDD = IDD_TESTGRID_DIALOG };
	int		m_nCol;
	int		m_nStyle;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestgridDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CGrid m_wndGrid;
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestgridDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelendokComboColstyle();
	afx_msg void OnButtonSetcolor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTGRIDDLG_H__761FA606_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_)
