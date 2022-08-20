// GridControl.h: interface for the CGridControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDCONTROL_H__DE372A40_652C_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRIDCONTROL_H__DE372A40_652C_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "grid.h"

class CGridControl
{
public:
	// Set margin between text and border.
	void SetMargin(int nMargin);

	// Get the current text.
	virtual void GetText(CString & strText);

	// Process char message from the grid.
	virtual void OnGridChar(UINT ch);

	// Draw a cell.
	virtual void Draw(CDC * pDC, CellStyle * pStyle, BOOL bIsCurrentCell);

	// Init cell.
	virtual void Init(CellStyle * pStyle);
	CGridControl();
	virtual ~CGridControl();

	// The window pointer of this control.
	CWnd * m_pWnd;
protected:
	// Init choice list array from string.
	void InitChoiceList(CString strList);

	// Draw the background.
	void DrawBackGround(CDC * pDC, CellStyle * pStyle, BOOL IsCurrentCell);

	// Is current cell dirty?
	BOOL m_bDirty;

	// Do when lost focus.
	void DoKillFocus();

	// The grid.
	CGrid * m_pGrid;

	// The last char which activates this control.
    UINT m_nLastChar;

	// The original text before activation.
	CString m_strInitText;

	// The colors.
	COLORREF m_clrBack, m_clrFore;

	// The underlying data type.
	int m_nDataType;

	// The text alignment.
	int m_nAlignment;

	// Choice list array.
	CStringArray m_arItems;

	// Window ID.
	UINT m_nID;

	// The brush to draw the background.
	CBrush * m_pBrhBack;

	int m_nMargin;
};

#endif // !defined(AFX_GRIDCONTROL_H__DE372A40_652C_11D3_A7FE_0080C8763FA4__INCLUDED_)
