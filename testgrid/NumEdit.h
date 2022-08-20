#if !defined(AFX_NUMEDIT_H__B559E9E0_767B_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_NUMEDIT_H__B559E9E0_767B_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumEdit window

class CNumEdit : public CEdit
{
// Construction
public:
	CNumEdit();

// Attributes
public:

protected:
	// Length of two part of values.
	int m_nIntegralDigits, m_nDecimalDigits;

	// Text length of two part.
	int m_nIntegralLen, m_nDecimalLen;

	// The text format value of two part.
	CString m_strIntegralValue, m_strDecimalValue;

	// The current value.
	double m_fValue;

	// Should we include the prompt char in text?
	BOOL m_bPromptInclude;

protected:
	// Update window text and preserve caret position.
	void SetWndText(CString str);

	// Decides if one char is valid.
	BOOL IsValidChar(TCHAR nChar, int nPosition);

	// Avoid showing caret at not-digital position.
	void MoveRight();

	// Move caret right within digital text.
	void MoveCaretRight();

	// Move caret left within digital text.
	void MoveCaretLeft();

	// Calc window text from given text.
	void CalcText(LPCTSTR strNewText = NULL);

	// Calc display text from given text.
	void CalcDisplayText(CString & strText, BOOL bPromptInclude = TRUE);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetPromptInclude(BOOL b);
	virtual ~CNumEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNumEdit)
	afx_msg	void OnPaste();
	afx_msg	void OnCut();
	afx_msg void OnClear();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMEDIT_H__B559E9E0_767B_11D3_A7FE_0080C8763FA4__INCLUDED_)
