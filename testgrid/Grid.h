#if !defined(AFX_GRID_H__0E4822E0_558B_11D3_A7FE_0080C8763FA4__INCLUDED_)
#define AFX_GRID_H__0E4822E0_558B_11D3_A7FE_0080C8763FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Grid.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGrid window

// Cell states
#define GVIS_FOCUSED            0x0001
#define GVIS_SELECTED           0x0002
#define GVIS_DROPHILITED        0x0004
#define GVIS_READONLY           0x0008
#define GVIS_FIXED              0x0010  // not yet used
#define GVIS_MODIFIED           0x0020

// Cell Searching options
#define GVNI_FOCUSED            0x0001
#define GVNI_SELECTED           0x0002
#define GVNI_DROPHILITED        0x0004
#define GVNI_READONLY           0x0008
#define GVNI_FIXED              0x0010  // not yet used
#define GVNI_MODIFIED           0x0020

#define GVNI_ABOVE              LVNI_ABOVE
#define GVNI_BELOW              LVNI_BELOW
#define GVNI_TOLEFT             LVNI_TOLEFT
#define GVNI_TORIGHT            LVNI_TORIGHT
#define GVNI_ALL                (LVNI_BELOW|LVNI_TORIGHT|LVNI_TOLEFT)
#define GVNI_AREA               (LVNI_BELOW|LVNI_TORIGHT)

// Selection mode
#define SELECTMODE_NONE 0
#define SELECTMODE_ROW 1
#define SELECTMODE_COL 2
#define SELECTMODE_GROUP 3

// Column style
#define COLSTYLE_EDIT 0
#define COLSTYLE_COMBOBOX 1
#define COLSTYLE_COMBOBOX_DROPLIST 2
#define COLSTYLE_DATEMASK 3
#define COLSTYLE_CHECKBOX 4
#define COLSTYLE_EDITBUTTON 5
#define COLSTYLE_BUTTON 6
#define COLSTYLE_NUM 7

// Mouse tracking mode
#define TRACKCOLWIDTH 0
#define TRACKGROUPWIDTH 1
#define TRACKROWHEIGHT 2

// Get shift keys state
#define IsSHIFTPressed() ((GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT) * 8 - 1))) != 0)
#define IsCTRLPressed()  ((GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT) * 8 - 1))) != 0)

// Column object.
struct Col
{
	// The width.
	int nWidth;
	
	// The visibility.
	BOOL bVisible;
	
	// The style.
	int nStyle;
	
	// The choice list used in combo box style column.
	CString strChoiceList;
	
	// Decides if the prompt chars should be included in window text if has input mask.
	BOOL bPromptInclude;
	
	// The inupt mask and prompt char.
	CString strMask, strPromptChar;
	
	// The max length of window text.
	int nMaxLength;
	
	// The colors.
	COLORREF clrFore, clrBack, clrHeaderFore, clrHeaderBack;
	
	// Decides if this column is readonly.
	BOOL bReadOnly;
	
	// The case in showing text.
	int nCase;
	
	// The alignment of text and header.
	int nAlignment, nHeaderAlignment;
	
	// The underlying data type.
	int nDataType;
	
	// Decides if the user can resize column width use mouse.
	BOOL bAllowSizing;
	
	// The unique index of this column.
	int nColIndex;
	
	// Decide if the text in this column has been changed.
	BOOL bDirty;
	
	// The original text before modification.
	CString strSaved;
	
	// The column title.
	CString strTitle;
	
	// User defined attributes array.
	CStringArray arUserAttrib;
	
	// The name of this column.
	CString strName;

	Col()
	{
		bVisible = TRUE;
		nStyle = 0;
		nMaxLength = 255;
		clrFore = clrBack = clrHeaderFore = clrHeaderBack = DEFAULTCOLOR;
		bReadOnly = FALSE;
		nCase = CASE_NONE;
		nAlignment = nHeaderAlignment = -1;
		nDataType = DataTypeText;
		strPromptChar = _T("_");
		bAllowSizing = TRUE;
		bDirty = FALSE;
	}
};

// The level object in each group.
struct Level
{
	// The columns in this level.
	int nCols;
	
	// The visible columns in this level.
	int nColsVisible;

	Level()
	{
		nCols = nColsVisible = 0;
	}
};

// The group object.
struct Group
{
	// The array of levels in this object.
	CArray<Level *, Level *> arLevels;
	
	// The width.
	int nWidth;
	
	// The column count in this group.
	int nCols;
	
	// The group title.
	CString strTitle;
	
	// The visibility.
	BOOL bVisible;
	
	// The colors.
	COLORREF clrFore, clrBack;
	
	// The unique index of this group.
	int nGroupIndex;
	
	// Decides if the user can resize this group by mouse.
	BOOL bAllowSizing;
	
	// The name of this group.
	CString strName;

	Group()
	{
		nCols = 0;
		bVisible = TRUE;
		clrFore = clrBack = DEFAULTCOLOR;
		bAllowSizing = TRUE;
	}

	~Group()
	{
		// Clear the level array.
		for (int i = 0; i < arLevels.GetSize(); i ++)
			delete arLevels[i];

		arLevels.RemoveAll();
	}
};

// The cell object.
struct Cell
{
	// The text being shown.
	CString strText;
	
	// The state of this cell.
	UINT nState;

	Cell()
	{
		nState = 0;
	}
};

// The style of one cell.
struct CellStyle
{
	// The text being shown.
	CString strText;
	
	// The rect this cell occupies.
	CRect rect;
	
	// The choice list of combo box.
	CString strChoiceList;
	
	// Decides if the prompt chars should be include in text if has input mask.
	BOOL bPromptInclude;
	
	// The input mask and prompt char.
	CString strMask, strPromptChar;
	
	// The max length of the text.
	int nMaxLength;
	
	// The colors.
	COLORREF clrFore, clrBack;
	
	// The state.
	int nState;
	
	// The case when showing text.
	int nCase;
	
	// The text alignment.
	int nAlignment;
	
	// The type of the underlying data.
	int nDataType;
	
	// The row and col ordinal of this cell.
	int nRow, nCol;

	CellStyle()
	{
		bPromptInclude = TRUE;
		nMaxLength = 255;
		nState = 0;
		nAlignment = TA_CENTER;
		nDataType = DataTypeText;
		strPromptChar = _T("_");
	}
};

class CGridEditCtrl;
class CGridComboCtrl;
class CGridControl;
class CGridDateMaskCtrl;
class CGridCheckBoxCtrl;
class CGridEditBtnCtrl;
class CGridNumCtrl;

typedef	CArray<Group *, Group *> CGroupArray;
typedef CArray<Cell *, Cell *> CCellArray;
typedef CArray<Col *, Col *> CColArray;


// The grid class.
class CGrid : public CWnd
{
// Construction
public:
	CGrid();

	// The direction in navigation.
	enum
	{
		NV_LEFT = 0,
		NV_RIGHT,
		NV_UP,
		NV_DOWN,
		NV_PRIOR,
		NV_NEXT,
		NV_HOME,
		NV_END,
		NV_TAB
	};

	// The type of divider lines.
	enum
	{
        	DividerTypeNone = 0,
	        DividerTypeVertical,
        	DividerTypeHorizontal,
	        DividerTypeBoth
	};

// Attributes
public:

protected:
	// The array of groups.
	CGroupArray m_arGroups;
	
	// The array of cells.
	CCellArray m_arCells;
	
	// The array of cols.
	CColArray m_arCols;
	
	// The array of the ordinals of the selected rows/cols/gropus.
	CArray<int, int> m_arSelection;
	
	// The array of the unique indexes of row.
	CArray<int, int> m_arRowIndex;

	// The mode of current selection.
	int m_nSelectMode;

	// Decides if this grid is readonly.
	BOOL m_bReadOnly;
	
	// Decides if this grid will likes like a list box.
	BOOL m_bListMode;
	
	// Decides if this grid allow inserting or deleting row manually.
	BOOL m_bAllowAddNew, m_bAllowDelete;
	
	// The alignment of the text and header.
	int m_nAlignment, m_nHeaderAlignment;
	
	// The margin between cell border and the text.
	int m_nMargin, m_nHeaderMargin;
	
	// Decides if there is pending new row.
	BOOL m_bAddNewMode;

	// Decides if the grid can update itself.
	BOOL m_bAllowRedraw;

	// The style and type of divider lines.
	int m_nDividerType, m_nDividerStyle;
	
	// The color of divider lines.
	COLORREF m_clrDivider;

	// The count of rows/cols/levels/groups.
	int m_nRows, m_nCols, m_nLevels, m_nGroups;
	
	// The rect to draw the grid.
	CRect m_rtGrid;

	// The list of images in row header.
	CImageList m_ImageList;

	// The count of groups/cols/rows had been created.
	int m_nGroupsUsed, m_nColsUsed, m_nRowsUsed;
	
	// The max range of the vertical/horizontal scroll bar.
	int m_nVScrollMax, m_nHScrollMax;
	
	// The height of each row and header.
	int m_nRowHeight, m_nHeaderHeight;
	
	// The height of each level or header level.
	int m_nLevelHeight, m_nHeaderLevelHeight;
	
	// The count of frozen cols/rows/groups.
	int m_nFrozenCols, m_nFrozenRows, m_nFrozenGroups;
	
	// The default col width in creating new column.
	int m_nDefColWidth;

	// The mouse mode.
    	int m_nMouseMode;
    	
    	// The last point the left mouse button was clicke down.
	CPoint m_LeftClickDownPoint;
	
	// The last point of the cursor.
	CPoint m_LastMousePoint;
	
	// The last row/col/groups the left mouse button was clicked.
	int m_nLeftClickDownRow, m_nLeftClickDownCol, m_nLeftClickDownGroup;

	// Modes of the mouse.
	enum eMouseModes
	{
		MOUSE_NOTHING,
		MOUSE_SELECT_COL,
		MOUSE_SELECT_ROW,
		MOUSE_SCROLLING_CELLS,
		MOUSE_OVER_ROW_DIVIDE,
		MOUSE_SIZING_ROW,
		MOUSE_OVER_COL_DIVIDE,
		MOUSE_SIZING_COL,
		MOUSE_OVER_GROUP_DIVIDE,
		MOUSE_SIZING_GROUP,
		MOUSE_PREPARE_EDIT,
		MOUSE_PREPARE_DRAG,
		MOUSE_DRAGGING
	};

	// Decides if we are already in the OnSize() procedure.
	BOOL m_bAlreadyInsideOnSize;
	
	// The permission to resize row height/col width/group width manually.
	BOOL m_bAllowColResize, m_bAllowRowResize, m_bAllowGroupResize;
	
	// The permission to show row header/group header/col header manually.
	BOOL m_bShowRowHeader, m_bShowGroupHeader, m_bShowColHeader;
	
	// The permission to move col/group manually.
	BOOL m_bAllowMoveCol, m_bAllowMoveGroup;
	
	// The permission to selete row.
	BOOL m_bAllowSelectRow;

	// The ordinal of current row/col.
	int m_nRow, m_nCol;

	// The ID of the timer.
	UINT m_nTimerID;

	// The font objects of cell and header.
	CFont m_fntCell, m_fntHeader;

	// Decides if we are inplace editing.
	BOOL m_bIPEdit;
	
	// Decides if current row is dirty.
	BOOL m_bRowDirty;

	// The pointer of varias style of columns object.
	CGridEditCtrl * m_pEditCtrl;
	CGridComboCtrl * m_pComboCtrl, * m_pComboListCtrl;
	CGridDateMaskCtrl * m_pDateCtrl;
	CGridCheckBoxCtrl * m_pCheckBoxCtrl;
	CGridEditBtnCtrl * m_pEditBtnCtrl, * m_pButtonCtrl;
	CGridNumCtrl * m_pNumCtrl;

	// The colors.
	COLORREF m_clrFore, m_clrBack, m_clrHeaderFore, m_clrHeaderBack;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGrid)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	// Get the header height of the total headers area.
	int GetVirtualHeaderHeight();
	
	// Get the height of each level.
	int GetLevelHeight();
	
	// Get the choice list of one col.
	CString GetColChoiceList(int nCol);
	
	// Get the prompt include property of one col.
	BOOL GetColPromptInclude(int nCol);
	
	// Get the prompt char of one col.
	CString GetColPromptChar(int nCol);
	
	// Get the input mask of one col.
	CString GetColMask(int nCol);
	
	// Get the header alignment of one col.
	int GetColHeaderAlignment(int nCol);
	
	// Get the style of one col.
	int GetColControl(int nCol);
	
	// Get the underlying data type of one col.
	int GetColDataType(int nCol);
	
	// Decides if one col is readonly.
	BOOL GetColReadOnly(int nCol);
	
	// Get the header back color of one col.
	COLORREF GetColHeaderBackColor(int nCol);
	
	// Get the header fore color of one col.
	COLORREF GetColHeaderForeColor(int nCol);
	
	// Get the permission to resizing the width of one col.
	BOOL GetColAllowSizing(int nCol);
	
	// Get the max length of text in one col.
	int GetColMaxLength(int nCol);
	
	// Get the text case in one col.
	int GetColCase(int nCol);
	
	// Get the back color in one col.
	COLORREF GetColBackColor(int nCol);
	
	// Get the fore color of one col.
	COLORREF GetColForeColor(int nCol);
	
	// Get the title of one col.
	CString GetColTitle(int nCol);
	
	// Get the fore color.
	COLORREF GetForeColor();
	
	// Decides if the given group name is unique.
	BOOL IsGroupNameUnique(CString strName);
	
	// Decides if the given col name is unique.
	BOOL IsColNameUnique(CString strName);
	
	// Calc the group ordinal from group name.
	int GetGroupFromName(CString strName);
	
	// Set the name of one group.
	BOOL SetGroupName(int nGroup, CString strName);
	
	// Get the group name.
	CString GetGroupName(int nGroup);
	
	// Calc the col ordinal from name.
	int GetColFromName(CString strName);
	
	// Set the name of one col.
	BOOL SetColName(int nCol, CString strName);
	
	// Get the name of one col.
	CString GetColName(int nCol);
	
	// Get the permission to resizing one group.
	BOOL GetGroupAllowSizing(int nGroup);
	
	// Get the fore color of one group.
	COLORREF GetGroupForeColor(int nGroup);
	
	// Get the back color of one group.
	COLORREF GetGroupBackColor(int nGroup);
	
	// Set the title of one group.
	void SetGroupTitle(int nGroup, CString strTitle);
	
	// Get the title of one group.
	CString GetGroupTitle(int nGroup);
	
	// Get the col count of one group.
	int GetGroupColCount(int nGroup);
	
	// Get a copy of the props of one col.
	void CloneColProp(int nCol, Col * pColNew);
	
	// Get the col alignment.
	int GetColAlignment(int nCol);
	
	// Get the user defined attributes of one col.
	CStringArray & GetColUserAttribRef(int nCol);
	
	// Set the col title.
	void SetColTitle(int nCol, CString strText);
	
	// Copy all properties in one group into the other.
	static void CopyGroup(Group * pGroup, Group * pGroupNew);
	
	// Copy all properties in one col into the other.
	static void CopyCol(Col * pCol, Col * pColNew);
	
	// Load saved layout.
	void LoadLayout(CGroupArray * pGroups, CColArray * pCols, CCellArray * pCells);
	
	// Save current layout.
	void SaveLayout(CGroupArray * pGroups, CColArray * pCols, CCellArray * pCells);
	
	// The text in one cell will be drawn.
	virtual void OnDrawCellText(CDC * pDC, CellStyle * pStyle);
	
	// Ensure one cell is visible.
	void EnsureVisible(int nRowDest, int nColDest);
	
	// Set the current row.
	BOOL MoveTo(int nRow);
	
	// Init the grid.
	void Init();
	
	// Set the prompt char of one col.
	void SetColPromptChar(int nCol, CString strChar);
	
	// Get the header alignment.
	int GetHeaderAlignment();
	
	// Get the alignment.
	int GetAlignment();
	
	// Decides if the given ordinal if that of current row.
	BOOL IsCurrentCell(int nRow, int nCol);
	
	// Decides if current row is dirty.
	BOOL IsRecordDirty();
	
	// Decides if current row is pending new row.
	BOOL IsAddRow();
	
	// Create grid window.
	void Create(DWORD style, const RECT& rect, CWnd * pParent, UINT id);
	
	// Get the record count.
	int OnGetRecordCount();
	
	// Can the grid redraw itself?
	BOOL IsLockUpdate();
	
	// Set the permission to redraw the grid.
	void LockUpdate(BOOL b = TRUE);
	
	// Set the permission to delete row manually.
	void SetAllowDelete(BOOL b);
	
	// Get the permission to delete row manually.
	BOOL GetAllowDelete();
	
	// Set the permission to insert new row manually.
	void SetAllowAddNew(BOOL b);
	
	// Get the permission to insert new row manually.
	BOOL GetAllowAddNew();
	
	// Make the specified the top visible row.
	void SetTopRow(int nRow);
	
	// Get the ordinal of the top visible row.
	int GetTopRow();
	
	// Make the specified group the first left visible group.
	void SetLeftGroup(int nGroup);
	
	// Make the specified col the first left visible col.
	void SetLeftCol(int nCol);
	
	// Get the ordinal of the left visible group.
	int GetLeftGroup();
	
	// Get the ordinal of the left visible col.
	int GetLeftCol();
	
	// Get the count of all visible groups.
	int GetVisibleGroups();
	
	// Get the count of all visible rows.
	int GetVisibleRows();
	
	// Get the count of all visible cols.
	int GetVisibleCols();
	
	// The grid will be redrawn.
	void OnGridDraw(CDC * pDC);
	
	// Set the rect to draw the grid.
	void SetGridRect(RECT * pRect);
	
	// One cell was clicked by the left mouse button.
	virtual void OnClickedCellButton(int nRow, int nCol);
	
	// Decides if one col is dirty.
	BOOL IsColDirty(int nCol);
	
	// Set if the grid acts like a list box.
	void SetListMode(BOOL b);
	
	// Set the header back color.
	void SetHeaderBackColor(COLORREF clr);
	
	// Set the header fore color.
	void SetHeaderForeColor(COLORREF clr);
	
	// Get the header back color.
	COLORREF GetHeaderBackColor();
	
	// Get the header fore color.
	COLORREF GetHeaderForeColor();
	
	// Get the back color.
	COLORREF GetBackColor();
	
	// Get the group ordinal from group index.
	int GetGroupFromIndex(int nIndex);
	
	// Get the row ordinal from row index.
	int GetRowFromIndex(int nIndex);
	
	// Get the col ordinal from col index.
	int GetColFromIndex(int nIndex);
	
	// Cancel all pending modification in current row.
	virtual void CancelRecord();
	
	// Set the default col width.
	void SetDefColWidth(int nWidth);
	
	// Set the divider color.
	void SetDividerColor(COLORREF clr);
	
	// Set the divider style.
	void SetDividerStyle(int nStyle);
	
	// Set the divider type.
	void SetDividerType(int nType);
	
	// Set the permission to resize group width manually.
	void SetGroupAllowSizing(int nGroup, BOOL b);
	
	// Set the permission to resize col width manually.
	void SetColAllowSizing(int nCol, BOOL b);
	
	// Get the index of one group.
	int GetGroupIndex(int nGroup);
	
	// Get the index of one col.
	int GetColIndex(int nCol);
	
	// Get the index of one row.
	int GetRowIndex(int nRow);
	
	// Get the underlying data type of one col.
	void SetColDataType(int nCol, int nType);
	
	// Set the header alignment of one col.
	void SetColHeaderAlignment(int nCol, int nAlign);
	
	// Set the alignment of one col.
	void SetColAlignment(int nCol, int nAlign);
	
	// Set the header alignment.
	void SetHeaderAlignment(int nAlign);
	
	// Set the alignment.
	void SetAlignment(int nAlign);
	
	// Set the text case of one col.
	void SetColCase(int nCol, int nCase);
	
	// Set the permission for modification of one col.
	void SetColReadOnly(int nCol, BOOL b);
	
	// Set the permission for modification.
	void SetReadOnly(BOOL b);
	
	// Decides if grid is readonly.
	BOOL GetReadOnly();
	
	// Set the header back color of one col.
	void SetColHeaderBackColor(int nCol, COLORREF clr);
	
	// Set the header fore color of one col.
	void SetColHeaderForeColor(int nCol, COLORREF clr);
	
	// Set the fore color of one col.
	void SetColForeColor(int nCol, COLORREF clr);
	
	// Set the back color of one col.
	void SetColBackColor(int nCol, COLORREF clr);
	
	// Set the fore color of one group.
	void SetGroupForeColor(int nGroup, COLORREF clr);
	
	// Set the back color of one group.
	void SetGroupBackColor(int nGroup, COLORREF clr);
	
	// Set the input mask of one col.
	void SetColMask(int nCol, CString strMask);
	
	// Set the max text length of one col.
	void SetColMaxLength(int nCol, int nLength);
	
	// Set the back color.
	void SetBackColor(COLORREF clr);
	
	// Set the fore color.
	void SetForeColor(COLORREF clrFore);
	
	// Set the permission to include prompt chars in text.
	void SetColPromptInclude(int nCol, BOOL b);
	
	// Set the choice list of one col.
	void SetColChoiceList(int nCol, CString strList);
	
	// Set the style of one col.
	void SetColControl(int nCol, int nStyle);
	
	// Set the text in one cell.
	BOOL SetCellText(int nRow, int nCol, CString strText);
	
	// Set the header font.
	void SetHeaderFont(LOGFONT *pLogFont);
	
	// Set the cell font.
	void SetCellFont(LOGFONT *pLogFont);
	
	// Set the permission to select rows.
	void SetAllowSelectRow(BOOL b);
	
	// Get the permission to select rows.
	BOOL GetAllowSelectRow();
	
	// Get the array of selected rows.
	int GetSelectedRows(CArray<int, int> & arRows);
	
	// Set the permission to move group.
	void SetAllowMoveGroup(BOOL b);
	
	// Set the permission to move group.
	BOOL GetAllowMoveGroup();
	
	// Set the permission to move col.
	void SetAllowMoveCol(BOOL b);
	
	// Get the permission to move col.
	BOOL GetAllowMoveCol();
	
	// Set the permission to resize group width manually.
	void SetAllowGroupResize(BOOL b);
	
	// Get the permissio to resize group width manually.
	BOOL GetAllowGroupResize();
	
	// Set the permission to resize row height manually.
	void SetAllowRowResize(BOOL b);
	
	// Get the permission to resize row height manually.
	BOOL GetAllowRowResize();
	
	// Set the permission to resize col width manually.
	void SetAllowColResize(BOOL b);
	
	// Get the permission to resize col width manually.
	BOOL GetAllowColResize();
	
	// Set the permission to show group header.
	void SetShowGroupHeader(BOOL b);
	
	// Get the permission to show group header.
	BOOL GetShowGroupHeader();
	
	// Set the permission to show col header.
	void SetShowColHeader(BOOL b);
	
	// Get the permission to show col header.
	BOOL GetShowColHeader();
	
	// Set the permission to show row header.
	void SetShowRowHeader(BOOL b);
	
	// Get the permission to show row header.
	BOOL GetShowRowHeader();
	
	// Decides if given group is selected.
	int IsGroupSelected(int nGroup);
	
	// Decides if given col is selected.
	int IsColSelected(int nCol);
	
	// Decides if given row is selected.
	int IsRowSelected(int nRow);
	
	// cancel all selection.
	void ResetSelection();
	
	// Select one row or not.
	void SelectRow(int nRow, BOOL bSelect = TRUE);
	
	// Set the count of frozen cols.
	BOOL SetFrozenCols(int nCols);
	
	// Get the count of frozen cols.
	int GetFrozenCols();
	
	// Navigate between cells.
	virtual int Navigate(int nStartRow, int nStartCol, int nDirection);
	
	// Set the state of one cell.
	BOOL SetCellState(int nRow, int nCol, UINT state);
	
	// Make one cell the current cell.
	virtual BOOL SetCurrentCell(int nRow, int nCol);
	
	// Get the current cell.
	void GetCurrentCell(int & nRow, int & nCol);
	
	// Make one col visible or not.
	BOOL SetColVisible(int nCol, int b);
	
	// Make one group visible or not.
	BOOL SetGroupVisible(int nGroup, BOOL b);
	
	// Get the visibility of one group.
	BOOL GetGroupVisible(int nGroup);
	
	// Get the visibility of one col.
	BOOL GetColVisible(int nCol);
	
	// Set the count of frozen groups.
	BOOL SetFrozenGroups(int nGroups);
	
	// Get the count of frozen groups.
	int GetFrozenGroups();
	
	// Set the count of frozen rows.
	BOOL SetFrozenRows(int nRows);
	
	// Get the count of frozen rows.
	int GetFrozenRows();
	
	// Set the height of header row.
	BOOL SetHeaderHeight(int nHeight);
	
	// Set the height of each row.
	BOOL SetRowHeight(int nHeight);
	
	// Set the width of one col.
	BOOL SetColWidth(int nCol, int nWidth);
	
	// Get the width of one col.
	int GetColWidth(int nCol);
	
	// Get the width of one group.
	int GetGroupWidth(int nGroup);
	
	// Set the width of one group.
	BOOL SetGroupWidth(int nGroup, int nWidth);
	
	// Get the rect one group occupies.
	BOOL GetGroupRect(int nRow, int nGroup, CRect& rtGroup);
	
	// Update the scroll bars.
	void ResetScrollBars();
	
	// Get the count of levels.
	int GetLevelCount();
	
	// Get the count of groupss
	int GetGroupCount();
	
	// Get the count of cols.
	int GetColCount();
	
	// Get the count of rows.
	int GetRowCount();
	
	// Set the count of rows.
	BOOL SetRowCount(int nCount);
	
	// Set the count of cols.
	BOOL SetColCount(int nCount);
	
	// Set the count of groups.
	BOOL SetGroupCount(int nCount);
	
	// Remove one row.
	BOOL RemoveRow(int nRowIndex = 0);
	
	// Insert one row.
	BOOL InsertRow(int nRowIndex = 0);
	
	// Set the count of levels.
	BOOL SetLevelCount(int nCount);
	
	// Insert one col into one group
	BOOL InsertColInGroup(int nGroup, int nColIndex = 0);
	
	// Remove one col.
	BOOL RemoveCol(int nColIndex = 0);
	
	// Insert one col.
	BOOL InsertCol(int nColIndex = 0);
	
	// Remove one group.
	BOOL RemoveGroup(int nGroupIndex = 0);
	
	// Insert one group.
	BOOL InsertGroup(int nGroupIndex = 0);
	
	// Redraw one cell.
	BOOL RedrawCell(int nRow, int nCol, CDC * pDC = NULL);
	
	// Redraw one row.
	BOOL RedrawRow(int nRow, CDC * pDC = NULL);
	
	// Redraw one col.
	void RedrawCol(int nCol);
	
	virtual ~CGrid();
	
	// Save pending modification in current row.
	virtual BOOL FlushRecord();
	
	// Get the cell of one cell.
	CString GetCellText(int nRow, int nCol);
	
	// Select one group or not.
	void SelectGroup(int nGroup, BOOL bSelect = TRUE);
	
	// Select one col or not.
	void SelectCol(int nCol, BOOL bSelect = TRUE);
	
	// Get the height of one row.
	int GetRowHeight(int nRow);
	
	// Get the rect one cell occupies.
	BOOL GetCellRect(int nCellRow, int nCellCol, CRect& rtCell);
	
	// Get the group ordinal one col belongs to.
	int GetGroupFromCol(int & nCol);
	
	// Get the global col ordinal from the ordinal in one group.
	int GetColFromGroup(int nGroup, int nCol);
	
	// Get the height of total grid.
	int GetVirtualHeight();
	
	// Get the width of total grid.
	int GetVirtualWidth();
	
	// Get the height of fixed rows.
	int GetFixedRowHeight();
	
	// Get the width of fixed cols.
	int GetFixedColWidth();

	// Generated message map functions
protected:
	// Save the new header height.
	virtual void StoreHeaderHeight(int nHeight);
	
	// Save the new row height.
	virtual void StoreRowHeight(int nHeight);
	
	// Find a unique name.
	CString FindUniqueName(CString & strRecm, int nOrdinal);
	
	// Draw the background of one cell.
	void DrawCellBackGround(CDC * pDC, CellStyle * pStyle);
	
	// Delete one record from underlying data.
	virtual BOOL DeleteRecord(int nRow);
	
	// One cell text will be loaded.
	virtual void OnLoadCellText(int nRow, int nCol, CString & strText);
	
	// Add one new record to underlying data.
	virtual BOOL AddNew();
	
	// Redraw one group.
	void RedrawGroup(int nGroup);
	
	// Redraw one group in one row.
	BOOL RedrawGroup(int nRow, int nGroup, CDC * pDC);
	
	// Save the new width of one col.
	virtual void StoreColWidth(int nCol, int nWidth);
	
	// Save the new width of one group.
	virtual void StoreGroupWidth(int nGroup, int nWidth);
	
	// Mouse tracking will begin.
	virtual BOOL OnStartTracking(int nRow, int nCol, int nTrackingMode);
	
	// One cell is clicked by left mouse button.
	virtual void OnLButtonClickedCell(int nRow, int nCol);
	
	// Decides if one col is readonly.
	BOOL IsColReadOnly(int nCol);
	
	// Save the value of one cell back into underlying data.
	virtual BOOL StoreCellValue(int nRow, int nCol, CString strNewValue, CString strOldValue);
	
	// Get the underlying control for one cell.
	CGridControl * GetColControl(int nRow, int nCol);
	
	// Get the style of one cell.
	void GetCellStyle(int nRow, int nCol, CellStyle & style, CRect rect);
	
	// The inplace editing in one cell has been finished.
	BOOL OnEndEditCell();
	
	// Inplace editing in one cell will begin.
	void OnEditCell(int nRow, int nCol, UINT nChar);
	
	// Move col inside one group.
	BOOL MoveColInGroup(int nGroup, int nLevelFrom, int nLevelTo, int nColFrom, int nColTo);
	
	// Move group to other place.
	virtual BOOL MoveGroup(int nGroupFrom, int nGroupTo);
	
	// Move col to other place.
	virtual BOOL MoveCol(int nColFrom, int nColTo);
	
	// Kill the timer.
	void KillTimer();
	
	// One group header has been clicked.
	virtual void OnGroupHeaderClick(int nGroup);
	
	// One col header has been clicked.
	void OnColHeaderClick(int nCol);
	
	// One row header has been clicked.
	void OnRowHeaderClick(int nRow);
	
	// The mouse is moving over the resize area of one group.
	BOOL MouseOverGroupResizeArea(CPoint& point);

	// The mouse is moving over the resize area of one col.
	BOOL MouseOverColResizeArea(CPoint& point);
	
	// The mouse is moving over the resize area of one row.
	BOOL MouseOverRowResizeArea(CPoint& point);
	
	// Decides if we have only no groups.
	BOOL IsSingleLevelMode() const { return m_nGroups == 0; }
	
	// Remove one col from group.
	BOOL RemoveColInGroup(int nGroup, int nColIndex = 0);
	
	// Set the width of one col in one group.
	BOOL SetColWidthInGroup(int nGroup, int nCol, int nWidth);
	
	// Set the visibility of one col in one group.
	BOOL SetColVisibleInGroup(int nGroup, int nCol, int b);
	
	// Get the global col ordinal from the ordinal in one level which is in one group.
	int GetColFromLevel(int nGroup, int nLevel, int nCol);
	
	// Get the underlying cell from one point.
	BOOL GetCellFromPt(CPoint pt, int & nRow, int & nCol);
	
	// Get the bottom right non fixed cell.
	void GetBottomrightNonFixedCell(int &nRow, int &nCol);
	
	// Get the bottom right non fixed group.
	void GetBottomrightNonFixedGroup(int &nRow, int &nGroup);
	
	// Recalc the width of all cols in one level which is in one group.
	void CalcColWidthInLevel(int nGroup, int nLevel);
	
	// Decides if one row is visible.
	BOOL IsRowVisible(int nRow);
	
	// Set the scroll bar position.
	BOOL SetScrollPos32(int nBar, int nPos, BOOL bRedraw = TRUE);
	
	// Get the scroll bar position.
	int GetScrollPos32(int nBar, BOOL bGetTrackPos = FALSE);
	
	// Get the topl left non fixed group.
	void GetTopleftNonFixedGroup(int & nRow, int & nGroup);
	
	// Get the topl left non fixed cell.
	void GetTopleftNonFixedCell(int & nRow, int & nCol);
	
	// Recalc the header height.
	void CalcHeaderHeight();
	
	// Recakc the row height.
	void CalcRowHeight();
	
	// Get the level ordinal from group ordinal and col ordinal.
	int GetLevelFromCol(int nGroup, int & nCol);
	
	// If given cell is a valid cell.
	BOOL IsValidCell(int nRow, int nCol);
	
	// Get the state of one cell.
	UINT GetCellState(int nRow, int nCol);
	
	// Decides if one cell is visible.
	BOOL IsCellVisible(int nRow, int nCol);
	
	// Decides if one group is visible.
	BOOL IsGroupVisible(int nRow, int nGroup);
	
	// Get the rect one group header occupies.
	BOOL GetGroupHeaderRect(int nGroup, CRect& rtCell);
	
	// Draw one group header cell.
	BOOL DrawGroupHeaderCell(int nGroup, CRect rect, CDC *pDC);
	
	// Draw one cell header cell.
	BOOL DrawColHeaderCell(int nRow, int nCol, CRect rect, CDC *pDC);
	
	// Draw one row header cell.
	BOOL DrawRowHeaderCell(int nRow, CRect rect, CDC *pDC);
	
	// Draw a fixed cell.
	BOOL DrawFixedCell(CRect& rect, CDC *pDC, COLORREF clrBack);
	
	// Draw a cell.
	BOOL DrawCell(int nRow, int nCol, CRect rect, CDC* pDC);
	
	// Get the pointer of one cell object.
	Cell * GetCell(int nRow, int nCol);
	
	// Register own window class.
	void RegisterWindowClass();
	
	//{{AFX_MSG(CGrid)
	afx_msg void OnPaint();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CGridEditCtrl;
	friend class CGridComboCtrl;
	friend class CGridControl;
	friend class CGridDateMaskCtrl;
	friend class CGridEditBtnCtrl;
	friend class CGridNumCtrl;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRID_H__0E4822E0_558B_11D3_A7FE_0080C8763FA4__INCLUDED_)
