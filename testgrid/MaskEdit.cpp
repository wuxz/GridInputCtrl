// MaskEdit.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "MaskEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMaskEdit

CMaskEdit::CMaskEdit()
{
	// Init the delimiter status array.
	for (int i = 0; i < 255; i ++)
		m_bIsDelimiter[i] = FALSE;

	m_nMaxLength = 255;
	m_strPromptChar = _T("_");
	m_bPromptInclude = TRUE;
}

CMaskEdit::~CMaskEdit()
{
}


BEGIN_MESSAGE_MAP(CMaskEdit, CEdit)
	//{{AFX_MSG_MAP(CMaskEdit)
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_PASTE, OnPaste)
	ON_MESSAGE_VOID(WM_CUT, OnCut)
	ON_MESSAGE_VOID(WM_CLEAR, OnClear)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMaskEdit message handlers

void CMaskEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	
	// Skip delimiters.
	MoveRight();
}

void CMaskEdit::CalcMask()
/*
Routine Description:
	Process mask string.

Parameters:
	None.

Return Value:
	None.
*/
{
	BOOL bDelimiter = FALSE;
	char cCurrChr;
	int i, j;
	CString strInit;

	m_strInputMask.Empty();
	for (i = 0; i < 255; i++)
		m_bIsDelimiter[i] = FALSE;

	if (m_strMask.IsEmpty())
		return;

	for (i = 0, j = 0; i < m_strMask.GetLength(); i++)
	{
		cCurrChr = m_strMask[i];
		if (!bDelimiter && cCurrChr == '/')
		{
			// '/' means the next char should be used as delimiter.
			bDelimiter = TRUE;
			continue;
		}

		// All other chars are trated as delimiter.
		if (cCurrChr != _T('#') && cCurrChr != _T('9') && cCurrChr != _T('?') && 
			cCurrChr != _T('C') && cCurrChr != _T('A') && cCurrChr != _T('a'))
			bDelimiter = TRUE;

		m_strInputMask += cCurrChr;
		
		if (bDelimiter)
			m_bIsDelimiter[j] = TRUE;

		j ++;
		bDelimiter = FALSE;
	}

	m_nMaxLength = m_strInputMask.GetLength();
}

BOOL CMaskEdit::IsValidChar(TCHAR nChar, int nPosition)
/*
Routine Description:
	Decides if the given char at given position is valid.

Parameters:
	nChar

	nPosition

Return Value:
	If is valid, return TRUE; Otherwise, return FALSE.
*/
{
	// Can not beyond the max length range.
	if (nPosition >= m_nMaxLength)
		return FALSE;

	// If have no mask or if this position is a delimiter, this char is valid always.
	if (m_strInputMask.GetLength() == 0 || m_bIsDelimiter[nPosition])
		return TRUE;

	switch (m_strInputMask[nPosition])
	{
	case _T('#'):
		return nChar >= '0' && nChar <= '9';
		break;

	case _T('9'):
		return nChar == ' ' || (nChar >= '0' && nChar <= '9');
		break;

	case _T('?'):
		return (nChar >= 'a' && nChar <= 'z') ||
			(nChar >= 'A' && nChar <= 'Z');
		break;

	case _T('C'):
		return nChar == ' ' || (nChar >= 'a' && nChar <= 'z') 
			|| (nChar >= 'A' && nChar <= 'Z');
		break;

	case _T('A'):
		return (nChar >= 'a' && nChar <= 'z') ||
			(nChar >= 'A' && nChar <= 'Z') ||
			(nChar >= '0' && nChar <= '9');
		break;

	case _T('a'):
		return (nChar >= 32 && nChar <= 126) ||
			(nChar >= 128 && nChar <= 255);
		break;
	};

	return FALSE;
}

BOOL CMaskEdit::IsValidMask(CString strMask)
/*
Routine Description:
	Decides if the given string is a valid mask.

Parameters:
	strMask		The string to be validated.

Return Value:
	If is valid, return TRUE; Otherwise, return FALSE.
*/
{
	// Empty mask is always valid.
	if (strMask.IsEmpty())
		return TRUE;

	// Can not be too long.
	if (strMask.GetLength() > 255)
		return FALSE;

	char cCurrChr;

	for (int i = 0; i < strMask.GetLength(); i++)
	{
		cCurrChr = strMask[i];

		if (cCurrChr == _T('#') || cCurrChr == _T('9') || cCurrChr == _T('?') ||
			cCurrChr == _T('C') || cCurrChr == _T('A') || cCurrChr == _T('a'))
		// Should be one digit at least.
			return TRUE;
	}

	// There is no digit, it is invalid.
	return FALSE;
}

void CMaskEdit::MoveRight()
/*
Routine Description:
	Avoid showing caret at the delimiter position.

Parameters:
	None.

Return Value:
	None.
*/
{
	// Get caret position.
	int nStart = 0, nEnd = 0;
	GetSel(nStart, nEnd);

	if (nEnd > m_nMaxLength || m_bIsDelimiter[nEnd])
	{
		// The caret should not be here.

		// Move the caret right.
		MoveCaretRight();

		// Get caret position.
		GetSel(nStart, nEnd);

		if (nEnd > m_nMaxLength || m_bIsDelimiter[nEnd])
		// If does not work, move the caret left.
			MoveCaretLeft();
	}
}

void CMaskEdit::MoveCaretRight()
/*
Routine Description:
	Move the caret right, skip delimiters.

Parameters:
	None.

Return Value:
	None.
*/
{
	// Get caret position.
	int nStart, nEnd;

	GetSel(nStart, nEnd);

	// Get the state of shift key.
	BOOL bShift = (GetKeyState(VK_SHIFT) < 0);

	if (m_strInputMask.IsEmpty())
	{
		// Have not mask, move caret as normal.
		if (bShift)
		// Is selecting.
			SetSel(nStart, nEnd + 1);
		else
			SetSel(nEnd, nEnd + 1);
		return;
	}

	// Skip delimiters.
	for (int i = nEnd + 1; i <= m_strMask.GetLength() && 
		m_bIsDelimiter[i]; i++);

	if (!m_bIsDelimiter[i])
		nEnd = i;

	// Update caret position.
	if (bShift)
	// Is selecting.
		SetSel(nStart, nEnd);
	else
		SetSel(nEnd, nEnd);
}

void CMaskEdit::MoveCaretLeft()
/*
Routine Description:
	Move the caret left, skip delimiters.

Parameters:
	None.

Return Value:
	None.
*/
{
	int nStart, nEnd;

	// Get status of shift key.
	BOOL bShift = (GetKeyState(VK_SHIFT) < 0);

	if (bShift)
	// Is selecting.
		GetSel(nEnd, nStart);
	else
		GetSel(nStart, nEnd);

	if (m_strInputMask.IsEmpty())
	{
		// Have no mask.

		if (bShift)
		// Is selecting.
			SetSel(nStart, nEnd - 1);
		else
			SetSel(nEnd - 1, nEnd - 1);

		return;
	}

	// Skip delimiters.
	for (int i = nEnd - 1; i >= 0 && m_bIsDelimiter[i]; i--);

	if (i >= 0 && !m_bIsDelimiter[i])
		nEnd = i;

	// Update caret position.
	if (bShift)
	// Is selecting.
		SetSel(nStart, nEnd);
	else
		SetSel(nEnd, nEnd);
}

void CMaskEdit::CalcText(LPCTSTR strNewText)
/*
Routine Description:
	Calc text from given string.

Parameters:
	strNewText		The input string. Default is NULL.

Return Value:
	None.
*/
{
	CString strText;
	
	if (!strNewText)
		GetWindowText(strText);
	else
		strText = strNewText;

	// Calc text.
	CalcDisplayText(strText);

	// Update window text.
	int nStart = 0, nEnd = 0;
	GetSel(nStart, nEnd);
	SetWindowText(strText);
	SetSel(nStart, nEnd);
	MoveRight();
}

void CMaskEdit::CalcDisplayText(CString &strText, BOOL bPromptInclude, int nDataType)
/*
Routine Description:
	Calc text from given string.

Parameters:
	strText		The input and output text.

	bPromptInclude	If include the delimiters.

	nDataType	The type of underlying data.

Return Value:
	None.
*/
{
	CString strResult;
	int i, j, nTextLen;
	int nMaskLen;

	if (m_strInputMask.GetLength() == 0)
	{
		// Have no mask, trim the given text and return.
		strResult = strText.Left(m_nMaxLength);
		strText = strResult;

		goto exit;
	}

	// Have mask.

	// The result text length.
	nTextLen = min(m_nMaxLength, strText.GetLength());

	// Mask string length.
	nMaskLen = m_strInputMask.GetLength();

	for (i = 0, j = 0; i < nTextLen && j < nMaskLen; i ++)
	{
		if (bPromptInclude)
		{
			// Include delimiters.

			if (m_bIsDelimiter[i])
			// Is delimiter
				strResult += m_strInputMask[i];
			else if (IsValidChar(strText[i], i))
			// Is valid text.
				strResult += strText[i];
			else
			// Is not valid text, replace it as prompt char.
				strResult += m_strPromptChar;
		}
		else
		{
			// Do not include delimiters.
			while (m_bIsDelimiter[j] && j < nMaskLen)
				j ++;

			if (j >= nMaskLen)
				break;

			if (IsValidChar(strText[i], i))
				strResult += strText[i];
		}
	}

	if (m_strInputMask.GetLength() && bPromptInclude)
	{
		// Fill the rest position with delimiters.
		for (i = nTextLen; i < m_nMaxLength; i ++)
		{
			if (m_bIsDelimiter[i])
				strResult += m_strInputMask[i];
			else
				strResult += m_strPromptChar;
		}
	}

	strText = strResult;

exit:
	// Convert to desired data type.

	switch (nDataType)
	{
	case DataTypeText:
		return;

	case DataTypeVARIANT_BOOL:
		strText = (atoi(strText) == 0) ? _T("0") : _T("1");
		return;

	case DataTypeByte:
		strText.Format("%d", (char)atoi(strText));
		return;

	case DataTypeInteger:
		strText.Format("%d", (short)atoi(strText));
		return;

	case DataTypeLong:
		strText.Format("%d", atoi(strText));
		return;

	case DataTypeCurrency:
	{
		COleCurrency cur;
		
		cur.ParseCurrency(strText);
		if (cur.GetStatus() == COleCurrency::valid)
			strText = cur.Format();
		else
			strText.Empty();

		return;
	}

	case DataTypeDate:
	{
		COleDateTime date;
		
		date.ParseDateTime(strText);
		if (date.GetStatus() == COleDateTime::valid)
			strText = date.Format();
		else
			strText.Empty();

		return;
	}
	}
}

void CMaskEdit::SetPromptInclude(BOOL bNewValue)
/*
Routine Description:
	Set if the delimiters should be include in result text.

Parameters:
	bNewValue

Return Value:
	None.
*/
{
	m_bPromptInclude = bNewValue;
}

void CMaskEdit::SetMaxLength(int nNewValue)
/*
Routine Description:
	Set the max text length.

Parameters:
	nNewValue

Return Value:
	None.
*/
{
	if (m_strInputMask.GetLength())
	// Have no means having input mask.
		return;

	if (nNewValue <= 0 || nNewValue > 255)
		return;

	m_nMaxLength = nNewValue;

	// Update edit window.
	SetLimitText(m_nMaxLength);
}

void CMaskEdit::SetMask(CString strMask)
/*
Routine Description:
	Set the input mask.

Parameters:
	strMask		The new value.

Return Value:
	None.
*/
{
	// Validate the given string.
	if (!IsValidMask(strMask))
		return;

	// Process new mask.
	m_strMask = strMask;
	CalcMask();
	
	if (IsWindowVisible())
	{
		CalcText();
		MoveRight();
	}

	if (m_strInputMask.GetLength())
	{
		// Update window.
		m_nMaxLength = m_strInputMask.GetLength();

		SetLimitText(m_nMaxLength);
	}
}

void CMaskEdit::SetPromptChar(CString strChar)
/*
Routine Description:
	Set the prompt char.

Parameters:
	strChar		The new char.

Return Value:
	None.
*/
{
	if (strChar.GetLength() != 1)
		return;

	m_strPromptChar = strChar;
	
	if (IsWindowVisible())
		CalcText();
}

void CMaskEdit::OnPaste()
{
	// Do default work.
	DefWindowProc(WM_PASTE, 0, 0);

	// Recalc text.
	CalcText();
	MoveRight();
}

void CMaskEdit::OnCut()
{
	// Do default work.
	DefWindowProc(WM_CUT, 0, 0);

	// Recalc text.
	CalcText();
	MoveRight();
}

void CMaskEdit::OnClear()
{
	// Do default work.
	DefWindowProc(WM_CLEAR, 0, 0);

	// Recalc text.
	CalcText();
	MoveRight();
}

void CMaskEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CString strText;
	GetWindowText(strText);
	int nStart, nEnd;
	
	if (nChar == VK_DELETE)
	{
		SendMessage(WM_KEYDOWN, VK_DELETE, 1);
		return;
	}

	if (m_strInputMask.GetLength())
	{
		GetSel(nStart, nEnd);

		if (nChar == VK_BACK)
		{
			MoveCaretLeft();
			GetSel(nStart, nEnd);
			strText.SetAt(nStart, m_strPromptChar[0]);
		}
		else if (IsValidChar(nChar, nStart))
		{
			GetSel(nStart, nEnd);
			strText.SetAt(nStart, nChar);
			MoveCaretRight();
			MoveRight();
		}

		CalcText(strText);

		return;
	}
	else
	{
		// let the default window procedure to update the text and caret :)
		CEdit::OnChar(nChar, nRepCnt, nFlags);

		CalcText();
	}
}

void CMaskEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
		nChar == VK_DOWN || nChar == VK_UP ||
		nChar == VK_RIGHT || nChar == VK_LEFT) &&
		GetKeyState(VK_CONTROL) < 0)
	{
		GetParent()->SetFocus();
		GetParent()->SendMessage(WM_KEYDOWN, nChar, 1);	// Hide this window.

		return;
	}

	CString strText;
	int nStart, nEnd;

	if (m_strInputMask.GetLength() == 0)
	{
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		GetWindowText(strText);

		if (nChar == VK_DELETE)
		// Let edit window process this message.
			CalcText(strText);

		return;
	}
	
	BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
	if (bCtrl && nChar == 'C')
	{
		// Ctrl + C
		Copy();
		return;
	}
	else if (bCtrl && nChar == 'V')
	{
		// Ctrl + V.
		Paste();
		return;
	}
	else if (bCtrl && nChar == 'X')
	{
		// Ctrl + X
		Cut();
		return;
	}

	GetWindowText(strText);
	GetSel(nStart, nEnd);
	
	if (nChar == VK_DELETE)
	{
		// Delete current char.
		if(nStart < m_nMaxLength)
		{
			// Replace current char with prompt char.
			GetSel(nStart, nEnd);
			strText.SetAt(nStart, m_strPromptChar[0]);
			CalcText(strText);
		}

		return;
	}

	switch (nChar)
	{
	case VK_HOME:
		SetSel(0, 0);
		MoveRight();
		break;

	case VK_LEFT:
	case VK_UP:
		MoveCaretLeft();
		break;

	case VK_END:
		SetSel(strText.GetLength(), strText.GetLength());
		MoveRight();
		break;

	case VK_RIGHT:
	case VK_DOWN:
		MoveCaretRight();
		MoveRight();
		break;

	default:
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CMaskEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CEdit::OnLButtonDown(nFlags, point);
	
	MoveRight();
}

UINT CMaskEdit::OnGetDlgCode() 
{
    return DLGC_WANTALLKEYS;
}
