// IPChangeDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IPChangeDlg2.h"
//#include "NetScanVision.h"

IMPLEMENT_DYNAMIC(CIPChangeDlg2, CDialog)

CIPChangeDlg2::CIPChangeDlg2(CWnd* pParent /*=NULL*/)
: CDialog(CIPChangeDlg2::IDD, pParent)
, m_strMACAddress(L"")
, m_strIPAddress(L"")
, m_strGatewayAddress(L"")
, m_nStreamPort(0)
, m_nHTTPPort(0)
, m_nScanCount(0)
, m_nVersion(1)
, m_nIsDHCP(0)
, m_nCurrentIsDHCP(0)
, m_strSubnetMask(L"255.255.255.0")
, m_strID(_T(""))
, m_strPassword(_T(""))
{
	m_nScanInfoCnt		= 0;
	m_pScanInfo			= NULL;

	m_nIPChangeEnd		= 0;
	m_nSelScanInfoCnt	= 0;
	m_pSelScanInfo		= NULL;

	m_pScanner			= NULL;

	m_pListItem			= NULL;
	m_hMutexScanInfo	= NULL;

	m_iCheckCount		= 0;
}

CIPChangeDlg2::~CIPChangeDlg2()
{
	for( int iItem=0; iItem<m_aSettingThread.GetCount(); iItem++ )
	{
		delete m_aSettingThread[iItem];
		m_aSettingThread[iItem]	= NULL;
	}

	m_aSettingThread.RemoveAll();

	if( m_pScanner )
	{
		m_pScanner->StopScan();
	}

	if( m_pScanInfo )
	{
		delete [] m_pScanInfo;
		m_pScanInfo		= NULL;
	}

	m_nScanInfoCnt	= 0;

	if(m_pSelScanInfo)
	{
		delete [] m_pSelScanInfo;
		m_pSelScanInfo = NULL;
	}

	m_nSelScanInfoCnt	= 0;

	if( m_pListItem )
	{
		PLISTITEM	pItems			= NULL;
		pItems			= m_pListItem->GetList();
		while( pItems )
		{
			delete (SCAN_INFO*)pItems->pData;
			pItems->pData	= NULL;
			pItems = pItems->pNext;
		}

		delete m_pListItem;
		m_pListItem	= NULL;
	}

	if( m_hMutexScanInfo )
	{
		CloseHandle(m_hMutexScanInfo);
		m_hMutexScanInfo	= NULL;
	}
}

void CIPChangeDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_START_IPADDR, m_ctrlIPAddress);
	DDX_Control(pDX, IDC_GWADDR, m_ctrlGatewayAddress);
	DDX_Text(pDX, IDC_STREAM_PORT_EDIT, m_nStreamPort);
	DDX_Text(pDX, IDC_HTTP_PORT_EDIT, m_nHTTPPort);
	DDX_Control(pDX, IDC_SUBNETMASK, m_ctrlSubnetMask);
	DDX_Text(pDX, IDC_USERID, m_strID);
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Control(pDX, IDC_SVR_LIST, m_cSvrList);
	DDX_Control(pDX, IDC_APPLY, m_ctlButtonApply);
	DDX_Control(pDX, IDCANCEL, m_ctlButtonCancel);
	DDX_Control(pDX, IDC_CHECK_SAMEIP, m_ctlCheckSameIP);
	DDV_MinMaxUInt(pDX, m_nStreamPort, 1, 65535);
	DDV_MinMaxUInt(pDX, m_nHTTPPort, 1, 65535);
}


BEGIN_MESSAGE_MAP(CIPChangeDlg2, CDialog)
//	ON_BN_CLICKED(IDC_BUT_RESET, &CIPChangeDlg2::OnBnClickedButReset)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SVR_LIST, &CIPChangeDlg2::OnLvnItemchangedSvrList)
	ON_EN_KILLFOCUS(IDC_START_IPADDR, &CIPChangeDlg2::RefreshData)
	ON_EN_KILLFOCUS(IDC_STREAM_PORT_EDIT, &CIPChangeDlg2::RefreshData)
	ON_EN_KILLFOCUS(IDC_HTTP_PORT_EDIT, &CIPChangeDlg2::RefreshData)
	ON_EN_CHANGE(IDC_PASSWORD, &CIPChangeDlg2::OnEnChangePassword)
	ON_EN_CHANGE(IDC_USERID, &CIPChangeDlg2::OnEnChangeUserid)
	ON_EN_CHANGE(IDC_STREAM_PORT_EDIT, &CIPChangeDlg2::OnEnChangeStreamPortEdit)
	ON_EN_CHANGE(IDC_HTTP_PORT_EDIT, &CIPChangeDlg2::OnEnChangeHttpPortEdit)
	ON_MESSAGE(WM_IPCHANGE_MESSAGE, &CIPChangeDlg2::OnIPChangeMessage)
	ON_MESSAGE(WM_SCAN_MSG, &CIPChangeDlg2::OnScanMsg)
	ON_BN_CLICKED(IDC_NET_DHCP, &CIPChangeDlg2::OnBnClickedNetDhcp)
	ON_BN_CLICKED(IDC_NET_STATIC2, &CIPChangeDlg2::OnBnClickedNetStatic2)
	ON_BN_CLICKED(IDC_CHECK_SAMEIP, &CIPChangeDlg2::OnBnClickedCheckSameip)
	ON_BN_CLICKED(IDC_APPLY, &CIPChangeDlg2::OnBnClickedApply)
	ON_BN_CLICKED(IDCANCEL, &CIPChangeDlg2::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

// CIPChangeDlg2 메시지 처리기입니다.

BOOL CIPChangeDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_editMACAddr.SetPromptSymbol(_T('0'));
	m_editMACAddr.SetMask(_T("hh:hh:hh:hh:hh:hh"));

	GetClientRect(&m_rcWnd);
	CString str; str.LoadString(IDS_IP_CHANGE);

	GetDlgItem(IDC_NET_DHCP)->EnableWindow(TRUE);
	GetDlgItem(IDC_SUBNETMASK)->EnableWindow(TRUE);
	GetDlgItem(IDC_USERID)->EnableWindow(TRUE);
	GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);

	ASSERT(m_pSelScanInfo != NULL); // 다이얼로그 실행 위치에서 메모리 할당/초기화해줘야 한다

	// data setting before show dialog
	// show first item
	m_strMACAddress			= m_pSelScanInfo[0].szMAC;
	m_strIPAddress			= m_pSelScanInfo[0].szAddr;
	m_strGatewayAddress		= m_pSelScanInfo[0].szGateWay;
	m_nStreamPort			= m_pSelScanInfo[0].nStreamPort;
	m_nHTTPPort				= m_pSelScanInfo[0].nHTTPPort;
	m_nVersion				= m_pSelScanInfo[0].version;
	m_strSubnetMask			= m_pSelScanInfo[0].szSubnetMask;
	m_nIsDHCP				= m_pSelScanInfo[0].cIsDHCP;
	m_nCurrentIsDHCP		= m_nIsDHCP;

	if(m_strSubnetMask == L"")
		m_strSubnetMask = L"255.255.255.0";

	m_strOrgIPAddress		= m_strIPAddress;
	m_strOrgGatewayAddress	= m_strGatewayAddress;
	m_strOrgSubnetMask		= m_strSubnetMask;
	m_nOrgStreamPort		= m_nStreamPort;
	m_nOrgHTTPPort			= m_nHTTPPort;

	m_strCurIPAddress		= m_strIPAddress;
	m_strCurGatewayAddress	= m_strGatewayAddress;
	m_strCurSubnetMask		= m_strSubnetMask;
	m_nCurStreamPort		= m_nStreamPort;
	m_nCurHTTPPort			= m_nHTTPPort;

	OnBnClickedButReset();

	//////////////////////////////////////////////////////////////////////////
	// 2010-08-26 hkeins server scan
	// server list column initialize
	// ----------------------------------------------------------------------
	// 0    1             2       3                  4             5
	// IP | MAC Address | Model | Firmware version | Stream Port | Http Port 
	// ----------------------------------------------------------------------

	m_cSvrList.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	CString strItem; str.LoadString(IDS_ADDRESS);
	strItem = L"MAC " + str;
	m_cSvrList.InsertColumn(SUBITEM_MACADDRESS			, strItem, LVCFMT_CENTER, 110, 0 );	

	strItem = L"IP" + str;
	m_cSvrList.InsertColumn(SUBITEM_IPADDRESS			, strItem, LVCFMT_CENTER, 110, 0 );	
	
	strItem = L"Change IP" + str;
	m_cSvrList.InsertColumn(SUBITEM_TO_IPADDRESS		, strItem, LVCFMT_CENTER, 110, 0 );

	strItem = L"Change Gateway " + str;
	m_cSvrList.InsertColumn(SUBITEM_TO_GATEWAY			, strItem, LVCFMT_CENTER, 110, 0 );

	strItem.LoadString(IDS_IP_TYPE);
	m_cSvrList.InsertColumn(SUBITEM_IS_DHCP				, strItem, LVCFMT_CENTER, 50, 0 );	
	
	str.LoadString(IDS_PORT); 
	strItem.LoadString(IDS_STREAM);	
	strItem += L" "; strItem += str;
	m_cSvrList.InsertColumn(SUBITEM_PORTSTREAM			, strItem, LVCFMT_CENTER, 70, 0 );

	strItem = L"HTTP "; strItem += str;
	m_cSvrList.InsertColumn(SUBITEM_PORTHTTP			, strItem, LVCFMT_CENTER, 70, 0 );	
	
	strItem.LoadString(IDS_USER_ID);
	m_cSvrList.InsertColumn(SUBITEM_USER_ID				, strItem, LVCFMT_CENTER, 50, 0 );	

	strItem.LoadString(IDS_USER_PW);
	m_cSvrList.InsertColumn(SUBITEM_USER_PW				, strItem, LVCFMT_CENTER, 50, 0 );	

	strItem.LoadString(IDS_PROGRESS_STATUS);
	m_cSvrList.InsertColumn(SUBITEM_STATUS				, strItem, LVCFMT_CENTER, 110, 0 );	

	

/*	strItem.LoadString(IDS_SERVER_NAME);
	m_cSvrList.InsertColumn(SUBITEM_SYSTEMNAME			, strItem, LVCFMT_CENTER, 160, 0 );

	strItem.LoadString(IDS_MODEL);
	m_cSvrList.InsertColumn(SUBITEM_MODELTYPE			, strItem, LVCFMT_CENTER, 100, 0 );	
	
	strItem.LoadString(IDS_FIRMWARE_VERSION);
	m_cSvrList.InsertColumn(SUBITEM_FIRMWAREVERSION		, strItem, LVCFMT_CENTER, 120, 0 );	
	
	strItem.LoadString(IDS_RESOLUTIONS);
	m_cSvrList.InsertColumn(SUBITEM_SUPPORTRESOLUTION	, strItem, LVCFMT_CENTER, 180, 0 );	
	
	strItem.LoadString(IDS_VIDEO_FORMAT);
	m_cSvrList.InsertColumn(SUBITEM_VIDEOFORMAT			, strItem, LVCFMT_CENTER, 100, 0 );	
	
	strItem.LoadString(IDS_ALARM_IN_COUNT);
	m_cSvrList.InsertColumn(SUBITEM_ALARMINCOUNT		, strItem, LVCFMT_CENTER, 120, 0 );	
	
	strItem.LoadString(IDS_ALARM_OUT_COUNT);
	m_cSvrList.InsertColumn(SUBITEM_ALARMOUTCOUNT		, strItem, LVCFMT_CENTER, 120, 0 );	
	
	strItem.LoadString( IDS_AUDIO_IN_COUNT);
	m_cSvrList.InsertColumn(SUBITEM_AUDIOINCOUNT		, strItem, LVCFMT_CENTER, 120, 0 );	
	
	strItem.LoadString(IDS_AUDIO_OUT_COUNT);
	m_cSvrList.InsertColumn(SUBITEM_AUDIOOUTCOUNT		, strItem, LVCFMT_CENTER, 120, 0 );
*/

	AddData();

	m_pListItem	= new CListItem;

	if( m_pScanner )
	{
		m_pScanner->SetNotifyWindow( this->GetSafeHwnd(), WM_SCAN_MSG );
		m_hMutexScanInfo	= CreateMutex(NULL, FALSE, NULL);
		m_pScanner->StartScan();
	}


#ifdef _DEFAULT_ID_PASSWORD_
	//{{ 2013-07-19 hkeins : 생산 부 요청으로 ID/Pasword 디폴트 값을 admin/1234로 넣었음
	m_strID       = L"admin";
	m_strPassword = L"1234";

	SetDlgItemText( IDC_USERID, m_strID );
	SetDlgItemText( IDC_PASSWORD, m_strPassword );
	//}}
#endif _DEFAULT_ID_PASSWORD_

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CIPChangeDlg2::SetScanner( CNetScanVision * pScanner )
{
	m_pScanner	= pScanner;
}

void CIPChangeDlg2::SetScanInfo( int iScanInfoCount, SCAN_INFO*	pScanInfo, int iSelScanInfoCount, SCAN_INFO*	pSelScanInfo)
{
	m_nSelScanInfoCnt	= iSelScanInfoCount;
	if( m_pSelScanInfo )
	{
		delete [] m_pSelScanInfo;
		m_pSelScanInfo	= NULL;
	}

	m_pSelScanInfo	= new SCAN_INFO[m_nSelScanInfoCnt];
	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_pSelScanInfo[i]	= pSelScanInfo[i];
		//memcpy( m_pSelScanInfo, pSelScanInfo, sizeof(SCAN_INFO)*m_nSelScanInfoCnt );
	}


	m_nScanInfoCnt	= iScanInfoCount;
	if( m_pScanInfo )
	{
		delete [] m_pScanInfo;
		m_pScanInfo	= NULL;
	}

	m_pScanInfo	= new SCAN_INFO[m_nScanInfoCnt];
	for( int i=0; i<m_nScanInfoCnt; i++ )
	{
		m_pScanInfo[i]	= pScanInfo[i];
//		memcpy( m_pScanInfo, pScanInfo, sizeof(SCAN_INFO)*m_nScanInfoCnt );
	}
}


void CIPChangeDlg2::AddData()
{
	CString	strToIPAddress;
	CString	strToIPAddressBody;
	int		nStartIP		= 0;
	CString strTemp;

	AfxExtractSubString(strToIPAddress, m_strIPAddress, 3, '.');
	strToIPAddressBody	= m_strIPAddress.Left( m_strIPAddress.GetLength()-strToIPAddress.GetLength() );

	nStartIP = _ttoi(strToIPAddress);

	//UpgradeList에 데이터 입력
	for(int i=0;i<m_nSelScanInfoCnt;i++)
	{
		// add information into UI
		LV_ITEM item;

		memset(&item, 0, sizeof(item));

		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = SUBITEM_MACADDRESS;
		item.pszText = m_pSelScanInfo[i].szMAC;
		m_cSvrList.InsertItem(&item);

		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_IPADDRESS;
		item.pszText = m_pSelScanInfo[i].szAddr;
		item.lParam = (LPARAM)&m_pSelScanInfo[i];
		m_cSvrList.SetItem(&item);

		strToIPAddress.Format( L"%s%d", strToIPAddressBody.GetBuffer(32), (nStartIP+i)%255 );

		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_TO_IPADDRESS;
		item.pszText = (LPTSTR)(LPCTSTR)strToIPAddress;
		item.lParam = (LPARAM)&m_pSelScanInfo[i];
		m_cSvrList.SetItem(&item);
		m_cSvrList.CreateEdit(i, SUBITEM_TO_IPADDRESS, FALSE );
		m_cSvrList.SetEditText(i, SUBITEM_TO_IPADDRESS, strToIPAddress );

		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_TO_GATEWAY;
		item.pszText = (LPTSTR)(LPCTSTR)m_strGatewayAddress;
		item.lParam = (LPARAM)&m_pSelScanInfo[i];
		m_cSvrList.SetItem(&item);
		//m_cSvrList.CreateEdit(i, SUBITEM_TO_GATEWAY, FALSE );
		//m_cSvrList.SetEditText(i, SUBITEM_TO_GATEWAY, m_strGatewayAddress );


		if(m_pSelScanInfo[i].cIsDHCP==0)
			strTemp.LoadString(IDS_STATIC);
		else
			strTemp.LoadString(IDS_DHCP);
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_IS_DHCP;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.Format(_T("%d"), m_pSelScanInfo[i].nStreamPort);
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_PORTSTREAM;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);
		m_cSvrList.CreateEdit(i, SUBITEM_PORTSTREAM, FALSE );
		m_cSvrList.SetEditText( i, SUBITEM_PORTSTREAM , strTemp );

		strTemp.Format(_T("%d"), m_pSelScanInfo[i].nHTTPPort);
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_PORTHTTP;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);
		m_cSvrList.CreateEdit(i, SUBITEM_PORTHTTP, FALSE );
		m_cSvrList.SetEditText( i, SUBITEM_PORTHTTP , strTemp );

		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_USER_ID;
		item.pszText = (LPTSTR)(LPCTSTR)L"";
		m_cSvrList.SetItem(&item);
		m_cSvrList.CreateEdit(i, SUBITEM_USER_ID, FALSE );

		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_USER_PW;
		item.pszText = (LPTSTR)(LPCTSTR)L"";
		m_cSvrList.SetItem(&item);
		m_cSvrList.CreateEdit(i, SUBITEM_USER_PW, TRUE );
	}

	return;
}

LRESULT CIPChangeDlg2::OnScanMsg(WPARAM wParam, LPARAM lParam)
{
	SCAN_INFO*	pScanInfo		= (SCAN_INFO*)wParam;
	SCAN_INFO*	pOldScanInfo	= NULL;
	PLISTITEM	pItems			= NULL;
	BOOL		bFindOldItem	= FALSE;

	if(wParam == NULL)
	{
		return 0L;
	}

	if( m_nVersion != pScanInfo->version )
	{
		// 선택되어 있는 버젼과 받은 버젼이 틀리다면 삭제한다. 
		delete pScanInfo; 
		return 0L;
	}

	if(pScanInfo)
	{
		if( TRUE == ScanInfoLock() )
		{
			bFindOldItem	= FALSE;
			pItems			= m_pListItem->GetList();
			while( pItems )
			{
				pOldScanInfo	= (SCAN_INFO*)pItems->pData;
				if( 0 == wcscmp( pScanInfo->szMAC, pOldScanInfo->szMAC ) )
				{
					// 이전에 저장된 동일한 MAC Address  가 있다면 삭제 시키고 
					delete pOldScanInfo;
					pItems->pData	= NULL;

					// 업데이트 시킨다.
					pItems->pData	= pScanInfo;
					bFindOldItem	= TRUE;

					pScanInfo		= NULL;
					break;
				}
				pItems	= pItems->pNext;
			}

			if( FALSE == bFindOldItem )
			{
				PLISTITEM	pNetItem = new LISTITEM;
				pNetItem->pData	= pScanInfo;
				m_pListItem->Insert( pNetItem );
				pScanInfo		= NULL;
			}

			ScanInfoUnLock();
		}
	}

	if( pScanInfo )
		delete pScanInfo; 

	return 0;
}

LRESULT CIPChangeDlg2::OnIPChangeMessage(WPARAM wParam, LPARAM lParam)
{
	WCHAR			szProgressMessage[MAX_PATH]		= {0};
	int				iMessage						= (int)LOWORD(wParam);
	int				iItem							= (int)HIWORD(wParam);
	SCAN_INFO*		pOldScanInfo					= NULL;
	PLISTITEM		pItems							= NULL;
	IPCHANGEITEM*	pChangeItem						= (IPCHANGEITEM*)lParam;

	switch( iMessage )
	{
	//case MESSAGE_SUCESS_CONNECT		:	swprintf_s(szProgressMessage, L"Connect Success" );			break;
	//case MESSAGE_SUCESS_LOGIN			:	swprintf_s(szProgressMessage, L"Login Success" );			break;
	//case MESSAGE_FAILED_CONNECT		:	swprintf_s(szProgressMessage, L"Failed( Connect )" );			break;
	//case MESSAGE_FAILED_LOGIN			:	swprintf_s(szProgressMessage, L"Failed( Login )" );				break;
	//case MESSAGE_FAILED_DUPLICATIONIP	:	swprintf_s(szProgressMessage, L"Failed( Duplication IP )" );	break;
	case MESSAGE_SUCESS_MAC				:	swprintf_s(szProgressMessage, L"Mac Check Success" );			break;
	case MESSAGE_CHANGE_COMPLETE		:	swprintf_s(szProgressMessage, L"Complete" );					break;
	case MESSAGE_FAILED_MAC				:	swprintf_s(szProgressMessage, L"Failed( Mac Check )" );			break;
	case MESSAGE_FAILED_TIMEOUT			:	swprintf_s(szProgressMessage, L"Failed( Check Timeout )" );		break;
	case MESSAGE_FAILED_REQUEST_IPCHANGE:	swprintf_s(szProgressMessage, L"Failed( Invalid Version )" );	break;
	case MESSAGE_REQUEST_IPCHANGE		:	swprintf_s(szProgressMessage, L"Request Change" );				break;
	}

	if( MESSAGE_CHANGE_COMPLETE			== iMessage ||
		MESSAGE_FAILED_MAC				== iMessage ||
		MESSAGE_FAILED_REQUEST_IPCHANGE	== iMessage )
	{
		m_nIPChangeEnd++;
		if( m_nSelScanInfoCnt == m_nIPChangeEnd )
		{
			//GetDlgItem(IDC_NET_STATIC2		)->EnableWindow(TRUE);
			//GetDlgItem(IDC_NET_DHCP			)->EnableWindow(TRUE);
			//GetDlgItem(IDC_START_IPADDR		)->EnableWindow(TRUE);
			//GetDlgItem(IDC_GWADDR			)->EnableWindow(TRUE);
			//GetDlgItem(IDC_SUBNETMASK		)->EnableWindow(TRUE);
			//GetDlgItem(IDC_STREAM_PORT_EDIT	)->EnableWindow(TRUE);
			//GetDlgItem(IDC_HTTP_PORT_EDIT	)->EnableWindow(TRUE);
			//GetDlgItem(IDC_USERID			)->EnableWindow(TRUE);
			//GetDlgItem(IDC_PASSWORD			)->EnableWindow(TRUE);
			//GetDlgItem(IDC_APPLY			)->EnableWindow(TRUE);

			//SetDHCPMode(m_nIsDHCP);

			//for( int i=0; i<m_nSelScanInfoCnt; i++ )
			//{
			//	m_cSvrList.SetEditEnable( i, SUBITEM_TO_IPADDRESS	, TRUE );
			//	m_cSvrList.SetEditEnable( i, SUBITEM_USER_ID		, TRUE );
			//	m_cSvrList.SetEditEnable( i, SUBITEM_USER_PW		, TRUE );
			//}
		}

		m_cSvrList.SetItemText(iItem, SUBITEM_STATUS, szProgressMessage );
	}
	else if( MESSAGE_SCAN_START == iMessage )
	{
		m_pScanner->StartScan();
	}
	else if( MESSAGE_CHECK_IPCHANGE == iMessage )
	{
		if( m_pScanner )
		{
			if( VERSION_1 == m_nVersion )
				m_pScanner->SendScanRequest();
			else if( VERSION_2 == m_nVersion )
				m_pScanner->SendScanRequestExt();
		}

		m_iCheckCount = (m_iCheckCount+1)%5;

		swprintf_s(szProgressMessage, L"Check IP" );
		for( int i=0; i<m_iCheckCount; i++ )
		{
			wcscat_s( szProgressMessage, L"." );
		}

		if( TRUE == ScanInfoLock() )
		{
			if( MESSAGE_REQUEST_IPCHANGE == pChangeItem->iStatus )
			{
				pItems			= m_pListItem->GetList();
				while( pItems )
				{
					pOldScanInfo	= (SCAN_INFO*)pItems->pData;
					if( 0 == wcscmp( pChangeItem->szMACAddress, pOldScanInfo->szMAC ) )
					{
						if( TRUE == pChangeItem->nToIsDHCP )
						{
							if( pChangeItem->nToIsDHCP == pOldScanInfo->cIsDHCP )
							{
								pChangeItem->iStatus	= MESSAGE_CHANGE_COMPLETE;
								PostMessage( WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_CHANGE_COMPLETE, iItem) , 0);
								break;
							}
						}
						else
						{
							if( 0 == wcscmp( pChangeItem->szToIPAddress	, pOldScanInfo->szAddr			) &&
								0 == wcscmp( pChangeItem->szToGWAddress	, pOldScanInfo->szGateWay		) &&
								0 == wcscmp( pChangeItem->szToSubnetMask, pOldScanInfo->szSubnetMask	) &&
								pChangeItem->nToIsDHCP		== pOldScanInfo->cIsDHCP	&&
								pChangeItem->iToHttpPort	== pOldScanInfo->nHTTPPort	&&
								pChangeItem->iToStreamPort	== pOldScanInfo->nStreamPort )
							{
								pChangeItem->iStatus	= MESSAGE_CHANGE_COMPLETE;
								PostMessage( WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_CHANGE_COMPLETE, iItem) , 0);
								break;
							}
						}
					}

					pItems	= pItems->pNext;
				}

				if( MESSAGE_CHANGE_COMPLETE != pChangeItem->iStatus )
				{
					if( pChangeItem->tTimeOut < GetCurrentTime() )
					{
						pChangeItem->iStatus	= MESSAGE_FAILED_TIMEOUT;
						PostMessage( WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_FAILED_TIMEOUT , iItem), NULL);
					}
				}

				m_cSvrList.SetItemText(iItem, SUBITEM_STATUS, szProgressMessage );
			}

			ScanInfoUnLock();
		}
	}
	else
	{
		m_cSvrList.SetItemText(iItem, SUBITEM_STATUS, szProgressMessage );
		TRACE( szProgressMessage );
		TRACE( L"\n" );
	}

	return 0;
}

void CIPChangeDlg2::RefreshData()
{
	CString strTemp;
	CString strStartIP = L"";
	CString strTmpStartIP = L"";
	int		nStartIP = 0;
	int		nCurIP = 0;
	int		nPos = 0;

	GetDlgItemText(IDC_START_IPADDR	, m_strIPAddress);
	GetDlgItemText(IDC_GWADDR		, m_strGatewayAddress);
	GetDlgItemText(IDC_SUBNETMASK	, m_strSubnetMask);
	m_nStreamPort = GetDlgItemInt(IDC_STREAM_PORT_EDIT);
	m_nHTTPPort   = GetDlgItemInt(IDC_HTTP_PORT_EDIT);

	AfxExtractSubString(strStartIP, m_strIPAddress, 3, '.'); 

	nStartIP = _ttoi(strStartIP);

	strTmpStartIP = m_strIPAddress;

	//UpgradeList에 수정된 데이터 재입력

	for(int i=0;i<m_nSelScanInfoCnt;i++)
	{
		// add information into UI
		LV_ITEM item;
		memset(&item, 0, sizeof(item));

		item.lParam = (LPARAM)&m_pSelScanInfo[i];

//		TRACE(L"###### i = %d, strTmpStartIP = %s, m_strCurIPAddress = %s\n",i, strTmpStartIP, m_strCurIPAddress);
		if(m_strCurIPAddress.Compare(strTmpStartIP) != 0)
		{
			nCurIP = nStartIP++;

			if(nCurIP > 254)
			{
				nStartIP = 1;
			}

			strTemp = m_strIPAddress;
			nPos = strTemp.ReverseFind('.');
			strTemp.Delete(nPos, strTemp.GetLength()-nPos); 
			m_strIPAddress.Format(_T("%s.%d"), strTemp, nCurIP);

			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = i;
			item.iSubItem = SUBITEM_IPADDRESS;
			item.pszText = (LPTSTR)(LPCTSTR)m_strIPAddress;
			m_cSvrList.SetItem(&item);
		}

		if(m_nCurStreamPort != m_nStreamPort)
		{
			strTemp.Format(_T("%d"), m_nStreamPort);
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_PORTSTREAM;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cSvrList.SetItem(&item);
		}

		if(m_nCurHTTPPort != m_nHTTPPort)
		{
			strTemp.Format(_T("%d"), m_nHTTPPort);
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_PORTHTTP;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cSvrList.SetItem(&item);
		}
	}
	m_strCurIPAddress = strTmpStartIP;
	m_nCurStreamPort = m_nStreamPort;
	m_nCurHTTPPort = m_nHTTPPort;

	return;
}

void CIPChangeDlg2::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}

HBRUSH CIPChangeDlg2::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

LRESULT CIPChangeDlg2::OnMoveWnd(WPARAM wParam, LPARAM lParam)
{
	CRect rc; GetWindowRect(&rc);
	MoveWindow(rc.left - (LONG)wParam, rc.top - (LONG)lParam, rc.Width(),rc.Height());
	return 0L;
}

void CIPChangeDlg2::OnBnClickedButReset()
{
	SetDHCPMode(m_nIsDHCP); // DHCP - 0, Static - 1
	SetDlgItemInt(IDC_STREAM_PORT_EDIT	, m_nStreamPort			);
	SetDlgItemInt(IDC_HTTP_PORT_EDIT	, m_nHTTPPort			);
	SetDlgItemText(IDC_START_IPADDR		, m_strIPAddress		);
	SetDlgItemText(IDC_GWADDR			, m_strGatewayAddress	);
	SetDlgItemText(IDC_SUBNETMASK		, m_strSubnetMask		);
	SetDlgItemText(IDC_USERID			, L""					);
	SetDlgItemText(IDC_PASSWORD			, L""					);
}

void CIPChangeDlg2::OnDestroy()
{
	CDialog::OnDestroy();
	// FIX ME : not entered here why?
}

void CIPChangeDlg2::OnBnClickedNetDhcp()
{
	m_nCurrentIsDHCP = 1;
	SetDHCPMode(TRUE);
}

void CIPChangeDlg2::OnBnClickedNetStatic2()
{
	m_nCurrentIsDHCP = 0;
	SetDHCPMode(FALSE);
}


BOOL CIPChangeDlg2::ScanInfoLock()
{
	if( WAIT_TIMEOUT == ::WaitForSingleObject(m_hMutexScanInfo, 5000 ) )
	{
		return FALSE;
	}

	return TRUE;
}

void CIPChangeDlg2::ScanInfoUnLock()
{
	ReleaseMutex(m_hMutexScanInfo);
}

void CIPChangeDlg2::SetDHCPMode(BOOL bMode)
{
	CButton *pButton[2];
	pButton[0] = (CButton*)GetDlgItem(IDC_NET_DHCP);
	pButton[1] = (CButton*)GetDlgItem(IDC_NET_STATIC2);

	if(pButton[0] && pButton[1])
	{
		pButton[0]->SetCheck(bMode);
		pButton[1]->SetCheck(!bMode);
	}
	else
	{
		ASSERT(0);
	}

	GetDlgItem(IDC_STATIC_START_IPADDR)->EnableWindow(!bMode);
	GetDlgItem(IDC_START_IPADDR)->EnableWindow(!bMode);
	GetDlgItem(IDC_STATIC_GATEWAY)->EnableWindow(!bMode);
	GetDlgItem(IDC_GWADDR)->EnableWindow(!bMode);
	GetDlgItem(IDC_STATIC_MASK)->EnableWindow(!bMode);
	GetDlgItem(IDC_SUBNETMASK)->EnableWindow(!bMode);

	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_cSvrList.SetEditEnable( i, SUBITEM_TO_IPADDRESS, !bMode );
	}
}

//BOOL CIPChangeDlg2::_ValidateMacAddress(CString strMacAddress)
//{
//	int i;
//
//	// 빈 경우
//	if(strMacAddress.Compare(L"") == 0)
//		return FALSE;
//
//	// 자리수가 넘어간 경우
//	if(strMacAddress.GetLength() != 17)
//		return FALSE;
//
//	// 허용되지 않는 문자가 들어간 경우
//	WCHAR ch;
//
//	for(i = 0; i < strMacAddress.GetLength(); i++)
//	{	
//		ch = strMacAddress.GetAt(i);
//		if((ch == L':') ||
//			(ch >= L'0' && ch <= L'9') ||
//			(ch >= L'a' && ch <= L'f') ||
//			(ch >= L'A' && ch <= L'F'))
//		{
//			// OK bypass
//		}
//		else
//		{
//			return FALSE;
//		}
//
//	}
//	// 요런 포멧이 아닌 경우
//	// 00:00:00:00:00:00
//	int nPos = 0;
//	int nColumnCount = 0;
//	while(nPos >= 0)
//	{
//		nPos = strMacAddress.Find(L':', nPos + 1);
//		if(nPos >= 0) nColumnCount++;
//	}
//
//	if(nColumnCount != 5) // : 개수가 5개가 아닌 경우
//		return FALSE;
//
//	return TRUE;
//}

void CIPChangeDlg2::OnEnChangeStartMacaddr()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CIPChangeDlg2::OnLvnItemchangedSvrList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	*pResult = 0;
}

void CIPChangeDlg2::OnEnChangeUserid()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetDlgItemText( IDC_USERID, m_strID );

	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_cSvrList.SetEditText( i, SUBITEM_USER_ID, m_strID );
	}
}

void CIPChangeDlg2::OnEnChangePassword()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetDlgItemText( IDC_PASSWORD, m_strPassword );

	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_cSvrList.SetEditText( i, SUBITEM_USER_PW, m_strPassword );
	}
}

void CIPChangeDlg2::OnEnChangeStreamPortEdit()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szPortStream[32]	= {0};

	m_nStreamPort	= GetDlgItemInt( IDC_STREAM_PORT_EDIT );
	if( 65535 < m_nStreamPort )
	{
		m_nStreamPort	= 65535;
		SetDlgItemInt(IDC_STREAM_PORT_EDIT, m_nStreamPort);
	}

	swprintf_s(szPortStream, L"%d", m_nStreamPort );

	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_cSvrList.SetEditText( i, SUBITEM_PORTSTREAM, szPortStream );
	}
}

void CIPChangeDlg2::OnEnChangeHttpPortEdit()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szPortHttp[32]	= {0};

	m_nHTTPPort	= GetDlgItemInt( IDC_HTTP_PORT_EDIT );
	if( 65535 < m_nHTTPPort )
	{
		m_nHTTPPort	= 65535;
		SetDlgItemInt(IDC_HTTP_PORT_EDIT, m_nHTTPPort);
	}
	swprintf_s(szPortHttp, L"%d", m_nHTTPPort );

	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_cSvrList.SetEditText( i, SUBITEM_PORTHTTP, szPortHttp );
	}
}


BOOL CIPChangeDlg2::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	WORD	wCtrlID		= LOWORD(wParam);
	WORD	wMessage	= HIWORD(wParam);
	
	if( IDC_START_IPADDR == wCtrlID	)
	{
		TRACE( L"IDC_START_IPADDR = %d\n", wMessage );

		if( EN_CHANGE == wMessage )
		{
			GetDlgItemText( IDC_START_IPADDR, m_strIPAddress );

			CString	strToIPAddress;
			CString	strToIPAddressBody;
			int		nStartIP		= 0;
			CString strTemp;
		
			AfxExtractSubString(strToIPAddress, m_strIPAddress, 3, '.');
			strToIPAddressBody	= m_strIPAddress.Left( m_strIPAddress.GetLength()-strToIPAddress.GetLength() );
		
			nStartIP = _ttoi(strToIPAddress);
		
			for( int i=0; i<m_nSelScanInfoCnt; i++ )
			{
				strToIPAddress.Format( L"%s%d", strToIPAddressBody.GetBuffer(32), (nStartIP+i)%255 );
				m_cSvrList.SetEditText( i, SUBITEM_TO_IPADDRESS, strToIPAddress );
			}
		}
	}
	else if( IDC_GWADDR == wCtrlID	)
	{
		LV_ITEM item;
		TRACE( L"IDC_GWADDR = %d\n", wMessage );

		if( EN_CHANGE == wMessage )
		{
			GetDlgItemText( IDC_GWADDR, m_strGatewayAddress );

			for( int i=0; i<m_nSelScanInfoCnt; i++ )
			{
				memset(&item, 0, sizeof(item));
				item.mask = LVIF_TEXT;
				item.iItem = i;
				item.iSubItem = SUBITEM_TO_GATEWAY;
				item.pszText = (LPTSTR)(LPCTSTR)m_strGatewayAddress;
				m_cSvrList.SetItem(&item);
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CIPChangeDlg2::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	if( GetSafeHwnd() && m_cSvrList.GetSafeHwnd() && m_ctlButtonApply.GetSafeHwnd() && m_ctlButtonCancel.GetSafeHwnd() )
	{
		m_cSvrList.MoveWindow( 10, 165, cx-20, cy-10-165-30 );
		m_ctlButtonApply.MoveWindow( cx/2-80, cy-21-10, 73, 21 );
		m_ctlButtonCancel.MoveWindow( cx/2+3, cy-21-10, 73, 21 );
	}
}

void CIPChangeDlg2::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	lpMMI->ptMinTrackSize.x = 869;
	lpMMI->ptMinTrackSize.y = 424;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CIPChangeDlg2::OnBnClickedCheckSameip()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItemText( IDC_START_IPADDR, m_strIPAddress );

	if( 1 == m_ctlCheckSameIP.GetCheck() )
	{
		for( int i=0; i<m_nSelScanInfoCnt; i++ )
		{
			m_cSvrList.SetEditText( i, SUBITEM_TO_IPADDRESS, m_strIPAddress );
		}
	}
	else
	{
		CString	strToIPAddress;
		CString	strToIPAddressBody;
		int		nStartIP		= 0;
		CString strTemp;

		AfxExtractSubString(strToIPAddress, m_strIPAddress, 3, '.');
		strToIPAddressBody	= m_strIPAddress.Left( m_strIPAddress.GetLength()-strToIPAddress.GetLength() );

		nStartIP = _ttoi(strToIPAddress);

		for( int i=0; i<m_nSelScanInfoCnt; i++ )
		{
			strToIPAddress.Format( L"%s%d", strToIPAddressBody.GetBuffer(32), (nStartIP+i)%255 );
			m_cSvrList.SetEditText( i, SUBITEM_TO_IPADDRESS, strToIPAddress );
		}
	}
}

void CIPChangeDlg2::OnBnClickedApply()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strProgressMessage;
	m_nStreamPort	= GetDlgItemInt(IDC_STREAM_PORT_EDIT);
	m_nHTTPPort		= GetDlgItemInt(IDC_HTTP_PORT_EDIT);
	GetDlgItemText(IDC_START_IPADDR	, m_strIPAddress		);
	GetDlgItemText(IDC_GWADDR		, m_strGatewayAddress	);
	GetDlgItemText(IDC_SUBNETMASK	, m_strSubnetMask		);

	if(m_strIPAddress == L"")
	{
		AfxMessageBox(_T("Please insert a Start IP Address..."), MB_ICONWARNING);
		GetDlgItem(IDC_START_IPADDR)->SetFocus();
		return;
	}
	else if(m_strGatewayAddress == L"")
	{
		AfxMessageBox(_T("Please insert a Gateway Address..."), MB_ICONWARNING);
		GetDlgItem(IDC_GWADDR)->SetFocus();
		return;
	}
	else if(m_strSubnetMask == L"")
	{
		AfxMessageBox(_T("Please insert a SubnetMask..."), MB_ICONWARNING);
		GetDlgItem(IDC_SUBNETMASK)->SetFocus();
		return;
	}
	else if(m_nStreamPort == 0)
	{
		AfxMessageBox(_T("Please insert a Stream Port..."), MB_ICONWARNING);
		GetDlgItem(IDC_STREAM_PORT_EDIT)->SetFocus();
		return;
	}
	else if(m_nHTTPPort == 0)
	{
		AfxMessageBox(_T("Please insert a HTTP Port..."), MB_ICONWARNING);
		GetDlgItem(IDC_STREAM_PORT_EDIT)->SetFocus();
		return;
	}
	else if(m_strID.IsEmpty())
	{
		CString str;
		str.LoadString(IDS_ENTER_LOGIN_INFO);
		AfxMessageBox(str, MB_ICONWARNING);
		GetDlgItem(IDC_USERID)->SetFocus();
		return;
	}
	else if(m_strPassword.IsEmpty())
	{
		CString str;
		str.LoadString(IDS_ENTER_PASSWORD_INFO);
		AfxMessageBox(str, MB_ICONWARNING);
		GetDlgItem(IDC_PASSWORD)->SetFocus();
		return;
	}

	CString receive = L"";
	CString strTemp=L"";

	DWORD	dwSubNetMask	= 0x00000000;
	DWORD	dwGatewayAdd	= 0x00000000;
	DWORD	dwIPAddress		= 0x00000000;
	CString strIPAddress;

	m_ctrlIPAddress.GetAddress(dwIPAddress);
	m_ctrlSubnetMask.GetAddress( dwSubNetMask );
	m_ctrlGatewayAddress.GetAddress( dwGatewayAdd );

	dwGatewayAdd	= dwGatewayAdd & dwSubNetMask;

	LPHOSTENT	lphostent		= NULL;
	ULONG		uAddr			= INADDR_NONE;
	BOOL		bSameIPRange	= TRUE;

	for(int i=0;i<m_nSelScanInfoCnt;i++)
	{
		strIPAddress	= m_cSvrList.GetEditText( i, SUBITEM_TO_IPADDRESS );

#ifdef _UNICODE
		char strHost[256] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, strIPAddress.GetBuffer(MAX_PATH), -1, strHost, sizeof(strHost), NULL, NULL );
#else
		LPCTSTR strHost = strIPAddress.GetBuffer(MAX_PATH);
#endif

		lphostent = gethostbyname( strHost );
		if( NULL != lphostent )
		{
			uAddr	= *((ULONG *) lphostent->h_addr_list[0]);
			uAddr	= ntohl( uAddr );

			if( dwGatewayAdd != ( uAddr & dwSubNetMask ) )
			{
				bSameIPRange	= FALSE;
				break;
			}
		}
		else
		{
			bSameIPRange	= FALSE;
			break;
		}
	}

	if( FALSE == bSameIPRange )
	{
		CString str;
		str.LoadString(IDS_DIFFERENT_BAND);
		AfxMessageBox(str, MB_ICONWARNING);
		GetDlgItem(IDC_PASSWORD)->SetFocus();
		return;
	}

	GetDlgItem(IDC_NET_STATIC2		)->EnableWindow(FALSE);
	GetDlgItem(IDC_NET_DHCP			)->EnableWindow(FALSE);
	GetDlgItem(IDC_START_IPADDR		)->EnableWindow(FALSE);
	GetDlgItem(IDC_GWADDR			)->EnableWindow(FALSE);
	GetDlgItem(IDC_SUBNETMASK		)->EnableWindow(FALSE);
	GetDlgItem(IDC_STREAM_PORT_EDIT	)->EnableWindow(FALSE);
	GetDlgItem(IDC_HTTP_PORT_EDIT	)->EnableWindow(FALSE);
	GetDlgItem(IDC_USERID			)->EnableWindow(FALSE);
	GetDlgItem(IDC_PASSWORD			)->EnableWindow(FALSE);
	GetDlgItem(IDC_APPLY			)->EnableWindow(FALSE);

	for( int i=0; i<m_nSelScanInfoCnt; i++ )
	{
		m_cSvrList.SetEditEnable( i, SUBITEM_TO_IPADDRESS	, FALSE );
		m_cSvrList.SetEditEnable( i, SUBITEM_USER_ID		, FALSE );
		m_cSvrList.SetEditEnable( i, SUBITEM_USER_PW		, FALSE );
	}			

	strProgressMessage	= L"";
	strProgressMessage.Format(_T("[Warning] Program is changing IP..."));
	//	SetDlgItemText(IDC_STC_PROCESSING, strProgressMessage);

	IPCHANGEITEM*		apIPChangeItem	= NULL;

	apIPChangeItem	= new IPCHANGEITEM[m_nSelScanInfoCnt];

	for(int iItem=0;iItem<m_nSelScanInfoCnt;iItem++)
	{
		m_strMACAddress = m_pSelScanInfo[iItem].szMAC;
		m_strMACAddress.MakeLower();

		m_strIPAddress		= m_cSvrList.GetEditText( iItem, SUBITEM_TO_IPADDRESS );
		swscanf_s( m_cSvrList.GetEditText( iItem, SUBITEM_PORTSTREAM	), L"%d", &m_nStreamPort);
		swscanf_s( m_cSvrList.GetEditText( iItem, SUBITEM_PORTHTTP		), L"%d", &m_nHTTPPort);
		m_strID				= m_cSvrList.GetEditText( iItem, SUBITEM_USER_ID );
		m_strPassword		= m_cSvrList.GetEditText( iItem, SUBITEM_USER_PW );
		m_nIsDHCP			= m_nCurrentIsDHCP;

		swprintf_s( apIPChangeItem[iItem].szMACAddress		, L"%s", m_strMACAddress );


		swprintf_s( apIPChangeItem[iItem].szFromIPAddress	, L"%s", m_pSelScanInfo[iItem].szAddr );
		swprintf_s( apIPChangeItem[iItem].szToIPAddress		, L"%s", m_strIPAddress );

		swprintf_s( apIPChangeItem[iItem].szFromGWAddress	, L"%s", m_pSelScanInfo[iItem].szGateWay );
		swprintf_s( apIPChangeItem[iItem].szToGWAddress		, L"%s", m_strGatewayAddress );

		swprintf_s( apIPChangeItem[iItem].szFromSubnetMask	, L"%s", m_pSelScanInfo[iItem].szSubnetMask );
		swprintf_s( apIPChangeItem[iItem].szToSubnetMask	, L"%s", m_strSubnetMask );

		swprintf_s( apIPChangeItem[iItem].szUserID			, L"%s", m_strID );
		swprintf_s( apIPChangeItem[iItem].szUserPW			, L"%s", m_strPassword );

		apIPChangeItem[iItem].nFromIsDHCP		= m_pSelScanInfo[iItem].cIsDHCP;
		apIPChangeItem[iItem].nToIsDHCP			= m_nIsDHCP;

		apIPChangeItem[iItem].iFromStreamPort	= m_pSelScanInfo[iItem].nStreamPort;
		apIPChangeItem[iItem].iToStreamPort		= m_nStreamPort;

		apIPChangeItem[iItem].iFromHttpPort		= m_pSelScanInfo[iItem].nHTTPPort;
		apIPChangeItem[iItem].iToHttpPort		= m_nHTTPPort;



		for(int i = 0 ; i < m_pSelScanInfo[iItem].nExtraFieldCount; i++)
		{
			if( 0 == wcscmp( L"Model Type", m_pSelScanInfo[iItem].pExtScanInfos[i].szCaption ) )
			{
				swprintf_s( apIPChangeItem[iItem].szModel	, L"%s", m_pSelScanInfo[iItem].pExtScanInfos[i].lpszValue );
				break;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// TO IP ADDRESS Check ///////////////////////////////////////////////////

		apIPChangeItem[iItem].iStatus		= MESSAGE_CLOSE_THREAD;
		//for( int i=0; i<m_nScanInfoCnt; i++ )
		//{
		//	if( 0 != wcscmp( m_pScanInfo[i].szMAC		, apIPChangeItem[iItem].szMACAddress  ) )
		//	{
		//		if( 0 == wcscmp( m_pScanInfo[i].szAddr		, apIPChangeItem[iItem].szToIPAddress ) &&
		//			0 == wcscmp( m_pScanInfo[i].szGateWay	, apIPChangeItem[iItem].szToGWAddress ) &&
		//			0 == wcscmp( m_pScanInfo[i].szSubnetMask, apIPChangeItem[iItem].szSubnetMask  ) &&
		//			m_pScanInfo[i].cIsDHCP == apIPChangeItem[iItem].nIsDHCP							)
		//		{
		//			apIPChangeItem[iItem].iStatus		= MESSAGE_FAILED_DUPLICATIONIP;
		//			break;
		//		}
		//	}
		//}


		// TO IP ADDRESS Check ///////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////


		m_strID.ReleaseBuffer();
		m_strPassword.ReleaseBuffer();

		// wait a seconds progress dialog here
		// FIX ME : show dialog:!
	}

	m_iCheckCount		= 0;
	m_nIPChangeEnd		= 0;

	SCAN_INFO*	pScanInfo = NULL;
	for( int iItem=0; iItem<m_nSelScanInfoCnt; iItem++ )
	{
		pScanInfo = (SCAN_INFO*)m_cSvrList.GetItemData(iItem);

		m_aSettingThread[iItem]	= new CIPChangeThread();
		m_aSettingThread[iItem]->SetThreadInfo( this->GetSafeHwnd()	, m_nVersion, iItem, apIPChangeItem[iItem]	);
		m_aSettingThread[iItem]->StartIPChanage();

	}

	delete [] apIPChangeItem;
	apIPChangeItem	= NULL;
}

void CIPChangeDlg2::OnBnClickedCancel()
{
	if( 0 < m_nSelScanInfoCnt )
	{
		OnOK();
	}
	else
	{
		OnCancel();
	}
}