#if !defined(AFX_MASKEDIT_H__813CD3A1_6507_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_MASKEDIT_H__813CD3A1_6507_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MaskEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMaskEdit window

class CMaskEdit : public CEdit
{
// Construction
public:
	CMaskEdit();

// Attributes
public:

protected:
	// Input mask.
	CString m_strMask;

	// Mask length.
	int m_nMaxLength;

	// Prompt char.
	CString m_strPromptChar;

	// If include delimiters into text.
	BOOL m_bPromptInclude;

	// Processed input mask.
	CString m_strInputMask;

	// Delimiter status array.
	BOOL m_bIsDelimiter[255];
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaskEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMaskEdit();

	// Generated message map functions
protected:
	// Set prompt char.
	void SetPromptChar(CString strChar);

	// Set mask.
	void SetMask(CString strMask);

	// Set max lengtht.
	void SetMaxLength(int nNewValue);

	// Set prompt include.
	void SetPromptInclude(BOOL bNewValue);

	// Calc display text.
	void CalcDisplayText(CString & strText, BOOL bPromptInclude = TRUE, int nDataType = DataTypeText);

	// Calc text.
	void CalcText(LPCTSTR strNewText = NULL);
	
	// Move caret left, skip delimiters.
	void MoveCaretLeft();

	// Move caret right, skip delimiters.
	void MoveCaretRight();

	// Skip delimiters.
	void MoveRight();

	// Decides if given string is a valid mask.
	BOOL IsValidMask(CString strMask);

	// Decides if given char at given position is valid.
	BOOL IsValidChar(TCHAR nChar, int nPosition);

	// Process input mask.
	void CalcMask();

	//{{AFX_MSG(CMaskEdit)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg	void OnPaste();
	afx_msg	void OnCut();
	afx_msg void OnClear();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASKEDIT_H__813CD3A1_6507_11D3_A7FE_0080C8763FA4__INCLUDED_)
