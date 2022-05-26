#include "stdafx.h"
#include "ProgressListCtrl.h"

#define UM_RESIZEITEM		WM_USER+100

/////////////////////////////////////////////////////////////////////////////
// CProgressListCtrl

CProgressListCtrl::CProgressListCtrl()
{
	m_pEditFont	= new CFont();
	m_pEditFont->CreateFont(14, 
		0, 
		0,
		0,
		FW_NORMAL, 
		FALSE, 
		FALSE, 
		0, 
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, 
		CLIP_DEFAULT_PRECIS, 
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, 
		L"Arial" );
}

CProgressListCtrl::~CProgressListCtrl()
{
	CProgressEntry* pProgEntry	= NULL;
	int				Index=0;

	POSITION pos = m_ProgEntries.GetStartPosition();
	while (pos != NULL)
	{
		m_ProgEntries.GetNextAssoc(pos, Index, pProgEntry);
		if (pProgEntry) 
		{
			delete pProgEntry;
		}
	}
	m_ProgEntries.RemoveAll();

	CEditEntry* pEditEntry	= NULL;
	Index		= 0;
	pos = m_EditEntries.GetStartPosition();
	while (pos != NULL)
	{
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			delete pEditEntry;
		}
	}
	m_EditEntries.RemoveAll();

	delete m_pEditFont;
	m_pEditFont	= NULL;
}


BEGIN_MESSAGE_MAP(CProgressListCtrl, CListCtrl)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_NOTIFY(HDN_TRACKA, 0, &CProgressListCtrl::OnHdnTrack)
	ON_NOTIFY(HDN_TRACKW, 0, &CProgressListCtrl::OnHdnTrack)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, &CProgressListCtrl::OnLvnItemchanging)
	ON_MESSAGE( UM_RESIZEITEM, &CProgressListCtrl::OnResizeItems)
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, &CProgressListCtrl::OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, &CProgressListCtrl::OnHdnItemchanging)
	ON_NOTIFY_REFLECT(NM_THEMECHANGED, &CProgressListCtrl::OnNMThemeChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressListCtrl message handlers

void CProgressListCtrl::CreateEdit(int iIndex, int iSubIndex, BOOL bPassword )
{
	// can only create progress for an existing item
	if (iIndex >= GetItemCount())
		return;

	CEditEntry* EditEntry = new CEditEntry(iIndex, iSubIndex );
	CRect ItemRect;
	GetSubItemRect(iIndex, EditEntry->m_SubIndex, LVIR_BOUNDS, ItemRect);

	int left	= ItemRect.left+1;
	int top		= ItemRect.top+1;
	int right	= ItemRect.right-1;
	int bottom	= ItemRect.bottom-1;

	if( TRUE == bPassword )
		(EditEntry->m_Edit).Create( WS_CHILD | WS_VISIBLE | ES_CENTER | ES_PASSWORD , CRect(left, top, right, bottom), this, IDC_LISTVIEW_EDIT);
	else 
		(EditEntry->m_Edit).Create( WS_CHILD | WS_VISIBLE | ES_CENTER , CRect(left, top, right, bottom), this, IDC_LISTVIEW_EDIT);

	(EditEntry->m_Edit).SetFont(m_pEditFont);
	(EditEntry->m_Edit).ShowWindow(SW_SHOW);

	m_EditEntries[ m_EditEntries.GetCount() ] = EditEntry;
}

void CProgressListCtrl::SetEditEnable(int iIndex, int iSubIndex, BOOL bEnable )
{
	CEditEntry* pEditEntry;
	int			Index	= 0;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) {
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			if( pEditEntry->m_Index == iIndex && pEditEntry->m_SubIndex == iSubIndex )
			{
				(pEditEntry->m_Edit).EnableWindow(bEnable);
				break;
			}
		}
	}
}

void CProgressListCtrl::SetLimitText(int iIndex, int iSubIndex, int iMax)
{
	CEditEntry* pEditEntry;
	int			Index	= 0;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) {
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			if( pEditEntry->m_Index == iIndex && pEditEntry->m_SubIndex == iSubIndex )
			{
				(pEditEntry->m_Edit).SetLimitText(iMax);
				break;
			}
		}
	}
}

void CProgressListCtrl::SetEditText(int iIndex, int iSubIndex, CString strText)
{
	CEditEntry* pEditEntry;
	int			Index	= 0;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) {
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			if( pEditEntry->m_Index == iIndex && pEditEntry->m_SubIndex == iSubIndex )
			{
				(pEditEntry->m_Edit).SetWindowText(strText);
				break;
			}
		}
	}
}

CString CProgressListCtrl::GetEditText(int iIndex, int iSubIndex )
{
	CEditEntry* pEditEntry;
	int			Index	= 0;
	CString		strText;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) {
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			if( pEditEntry->m_Index == iIndex && pEditEntry->m_SubIndex == iSubIndex )
			{
				(pEditEntry->m_Edit).GetWindowText(strText);
				break;
			}
		}
	}

	return strText;
}

void CProgressListCtrl::DeleteEdit(int iIndex, int iSubIndex)
{
	//	TRACE(L"##### DeleteEdit, Index = %d\n",Index);
	CEditEntry* pEditEntry;
	int			Index	= 0;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) {
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			if( pEditEntry->m_Index == iIndex && pEditEntry->m_SubIndex == iSubIndex )
			{
				(pEditEntry->m_Edit).ShowWindow(SW_HIDE);
				m_EditEntries.RemoveKey(Index); // memory leak bug fix.
				delete pEditEntry;
				break;
			}
		}
	}
}

void CProgressListCtrl::SetFocusEdit(int iIndex, int iSubIndex ) // set focus edit function add
{
	//	TRACE(L"##### SetFocusEdit, Index = %d\n",Index);
	CEditEntry* pEditEntry;
	int			Index	= 0;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) {
		m_EditEntries.GetNextAssoc(pos, Index, pEditEntry);
		if (pEditEntry) 
		{
			if( pEditEntry->m_Index == iIndex && pEditEntry->m_SubIndex == iSubIndex )
			{
				(pEditEntry->m_Edit).SetSel(0, -1);
				(pEditEntry->m_Edit).SetFocus();
				break;
			}
		}
	}
}

void CProgressListCtrl::CreateProgress(int iIndex, int iSubIndex)
{
	// can only create progress for an existing item
	if (iIndex >= GetItemCount())
		return;
 
	CProgressEntry* ProgEntry = new CProgressEntry(iIndex, iSubIndex);
	CRect ItemRect;
	GetSubItemRect(iIndex, ProgEntry->m_SubIndex, LVIR_BOUNDS, ItemRect);

	int left = ItemRect.left;
	int top = ItemRect.top;
	int right = ItemRect.right;
	int bottom = ItemRect.bottom;

	(ProgEntry->m_Prog).Create(PBS_SMOOTH | WS_CHILD | WS_VISIBLE, CRect(left, top, right, bottom), this, 1);
	(ProgEntry->m_Prog).ShowWindow(SW_SHOW);
	(ProgEntry->m_Prog).SetRange(0, 100);
	(ProgEntry->m_Prog).SetPos(0);

	m_ProgEntries[iIndex] = ProgEntry;

//  TRACE(L"##### CreateProgress, Index = %d\n",Index);
}

void CProgressListCtrl::SetProgress(int iIndex, int iSubIndex, int prog)
{
//	TRACE(L"##### SetProgress, Index = %d, prog = %d\n",iIndex, prog);

	int				Index	= 0;
	CProgressEntry* pProgEntry;
	POSITION pos = m_ProgEntries.GetStartPosition();
	while (pos != NULL) {
		m_ProgEntries.GetNextAssoc(pos, Index, pProgEntry);
		if (pProgEntry) 
		{
			if( pProgEntry->m_Index == iIndex && pProgEntry->m_SubIndex == iSubIndex )
			{
				(pProgEntry->m_Prog).SetPos(prog);
				break;
			}
		}
	}

	//CProgressEntry* ProgEntry;
	//if (m_ProgEntries.Lookup(Index, ProgEntry) == TRUE)
	//	(ProgEntry->m_Prog).SetPos(prog);
}

void CProgressListCtrl::DeleteProgress(int iIndex, int iSubIndex)
{
	int				Index	= 0;
	CProgressEntry* pProgEntry;
	POSITION pos = m_ProgEntries.GetStartPosition();
	while (pos != NULL) {
		m_ProgEntries.GetNextAssoc(pos, Index, pProgEntry);
		if (pProgEntry) 
		{
			if( pProgEntry->m_Index == iIndex && pProgEntry->m_SubIndex == iSubIndex )
			{
				(pProgEntry->m_Prog).ShowWindow(SW_HIDE);
				m_ProgEntries.RemoveKey(Index); // memory leak bug fix.
				delete pProgEntry;
				break;
			}
		}
	}

	//(m_ProgEntries[Index]->m_Prog).ShowWindow(SW_HIDE);
//	TRACE(L"##### DeleteProgress, Index = %d\n",Index);
}

void CProgressListCtrl::InitHdr()
{
	m_AdvHdr.Init(GetHeaderCtrl());
}

/////////////////////////////////////////////////////////////////////////////
// CProgressListCtrl message handlers

void CProgressListCtrl::CreateColumns()
{
	CString str;
	CString strItem; 

	//UpgradeList 초기화
	InitHdr();

	SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	str.LoadString(IDS_ADDRESS);
	strItem = L"IP " + str;
	InsertColumn(0, strItem, LVCFMT_CENTER, 100, -1 );			// IP Address
	strItem.LoadString(IDS_MODEL);
	InsertColumn(1, strItem, LVCFMT_CENTER, 100, -1 );			// Model
	strItem.LoadString(IDS_FIRMWARE_VERSION);
	InsertColumn(2, strItem, LVCFMT_CENTER, 150, -1 );			// Firmware Version
	strItem.LoadString(IDS_USER_ID);
	InsertColumn(3, strItem, LVCFMT_CENTER, 50, -1 );			// ID
	strItem.LoadString(IDS_USER_PW);
	InsertColumn(4, strItem, LVCFMT_CENTER, 50, -1 );			// Password
	strItem.LoadString(IDS_UPGRADE_PROGRESS);
	InsertColumn(5, strItem, LVCFMT_CENTER, 100, -1 );			// Progress
	strItem.LoadString(IDS_PROGRESS_STATUS);
	InsertColumn(6, strItem, LVCFMT_CENTER, 220, -1 );			// Status


	DeleteAllItems();
}

CProgressListCtrl::CEditEntry::CEditEntry(int Index, int SubIndex) :
m_Index(Index), m_SubIndex(SubIndex)
{
}

CProgressListCtrl::CEditEntry::~CEditEntry()
{
}


CProgressListCtrl::CProgressEntry::CProgressEntry(int Index, int SubIndex) :
m_Index(Index), m_SubIndex(SubIndex)
{
}

CProgressListCtrl::CProgressEntry::~CProgressEntry()
{
}

void CProgressListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);

	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
//	settingScroll();
}

void CProgressListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
//	settingScroll();
}

BOOL CProgressListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );

	return TRUE;
}

BOOL CProgressListCtrl::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnMouseHWheel(nFlags, zDelta, pt);
	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );

	return TRUE;
}

void CProgressListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
}

void CProgressListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
}

void CProgressListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
}

void CProgressListCtrl::ResizeEdit()
{
	CRect ItemRect;
	CEditEntry* EditEntry=0;
	int Index=0;
	POSITION pos = m_EditEntries.GetStartPosition();
	while (pos != NULL) 
	{
		m_EditEntries.GetNextAssoc(pos, Index, EditEntry);
		GetSubItemRect(EditEntry->m_Index, EditEntry->m_SubIndex, LVIR_BOUNDS, ItemRect);

		int left = ItemRect.left+1;
		int top = ItemRect.top+1;
		int right = ItemRect.right-1;
		int bottom = ItemRect.bottom-1;

		(EditEntry->m_Edit).MoveWindow(left, top, (right - left), (bottom - top), TRUE);

		CRect	rect;
		GetHeaderCtrl()->GetClientRect(rect);

		if(top>=rect.bottom)
			(EditEntry->m_Edit).MoveWindow(left, top, (right - left), (bottom - top));
		else
			(EditEntry->m_Edit).MoveWindow(left, -20, (right - left), (bottom - top));

		GetItemRect(EditEntry->m_Index,&rect,LVIR_BOUNDS);
		InvalidateRect(&rect);
	}
}

void CProgressListCtrl::ResizeProg()
{
	CRect ItemRect;
	CProgressEntry* ProgEntry=0;
	int Index=0;
	POSITION pos = m_ProgEntries.GetStartPosition();
	while (pos != NULL) {
		m_ProgEntries.GetNextAssoc(pos, Index, ProgEntry);
		GetSubItemRect(ProgEntry->m_Index, ProgEntry->m_SubIndex, LVIR_BOUNDS, ItemRect);
		int left = ItemRect.left;
		int top = ItemRect.top;
		int right = ItemRect.right;
		int bottom = ItemRect.bottom;

		(ProgEntry->m_Prog).MoveWindow(left, top, (right - left), (bottom - top), TRUE);

		CRect	rect;
		GetHeaderCtrl()->GetClientRect(rect);

		if(top>=rect.bottom)
			(ProgEntry->m_Prog).MoveWindow(left, top, (right - left), (bottom - top));
		else
			(ProgEntry->m_Prog).MoveWindow(left, -20, (right - left), (bottom - top));

		GetItemRect(ProgEntry->m_Index,&rect,LVIR_BOUNDS);
		InvalidateRect(&rect);
	}
}

void CProgressListCtrl::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CListCtrl::OnWindowPosChanged(lpwndpos);

	// TODO: Add your message handler code here
	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
}


/////////////////////////////////////////////////////////////////////////////
// CAdvHeaderCtrl

CAdvHeaderCtrl::CAdvHeaderCtrl()
{
}

CAdvHeaderCtrl::~CAdvHeaderCtrl()
{
}


BEGIN_MESSAGE_MAP(CAdvHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CAdvHeaderCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(HDN_ENDTRACKW, OnEndTrack)
	ON_NOTIFY_REFLECT(HDN_ENDTRACKA, OnEndTrack)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvHeaderCtrl message handlers

/////////////////////////////////////////////////////////////////////////////
/*
	Init

	This initialization function must be called after the initial CHeaderCtrl
	has been created.  It subclasses the pHeader control passed in as a 
	parameter so that the CAdvHeaderCtrl can handle the reflected messages.
	
	Params:
		pHeader	pointer to created CHeaderCtrl

	Returns:
		BOOL	could the existing header be subclassed?
*/
/////////////////////////////////////////////////////////////////////////////
BOOL CAdvHeaderCtrl::Init(CHeaderCtrl *pHeader)
{	
	ASSERT(pHeader && pHeader->GetSafeHwnd());
	if (!SubclassWindow(pHeader->GetSafeHwnd()))
	{
		OutputDebugString(_T("Unable to subclass existing header!\n"));
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/*
	OnEndTrack
	
	Params:
		pNMHDR	pointer to NMHDR structure
		pResult	pointer to LRESULT code

	Returns:
		None
*/
/////////////////////////////////////////////////////////////////////////////
void CAdvHeaderCtrl::OnEndTrack(NMHDR * pNMHDR, LRESULT* pResult)
{
	NMHEADER *pHdr = (NMHEADER*)pNMHDR;

	//((CProgressListCtrl*)GetParent())->ResizeProg();
	//((CProgressListCtrl*)GetParent())->ResizeEdit();


	((CProgressListCtrl*)GetParent())->PostMessage( UM_RESIZEITEM, NULL, NULL );
	*pResult = 0;
}


void CProgressListCtrl::OnHdnTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );

	*pResult = 0;
}

void CProgressListCtrl::OnLvnItemchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	//ResizeProg();
	//ResizeEdit();

	PostMessage( UM_RESIZEITEM, NULL, NULL );
	*pResult = 0;
}

LRESULT CProgressListCtrl::OnResizeItems( WPARAM wParam, LPARAM lParam )
{
	ResizeProg();
	ResizeEdit();

	return 0;
}

void CProgressListCtrl::settingScroll()
{
	// 작업 가로

	// 사용할 스크롤 의 가로 길이 ( 이미지 라면 이미지 가로 사이즈를 넣어주면 된다.)
	int		iWidth			= 0;	
	RECT	rtItemRect		= {0};
	int		iColumnCount	= this->GetHeaderCtrl()->GetItemCount();

	CRect	ItemRect;

	TRACE( "////////////////\n" );
	for( int i=0; i<iColumnCount ;i++)
	{
		GetHeaderCtrl()->GetItemRect(i,ItemRect);

		iWidth	+= ItemRect.Width();
		TRACE( "%d \n", ItemRect.Width() );
	}

	// 윈도우 렉트
	CRect rt;
	GetClientRect( &rt );

	// 스크롤 정보
	SCROLLINFO si;
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL;

	///////////////////////////////////////////////////////
	// 가로
	if( rt.Width() < iWidth )
	{
		si.nMin = 0;
		si.nMax = iWidth;
		si.nPage = rt.Width();
		si.nPos = GetScrollPos(SB_HORZ);;

		SetScrollInfo( SB_HORZ, &si, TRUE );
		ShowScrollBar(SB_HORZ, TRUE);
	}
	else
	{
		ShowScrollBar(SB_HORZ, FALSE);
		UpdateWindow();
	}
}

void CProgressListCtrl::settingScrollShow()
{
	// 작업 가로

	// 사용할 스크롤 의 가로 길이 ( 이미지 라면 이미지 가로 사이즈를 넣어주면 된다.)
	int		iWidth			= 0;	
	RECT	rtItemRect		= {0};
	int		iColumnCount	= this->GetHeaderCtrl()->GetItemCount();
	CRect	ItemRect;

	TRACE( "////////////////\n" );
	for( int i=0; i<iColumnCount ;i++)
	{
		GetSubItemRect(0, i, LVIR_BOUNDS, ItemRect);

		iWidth	+= ItemRect.Width();
		TRACE( "%d \n", iWidth );
	}

	// 윈도우 렉트
	CRect rt;
	GetClientRect( &rt );

	// 스크롤 정보
	SCROLLINFO si;
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL;

	///////////////////////////////////////////////////////
	// 가로
	if(rt.Width() < iWidth)
	{
		ShowScrollBar(SB_HORZ, TRUE);
	}
	else
	{
		ShowScrollBar(SB_HORZ, FALSE);
		InvalidateRect(NULL);
	}
}


void CProgressListCtrl::OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	//ResizeProg();
	//ResizeEdit();

	////PostMessage( UM_RESIZEITEM, NULL, NULL );
	//UpdateWindow();
	//InvalidateRect(NULL);

//	settingScroll();

	*pResult = 0;
}

void CProgressListCtrl::OnNMThemeChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 이 기능을 사용하려면 Windows XP 이상이 필요합니다.
	// _WIN32_WINNT 기호는 0x0501보다 크거나 같아야 합니다.
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}
