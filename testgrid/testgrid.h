// testgrid.h : main header file for the TESTGRID application
//

#if !defined(AFX_TESTGRID_H__761FA604_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_TESTGRID_H__761FA604_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestgridApp:
// See testgrid.cpp for the implementation of this class
//

class CTestgridApp : public CWinApp
{
public:
	CTestgridApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestgridApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestgridApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTGRID_H__761FA604_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_)
