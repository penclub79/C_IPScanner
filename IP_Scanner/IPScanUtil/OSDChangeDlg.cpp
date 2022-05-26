// OSDChangeDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "OSDChangeDlg.h"
#include "ARP.h"


// COSDChangeDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(COSDChangeDlg, CDialog)

COSDChangeDlg::COSDChangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COSDChangeDlg::IDD, pParent)
	, m_strOverlayText(_T(""))
{
	m_iAdapterID	= 0;
	m_nScanInfoCnt	= 0;
	m_pScanInfo		= NULL;

	m_pSession		= NULL;
	m_pHttpConnect	= NULL;
	m_pHttpFile		= NULL;
}

COSDChangeDlg::~COSDChangeDlg()
{
	for( int iItem=0; iItem<m_aSettingThread.GetCount(); iItem++ )
	{
		delete m_aSettingThread[iItem];
		m_aSettingThread[iItem]	= NULL;
	}

	m_aSettingThread.RemoveAll();

	if(m_pHttpFile)
	{
		m_pHttpFile->Flush();
		m_pHttpFile->Close();
		delete m_pHttpFile;
		m_pHttpFile = NULL;
	}

	if(m_pHttpConnect)
	{
		m_pHttpConnect->Close();
		delete m_pHttpConnect;
		m_pHttpConnect = NULL;
	}

	if(m_pSession)
	{
		m_pSession->Close();
		delete m_pSession;
		m_pSession = NULL;
	}


	if( m_pScanInfo	)
	{
		delete [] m_pScanInfo	;
		m_pScanInfo		= NULL;
	}
}

void COSDChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STC_OVERLAYTEXT	, m_ctlStaticOverlayText	);
	DDX_Control(pDX, IDC_EDIT_OVERLAYTEXT	, m_ctlEditOverlayText		);
	DDX_Control(pDX, IDC_CHECK_SETOSD_IPADD	, m_ctlCheckSetOSDIPADD		);
	DDX_Control(pDX, IDC_DEVICE_LIST		, m_ctlListDevice			);
	DDX_Control(pDX, IDC_BUTTON_APPLY		, m_ctlButtonApply			);
	DDX_Control(pDX, IDC_BUTTON_CANCEL		, m_ctlButtonCancel			);
	DDX_Control(pDX, IDC_COMBO_POSITION, m_ctlComboPosition);
	DDX_Control(pDX, IDC_CHECK_USED, m_ctlCheckUsed);
	DDX_Control(pDX, IDC_STC_TEXTPOSITION, m_ctlStaticTextPosition);
	DDX_Text(pDX, IDC_EDIT_OVERLAYTEXT, m_strOverlayText);
	DDV_MaxChars(pDX, m_strOverlayText, 16);
}

BEGIN_MESSAGE_MAP(COSDChangeDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_APPLY		, &COSDChangeDlg::OnBnClickedButtonApply		)
	ON_BN_CLICKED(IDC_CHECK_SETOSD_IPADD, &COSDChangeDlg::OnBnClickedCheckSetosdIpadd	)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL		, &COSDChangeDlg::OnBnClickedButtonCancel		)
	ON_EN_CHANGE(IDC_EDIT_OVERLAYTEXT	, &COSDChangeDlg::OnEnChangeEditOverlaytext		)
	ON_EN_CHANGE(IDC_USERID				, &COSDChangeDlg::OnEnChangeUserid				)
	ON_EN_CHANGE(IDC_PASSWORD			, &COSDChangeDlg::OnEnChangePassword			)
	ON_MESSAGE( UM_STATUS_MESSAGE		, &COSDChangeDlg::OnMessageStatus				)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_USED, &COSDChangeDlg::OnBnClickedCheckUsed)
END_MESSAGE_MAP()


// COSDChangeDlg 메시지 처리기입니다.

BOOL COSDChangeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	int iIndex	= 0;
	iIndex	= m_ctlComboPosition.AddString(L"Top left");
	m_ctlComboPosition.SetItemData( iIndex, COSDChangeThread::POSITION_TOPLEFT );
	iIndex	= m_ctlComboPosition.AddString(L"Top right");
	m_ctlComboPosition.SetItemData( iIndex, COSDChangeThread::POSITION_TOPRIGHT );

	m_ctlComboPosition.SetCurSel(0);

	m_ctlCheckUsed.SetCheck(FALSE);
	m_ctlCheckSetOSDIPADD.SetCheck(TRUE);
	m_ctlCheckSetOSDIPADD.EnableWindow(FALSE);
	m_ctlStaticOverlayText.EnableWindow(FALSE);
	m_ctlEditOverlayText.EnableWindow(FALSE);
	m_ctlComboPosition.EnableWindow(FALSE);

	CString strItem; 
	CString str; 

	m_ctlListDevice.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);


	strItem.LoadString(IDS_MODEL);
	m_ctlListDevice.InsertColumn(SUBITEM_MODEL				, strItem, LVCFMT_CENTER, 110, 0 );	

	str.LoadString(IDS_ADDRESS);
	strItem = L"IP" + str;
	m_ctlListDevice.InsertColumn(SUBITEM_IPADDRESS			, strItem, LVCFMT_CENTER, 110, 0 );	

	strItem = L"Overlay Text";
	m_ctlListDevice.InsertColumn(SUBITEM_SETTEXT			, strItem, LVCFMT_CENTER, 110, 0 );	

	strItem.LoadString(IDS_USER_ID);
	m_ctlListDevice.InsertColumn(SUBITEM_USER_ID			, strItem, LVCFMT_CENTER, 50, 0 );	

	strItem.LoadString(IDS_USER_PW);
	m_ctlListDevice.InsertColumn(SUBITEM_USER_PW			, strItem, LVCFMT_CENTER, 50, 0 );	

	strItem.LoadString(IDS_PROGRESS_STATUS);
	m_ctlListDevice.InsertColumn(SUBITEM_STATUS				, strItem, LVCFMT_CENTER, 250, 0 );	

	int			iItemCount	= m_ctlListDevice.GetItemCount();
	CString		strTemp;
	LV_ITEM		item;

	for(int iItemIndex=0;iItemIndex<m_nScanInfoCnt;iItemIndex++)
	{
		strTemp = m_pScanInfo[iItemIndex]._ReadValue(L"Model Type");	
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = iItemIndex;
		item.iSubItem = SUBITEM_MODEL;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		item.lParam = (LPARAM)&m_pScanInfo[iItemIndex];
		m_ctlListDevice.InsertItem(&item);

		strTemp = m_pScanInfo[iItemIndex].szAddr;
		item.mask = LVIF_TEXT;
		item.iItem = iItemIndex;
		item.iSubItem = SUBITEM_IPADDRESS;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_ctlListDevice.SetItem(&item);

		if( TRUE == m_ctlCheckSetOSDIPADD.GetCheck()  )
		{
			strTemp = m_pScanInfo[iItemIndex].szAddr;	
		}
		else
		{
			strTemp = "";	
		}

		item.mask = LVIF_TEXT;
		item.iItem = iItemIndex;
		item.iSubItem = SUBITEM_SETTEXT;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_ctlListDevice.SetItem(&item);
		m_ctlListDevice.CreateEdit(iItemIndex, SUBITEM_SETTEXT, FALSE);
		m_ctlListDevice.SetLimitText(iItemIndex, SUBITEM_SETTEXT, 16);
		m_ctlListDevice.SetEditText(iItemIndex, SUBITEM_SETTEXT, strTemp);

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = iItemIndex;
		item.iSubItem = SUBITEM_USER_ID;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_ctlListDevice.SetItem(&item);
		m_ctlListDevice.CreateEdit(iItemIndex, SUBITEM_USER_ID, FALSE);
		m_ctlListDevice.SetEditText(iItemIndex, SUBITEM_USER_ID, strTemp);

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = iItemIndex;
		item.iSubItem = SUBITEM_USER_PW;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_ctlListDevice.SetItem(&item);
		m_ctlListDevice.CreateEdit(iItemIndex, SUBITEM_USER_PW, TRUE);
		m_ctlListDevice.SetEditText(iItemIndex, SUBITEM_USER_PW, strTemp);

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = iItemIndex;
		item.iSubItem = SUBITEM_STATUS;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_ctlListDevice.SetItem(&item);

		m_ctlListDevice.SetItemData( iItemIndex, (DWORD_PTR)&m_pScanInfo[iItemIndex] );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void COSDChangeDlg::OnBnClickedButtonApply()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szUserID[32]		= {0};
	WCHAR	szUserPW[32]		= {0};
	WCHAR	szOverlayText[32]		= {0};

	GetDlgItemText( IDC_USERID	, szUserID, 32 );
	GetDlgItemText( IDC_PASSWORD, szUserPW, 32 );

	CString strError;

	if( 0 == wcslen(szUserID) )
	{
		strError.LoadString(IDS_ENTER_LOGIN_INFO);
		AfxMessageBox(strError, MB_ICONWARNING);
		GetDlgItem(IDC_USERID)->SetFocus();
		return;
	}
	else if( 0 == wcslen(szUserPW) )
	{
		strError.LoadString(IDS_ENTER_PASSWORD_INFO);
		AfxMessageBox(strError, MB_ICONWARNING);
		GetDlgItem(IDC_PASSWORD)->SetFocus();
		return;
	}

	EnableControlMain(FALSE);


	for( int iItem=0; iItem<m_aSettingThread.GetCount(); iItem++ )
	{
		delete m_aSettingThread[iItem];
		m_aSettingThread[iItem]	= NULL;
	}

	m_aSettingThread.RemoveAll();

	CheckArp( m_iAdapterID );


	SETUP_OSDINFO	setupOSDInfo	= {0};
	SCAN_INFO*		pScanInfo		= NULL;

	for( int iItem=0; iItem<m_ctlListDevice.GetItemCount(); iItem++ )
	{
		pScanInfo = (SCAN_INFO*)m_ctlListDevice.GetItemData(iItem);

		swprintf_s( szOverlayText, L"%s", m_ctlListDevice.GetEditText(iItem, SUBITEM_SETTEXT) );
		swprintf_s( szUserID, L"%s", m_ctlListDevice.GetEditText(iItem, SUBITEM_USER_ID) );
		swprintf_s( szUserPW, L"%s", m_ctlListDevice.GetEditText(iItem, SUBITEM_USER_PW) );
		

		setupOSDInfo.bUsed				= (BOOL)m_ctlCheckUsed.GetCheck();
		setupOSDInfo.strOverlayText		= szOverlayText;
		setupOSDInfo.iDisplayPosition	= m_ctlComboPosition.GetItemData(m_ctlComboPosition.GetCurSel());

		m_aSettingThread[iItem]	= new COSDChangeThread(	this->GetSafeHwnd(),
			iItem,
			pScanInfo->szAddr,
			pScanInfo->nHTTPPort,
			szUserID,
			szUserPW, 
			setupOSDInfo );
	}
}

void COSDChangeDlg::OnBnClickedCheckUsed()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BOOL	bUsed	= (BOOL)m_ctlCheckUsed.GetCheck();

	m_ctlStaticOverlayText.EnableWindow(bUsed);
	m_ctlEditOverlayText.EnableWindow(bUsed);
	m_ctlStaticTextPosition.EnableWindow(bUsed);
	m_ctlComboPosition.EnableWindow(bUsed);
	m_ctlCheckSetOSDIPADD.EnableWindow(bUsed);

	if( m_ctlCheckSetOSDIPADD.GetCheck() )
	{
		m_ctlStaticOverlayText.EnableWindow(FALSE);
		m_ctlEditOverlayText.EnableWindow(FALSE);
	}
}


void COSDChangeDlg::EnableControlMain( BOOL bEnable )
{
	m_ctlStaticOverlayText.EnableWindow(bEnable);
	m_ctlEditOverlayText.EnableWindow(bEnable);

	m_ctlCheckSetOSDIPADD.EnableWindow(bEnable);
	m_ctlStaticTextPosition.EnableWindow(bEnable);
	m_ctlComboPosition.EnableWindow(bEnable);
	m_ctlCheckUsed.EnableWindow(bEnable);

	if( TRUE == bEnable )
	{
		if( m_ctlCheckUsed.GetCheck() )
		{
			m_ctlStaticOverlayText.EnableWindow(TRUE);
			m_ctlEditOverlayText.EnableWindow(TRUE);
			m_ctlStaticTextPosition.EnableWindow(TRUE);
			m_ctlComboPosition.EnableWindow(TRUE);
			m_ctlCheckSetOSDIPADD.EnableWindow(TRUE);

			if( m_ctlCheckSetOSDIPADD.GetCheck() )
			{
				m_ctlStaticOverlayText.EnableWindow(FALSE);
				m_ctlEditOverlayText.EnableWindow(FALSE);
			}
		}
		else
		{
			m_ctlStaticOverlayText.EnableWindow(FALSE);
			m_ctlEditOverlayText.EnableWindow(FALSE);
			m_ctlStaticTextPosition.EnableWindow(FALSE);
			m_ctlComboPosition.EnableWindow(FALSE);
			m_ctlCheckSetOSDIPADD.EnableWindow(FALSE);
		}
	}
	
	GetDlgItem(IDC_STC_GROUP_LOGIN	)->EnableWindow(bEnable);
	GetDlgItem(IDC_STC_USERID		)->EnableWindow(bEnable);
	GetDlgItem(IDC_STC_PASSWORD		)->EnableWindow(bEnable);
	GetDlgItem(IDC_USERID			)->EnableWindow(bEnable);
	GetDlgItem(IDC_PASSWORD			)->EnableWindow(bEnable);

	for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
	{
		m_ctlListDevice.SetEditEnable( i, SUBITEM_SETTEXT		, bEnable );
		m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_ID		, bEnable );
		m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_PW		, bEnable );
	}	

	if( bEnable )
	{
		OnBnClickedCheckSetosdIpadd();
	}
}

void COSDChangeDlg::OnBnClickedCheckSetosdIpadd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szOverlayText[32]	= {0};

	if( m_ctlCheckUsed.GetCheck() )
	{
		if( m_ctlCheckSetOSDIPADD.GetCheck() )
		{
			m_ctlStaticOverlayText.EnableWindow(FALSE);
			m_ctlEditOverlayText.EnableWindow(FALSE);

			for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
			{
				m_ctlListDevice.SetEditText( i, SUBITEM_SETTEXT, m_pScanInfo[i].szAddr );
			}
		}
		else
		{
			m_ctlStaticOverlayText.EnableWindow(TRUE);
			m_ctlEditOverlayText.EnableWindow(TRUE);
		}
	}
	else
	{
		m_ctlStaticOverlayText.EnableWindow(FALSE);
		m_ctlEditOverlayText.EnableWindow(FALSE);
	}
}

LRESULT COSDChangeDlg::OnMessageStatus(WPARAM wParam, LPARAM lParam)
{
	int		iMessageID			= (int)wParam;
	int		iItemIndex			= (int)lParam;
	WCHAR	szMessage[MAX_PATH]	= {0};

	switch( iMessageID )
	{
	case COSDChangeThread::MESSAGE_ID_OSD_SET_START		:	swprintf_s(szMessage, L"Text Set started" );		break;
	case COSDChangeThread::MESSAGE_ID_OSD_SET_END		:	swprintf_s(szMessage, L"Text Set completed" );		break;
	case COSDChangeThread::MESSAGE_ID_OSD_SET_ERROR		:	swprintf_s(szMessage, L"Text Set error !!" );		break;
	case COSDChangeThread::MESSAGE_ID_OSD_SET_ERROR_AUTH:	swprintf_s(szMessage, L"Text Set error(Auth)!!" );	break;
	}

	LV_ITEM item;
	memset(&item, 0, sizeof(item));

	if( 0 < wcslen( szMessage ) )
	{
		item.mask		= LVIF_TEXT;
		item.iItem		= iItemIndex;
		item.iSubItem	= SUBITEM_STATUS;
		item.pszText	= (LPTSTR)(LPCTSTR)szMessage;
		m_ctlListDevice.SetItem(&item);
	}

	if( COSDChangeThread::MESSAGE_ID_OSD_SET_END		== iMessageID ||
		COSDChangeThread::MESSAGE_ID_OSD_SET_ERROR		== iMessageID ||
		COSDChangeThread::MESSAGE_ID_OSD_SET_ERROR_AUTH	== iMessageID )
	{
		delete m_aSettingThread[iItemIndex];
		m_aSettingThread[iItemIndex]	= NULL;

		int		iCount	= 0;
		for( int i=0; i<m_aSettingThread.GetCount(); i++ )
		{
			if( NULL != m_aSettingThread[i] )
			{
				iCount++;
			}
		}

		if( 0 == iCount )
		{
			EnableControlMain(TRUE);

			//SetDlgItemText( IDC_USERID	, L"" );
			//SetDlgItemText( IDC_PASSWORD, L"" );
		}
	}

	return 0;
}

void COSDChangeDlg::CheckArp(int iAdapterID)
{
	CARP		arp;
	unsigned char	cIPAddress[4];
	arpTable		Table[256];
	int				Counter, TableLength	= sizeof(Table)/sizeof(arpTable);

	Counter	= arp.GetEntries(Table, TableLength, iAdapterID);

	for( int i=0; i<m_nScanInfoCnt; i++ )
	{
		arp.ParseIPAddress(m_pScanInfo[i].szAddr, cIPAddress[0], cIPAddress[1], cIPAddress[2], cIPAddress[3] );

		for( int iTable=0; iTable<TableLength; iTable++ )
		{
			if( 0				!= Table[iTable].Type			&&
				cIPAddress[0]	== Table[iTable].IPAddress[0]	&&
				cIPAddress[1]	== Table[iTable].IPAddress[1]	&&
				cIPAddress[2]	== Table[iTable].IPAddress[2]	&&
				cIPAddress[3]	== Table[iTable].IPAddress[3]	) 
			{
				arp.EditEntry( Table[iTable].IPAddress, Table[iTable].MACAddress, 2, iAdapterID );
				break;
			}
		}
	}
}

int	COSDChangeDlg::GetSelectScanInfoCount()
{
	return m_nScanInfoCnt;
}

BOOL COSDChangeDlg::SetScaninfo( int iCount, SCAN_INFO* pScanInfo, int iAdapterID)
{
	if( m_pScanInfo )
	{
		delete [] m_pScanInfo;
		m_pScanInfo	= NULL;
	}

	CString strModelType;
	m_iAdapterID	= iAdapterID;
	m_nScanInfoCnt	= 0;
	m_pScanInfo		= new SCAN_INFO[iCount];

	for( int i=0; i<iCount; i++ )
	{
		if(pScanInfo[i].nExtraFieldCount)
		{
			strModelType = pScanInfo[i]._ReadValue(L"Model Type");

			if( 0 <= strModelType.Find(L"Mi") 	|| 
				0 <= strModelType.Find(L"M2i") 	|| 
				0 <= strModelType.Find(L"M3i") 	|| 
				0 <= strModelType.Find(L" i") 	|| 
				0 <= strModelType.Find(L"NVS102") )
			{
				// Next Chip 시리즈는 기능 적용시키지 않는다. 
			}
			else
			{
				m_pScanInfo[m_nScanInfoCnt] = pScanInfo[i];
				m_nScanInfoCnt	++;
			}
		}
	}

	if( 0 == m_nScanInfoCnt )
		return FALSE;

	return TRUE;
}

void COSDChangeDlg::OnBnClickedButtonCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnCancel();
}

void COSDChangeDlg::OnEnChangeEditOverlaytext()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.

	WCHAR	szOverlayText[32]		= {0};
	WCHAR	szOverlayTextSet[32]	= {0};
	int		iInsertPos				= 0;

	GetDlgItemText( IDC_EDIT_OVERLAYTEXT, szOverlayText, 17 );

	for( int i=0; i<(int)wcslen(szOverlayText); i++ )
	{
		if( ( L'0' <= szOverlayText[i] && L'9' >= szOverlayText[i] ) ||
			( L'a' <= szOverlayText[i] && L'z' >= szOverlayText[i] ) ||
			( L'A' <= szOverlayText[i] && L'Z' >= szOverlayText[i] ) ||
			L'#' == szOverlayText[i] ||	L'$' == szOverlayText[i] ||
			L'&' == szOverlayText[i] ||	L'+' == szOverlayText[i] ||
			L'(' == szOverlayText[i] ||	L')' == szOverlayText[i] ||
			L'_' == szOverlayText[i] ||	L'-' == szOverlayText[i] ||
			L'\\' == szOverlayText[i]|| L'.' == szOverlayText[i] ||
			L':' == szOverlayText[i] ||	L';' == szOverlayText[i] ||
			L'/' == szOverlayText[i] || L' ' == szOverlayText[i] )		 
		{
			szOverlayTextSet[iInsertPos]	= szOverlayText[i];
			iInsertPos++;
		}
		else
		{

		}
	}

	if( 0 != wcscmp(szOverlayTextSet, szOverlayText) )
	{
		SetDlgItemText( IDC_EDIT_OVERLAYTEXT, szOverlayTextSet );
		GetDlgItem( IDC_EDIT_OVERLAYTEXT)->SetFocus();
	}

	for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
	{
		m_ctlListDevice.SetEditText( i, SUBITEM_SETTEXT, szOverlayTextSet );
	}
}

void COSDChangeDlg::OnEnChangeUserid()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szUserID[32]	= {0};
	GetDlgItemText( IDC_USERID, szUserID, 32 );

	for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
	{
		m_ctlListDevice.SetEditText( i, SUBITEM_USER_ID, szUserID );
	}
}

void COSDChangeDlg::OnEnChangePassword()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szUserPW[32]	= {0};
	GetDlgItemText( IDC_PASSWORD, szUserPW, 32 );

	for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
	{
		m_ctlListDevice.SetEditText( i, SUBITEM_USER_PW, szUserPW );
	}
}

void COSDChangeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if( GetSafeHwnd() && m_ctlListDevice.GetSafeHwnd() )
	{
		m_ctlListDevice.MoveWindow( 10, 107, cx-20, cy-10-107-30 );
		m_ctlButtonApply.MoveWindow( cx/2-80, cy-21-10, 73, 21 );
		m_ctlButtonCancel.MoveWindow( cx/2+3, cy-21-10, 73, 21 );
	}
}

BOOL COSDChangeDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE) // by pass ESC key
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
