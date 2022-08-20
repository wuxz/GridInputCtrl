// DateMaskEdit.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "DateMaskEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDateMaskEdit

CDateMaskEdit::CDateMaskEdit()
{
	nCurrentPosition = 0;

	m_bMingGuo = FALSE;
	m_bFormatMingGuo = FALSE;
	nYearFinish = nMonthFinish = nDayFinish = 0;
	m_bTextNull = TRUE;
	m_bPromptInclude = TRUE;

	strFormat = _T("yyyy/mm/dd");
	year = 0;
	month = 0;
	day = 0;
	m_PrevData.wYear = year;
	m_PrevData.wMonth = month;
	m_PrevData.wDay = day;
}

CDateMaskEdit::~CDateMaskEdit()
{
}


BEGIN_MESSAGE_MAP(CDateMaskEdit, CEdit)
	//{{AFX_MSG_MAP(CDateMaskEdit)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE_VOID(WM_PASTE, OnPaste)
	ON_MESSAGE_VOID(WM_CUT, OnCut)
	ON_MESSAGE_VOID(WM_CLEAR, OnClear)
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDateMaskEdit message handlers

void CDateMaskEdit::CalcDisplayText(CString &strText, BOOL bPromptInclude, int nDataType)
/*
Routine Description:
	Calc the text should be shown currently base on the given text.
	
Parameters:
	strText		The in and out text.
	
	bPromptInclude	If the prompt char should be included.
	
	nDataType	The underlying data type.
	
Return Value:
	None.
*/
{
	// Is the given text valid?
	BOOL bValidText = SetTextIn(strText, nDataType) && !m_bTextNull;

	// Empty the result text.
	strText.Empty();
	
	// Calc data displayed when setfocus.
	
	// Reset signal variables.
	nYearLength = nMonthLength = nDayLength = 0;
	nYearPosition[0] = nYearPosition[1] = -1;
	nMonthPosition[0] = nMonthPosition[1] = -1;
	nDayPosition[0] = nDayPosition[1] = -1;
	int i;
	nDelimiters = 0;
	
	
	// Calc the Mingguo text.
	CString str, str2;
	str2.Empty();
	str.Format("%3d", abs(year - 1911));
	str.TrimLeft();
	str2 += str;
	str.Format("%02d", month);
	str2 += str;
	str.Format("%02d", day);
	str2 += str;
	
	// The year is negative.
	if (year < 1911)
		str2 = "-" + str2;
		
	// Are we using Mingguo freely format?
	m_bMingGuo = strFormat == "ggggggg" ? TRUE : FALSE;
	
	// Yes, we are.
	if (m_bMingGuo)
	{
		if (!m_bTextNull)
			strText = str2;
		else
		// Current data is null.
			strText.Empty();
	}

	// Init signal array.
	for (i = 0; i < 8; i++)
	{
		// Reset each delimiter's position.
		nDelimiterPosition[i] = 0;
		
		// Empty each delimiter string.
		strDelimiter[i / 2].Empty();
	}

	// Current char.
	TCHAR chr;
	
	// Previous char.
	TCHAR chrPrev = '/';
	
	// We are not using Mingguo format.
	for (i = 0; i < strFormat.GetLength() && !m_bMingGuo; i ++)
	{
		// Get the current char.
		chr = strFormat[i];
		switch (chr)
		{
			case 'y':
			{
				// This char locates in year part.
				
				// Is not Mingguo format.
				m_bFormatMingGuo = FALSE;
				
				// Increase the length of year part.
				nYearLength ++;
				
				if (chrPrev != chr)
				// The start of current field.
					nYearPosition[0] = i;
					
				// Update the previous char.
				chrPrev = 'y';
				
				if (i == strFormat.GetLength() - 1 ||
					strFormat[i + 1] != 'y') // End of year part.
				{
					// The length should be valid.
					if (nYearLength == 1 || nYearLength == 3)
					{
						// Extend the year part to 2 or 4 chars long.
						strFormat = strFormat.Left(nYearPosition[0] 
							+ nYearLength) + 'y' + strFormat.Right(
							strFormat.GetLength() - nYearLength - 
							nYearPosition[0]);
						i ++;
						nYearLength ++;
					}
					
					// The end of year part.
					nYearPosition[1] = i;
					
					CString strYear;
					if (m_bTextNull)
					{
						// The year data is null.
						if (bPromptInclude)
						// Prompt chars should be included.
							strYear = "____";
					}
					else
					// Construct the year string.
						strYear.Format("%04d", year);
						
					// Retrieve the desired length.
					strYear = strYear.Right(nYearLength);
					
					// Add new string to result string.
					strText += strYear;
				}
				break;
			}

			case 'g':
			{
				// The year part.
				
				// Format is Mingguo.
				m_bFormatMingGuo = TRUE;
				
				nYearLength ++;
				
				// The start of current field.
				if (chrPrev != chr)
					nYearPosition[0] = i;
					
				// Update the previous char.
				chrPrev = 'g';
				
				if (i == strFormat.GetLength() - 1 ||
					strFormat[i + 1] != 'g') // The end of year data.
				{
					// Extend the year part to 3 chars long. 
					
					if (nYearLength == 2)
					{
						strFormat = strFormat.Left(nYearPosition[0] 
							+ nYearLength) + 'g' + strFormat.Right(
							strFormat.GetLength() - nYearLength - 
							nYearPosition[0]);
						i ++;
						nYearLength = 3;
					}
					if (nYearLength == 1)
					{
						strFormat = strFormat.Left(nYearPosition[0] 
							+ nYearLength) + "gg" + strFormat.Right(
							strFormat.GetLength() - nYearLength - 
							nYearPosition[0]);
						i += 2;
						nYearLength = 3;
					}
					
					// The end of currend field.
					nYearPosition[1] = i;
					
					// Construct year part string.
					
					CString strYear;
					if (m_bTextNull)
					{
						// The year data is null.
						if (bPromptInclude)
							strYear = "___";
					}
					else
						strYear.Format("%03d", year - 1911);
					strYear = strYear.Right(nYearLength);
					
					// Add new string to the result.
					strText += strYear;
				}
				break;
			}

			case 'm':
			{
				// The month part.
				
				nMonthLength++;
				if (chrPrev != chr)
				// The start point of month part.
					nMonthPosition[0] = i;
					
				// Update the previous char.
				chrPrev = 'm';
				
				if (i == strFormat.GetLength() - 1 ||
					strFormat[i + 1] != 'm') // The end of month data.
				{
					nMonthPosition[1] = i;
					
					// Construct the month part string.
					
					CString strMonth;
					if(m_bTextNull)
					{
						// The month data is null.
						if (bPromptInclude)
							strMonth = "__";
					}
					else
						strMonth.Format("%02d", month);
					strMonth = strMonth.Right(nMonthLength);
					
					// Add new string to result string.
					strText += strMonth;
				}
				break;
			}

			case 'd':
			{
				// The day part.
				
				nDayLength++;
				if (chrPrev != chr)
				// The start point of day data.
					nDayPosition[0] = i;
					
				// Update the previous char.
				chrPrev = 'd';
				
				if (i == strFormat.GetLength() - 1 ||
					strFormat[i + 1] != 'd') // The end of day data.
				{
					nDayPosition[1] = i;
					
					// Construct day string.
					CString strDay;
					if (m_bTextNull)
					{
						// The day data is null.
						if (bPromptInclude)
							strDay = "__";
					}
					else
						strDay.Format("%02d", day);
					strDay = strDay.Right(nDayLength);
					
					// Add the new string to result string.
					strText += strDay;
				}
				break;
			}

			// The delimiter part.
			default:
			{
				if (bPromptInclude)
					strText += chr;

				// Add current char to current delimiter string.
				strDelimiter[nDelimiters] += chr;
				
				// Set the previous char to '/' means that now is in delimiter part.
				if (chrPrev != '/')
					nDelimiterPosition[2 * nDelimiters] = i;
				chrPrev = '/';

				if (i == strFormat.GetLength() - 1 ||
					strFormat[i + 1] == 'y' ||
						strFormat[i + 1] == 'm' || 
						strFormat[i + 1] == 'd')// The end of this part of delimiters.
				{
					nDelimiterPosition[2 * nDelimiters + 1] = i;
					nDelimiters ++;
				}
				
				break;
			}
		}
	}

	if (!IsFinished())
	{
		// Each time this function is called and if there is no date valid data yet, we restore the original data.
		year = m_PrevData.wYear;
		month = m_PrevData.wMonth;
		
		// Month can not be 0.
		month = month == 0 ? 1 : month;
		day = m_PrevData.wDay;
		
		// Day can not be 0.
		day = day == 0 ? 1 : day;
		
		if (!m_bTextNull)
		// Update the signal variables.
			nYearFinish = nMonthFinish = nDayFinish = 4;
		else
		{
			nYearFinish = nMonthFinish = nDayFinish = 0;
			nCurrentPosition = 0;
		}
	}

	// Take the data type into account.
	
	if (!bValidText && (!bPromptInclude || nDataType == DataTypeDate))
	{
		strText.Empty();

		return;
	}

	if (nDataType == DataTypeDate)
	{
		// We should use date type string instead.
		strText.Format("%4d%02d%02d", year, month, day);

		return;
	}
	else if (m_bMingGuo && !m_bTextNull)
	{
		// In Mingguo free format, should not display delimiters.
		strText.Format("%03d%02d%02d", year - 1911, month, day);
		return;
	}
}

void CDateMaskEdit::CalcData()
/*
Routine Description:
	Calc the data in special cases.	

Parameters:
	
Return Value:
	None.
*/
{
	if (!IsFinished())
	{
		// There is no valid data yet.
		year = m_PrevData.wYear;
		month = m_PrevData.wMonth;
		day = m_PrevData.wDay;
		if (!m_bTextNull)
			nYearFinish = nMonthFinish = nDayFinish = 4;
		else
		{
			nYearFinish = nMonthFinish = nDayFinish = 0;
			nCurrentPosition = 0;
		}
	}
	
	if (m_bMingGuo)
	// It is freely Mingguo format now.
		CalcMingGuo();
}

void CDateMaskEdit::CalcMingGuo()
/*
Routine Description:
	Calc the correct text should be displayed in freely Mingguo fromat.
	
Parameters:
	None.
	
Return Value:
	None.
*/
{
	int i;
	char chr;
	CString strYear, strMonth, strDay, strTotal;
	int nNewYear, nNewMonth, nNewDay;
	nNewYear = nNewMonth = nNewDay = 0;
	BOOL bValid = TRUE;
	strYear.Empty();
	strMonth.Empty();
	strDay.Empty();
	strTotal.Empty();
	
	// Get the current text first.
	CString strData;
	GetWindowText(strData);

	if (strData.IsEmpty())
	// Text is null, it's valid. Return now.
		return;
	else
	{
		for (i = 0; i < strData.GetLength(); i++)
		{
			// Filter invalid chars out.
			chr = strData[i];
			if ((chr >= '0' && chr <= '9') || (chr == '-' &&
				strTotal.IsEmpty()))
				strTotal += chr;
		}

		// Verify the filtered text.
		if (strTotal.IsEmpty() || (strTotal[0] == '-' && strTotal.
			GetLength() < 6) || (strTotal[0] != '-' && strTotal.
			GetLength() < 5))
			return;
		
		// Get the 3 parts of data. The format is "yyymmdd" or "-yyymmdd".
		strDay = strTotal.Right(2);
		strMonth = strTotal.Mid(strTotal.GetLength() - 4, 2);
		strYear = strTotal.Left(strTotal.GetLength() - 4);
		
		// Filter the negative signal out.
		i = strYear[0] == '-' ? 1 : 0;
		
		// Calc each part of data.
		
		for (; i < strYear.GetLength(); i++)
			nNewYear = nNewYear * 10 + strYear[i] - '0';
		if (strYear[0] == '-')
		// Year is negative.
			nNewYear = -nNewYear;
		for (i = 0; i < 2; i++)
			nNewMonth = nNewMonth * 10 + strMonth[i] - '0';
		for (i = 0; i < 2; i++)
			nNewDay = nNewDay * 10 + strDay[i] - '0';

		// Validate the result data.
		COleDateTime date;
		date.SetDate(nNewYear + 1911, nNewMonth, nNewDay);
		if (date.GetStatus() == COleDateTime::valid)
		// The data is valid.
			bValid = TRUE;
		else
		// The data is invalid.
			return;
	}

	if (bValid)
	// Update window text now.
		SetDate(nNewYear, nNewMonth, nNewDay);
}

BOOL CDateMaskEdit::IsValidFormat(CString strNewFormat)
/*
Routine Description:
	Decide if the given format is valid.
		
Parameters:
	strNewFormat		The format string to be validated.
	
Return Value:
	If it is a valid format, return TRUE; Otherwise, return FALSE.
*/
{
	if (strNewFormat == "ggggggg")
	// It is freely Mingguo foramt.
		return TRUE;

	// The current char.
	TCHAR chr;
	
	// The previous char.
	TCHAR chrPrev = '/';
	
	int i = 0;
	
	// The length of each part.
	int nYearPosition = 0, nMonthPosition = 0, nDayPosition = 0;
	
	// If each part appears.
	BOOL bHasYear = FALSE, bHasMonth = FALSE, bHasDay = FALSE;
	
	for (i = 0; i < strNewFormat.GetLength(); i ++)
	{
		// Get one char.
		chr = strNewFormat[i];
		switch (chr)
		{
			case 'y':
			{
				// It is in regular year part.
				if (chrPrev == chr)
				{
					// Year length can not exceed 4.
					if (++ nYearPosition > 4)
						return FALSE;
				}
				else
				{
					// The start point of a year part.
					
					if (bHasYear != TRUE)
					{
						// There is no start point yet. It's ok.
						
						// Update the previous char.
						chrPrev = 'y';
						
						// Year format now begins.
						bHasYear = TRUE;
						if (++ nYearPosition > 4)
							return FALSE;
					}
					else
					// There is already a start point of year part, it's a invalid format.
						return FALSE;
				}
			}
			break;

			case 'g':
			{
				// It is in Mingguo year part.
				if (chrPrev == chr)
				{
					// Year length can not exceed 4.
					if (++ nYearPosition > 3)
						return FALSE;
				}
				else
				{
					// It is the start point of a year part.
					
					if (bHasYear != TRUE)
					{
						// Update the previous char.
						chrPrev = 'g';
						
						// Year part now begins.
						bHasYear = TRUE;
						if (++ nYearPosition > 3)
							return FALSE;
					}
					else
					// There is already a start point now, so it's a invalid format.
						return FALSE;
				}
			}
			break;

			case 'm':
			{
				// It is in month part.
				
				if (chrPrev == chr)
				{
					// Month length can not exceed 2.
					if (++ nMonthPosition > 2)
						return FALSE;
				}
				else
				{
					// It is the start point of a month part.
					if (bHasMonth != TRUE)
					{
						// Month now begins.
						bHasMonth = TRUE;
						if (++ nMonthPosition > 2)
							return FALSE;
					}
					else
					// There is already a start point of month part, so it's a invalid format.
						return FALSE;
				}
				
				// Update the previous char.
				chrPrev = 'm';
			}
			break;

			case 'd':
			{
				// It is in day part.
				
				if (chrPrev == chr)
				{
					// The day part length can exceed 2.
					if (++ nDayPosition > 2)
						return FALSE;
				}
				else
				{
					// It is the start point of a day part.
					
					if (bHasDay != TRUE)
					{
						// The day part begins.
						bHasDay = TRUE;
						if (++ nDayPosition > 2)
							return FALSE;
					}
					else
					// There is already a start of day part, so it's a invalid format.
						return FALSE;
				}
				
				// Update the previous char.
				chrPrev = 'd';
			}
			break;

			default:
			{
				// Previous character is delimiter
				chrPrev = '/';
				break;
			}
		}
	}
	
	// 3 parts can not be all empty.
	if (nYearPosition == 0 && nMonthPosition == 0 && nDayPosition == 0)
		return FALSE;
	else
		return TRUE;
}

BOOL CDateMaskEdit::IsFinished()
/*
Routine Description:
	Decides if there is a valid date data.
	
Parameters:
	None.
	
Return Value:
	If there is, return TRUE; Otherwise, return FALSE.
*/
{
	return (nYearFinish >= nYearLength && nMonthFinish >= nMonthLength
		&& nDayFinish >= nDayLength);
}

void CDateMaskEdit::MoveRight(int nSteps)
/*
Routine Description:
	Move the caret right for specified steps.
		
Parameters:
	nSteps		The desired steps.
	
Return Value:
	None.
*/
{
	// Get the text being shown.
	CString strData;
	GetWindowText(strData);

	if (m_bMingGuo)
	{
		// It is freely Mingguo format.
		
		// Move current position right.
		nCurrentPosition += nSteps;
		
		// Assure that there is one char at least.
		if (strData.IsEmpty() || (strData.Right(1) != '_' && strData.GetLength() < 7))
			strData += CString("_");

		// Current position should be in valid range.
		if (nCurrentPosition >= strData.GetLength())
			nCurrentPosition = strData.GetLength() - 1;
		if (nCurrentPosition < 0)
			nCurrentPosition = 0;
			
		// Update the caret.
		SetSel(nCurrentPosition, nCurrentPosition);
		return;
	}

	if (nSteps > 0 && (nCurrentPosition == nYearPosition[0] + nYearFinish)
		&& nCurrentPosition <= nYearPosition[1])
		return;

	if (nSteps > 0 && (nCurrentPosition == nMonthPosition[0] + nMonthFinish)
		&& nCurrentPosition <= nMonthPosition[1])
		return;
	
	if (nSteps > 0 && (nCurrentPosition == nDayPosition[0] + nDayFinish)
		&& nCurrentPosition <= nDayPosition[1])
		return;

	int nNewPosition = nCurrentPosition + nSteps;
	if (nNewPosition >= strFormat.GetLength())
		nNewPosition = strFormat.GetLength() - 1;

	while (nNewPosition <= strFormat.GetLength())
	{
		// Skip the delimiters.
		if (!( (nNewPosition >= nYearPosition[0] && nNewPosition <= nYearPosition[1])
			|| (nNewPosition >= nMonthPosition[0] && nNewPosition <= nMonthPosition[1])
			|| (nNewPosition >= nDayPosition[0] && nNewPosition <= nDayPosition[1])))
			nNewPosition ++;
		else
			break;
	}
	if (nNewPosition > strFormat.GetLength()) // Can not move ahead now.
	{
		// Move left instead.
		while (nNewPosition >= 0)
		{
			if (!( (nNewPosition >= nYearPosition[0] && nNewPosition <= nYearPosition[1])
				|| (nNewPosition >= nMonthPosition[0] && nNewPosition <= nMonthPosition[1])
				|| (nNewPosition >= nDayPosition[0] && nNewPosition <= nDayPosition[1])))
				nNewPosition --;
			else
				break;
		}
	}
	
	// Update the caret position.
	nCurrentPosition = nNewPosition;
	SetSel(nCurrentPosition, nCurrentPosition);
}

void CDateMaskEdit::MoveLeft(int nSteps)
/*
Routine Description:
	Move the caret in specified steps.
		
Parameters:
	nSteps		The desired steps.
	
Return Value:
	None.
*/
{
	CString strData;
	GetWindowText(strData);

	if (m_bMingGuo)
	{
		// It is freely Mingguo format.
		
		// Move curren position left.
		nCurrentPosition -= nSteps;
		
		// Current position should be in valid range.
		if (nCurrentPosition >= strData.GetLength())
			nCurrentPosition = strData.GetLength() -1;
		if (nCurrentPosition < 0)
			nCurrentPosition = 0;
			
		// Update the caret position.
		SetSel(nCurrentPosition, nCurrentPosition);
		return;
	}

	if (nSteps < 0 && (nCurrentPosition == nYearPosition[0] + nYearFinish)
		&& nCurrentPosition <= nYearPosition[1])
		return;

	if (nSteps < 0 && (nCurrentPosition == nMonthPosition[0] + nMonthFinish)
		&& nCurrentPosition <= nMonthPosition[1])
		return;
	
	if (nSteps < 0 && (nCurrentPosition == nDayPosition[0] + nDayFinish)
		&& nCurrentPosition <= nDayPosition[1])
		return;

	int nNewPosition = nCurrentPosition - nSteps;
	if (nNewPosition < 0)
		nNewPosition = 0;

	// Skip the delimiters.
	while (nNewPosition >= 0)
	{
		if (!( (nNewPosition >= nYearPosition[0] && nNewPosition <= nYearPosition[1])
			|| (nNewPosition >= nMonthPosition[0] && nNewPosition <= nMonthPosition[1])
			|| (nNewPosition >= nDayPosition[0] && nNewPosition <= nDayPosition[1])))
			nNewPosition --;
		else
			break;
	}
	if (nNewPosition < 0) // Can not move left, move right instead.
	{
		while (nNewPosition <= strFormat.GetLength())
		{
			if (!( (nNewPosition >= nYearPosition[0] && nNewPosition <= nYearPosition[1])
				|| (nNewPosition >= nMonthPosition[0] && nNewPosition <= nMonthPosition[1])
				|| (nNewPosition >= nDayPosition[0] && nNewPosition <= nDayPosition[1])))
				nNewPosition ++;
			else
				break;
		}
	}

	// Update the caret position.
	nCurrentPosition = nNewPosition;
	SetSel(nCurrentPosition, nCurrentPosition);
}

void CDateMaskEdit::SetMask(CString strMask)
/*
Routine Description:
	Set the input mask.
	
Parameters:
	strMask		The new mask string.
	
Return Value:
	None.
*/
{
	strMask.MakeLower();
	if (IsValidFormat(strMask))
	{
		if (strMask == "ggggggg")
			m_bMingGuo = TRUE;
		else
			m_bMingGuo = FALSE;

		strFormat = strMask;
		
		// Update the text according to new mask.
		if (IsWindowVisible())
		{
			CalcText();
			MoveRight();
		}
	}
}

void CDateMaskEdit::CalcText(LPCTSTR strNewText)
/*
Routine Description:
	Calc text from given string.
	
Parameters:
	strNewText		The initial string pointer. If be null, means use current text.
	
Return Value:
	None.
*/
{
	CString strText;
	
	if (!strNewText)
	// Use current text instead.
		GetWindowText(strText);
	else
		strText = strNewText;

	// Calc the desired text.
	CalcDisplayText(strText);

	// Update the window text.
	SetWndText(strText);
}

void CDateMaskEdit::SetDate(short nYear, short nMonth, short nDay)
/*
Routine Description:
	Modifys the underlying data.
		
Parameters:
	nYear		The year data.
	
	nMonth		The month data.
	
	nDay		The day data.
	
Return Value:
	None.
*/
{
	// Calc current data first.
	CalcText();

	COleDateTime date;
	int nNewYear = nYear;
	
	// Validate the given data.
	
	if (m_bMingGuo || m_bFormatMingGuo)
		nNewYear = nYear + 1911;
	date.SetDate(nNewYear, nMonth, nDay);
	if (date.GetStatus() == COleDateTime::valid)
	{
		m_bTextNull = FALSE;
		year = nNewYear;
		month = nMonth;
		day = nDay;
		m_PrevData.wYear = year;
		m_PrevData.wMonth = month;
		m_PrevData.wDay = day;
	}
	
	
	// Update the window text.
	CalcText();
}

void CDateMaskEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CString strData;
	GetWindowText(strData);

	switch (nChar)
	{
		case VK_HOME:
		{
			// Move the caret to home.
			MoveLeft(nCurrentPosition);
		}
		break;

		case VK_LEFT:
		{
			// Move the caret left.
			MoveLeft(1);
		}
		break;

		case VK_END:
		{
			// Move the caret to the end.
			int pos = nCurrentPosition;
			for (int i = 0; i < strData.GetLength() - pos - 1; i++)
				MoveRight(1);
		}
		break;

		case VK_RIGHT:
		{
			// Move the caret right.
			MoveRight(1);
		}
		break;

		case VK_DELETE:
		{
			// Delete current part of string.
			OnChar(VK_DELETE, 1, 1);
		}
		break;
	}
}

void CDateMaskEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL bFinish = TRUE;

	// Get the current text first.
	CString strData;
	GetWindowText(strData);

	if (m_bMingGuo)
	{
		// It is in freely Mingguo format.
		
		if ((nChar < '0' || nChar > '9') && nChar != 
			VK_BACK && nChar != VK_DELETE && nChar != '/' && nChar != '-') // Invalid char.
			return;
			
		if (nChar == VK_BACK)
		{
			// Delete previous char.
			if (nCurrentPosition == 0 && strData.IsEmpty())
				return;

			// Move the right data in the same part left.
			strData = strData.Left(nCurrentPosition - 1) + strData.
				Right(strData.GetLength() - nCurrentPosition);
				
			if (strData.IsEmpty() || strData == '_')
			// The data is null.
				CalcText("");
			if (strData.IsEmpty() || strData.Right(1) != '_')
			// Replace previous char with '_'.
				strData += CString("_");
			
			// Move the caret.
			MoveLeft(1);
			
			// Update the window text.
			SetWndText(strData);

			return;
		}

		if (nChar == VK_DELETE)
		{
			// Delete current char.
			strData = strData.Left(nCurrentPosition) + strData.
				Right(strData.GetLength() - nCurrentPosition - 1);
			if (strData.IsEmpty() || strData == '_')
				CalcText("");
			if (strData.IsEmpty() || strData.Right(1) != '_')
				strData += CString("_");
			
			// Move the caret left.
			MoveLeft();
			
			// Update the window text.
			SetWndText(strData);

			return;
		}

		else
		// Replace current char with new char.
			if (strData.IsEmpty())
				strData += (char)nChar;
			else
				strData.SetAt(nCurrentPosition, (char)nChar);
				
		// Now the value is accpeted.

		if (strData.IsEmpty() || (strData.Right(1) != '_' && strData.GetLength() < 7))
			strData += CString("_");
			
		// Move the caret right.
		MoveRight(1);
		
		// Update the window text.
		SetWndText(strData);

		return;
	}

	if ((nChar < '0' || nChar > '9') && nChar != VK_BACK && nChar !=
		VK_DELETE && nChar != '/' && nChar != '-')// Invalid characters.
		return;

	COleDateTime temptime;
	CString text;
	text = strData;

	int i = 0;
	int nNewYear = 0, nNewMonth = 0, nNewDay = 0;
	
	// Is new data valid?
	BOOL bVerify = TRUE;

	if ((nChar == VK_BACK || nChar == VK_DELETE) && IsFinished()
		&& !m_bTextNull)
	{
		// The data will be modified, save current data now.
		
		m_PrevData.wYear = year;
		m_PrevData.wMonth = month;
		m_PrevData.wDay = day;
	}
	
	// One part of data will be empty, so there will not be valid data.
	if (nChar == VK_BACK || nChar == VK_DELETE)
		bFinish = FALSE; 
		
	int nNewChar = nChar - '0';

	if (nCurrentPosition >= nYearPosition[0] 
		&& nCurrentPosition <= nYearPosition[1]) 
	{
		// Is is in year part.
		
		// Is the year is negatve?
		BOOL bNegative = FALSE;

		// Only the first char in Mingguo foramt can be '-'.
		if (nChar == '-' && !(m_bFormatMingGuo
			&& nCurrentPosition == nYearPosition[0]))
			return;

		if (!bFinish)
		{
			// Not finish yet.
			
			// Replace the rest chars with '_'.
			for (i = nCurrentPosition; i <= nYearPosition[1]; i++)
				strData.SetAt(i, '_');

			// The length of year part which has been filled.
			nYearFinish = nCurrentPosition - nYearPosition[0];
			
			if (nYearFinish == 0 && nMonthFinish == 0 && nDayFinish == 0)
			{
				// The data is null.
				CalcText("");
				return;
			}
			
			i = 0;
			
			if (text[nYearPosition[0]] == '-')
			{
				// The year is negative.
				bNegative = TRUE;
				i ++;
			}
			
			// Calc year data.
			for (; i < nYearFinish; i++)
				nNewYear = nNewYear * 10 + text[nYearPosition[0] + i] - '0';
			if (bNegative)
			// Year is negative.
				nNewYear = -nNewYear;

			year = nNewYear + (m_bFormatMingGuo ? 1911 : 0);
			
			if (nChar == VK_BACK)
			// Move the caret left.
				MoveLeft(1);
			
			// Update the window text.
			SetWndText(strData);

			return;
		}

		// Curent position is the only empty position in year part.
		
		if (nChar != '/')
		{
			i = 0;
			
			if (text[nYearPosition[0]] == '-' || (nCurrentPosition 
				== nYearPosition[0] && nChar == '-'))
			{
				// Year is negative.
				bNegative = TRUE;
				i ++;
			}
			if (nCurrentPosition == nYearPosition[0] && 
				nChar != '-' && bNegative)
			{
				// Year is not negative.
				bNegative = FALSE;
				i --;
			}
			
			// Calc the year data.
			for (; i < nCurrentPosition - nYearPosition[0]; i++)
				nNewYear = nNewYear * 10 + text[nYearPosition[0] + i] - '0';
			if (!bNegative || nCurrentPosition != nYearPosition[0])
				nNewYear = nNewYear * 10 + nNewChar;
				
			// The finished length increases.
			if (nCurrentPosition - nYearPosition[0] == nYearFinish)
				nYearFinish ++;
				
			if (nYearFinish >= nYearLength)
			{
				// The year part is finished.
				for (i = 1; i <= nYearPosition[1] - nCurrentPosition; i++)
					nNewYear = nNewYear * 10 + text[nCurrentPosition + i] - '0';
			}
			if (bNegative)
			// The year is negative.
				nNewYear = -nNewYear;

			// Validate the new data.
			nNewYear += m_bFormatMingGuo ? 1911 : 0;
			temptime.SetDate(nNewYear, month, day);
			if (temptime.GetStatus() == COleDateTime::invalid && (
				IsFinished() || (nCurrentPosition == nYearPosition[1]
				&& !m_bTextNull)))
			{
				// The new data is invalid.
				bVerify = FALSE;
				if (text[nCurrentPosition] == '_')
				// The finished length should be descreased.
					nYearFinish --;
			}
			else if (nYearLength != 0)
			{
				// The new data is valid.
				int i;
				for (i = 0; i < nYearLength; i++)
					year /= 10;
				for (i = 0; i < nYearLength; i++)
					year *= 10;
				year = nNewYear + (m_bFormatMingGuo ? 0 : year);
			}
		}
		else
		{
			// If the user press '/', I should supplement the space
			// he left, and move to next field.
			
			i = 0;
			
			if (text[nYearPosition[0]] == '-')
			{
				// The year is negative.
				bNegative = TRUE;
				i ++;
			}
			
			// Calc the new year data.
			for (; i < nYearLength && text[i + nYearPosition[0]] != '_' ; i++)
				nNewYear = nNewYear * 10 + text[nYearPosition[0] + i] - '0';
			if (bNegative)
				nNewYear = -nNewYear;
			nNewYear += m_bFormatMingGuo ? 1911 : 0;

			// Validate the new data.
			temptime.SetDate(nNewYear, month, day);
			
			// Now, variable i is the number of space the the user input.
			if (temptime.GetStatus() == COleDateTime::valid)
			{
				// The new data is valid.
				
				// Fill the year data.
				int j;
				for (j = 1; j <= i; j ++)
					strData.SetAt(nYearLength - j + nYearPosition[0], strData[i - j + nYearPosition[0]]);
				
				// We finished the year part.
				nYearFinish = 4;

				j = 0;
				if (bNegative)
				{
					// The year is negative.
					strData.SetAt(nYearPosition[0], '-');
					j = 1;
				}
				
				// The left chars should be filled with '0'.
				for (; j < nYearLength - i; j ++)
					strData.SetAt(j + nYearPosition[0], '0');
				
				// Move the caret to next field.	
				i = nCurrentPosition;
				for (j = 0; j <= nYearPosition[1] - i; j ++)
					MoveRight(1);
			}
		}
	}

	else if (nCurrentPosition >= nMonthPosition[0] 
		&& nCurrentPosition <= nMonthPosition[1]) 
	{
		// It is in month part.
		
		if (!bFinish)
		{
			// We have not finished month part.
			
			// Fill all chars in month part with '_'.
			for (i = nMonthPosition[0]; i <= nMonthPosition[1]; i++)
				strData.SetAt(i, '_');
			nMonthFinish = 0;
			
			// The month is empty, use 1 instead.
			month = 1;
			
			// Move the caret left.
			MoveLeft(nCurrentPosition - nMonthPosition[0]);
			if (nChar == VK_BACK)
				MoveLeft(1);
			
			// Update the window text.
			SetWndText(strData);

			return;
		}

		if (nChar != '/')
		{
			// Calc the new month data.
			
			for (i = 0; i < nCurrentPosition - nMonthPosition[0]; i++)
				nNewMonth = nNewMonth * 10 + text[nMonthPosition[0] + i] - '0';
			nNewMonth = nNewMonth * 10 + nNewChar;
			for (i = 1; i <= nMonthPosition[1] - nCurrentPosition; i++)
			{
				if (text[nCurrentPosition + i] == '_')
					nNewMonth = nNewMonth * 10;
				else
					nNewMonth = nNewMonth * 10 + text[nCurrentPosition + i] - '0';
			}

			// Validate the new month data.
			temptime.SetDate(year, nNewMonth, day);
			if (temptime.GetStatus() == COleDateTime::invalid)
			{
				// The new data is invalid, guess the valid text.
				
				bVerify = FALSE;
				
				//How about use this char as the month data instead?
				temptime.SetDate(year, nChar - '0', day);

				if (temptime.GetStatus() == COleDateTime::valid)
				{
					// That's ok, replace other digits with 0.
					for (i = nMonthPosition[0]; i <= nMonthPosition[1]; i++)
						strData.SetAt(i, '0');

					// If the new month is too big, only keep the lowerst
					// position number.
					nCurrentPosition = i - 1;
					month = nChar - '0';
					nMonthFinish = nMonthLength;
					bVerify = TRUE;
				}
				else if (nChar == '0' && nCurrentPosition == nMonthPosition[0])
				{
					// This is a '0' char in first position, move to the second position.
					bVerify = TRUE;
					nMonthFinish = max(nMonthFinish, nCurrentPosition - nMonthPosition[0] + 1);
				}
			}

			else if (nMonthLength != 0)
			{
				// We may have finished month part.
				
				// Calc new month data.
				int i;
				for (i = 0; i < nMonthLength; i++)
					month /= 10;
				for (i = 0; i < nMonthLength; i++)
					month *= 10;
				month += nNewMonth;
				nMonthFinish = max(nMonthFinish, nCurrentPosition - nMonthPosition[0] + 1);
			}
		}
		else
		{
			// If the user press '/', I should supplement the space
			// he left, and move to next field.
			for (i = 0; i < nMonthLength && text[i + nMonthPosition[0]] != '_' ; i++)
				nNewMonth = nNewMonth * 10 + text[nMonthPosition[0] + i] - '0';
			temptime.SetDate(year, nNewMonth, day);
			
			// Now, i is the number of space the the user input.
			if (temptime.GetStatus() == COleDateTime::valid)
			{
				int j;
				for (j = 1; j <= i; j ++)
					strData.SetAt(nMonthPosition[0] + nMonthLength - j, strData[i - j + nMonthPosition[0]]);
				nMonthFinish = 4;
				for (j = 0; j < nMonthLength - i; j ++)
					strData.SetAt(j + nMonthPosition[0], '0');
				i = nCurrentPosition;
				for (j = 0; j <= nMonthPosition[1] - i; j ++)
					MoveRight(1);

				month = nNewMonth;
			}
		}
	}

	else if (nCurrentPosition >= nDayPosition[0] 
		&& nCurrentPosition <= nDayPosition[1]) 
	{
		// It is in day part.
		if (!bFinish)
		{
			// We have not finished day part yet.
			// Replace all position in day part with '_'.
			for (i = nDayPosition[0]; i <= nDayPosition[1]; i++)
				strData.SetAt(i, '_');
			nDayFinish = 0;
			day = 1;
			
			// Move the caret left.
			MoveLeft(nCurrentPosition - nDayPosition[0]);
			
			if (nChar == VK_BACK)
				MoveLeft(1);
			SetWndText(strData);

			return;
		}

		if (nChar != '/')
		{
			// Calc the new day data.
			
			for (i = 0; i < nCurrentPosition - nDayPosition[0]; i++)
				nNewDay = nNewDay * 10 + text[nDayPosition[0] + i] - '0';
			nNewDay = nNewDay * 10 + nNewChar;
			for (i = 1; i <= nDayPosition[1] - nCurrentPosition; i++)
			{
				if (text[nCurrentPosition + i] == '_')
					nNewDay = nNewDay * 10;
				else
					nNewDay = nNewDay * 10 + text[nCurrentPosition + i] - '0';
			}

			// Validate the new data.
			temptime.SetDate(year, month, nNewDay);
			if (temptime.GetStatus() == COleDateTime::invalid)
			{
				// The new data is invalid. We should guess the correct data now.
				
				bVerify = FALSE;
				
				// Eliminate the lower position number to make it valid.
				temptime.SetDate(year, month, (nChar - '0') * 10);
				if (temptime.GetStatus() == COleDateTime::valid && 
					nCurrentPosition == nDayPosition[0])
				{
					// Ok.
					for (i = nDayPosition[0]; i <= nDayPosition[1]; i++)
						strData.SetAt(i, '0');

					day = (nChar - '0') * 10;
					nCurrentPosition = nDayPosition[0];
					nDayFinish = nDayLength;
					bVerify = TRUE;
				}
				// If the method above does not work, eliminate the lower.
				else
				{
					temptime.SetDate(year, month, nChar - '0');

					if (temptime.GetStatus() == COleDateTime::valid)
					{
						// Ok.
						for (i = nDayPosition[0]; i <= nDayPosition[1]; 
							i++)
							strData.SetAt(i, '0');

						nCurrentPosition = i - 1;
						day = nChar - '0';
						nDayFinish = nDayLength;
						bVerify = TRUE;
					}
					else if (nChar == '0' && nCurrentPosition == nDayPosition[0])
					{
						// This is a '0' at the first position, move to the second position.
						bVerify = TRUE;
						nDayFinish = max(nDayFinish, nCurrentPosition - nDayPosition[0] + 1);
					}
				}
			}
			else if (nDayLength != 0)
			{
				// We have finished the day part.
				
				int i;
				for (i = 0; i < nDayLength; i++)
					day /= 10;
				for (i = 0; i < nDayLength; i++)
					day *= 10;
				day += nNewDay;
				nDayFinish = max(nDayFinish, nCurrentPosition - nDayPosition[0] + 1);
			}
		}
		else
		{
			// If the user press '/', I should supplement the space
			// he left, and move to next field.
			
			for (i = 0; i < nDayLength && text[i + nDayPosition[0]] != '_' ; i++)
				nNewDay = nNewDay * 10 + text[nDayPosition[0] + i] - '0';
			temptime.SetDate(year, month, nNewDay);
			
			// Now, i is the number of space the the user input.
			if (temptime.GetStatus() == COleDateTime::valid)
			{
				int j;
				for (j = 1; j <= i; j ++)
					strData.SetAt(nDayLength - j + nDayPosition[0], strData[i - j + nDayPosition[0]]);
				nDayFinish = 4;
				for (j = 0; j < nDayLength - i; j ++)
					strData.SetAt(j + nDayPosition[0], '0');
				i = nCurrentPosition;
				for (j = 0; j <= nDayPosition[1] - i; j ++)
					MoveRight(1);
				
				day = nNewDay;
			}
		}

	}

	if (bVerify)
	{
		// The new char is valid.
		if (nChar != '/')
		{
			// Update current character
			strData.SetAt(nCurrentPosition, (char)nChar);
			
			// Move the caret right
			MoveRight(1);
			
			// skip the delimiters.
			MoveRight();
		}

		if (IsFinished())
		{
			// We got a new date data.
			m_bTextNull = FALSE;
		}

		// Update the window text.
		SetWndText(strData);
	}
}

void CDateMaskEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Avoid showing caret at the position of a delimiter by clicking mouse button.
	
	CEdit::OnLButtonDown(nFlags, point);

	int nStart, nEnd;
	GetSel(nStart, nEnd);
	if (nEnd < 0)
		nEnd = 0;
	else if(nEnd >= strFormat.GetLength())
		nEnd = strFormat.GetLength() - 1;

	nCurrentPosition = nEnd;
	SetSel(nEnd, nEnd);
	MoveLeft();
}

void CDateMaskEdit::OnPaste()
{
	// calc new text after pasting.
	DefWindowProc(WM_PASTE, 0, 0);

	CalcText();
	MoveRight();
}

void CDateMaskEdit::OnCut()
{
	// calc new text after cutting.
	DefWindowProc(WM_CUT, 0, 0);

	CalcText();
	MoveRight();
}

void CDateMaskEdit::OnClear()
{
	// calc new text after clearing contents.
	DefWindowProc(WM_CLEAR, 0, 0);

	CalcText();
	MoveRight();
}

void CDateMaskEdit::SetPromptInclude(BOOL b)
/*
Routine Description:
	Set if the prompt chars should be included in text.
		
Parameters:
	b		The permission.
	
Return Value:
	None.
*/
{
	m_bPromptInclude = b;
}

BOOL CDateMaskEdit::SetTextIn(CString strNewValue, int nDataType)
/*
Routine Description:
	Calc new text from given text and data type.
		
Parameters:
	strNewValue		The desired new text.
	
	nDataType		The type of underlying data.
	
Return Value:
	None.
*/
{
	CString strData = strNewValue;

	// Reset these variables.
	
	int i;
	char chr;
	CString strYear, strMonth, strDay, strTotal;
	int nNewYear, nNewMonth, nNewDay;
	nNewYear = nNewMonth = nNewDay = 0;
	BOOL bValid = TRUE;
	strYear.Empty();
	strMonth.Empty();
	strDay.Empty();
	strTotal.Empty();
	m_bTextNull = FALSE;

	if (nDataType == DataTypeDate)
	{
		// Use date format string instead.
		COleDateTime date;
		date.ParseDateTime(strNewValue);
		if (date.GetStatus() != COleDateTime::valid)
			return FALSE;

		year = date.GetYear();
		month = date.GetMonth();
		day = date.GetDay();

		return TRUE;
	}

	if (strData.IsEmpty() || strData == '_')
	{
		// The new data is empty.
		
		m_bTextNull = TRUE;
		nYearFinish = nMonthFinish = nDayFinish = 0;
		nCurrentPosition = 0;
	}
	else
	{
		// Fillter invalid chars out.
		for (i = 0; i < strData.GetLength(); i++)
		{
			chr = strData[i];
			if ((chr >= '0' && chr <= '9') || (chr == '-' &&
				strTotal.IsEmpty()))
				strTotal += chr;
			else if (chr == '_')
			{
				bValid = FALSE;
				strTotal.Empty();
				break;
			}
		}

		// The length should be enough.
		if (strTotal.IsEmpty() || (strTotal[0] == '-' && strTotal.
			GetLength() < 6) || (strTotal[0] != '-' && strTotal.
			GetLength() < 5))
			return FALSE;
		
		// Retrieve each part of text.
		strDay = strTotal.Right(2);
		strMonth = strTotal.Mid(strTotal.GetLength() - 4, 2);
		strYear = strTotal.Left(strTotal.GetLength() - 4);
	
		// Is the year negative?
		i = strYear[0] == '-' ? 1 : 0;
		if (i == 1 && !m_bMingGuo)
			return FALSE;

		// Calc new data.
		for (; i < strYear.GetLength(); i++)
			nNewYear = nNewYear * 10 + strYear[i] - '0';
		if (strYear[0] == '-')
			nNewYear = -nNewYear;
		for (i = 0; i < 2; i++)
			nNewMonth = nNewMonth * 10 + strMonth[i] - '0';
		for (i = 0; i < 2; i++)
			nNewDay = nNewDay * 10 + strDay[i] - '0';

		// Verify new data.
		COleDateTime date;
		date.SetDate(nNewYear + ((m_bMingGuo || m_bFormatMingGuo)? 
			1911 : 0), nNewMonth, nNewDay);
		if (date.GetStatus() == COleDateTime::valid)
		// The new data is valid.
			bValid = TRUE;
		else
		// The new dta is invalid.
			return FALSE;
	}

	if (bValid)
	{
		if (!m_bTextNull)
		{
			// The new data is not null.
			year = nNewYear;
			month = nNewMonth;
			day = nNewDay;
		}
		else
		{
			// The new data is null.
			m_bTextNull = TRUE;
			month = day = 1;
		}
	}

	return TRUE;
}

void CDateMaskEdit::SetWndText(CString str)
/*
Routine Description:
	Update the window text and keep the caret position from changing.
		
Parameters:
	str		The new window string.
	
Return Value:
	None.
*/
{
	// Get the current caret position.
	int nStart, nEnd;
	GetSel(nStart, nEnd);

	// Update window text, and the caret will move to home.
	SetWindowText(str);
	
	// Restore the caret position.
	SetSel(nStart);
	
	// Skip the delimiters.
	MoveRight();
}

UINT CDateMaskEdit::OnGetDlgCode() 
{
	return DLGC_WANTALLKEYS;
}
