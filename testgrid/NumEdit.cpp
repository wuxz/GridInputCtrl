// NumEdit.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "NumEdit.h"
#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumEdit

CNumEdit::CNumEdit()
{
	m_fValue = 0.0;
	m_bPromptInclude = TRUE;
}

CNumEdit::~CNumEdit()
{
}


BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	//{{AFX_MSG_MAP(CNumEdit)
	ON_MESSAGE_VOID(WM_PASTE, OnPaste)
	ON_MESSAGE_VOID(WM_CUT, OnCut)
	ON_MESSAGE_VOID(WM_CLEAR, OnClear)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumEdit message handlers

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Get current text.
	CString strText;
	GetWindowText(strText);

	// Get caret position.
	int nStart, nEnd;
	GetSel(nStart, nEnd);

	if (nChar == VK_BACK)
	{
		// Delete left char.

		// Move the caret left.
		MoveCaretLeft();

		// Delete current char.
		OnKeyDown(VK_DELETE, 1, 1);

		return;
	}
	else if (IsValidChar(nChar, nStart))
	{
		// Should we move the caret right?
		int bMoveRight = 0;

		if (nChar == _T('-'))
		{
			// The negative signal.
			if (strText.GetLength())
			{
				if (strText[0] == _T('-'))
				{
					// Already have negative value, convert to positive value.
					strText.Delete(0);
					bMoveRight = -1;
				}
				else
				{
					// Current value is positive, convert to be negative.
					strText.Insert(0, _T('-'));
					bMoveRight = 1;
				}
			}

			if (bMoveRight > 0)
			// Move caret right.
				SetSel(nStart + 1, nStart + 1);
			else if (bMoveRight < 0)
			// Move caret left.
				SetSel(nStart - 1, nStart - 1);

			// Update window text.
			CalcText(strText);
			return;
		}
		else if (nChar == _T('+'))
		{
			// Positive signal.
			if (strText.GetLength() && strText[0] == _T('-'))
			{
				// Current value is negative, convert it to be positive.
				strText.Delete(0);
				bMoveRight = -1;
			}

			if (bMoveRight > 0)
			// Move caret right.
				SetSel(nStart + 1, nStart + 1);
			else if (bMoveRight < 0)
			// Move caret left.
				SetSel(nStart - 1, nStart - 1);

			// Update window text.
			CalcText(strText);

			return;
		}
		else if (nChar == _T('.'))
		{
			// Position the caret to the decimal signal.
			for (int i = 0; i < strText.GetLength() && strText[i] != _T('.'); i ++);
			SetSel(i, i);
			MoveCaretRight();

			return;
		}

		if (nStart >= strText.GetLength() && m_nDecimalDigits != 0)
			return;

		// Does caret locate at the end of integral part?
		BOOL bEnd = nStart >= strText.GetLength() || strText[nStart] == _T('.');

		if (bEnd)
		{
			if (nStart == m_nIntegralLen)
			// The integral part is full.
				return;

			// Insert new char here.
			strText.Insert(nStart, nChar);
			bMoveRight = 1;
		}
		else
		{
			// Replace current char with new char.
			strText.SetAt(nStart, nChar);
			
			if (nChar != _T('0'))
				bMoveRight = 1;
		}

		if (bMoveRight > 0)
			SetSel(nStart + 1, nStart + 1);
		else if (bMoveRight < 0)
			SetSel(nStart - 1, nStart - 1);

		// Update window text.
		CalcText(strText);

		MoveRight();

		if (bEnd)
		{
			GetWindowText(strText);
			for (int i = 0; i < strText.GetLength() && strText[i] != _T('.'); i ++);
			SetSel(i, i);
		}
	}
}

void CNumEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CString strText;
	int nStart, nEnd;

	GetWindowText(strText);
	GetSel(nStart, nEnd);
	
	if (nChar == VK_DELETE)
	{
		// Delete current char.
		if (nStart >= strText.GetLength())
			return;

		TCHAR ch = strText[nStart];

		if (ch == _T('.') || nStart == m_nIntegralLen)
		{
			// The end of integral part, delete left char.
			strText.Delete(nStart - 1);
		}
		else
		{
			// Delete current char.
			strText.Delete(nStart);
		}

		// Update window text.
		CalcText(strText);

		if (ch != _T('.'))
			MoveCaretRight();

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
		break;

	default:
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CNumEdit::SetWndText(CString str)
/*
Routine Description:
	Set window text and preserve caret position.

Parameters:
	str		The new text.

Return Value:
	None.
*/
{
	// Get caret position.
	int nStart, nEnd;
	GetSel(nStart, nEnd);

	// Set window text.
	SetWindowText(str);

	// Restore caret position.
	SetSel(nStart, nStart);
	MoveRight();
}

void CNumEdit::CalcDisplayText(CString &strText, BOOL bPromptInclude)
/*
Routine Description:
	Calc text from given text.

Parameters:
	strText		The input and output text.

	bPromptInclude	If include the delimiters.

Return Value:
	None.
*/
{
	int i, j, nTextLen;

	BOOL bIntegralPart = TRUE, bNegative = FALSE;
	m_strIntegralValue.Empty();
	m_strDecimalValue.Empty();
	BOOL bZero = TRUE;

	for (i = 0; i < strText.GetLength(); i ++)
	{
		// Get current char.
		TCHAR ch = strText[i];

		if ((ch < _T('0') || ch > _T('9')) && ch != _T('-') && ch != _T('.'))
		// Skip other chars.
			continue;

		if (i == 0 && ch == _T('-'))
		// Is negative value.
			bNegative = TRUE;
		else if (ch == _T('.'))
		// The next text is decimal value.
			bIntegralPart = FALSE;
		else if (bIntegralPart)
		{
			if (ch == _T('0') && m_strIntegralValue.GetLength() == 0)
			// Ignore the leading '0'
				continue;
		
			// Add current char into integral part.
			m_strIntegralValue += ch;

			// The value is not zero.
			bZero = FALSE;
		}
		else
		{
			// Add current char into decimal part.
			m_strDecimalValue += ch;

			// The value is not zero.
			bZero = FALSE;
		}
	}

	// The integral part text length.
	int nIntegralLen = (m_nIntegralDigits ? m_nIntegralDigits : 1) + (bPromptInclude ? (m_nIntegralDigits - 1) / 3 : 0);

	// The decimal part text length.
	int nDecimalLen = m_nDecimalDigits;

	// The result text length.
	nTextLen = nIntegralLen + (nDecimalLen ? nDecimalLen + 1 : 0);
	
	// Construct result text, filled with spaces.
	CString strResult(_T(' '), nTextLen);

	if (nDecimalLen)
	// Have decimal part, fill the decimal signal first.
		strResult.SetAt(nIntegralLen, _T('.'));

	if (bZero)
		// Value is zero.
		bNegative = FALSE;
	else if (bNegative)
	{
		// Value is negative, fill the negative signal first.
		strResult.Insert(0, _T('-'));
		nTextLen ++;
		nIntegralLen ++;
	}
	
	// Construct string format value.
	CString strValue;
	strValue = m_strIntegralValue + _T(".") + m_strDecimalValue;

	// Get the result value.
	m_fValue = atof(strValue);
	if (bNegative)
		m_fValue = -m_fValue;

	for (i = nIntegralLen - 1, j = 0; i >= (bNegative ? 1 : 0); i --)
	{
		// Fill integral part.
		if (m_strIntegralValue.GetLength())
		{
			j ++;

			if ((j - 1) / 3 && (j - 1) % 3 == 0 && i > (bNegative ? 1 : 0) && bPromptInclude)
			{
				// Add ',' every 3 digits in integral part.
				strResult.SetAt(i --, _T(','));
			}

			strResult.SetAt(i, m_nIntegralDigits ? m_strIntegralValue.Right(1)[0] : _T('0'));
			m_strIntegralValue.Delete(m_strIntegralValue.GetLength() - 1);
		}
		else if (i == nIntegralLen - 1)
			strResult.SetAt(i, _T('0'));
	}

	// Fill decimal part.
	strValue = m_strDecimalValue + _T("000000000");
	strValue = strValue.Left(nDecimalLen);
	for (i = 0; i < nDecimalLen; i ++)
		strResult.SetAt(nTextLen - i - 1, strValue[strValue.GetLength() - i - 1]);

	// Remove spaces.
	strResult.Remove(_T(' '));

	// Return result text.
	strText = strResult;
}

void CNumEdit::CalcText(LPCTSTR strNewText)
/*
Routine Description:
	Calc window text.

Parameters:
	strNewText		The input text.

Return Value:
	None.
*/
{
	CString strText;
	
	if (!strNewText)
		GetWindowText(strText);
	else
		strText = strNewText;

	// Calc and update window text.
	CalcDisplayText(strText, m_bPromptInclude);

	SetWndText(strText);
}

void CNumEdit::MoveCaretLeft()
/*
Routine Description:
	Move the caret left within digital part.

Parameters:
	None.

Return Value:
	None.
*/
{
	// If have no focus, do nothing.
	if (GetFocus() != this)
		return;

	// Get caret position.
	int nStart, nEnd;
	GetSel(nStart, nEnd);

	CString strText;
	GetWindowText(strText);

	// Skip the non-digital chars.
	for (int i = nEnd - 1; i >= 0 && (strText[i] > _T('9') || strText[i] < _T('0')) && strText[i] != _T('.'); i --);

	if (i >= 0)
		nEnd = i;

	// Update caret position.
	SetSel(nEnd, nEnd);
}

void CNumEdit::MoveCaretRight()
/*
Routine Description:
	Move caret right within digital part.

Parameters:
	None.

Return Value:
	None.
*/
{
	// If have no focus, do nothing.
	if (GetFocus() != this)
		return;

	// Get caret position.
	int nStart, nEnd;
	GetSel(nStart, nEnd);

	CString strText;
	GetWindowText(strText);

	if (m_nDecimalDigits == 0 && nEnd >= strText.GetLength())
		return;

	// Skip non-digital chars.
	for (int i = nEnd + 1; i < strText.GetLength() && (strText[i] > _T('9') || strText[i] < _T('0')) && strText[i] != _T('.'); i ++);

	if (i < strText.GetLength() || (m_nDecimalDigits == 0 && i >= strText.GetLength()))
		nEnd = i;

	// Update caret position.
	SetSel(nEnd, nEnd);
}

void CNumEdit::MoveRight()
/*
Routine Description:
	Avoid showing caret at non-digital position.

Parameters:
	None.

Return Value:
	None.
*/
{
	if (GetFocus() != this)
		return;

	int nStart = 0, nEnd = 0;
	GetSel(nStart, nEnd);
	CString strText;
	GetWindowText(strText);

	if (m_nDecimalDigits == 0 && nEnd >= strText.GetLength())
		return;

	if (nEnd >= strText.GetLength() || strText[nEnd] > _T('9') || strText[nEnd] < _T('0') && strText[nEnd] != _T('.'))
	{
		MoveCaretRight();
		GetSel(nStart, nEnd);
		
		if (m_nDecimalDigits == 0 && nEnd > strText.GetLength())
			return;

		if (nEnd >= strText.GetLength() || strText[nEnd] > _T('9') || strText[nEnd] < _T('0') && strText[nEnd] != _T('.'))
			MoveCaretLeft();
	}
}

BOOL CNumEdit::IsValidChar(TCHAR nChar, int nPosition)
/*
Routine Description:
	Determin if given char is valid.

Parameters:
	/*
Routine Description:
	Set window text and preserve caret position.

Parameters:
	nChar		The char.

	nPosition	The char position.

Return Value:
	None.
*/


Return Value:
	None.
*/
{
	return (nChar >= _T('0') && nChar <= _T('9')) || nChar == _T('.') || nChar == _T('-') || nChar == _T('+');
}

void CNumEdit::OnClear()
{
	return;
}

void CNumEdit::OnCut()
{
	return;
}

void CNumEdit::OnPaste()
{
	return;
}

void CNumEdit::SetPromptInclude(BOOL b)
/*
Routine Description:
	Set if include prompt char in text.

Parameters:
	b

Return Value:
	None.
*/
{
	m_bPromptInclude = b;
	m_nIntegralLen = (m_nIntegralDigits ? m_nIntegralDigits : 1) + (m_bPromptInclude ? (m_nIntegralDigits - 1) / 3 : 0);
	CalcText();
}
