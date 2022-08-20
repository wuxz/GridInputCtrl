// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__761FA608_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_STDAFX_H__761FA608_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#include <afxpriv.h>
#include <afxdisp.h>

#define BHMGRID_CLASSNAME _T("BHM_GridClass")
#define DEFAULTCOLOR RGB(0, 0, 1)

#define IDC_GRIDCOMBOEDIT 1001
#define IDC_INPLACE_EDIT 8 // ID of inplace edit controls

#define INVALID 0xFFFFFFFF

#define CASE_NONE 0
#define CASE_UPPER 1
#define CASE_LOWER 2

#define DataTypeText 0
#define DataTypeVARIANT_BOOL 1
#define DataTypeByte 2
#define DataTypeInteger 3
#define DataTypeLong 4
#define DataTypeSingle 5
#define DataTypeCurrency 6
#define DataTypeDate 7


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__761FA608_39F6_11D3_A7FE_0080C8763FA4__INCLUDED_)
