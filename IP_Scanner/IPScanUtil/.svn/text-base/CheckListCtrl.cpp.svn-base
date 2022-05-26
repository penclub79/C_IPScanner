//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: ip camera connection check class
// 2013-01-30 hkeins : Check List ctrl code separate, jwjang add
// first column check box and column click separation
//
//
#include "stdafx.h"
#include "CheckListCtrl.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CCheckHeadCtrl

CCheckHeadCtrl::CCheckHeadCtrl():
m_bAscending(FALSE),
m_bChecked(FALSE),
m_nLastOrient(-1)
{
}

CCheckHeadCtrl::~CCheckHeadCtrl()
{
}

BEGIN_MESSAGE_MAP(CCheckHeadCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CCheckHeadCtrl)
	ON_NOTIFY_REFLECT(HDN_ITEMCLICK, OnItemClicked)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCheckHeadCtrl::PreSubclassWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CHeaderCtrl::PreSubclassWindow();
}

BOOL CCheckHeadCtrl::ClearState()
{
	m_bAscending	= FALSE;
	m_bChecked		= FALSE;
	m_nLastOrient	= -1;

	return TRUE;
}

void CCheckHeadCtrl::SetChecked(BOOL nChecked)
{
	this->m_bChecked = nChecked;
}
/////////////////////////////////////////////////////////////////////////////
// CCheckHeadCtrl message handlers
void CCheckHeadCtrl::OnItemClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pNMHead = (NMHEADER*)pNMHDR;
	BOOL      bSendSortRequest = FALSE;
	*pResult = 0;
	HDITEM hdItem;

	int nItem = pNMHead->iItem;
	int i = 0;

	if(nItem < 0)
		return;

	if( 0 == nItem)
	{
		// 2013-01-30 hkeins : first column check box and column click separation
		POINT pt = {0, };
		::GetCursorPos(&pt);
		::ScreenToClient(GetSafeHwnd(), &pt);
		TRACE(L"CurPos(%d, %d)\n", pt.x, pt.y);
		
		if(pt.x <= 21)
		{
			hdItem.mask = HDI_IMAGE;
			VERIFY( GetItem(nItem, &hdItem) );

			m_bChecked = !m_bChecked;
			if (m_bChecked)
				hdItem.iImage = 2;
			else
				hdItem.iImage = 1;

			VERIFY( SetItem(nItem, &hdItem) );

			BOOL bl = m_bChecked;
			CListCtrl* pListCtrl = (CListCtrl*)GetParent();
			int nCount = pListCtrl->GetItemCount();	
			for(nItem = 0; nItem < nCount; nItem++)
			{
				ListView_SetCheckState(pListCtrl->GetSafeHwnd(), nItem, bl);
			}
		}
		else
		{
			bSendSortRequest = TRUE;
		}
	}
	else
	{
		bSendSortRequest = TRUE;
	}

	if(bSendSortRequest)
	{
		m_nLastOrient = nItem;
		CWnd* pListCtrl = GetParent();
		CWnd* pDialog   = (pListCtrl != NULL)?pListCtrl->GetParent():NULL;
		if(pDialog)
		{
			m_bAscending = !m_bAscending;
			pDialog->PostMessage(WM_SORT_REQUEST, nItem, m_bAscending);
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
// CCheckListCtrl

CCheckListCtrl::CCheckListCtrl() : m_blInited(FALSE)
{
}

CCheckListCtrl::~CCheckListCtrl()
{
}


BEGIN_MESSAGE_MAP(CCheckListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CCheckListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)		
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckListCtrl message handlers
void CCheckListCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pNMLV = (NMLISTVIEW*)pNMHDR;
	*pResult = 0;

	if ( m_blInited && LVIF_STATE == pNMLV->uChanged)
	{
		BOOL blAllChecked = TRUE;
		int nCount = GetItemCount();
		for(int nItem = 0; nItem < nCount; nItem++)
		{
			if ( !ListView_GetCheckState(GetSafeHwnd(), nItem) )
			{
				blAllChecked = FALSE;
				break;
			}
		}

		HDITEM hdItem;
		hdItem.mask = HDI_IMAGE;
		if (blAllChecked)
		{
			m_checkHeadCtrl.SetChecked(TRUE);
			hdItem.iImage = 2;
		}
		else
		{
			m_checkHeadCtrl.SetChecked(FALSE);
			hdItem.iImage = 1;
		}
		VERIFY( m_checkHeadCtrl.SetItem(0, &hdItem) );
	}

}

BOOL CCheckListCtrl::Init()
{
	if (m_blInited)
		return TRUE;

	CHeaderCtrl* pHeadCtrl = this->GetHeaderCtrl();
	ASSERT(pHeadCtrl->GetSafeHwnd());

	VERIFY( m_checkHeadCtrl.SubclassWindow(pHeadCtrl->GetSafeHwnd()) );
	VERIFY( m_checkImgList.Create(IDB_CHECKBOXES, 16, 3, RGB(255,0,255)) ); // image list expand
	int i = m_checkImgList.GetImageCount();
	m_checkHeadCtrl.SetImageList(&m_checkImgList);
	m_checkHeadCtrl.SetChecked(FALSE);

	HDITEM hdItem;
	hdItem.mask = HDI_IMAGE | HDI_FORMAT;
	VERIFY( m_checkHeadCtrl.GetItem(0, &hdItem) );
	hdItem.iImage = 1;
	hdItem.fmt |= HDF_IMAGE;

	VERIFY( m_checkHeadCtrl.SetItem(0, &hdItem) );
	ClearState();

	m_blInited = TRUE;

	return TRUE;
}

BOOL CCheckListCtrl::ClearState() // 초기 상태로
{
	m_checkHeadCtrl.ClearState();

	return TRUE;
}
