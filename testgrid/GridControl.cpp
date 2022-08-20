// GridControl.cpp: implementation of the CGridControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "testgrid.h"
#include "GridControl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridControl::CGridControl()
{
	m_pWnd = NULL;

	m_pBrhBack = NULL;

	m_nMargin = 0;
}

CGridControl::~CGridControl()
{
	if (m_pBrhBack)
	{
		delete m_pBrhBack;
		m_pBrhBack = NULL;
	}
}

void CGridControl::Init(CellStyle *pStyle)
/*
Routine Description:
	Init control from saved styles.

Parameters:
	pStyle

Return Value:
	None.
*/
{
	// Save the given properties.
	m_strInitText = pStyle->strText;
	m_nDataType = pStyle->nDataType;
	m_nAlignment = pStyle->nAlignment;

	m_bDirty = TRUE;
 	m_nLastChar = 0; 
	
	m_clrBack = pStyle->clrBack;
	m_clrFore = pStyle->clrFore;

	// Init the choice list array.
	InitChoiceList(pStyle->strChoiceList);
}

void CGridControl::Draw(CDC *pDC, CellStyle *pStyle, BOOL bIsCurrentCell)
/*
Routine Description:
	Draw the control.

Parameters:
	pDC

	pStyle

	bIsCurrentCell		If this cell has the focus.

Return Value:
	None.
*/
{
	// Let the grid process it first.
	m_pGrid->OnDrawCellText(pDC, pStyle);
	
//	DrawBackGround(pDC, pStyle, bIsCurrentCell);

	pDC->SetTextColor(pStyle->clrFore);
	pDC->SelectObject(&m_pGrid->m_fntCell);

	// Take care of text case.
	if (pStyle->nCase == CASE_UPPER)
		pStyle->strText.MakeUpper();
	else if (pStyle->nCase == CASE_LOWER)
		pStyle->strText.MakeLower();

	// Take care of text alignment.
	if (pStyle->nAlignment == DT_LEFT)
		pStyle->rect.left += m_nMargin;
	else if (pStyle->nAlignment == DT_RIGHT)
		pStyle->rect.right -= m_nMargin;

	// Center the text vertitcally.
	CRect rtDraw = pStyle->rect;
	int nHeight = pDC->DrawText(pStyle->strText, rtDraw, pStyle->nAlignment | DT_WORDBREAK | DT_CALCRECT);
	if (nHeight < pStyle->rect.Height())
		pStyle->rect.top += (pStyle->rect.Height() - nHeight) / 2;

	// Draw the text.
	pDC->DrawText(pStyle->strText, pStyle->rect, pStyle->nAlignment | DT_WORDBREAK);
}

void CGridControl::OnGridChar(UINT ch)
{

}

void CGridControl::GetText(CString &strText)
{

}

void CGridControl::DoKillFocus()
{
	if (m_pGrid->OnEndEditCell())
		m_pGrid->SendMessage(WM_KEYDOWN, m_nLastChar, 1);
	else
		m_pWnd->SetFocus();
}

void CGridControl::DrawBackGround(CDC *pDC, CellStyle *pStyle, BOOL IsCurrentCell)
{
	m_pGrid->DrawCellBackGround(pDC, pStyle);
}

void CGridControl::SetMargin(int nMargin)
{
	if (nMargin >= 0)
		m_nMargin = nMargin;
}

void CGridControl::InitChoiceList(CString strList)
/*
Routine Description:
	Construct string array from one string.

Parameters:
	strList

Return Value:
	None.
*/
{
	m_arItems.RemoveAll();

	char ch;
	CString strChoice;

	// String are delimited by '\n';
	for (int i = 0; i < strList.GetLength(); i ++)
	{
		ch = strList[i]; 
		if (ch == '\n')
		{
			m_arItems.Add(strChoice);
			strChoice.Empty();
		}
		else
			strChoice += ch;
	}

	if (ch != '\n')
		m_arItems.Add(strChoice);
}
