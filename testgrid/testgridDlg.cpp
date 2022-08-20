// testgridDlg.cpp : implementation file
//

#include "stdafx.h"
#include "testgrid.h"
#include "testgridDlg.h"

#define DWORD_PTR DWORD*
#include <wininet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestgridDlg dialog

CTestgridDlg::CTestgridDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestgridDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestgridDlg)
	m_nCol = 0;
	m_nStyle = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestgridDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestgridDlg)
	DDX_Text(pDX, IDC_EDIT_COL, m_nCol);
	DDX_CBIndex(pDX, IDC_COMBO_COLSTYLE, m_nStyle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestgridDlg, CDialog)
	//{{AFX_MSG_MAP(CTestgridDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELENDOK(IDC_COMBO_COLSTYLE, OnSelendokComboColstyle)
	ON_BN_CLICKED(IDC_BUTTON_SETCOLOR, OnButtonSetcolor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestgridDlg message handlers

BOOL CTestgridDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	SetWindowPos(NULL, 1, 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	m_wndGrid.SubclassDlgItem(IDC_GRID, this);

	m_wndGrid.SetColCount(2);
	m_wndGrid.SetRowCount(1);
/*	m_wndGrid.SetGroupCount(1);
	m_wndGrid.SetLevelCount(2);
	m_wndGrid.SetFrozenRows(1);
	m_wndGrid.SetFrozenGroups(1);
	m_wndGrid.SetLevelCount(3);
	m_wndGrid.SetHeaderHeight(60);
	m_wndGrid.SetRowHeight(45);
	m_wndGrid.InsertColInGroup(2, 1);
	m_wndGrid.InsertColInGroup(2, 1);
	m_wndGrid.InsertColInGroup(3, 1);
	m_wndGrid.InsertColInGroup(4, 1);
	m_wndGrid.InsertColInGroup(5, 1);
*/	
//	m_wndGrid.SetDividerStyle(PS_DOT);
//	m_wndGrid.SetDividerType(CGrid::DividerTypeHorizontal);
//	m_wndGrid.SetDividerColor(RGB(255, 0, 0));
//	m_wndGrid.SetBackColor(RGB(0, 0, 0));
	
	m_wndGrid.SetColControl(1, COLSTYLE_NUM);
//	m_wndGrid.SetColMask(1, _T("i10d0"));
	m_wndGrid.SetColPromptInclude(1, FALSE);
//	m_wndGrid.SetHeaderHeight(40);
//	m_wndGrid.SetShowGroupHeader(FALSE);

	m_wndGrid.ResetScrollBars();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestgridDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestgridDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTestgridDlg::OnSelendokComboColstyle() 
{
	CString strCol;
	GetDlgItem(IDC_EDIT_COL)->GetWindowText(strCol);
	m_nCol = atoi(strCol);

	m_nStyle = ((CComboBox *)GetDlgItem(IDC_COMBO_COLSTYLE))->GetCurSel();

	m_wndGrid.SetColDataType(m_nCol, DataTypeDate);
	m_wndGrid.SetColPromptInclude(m_nCol, FALSE);
	m_wndGrid.SetColControl(m_nCol, m_nStyle);
}

void CTestgridDlg::OnButtonSetcolor() 
{
//	m_wndGrid.SetAllowAddNew(!m_wndGrid.GetAllowAddNew());

//	m_wndGrid.Invalidate();
//	mfntsv1.u-aizu.ac.jp

	INTERNET_PER_CONN_OPTION_LIST    List; 
	INTERNET_PER_CONN_OPTION         Option[2]; 
	unsigned long                    nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST); 
	
	memset(Option,0,2*sizeof(INTERNET_PER_CONN_OPTION)); 
	
	Option[0].dwOption = INTERNET_PER_CONN_FLAGS; 
	Option[0].Value.dwValue = PROXY_TYPE_PROXY; 
	Option[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER; 
	Option[1].Value.pszValue= "mfntsv1.u-aizu.ac.jp:80"; 
	
	List.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST); 
	List.pszConnection = NULL; 
	List.dwOptionCount = 2; 
	List.dwOptionError = 0; 
	List.pOptions = Option; 
	
	InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &List, nSize);
}
