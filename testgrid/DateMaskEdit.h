#if !defined(AFX_DATEMASKEDIT_H__352724A2_669E_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_DATEMASKEDIT_H__352724A2_669E_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DateMaskEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDateMaskEdit window

class CDateMaskEdit : public CEdit
{
// Construction
public:
	CDateMaskEdit();

// Attributes
public:

protected:
	// Decides if the prompt chars will be included in window text.
	BOOL m_bPromptInclude;
	
	// Decides if we have a valid date data now.
	BOOL m_bTextNull;
	
	// Decides if it is freely Mingguo format or we should show Mingguo format date.
	BOOL m_bMingGuo, m_bFormatMingGuo;
	
	// Format used to input data.
	CString strFormat;
	
	// The delimiter groups at 4 possible positions.
	CString strDelimiter[4];

	// The actually data and their length.
	int year, month, day, nYearLength, nMonthLength, nDayLength;
	
	// The start and end position of 3 part of data.
	int nYearPosition[2], nMonthPosition[2], nDayPosition[2];
	
	// The amount of delimiers.
	int nDelimiters;
	
	// The start and end posiiton of 4 possible groups of delimiters.
	int nDelimiterPosition[8];
	
	// The position of the focus.
	int nCurrentPosition;
	
	// The length of the finished chars in 3 parts.
	int nYearFinish, nMonthFinish, nDayFinish;
	
	// The previous valid data.
	SYSTEMTIME m_PrevData;
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDateMaskEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDateMaskEdit();

	// Generated message map functions
protected:
	// Update window text.
	void SetWndText(CString str);
	
	// Helper function to modify data.
	BOOL SetTextIn(CString strNewValue, int nDataType);
	
	// Set the value of m_bPromptInclude.
	void SetPromptInclude(BOOL b);
	
	// Modify the date data undelying.
	void SetDate(short nYear, short nMonth, short nDay);
	
	// Calc the correct text according given text.
	void CalcText(LPCTSTR strNewText = NULL);
	
	// Set the mask.
	void SetMask(CString strMask);
	
	// Move the caret left, skips the delimiters.
	void MoveLeft(int nSteps = 0);
	
	// Move the caret right, skips the delimiters.
	void MoveRight(int nSteps = 0);
	
	// Decides if we have a valid date data.
	BOOL IsFinished();
	
	// Decides if the given format string is valid.
	BOOL IsValidFormat(CString strNewFormat);
	
	// Calc the data in Mingguo format.
	void CalcMingGuo();
	
	// Calc the data from current text.
	void CalcData();
	
	// Calc the result text from given text.
	void CalcDisplayText(CString & strText, BOOL bPromptInclude = TRUE, int nDataType = DataTypeText);
	//{{AFX_MSG(CDateMaskEdit)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg	void OnPaste();
	afx_msg	void OnCut();
	afx_msg void OnClear();
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATEMASKEDIT_H__352724A2_669E_11D3_A7FE_0080C8763FA4__INCLUDED_)
