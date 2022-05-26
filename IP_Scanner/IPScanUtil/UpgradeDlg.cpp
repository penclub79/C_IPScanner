// UpgradeDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include <afxinet.h>
#include "UpgradeDlg.h"
#include "UpgradeDefine.h"

const UINT UPGRADE_WINDOW_MIN_WIDTH  = 825;
const UINT UPGRADE_WINDOW_MIN_HEIGHT = 379;

CUpgradeDlg::CUpgradeDlg(CWnd* pParent /*=NULL*/)
: CDialog(CUpgradeDlg::IDD, pParent)
, m_strFilePath(L"")
, m_strFileName(L"")
, m_strID(_T(""))
, m_strPassword(_T(""))
, m_strModel(L"")
{
	m_bInit				= FALSE;
	m_pScanInfo			= NULL;
	m_iRebootCheck		= 0;
	m_nRetryUpgradeID	= 0;
}

CUpgradeDlg::~CUpgradeDlg()
{
	for(int i=0;i<m_aUpgradeThread.GetCount();i++)
	{
		if( NULL != m_aUpgradeThread[i] )
		{
			if( TYPE_TCP == m_aUpgradeThread[i]->iUpgradeType )
			{
				if( m_aUpgradeThread[i]->pUpgradeThreadTcp )
					delete m_aUpgradeThread[i]->pUpgradeThreadTcp;

				m_aUpgradeThread[i]->pUpgradeThreadTcp	= NULL;
			}
			else if( TYPE_HTTP == m_aUpgradeThread[i]->iUpgradeType )
			{
				if( m_aUpgradeThread[i]->pUpgradeThreadHttp )
				{
					m_aUpgradeThread[i]->pUpgradeThreadHttp->StopUpgrade();
					delete m_aUpgradeThread[i]->pUpgradeThreadHttp;
				}

				m_aUpgradeThread[i]->pUpgradeThreadHttp	= NULL;
			}

			if(m_aUpgradeThread[i])
			{
				delete m_aUpgradeThread[i];
				m_aUpgradeThread[i] = NULL;
			}
		}
	}

	m_aUpgradeThread.RemoveAll();
}

BEGIN_MESSAGE_MAP(CUpgradeDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_UPGRADE_RETRAY	, &CUpgradeDlg::OnRetryUpgrade)
	ON_BN_CLICKED(IDC_OPENFILE_BTN	, &CUpgradeDlg::OnBnClickedOpenFile)
	ON_BN_CLICKED(IDC_UPGRADE_BTN	, &CUpgradeDlg::OnBnClickedUpgrade)
	ON_BN_CLICKED(IDC_CLOSE			, &CUpgradeDlg::OnBnClickedClose)
	ON_CBN_SELCHANGE(IDC_MODEL_COMBO, &CUpgradeDlg::OnSelchangeCmbModel)
	ON_MESSAGE( WM_UPGRADE_MSG		, &CUpgradeDlg::OnUpgradeMsg )
	ON_MESSAGE( WM_RETRY_UPGRADE_MSG, &CUpgradeDlg::OnRetryUpgradeMsg )
	ON_EN_CHANGE(IDC_UPGRADE_USERID	, &CUpgradeDlg::OnEnChangeUpgradeUserid)
	ON_EN_CHANGE(IDC_UPGRADE_PASSWORD, &CUpgradeDlg::OnEnChangeUpgradePassword)
	ON_NOTIFY(NM_RCLICK				, IDC_UPGRADE_LIST, &CUpgradeDlg::OnNMRClickUpgradeList)
	ON_NOTIFY(LVN_ITEMCHANGED		, IDC_UPGRADE_LIST, &CUpgradeDlg::OnLvnItemchangedUpgradeList)
END_MESSAGE_MAP()

LRESULT CUpgradeDlg::OnRetryUpgradeMsg(WPARAM wParam, LPARAM lParam)
{
//	SW_PACKAGE_FILE_INFO	UpgradeFileInfo;
	int						iMagicCode				= 0;
	CString					strID;
	CString					strPass;
	CString					strTemp;
	CString					strSelected;
	BOOL					bMemoryClearVerion		= FALSE;
	DWORD					dwReadSize				= 0;
	DWORD					dwFileTotalSize			= 0;
	HANDLE					hFile					= NULL;
	int						iUpgradePort			= 0;
	UPGRADETHREAD*			pUpgradeThread			= NULL;
	INT						iRetryID				= (int)wParam;

//	memset( &UpgradeFileInfo, 0, sizeof(UpgradeFileInfo) );

	if( 0 <= iRetryID &&
		m_nUpgradeCnt > iRetryID )
	{
		hFile	= CreateFile(	m_strFilePath,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL ,
			NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			return 0;
		}

		dwFileTotalSize	= GetFileSize( hFile, 0 );


		SetFilePointer( hFile, 0, 0, FILE_BEGIN );
		
		ReadFile( hFile, &iMagicCode, sizeof(iMagicCode), &dwReadSize, NULL );
//		ReadFile( hFile, &UpgradeFileInfo, sizeof(UpgradeFileInfo), &dwReadSize, NULL );

		CloseHandle(hFile);
		hFile	= NULL;

		if( TYPE_TCP == m_aUpgradeThread[iRetryID]->iUpgradeType )
		{
			if( m_aUpgradeThread[iRetryID]->pUpgradeThreadTcp )
				delete m_aUpgradeThread[iRetryID]->pUpgradeThreadTcp;

			m_aUpgradeThread[iRetryID]->pUpgradeThreadTcp	= NULL;
		}
		else if( TYPE_HTTP == m_aUpgradeThread[iRetryID]->iUpgradeType )
		{
			if( m_aUpgradeThread[iRetryID]->pUpgradeThreadHttp )
			{
				m_aUpgradeThread[iRetryID]->pUpgradeThreadHttp->StopUpgrade();
				delete m_aUpgradeThread[iRetryID]->pUpgradeThreadHttp;
			}

			m_aUpgradeThread[iRetryID]->pUpgradeThreadHttp	= NULL;
		}


		if(m_cmbModel.GetCurSel() == 0)
		{
			strID	= m_cUpgradeList.GetEditText(iRetryID, SUBITEM_ID		);
			strPass = m_cUpgradeList.GetEditText(iRetryID, SUBITEM_PASSWORD	);

			if( iMagicCode == 0x7667 ||
				iMagicCode == 0x7668 )
			{
				iUpgradePort = 0;
				if( 0 < m_pScanInfo[iRetryID]._ReadValue(L"Upgrade Port").GetLength() )
					iUpgradePort = _wtoi( m_pScanInfo[iRetryID]._ReadValue(L"Upgrade Port") );

				pUpgradeThread	= new UPGRADETHREAD();
				pUpgradeThread->iUpgradeType	= TYPE_TCP;
				pUpgradeThread->pUpgradeThreadTcp = new CUpgradeThreadTcp(	this->GetSafeHwnd(), 
					iRetryID, 
					m_pScanInfo[iRetryID].szAddr, 
					iUpgradePort,
					strID, 
					strPass, 
					m_strFilePath, 
					m_strFileName);
				pUpgradeThread->pUpgradeThreadTcp->RetryUpgrade();
			}
			else 
			{
				//			memcpy((char*)m_pScanInfo, m_pBufScanInfo+sizeof(SCAN_INFO)*i,sizeof(SCAN_INFO));
				//			TRACE(L"###### START::HttpPostFile\n");
				//			TRACE(L"###### i = %d, stServerInfo.szAddr = %s, stServerInfo.nUpgradePort = %d, login = %s\n", i, stServerInfo.szAddr, stServerInfo.nUpgradePort, login);
				//			TRACE(L"###### sRequest = %s, upgrade_step = %s, strID = %s, strPass = %s, m_strFilePath = %s\n", sRequest, upgrade_step, strID, strPass, m_strFilePath);

				if( 0 < m_pScanInfo[iRetryID]._ReadValue(L"Upgrade Port").GetLength() )
					bMemoryClearVerion		= TRUE;
				else
					bMemoryClearVerion		= FALSE;

				pUpgradeThread	= new UPGRADETHREAD();
				pUpgradeThread->iUpgradeType	= TYPE_HTTP;
				pUpgradeThread->pUpgradeThreadHttp = new CUpgradeThreadHttp(this->GetSafeHwnd(), 
					iRetryID, 
					m_pScanInfo[iRetryID].szAddr, 
					m_pScanInfo[iRetryID].nHTTPPort, 
					strID, 
					strPass, 
					m_strFilePath, 
					m_strFileName,
					bMemoryClearVerion );
				pUpgradeThread->pUpgradeThreadHttp->StartUpgrade();
			}

			m_cUpgradeList.SetEditEnable( iRetryID, SUBITEM_ID		, FALSE );
			m_cUpgradeList.SetEditEnable( iRetryID, SUBITEM_PASSWORD, FALSE );

			m_aUpgradeThread[iRetryID] = pUpgradeThread;
			Sleep(100);
		}
		else
		{
			strID	= m_cUpgradeList.GetEditText(iRetryID, SUBITEM_ID		);
			strPass = m_cUpgradeList.GetEditText(iRetryID, SUBITEM_PASSWORD	);

			strTemp = m_pScanInfo[iRetryID]._ReadValue(L"Model Type");

			m_cmbModel.GetLBText(m_cmbModel.GetCurSel(), strSelected);

			if(strSelected.Compare(strTemp) == 0)
			{
				if( iMagicCode == 0x7667 ||
					iMagicCode == 0x7668 )
				{
					iUpgradePort = 0;
					if( 0 < m_pScanInfo[iRetryID]._ReadValue(L"Upgrade Port").GetLength() )
						iUpgradePort = _wtoi( m_pScanInfo[iRetryID]._ReadValue(L"Upgrade Port") );

					pUpgradeThread						= new UPGRADETHREAD();
					pUpgradeThread->iUpgradeType		= TYPE_TCP;
					pUpgradeThread->pUpgradeThreadTcp	= new CUpgradeThreadTcp(this->GetSafeHwnd(), 
						iRetryID, 
						m_pScanInfo[iRetryID].szAddr,
						iUpgradePort,
						strID, 
						strPass, 
						m_strFilePath, 
						m_strFileName);
					pUpgradeThread->pUpgradeThreadTcp->RetryUpgrade();
				}
				else
				{
					if( 0 < m_pScanInfo[iRetryID]._ReadValue(L"Upgrade Port").GetLength() )
						bMemoryClearVerion		= TRUE;
					else
						bMemoryClearVerion		= FALSE;

					pUpgradeThread						= new UPGRADETHREAD();
					pUpgradeThread->iUpgradeType		= TYPE_HTTP;
					pUpgradeThread->pUpgradeThreadHttp = new CUpgradeThreadHttp(this->GetSafeHwnd(), 
						iRetryID, 
						m_pScanInfo[iRetryID].szAddr, 
						m_pScanInfo[iRetryID].nHTTPPort, 
						strID, 
						strPass, 
						m_strFilePath, 
						m_strFileName,
						bMemoryClearVerion );
					pUpgradeThread->pUpgradeThreadHttp->StartUpgrade();
				}

				m_cUpgradeList.SetEditEnable( iRetryID, SUBITEM_ID		, FALSE );
				m_cUpgradeList.SetEditEnable( iRetryID, SUBITEM_PASSWORD, FALSE );
				m_aUpgradeThread[iRetryID] = pUpgradeThread;

				Sleep(100);
			}
		}
	}

	return 0;
}

LRESULT CUpgradeDlg::OnUpgradeMsg(WPARAM wParam, LPARAM lParam)
{
	short	iMessage		= LOWORD(wParam);
	short	iIndex			= HIWORD(wParam);
	WCHAR	szMessage[256]	= {0};

	switch( iMessage )
	{
	case UPGRADE_PROGRESS:
		{
			int		iPercentage	= (int)lParam;
			m_cUpgradeList.SetProgress( iIndex, SUBITEM_PROGRESS		, iPercentage );
		}										
		break;

	case UPGRADE_SETSTATUS:
		{
			if( _UPGRADE_STATUS_WAITFOR_REBOOT	!= lParam &&
				_UPGRADE_STATUS_ATTEMPT_REBOOT	!= lParam  )
			{
				m_iRebootCheck	= 0;
			}

			switch( lParam )
			{
			case _UPGRADE_STATUS_CONNECTING						:	wsprintf( szMessage, L"Attempt to connect"				);	break;
			case _UPGRADE_STATUS_FILE_TRANSFERING				: 	wsprintf( szMessage, L"Software file transfering"		);	break;
			case _UPGRADE_STATUS_READY_UPGRADE					: 	wsprintf( szMessage, L"Ready to upgrade"				);	break;
			case _UPGRADE_STATUS_UPGRADE_OS						: 	wsprintf( szMessage, L"Upgrade OS"						);	break;
			case _UPGRADE_STATUS_UPGRADE_APPLICATION			: 	wsprintf( szMessage, L"Upgrade application"				);	break;
			case _UPGRADE_STATUS_UPGRADE_APPLICATION_WEB		: 	wsprintf( szMessage, L"Upgrading Web Application"		);	break;
			case _UPGRADE_STATUS_FILE_RETRANSFERING				:  	wsprintf( szMessage, L"Re-request file transfering"		);	break;
			case _UPGRADE_STATUS_COMPLETE						: 	wsprintf( szMessage, L"Completed"						);	break;
			case _UPGRADE_STATUS_UPGRADE_PROGRESS				: 	wsprintf( szMessage, L""								);	break;
			case _UPGRADE_STATUS_READY_TO_SW_FILE_TRANSFERRING	: 	wsprintf( szMessage, L"Ready to S/W file transfering"	);	break;
			case _UPGRADE_STATUS_READY_TO_MCU_FILE_TRANSFERRING : 	wsprintf( szMessage, L"Ready to MCU file transfering"	);	break;
			case _UPGRADE_STATUS_PREPARE_UPGRADING				: 	wsprintf( szMessage, L""								);	break;
			case _UPGRADE_STATUS_RESTARTING_ERROR				: 	wsprintf( szMessage, L"Wait for rebooting"				);	break;
			case _UPGRADE_STATUS_RESTARTING_SW					: 	wsprintf( szMessage, L"Wait for retry S/W upgrade"		);	break;
			case _UPGRADE_STATUS_RESTARTING_MCU					: 	wsprintf( szMessage, L"Wait for retry MCU upgrade"		);	break;
			case _UPGRADE_STATUS_MCU_FILE_TRANSFERING			: 	wsprintf( szMessage, L"MCU file transfering"			);	break;
			case _UPGRADE_STATUS_UPGRADE_MCU					: 	wsprintf( szMessage, L"Upgrade MCU"						);	break;
			case _UPGRADE_STATUS_WAITFOR_REBOOT					:
				{
					wsprintf( szMessage, L"Wait for rebooting"			);	
					for( int i=0; i<m_iRebootCheck%30/3; i++ )
					{
						wcscat_s( szMessage, L"." );
					}
					m_iRebootCheck++;
				}
				break;
			case _UPGRADE_STATUS_ATTEMPT_REBOOT:
				{
					wsprintf( szMessage, L"Attempts to connect"			);	
					for( int i=0; i<m_iRebootCheck%10; i++ )
					{
						wcscat_s( szMessage, L"." );
					}
					m_iRebootCheck++;
				}
				break;
			case _UPGRADE_STATUS_ATTEMPT_RETRY:
				{
					wsprintf( szMessage, L"Wait for Retry Upgrade"			);	
				}
				break;
			}

			if( 0 < wcslen(szMessage) )
			{
				LV_ITEM item;
				memset(&item, 0, sizeof(item));
				item.mask = LVIF_TEXT;
				item.iItem = iIndex;
				item.iSubItem = SUBITEM_STATUS;
				item.pszText = (LPTSTR)(LPCTSTR)szMessage;
				m_cUpgradeList.SetItem(&item);
			}
		}
		break;

	case UPGRADE_ERROR:
		{
			switch( lParam )
			{
			case _UPGRADE_ERROR_OPEN_ERROR									:	wsprintf( szMessage, L"Failed(File open error)"										);	break;
			case _UPGRADE_ERROR_CONNECT_FAILED								: 	wsprintf( szMessage, L"Failed(Can not connect)"										);	break;
			case _UPGRADE_ERROR_DISCONNECTED								: 	wsprintf( szMessage, L"Failed(Disconnected)"										);	break;
			case _UPGRADE_ERROR_AUTH_FAILED									: 	wsprintf( szMessage, L"Failed(Authentication)"										);	break;
			case _UPGRADE_ERROR_NOT_SUPPORT_UPGRAE_FILE						: 	wsprintf( szMessage, L"Failed(This IP camera does not support the upgrade file)"	);	break;
			case _UPGRADE_ERROR_IN_ORDER_TO_VERSION_CHECKING_CAN_NOT_CONNECT:  	wsprintf( szMessage, L"Failed(In order to check the version, can not connect)"		);	break;
			case _UPGRADE_ERROR_INVALID_FIRMWARE							: 	wsprintf( szMessage, L"Failed(Invalid firmware)"									);	break;
			case _UPGRADE_ERROR_NO_MATCH_MODEL								: 	wsprintf( szMessage, L"Failed(No match model)"										);	break;
			case _UPGRADE_ERROR_NO_MATCH_VERSION							: 	wsprintf( szMessage, L"Failed(No match version)"									);	break;
			case _UPGRADE_ERROR_UPGRADE_IS_WORKING_IN_OTHER_PLACE			: 	wsprintf( szMessage, L"Failed(Upgrade is working in other places)"					);	break;
			case _UPGRADE_ERROR_RECEIVE_TIME_OUT							: 	wsprintf( szMessage, L"Failed(Receive Timeout)"										);	break;
			case _UPGRADE_ERROR_THIS_VERSION_DOES_NOT_SUPPORT				: 	wsprintf( szMessage, L"Failed(This version does not support)"						);	break;
			case _UPGRADE_ERROR_UPGRADE_FILE_IS_NOT_AVAILABLE				: 	wsprintf( szMessage, L"Failed(Upgrade file is not available)"						);	break;
			case _UPGRADE_ERROR_UPGRADE_CANCELED							: 	wsprintf( szMessage, L"Failed(Upgrade Canceled)"									);	break;
			case _UPGRADE_ERROR_PROTOCOL_MCU_INFO_FAIL						: 	wsprintf( szMessage, L"Failed(Don't support MCU upgrade)"							);	break;
			case _UPGRADE_ERROR_NO_MATCH_MCU_MODEL							: 	wsprintf( szMessage, L"Failed(No match MCU model)"									);	break;
			}

			if( 0 < wcslen(szMessage) )
			{
				LV_ITEM item;
				memset(&item, 0, sizeof(item));
				item.mask = LVIF_TEXT;
				item.iItem = iIndex;
				item.iSubItem = SUBITEM_STATUS;
				item.pszText = (LPTSTR)(LPCTSTR)szMessage;
				m_cUpgradeList.SetItem(&item);
			}		
		}
		break;
	}
	return 0;
}

// CUpgradeDlg 메시지 처리기입니다.
BOOL CUpgradeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT(m_pScanInfo != NULL);
	GetDlgItem(IDC_UPGRADE_USERID)->EnableWindow(TRUE);
	GetDlgItem(IDC_UPGRADE_PASSWORD)->EnableWindow(TRUE);

	CString str;
	CString strItem; 

	str.LoadString(IDS_ADDRESS);
	strItem = L"IP " + str;
	m_cUpgradeList.InsertColumn(SUBITEM_IPADDRESS			, strItem, LVCFMT_CENTER, 100, -1 );			// IP Address
	strItem.LoadString(IDS_MODEL);
	m_cUpgradeList.InsertColumn(SUBITEM_MODEL				, strItem, LVCFMT_CENTER, 80, -1 );				// Model
	str.LoadString(IDS_FIRMWARE_VERSION);
	strItem = L"From " + str;
	m_cUpgradeList.InsertColumn(SUBITEM_FIRMWAREVERSION_FROM, strItem, LVCFMT_CENTER, 140, -1 );			// From Firmware Version
	str.LoadString(IDS_FIRMWARE_VERSION);
	strItem = L"To " + str;
	m_cUpgradeList.InsertColumn(SUBITEM_FIRMWAREVERSION_TO	, strItem, LVCFMT_CENTER, 140, -1 );			// From Firmware Version
	strItem.LoadString(IDS_USER_ID);
	m_cUpgradeList.InsertColumn(SUBITEM_ID					, strItem, LVCFMT_CENTER, 50, -1 );				// ID
	strItem.LoadString(IDS_USER_PW);
	m_cUpgradeList.InsertColumn(SUBITEM_PASSWORD			, strItem, LVCFMT_CENTER, 50, -1 );				// Password
	strItem.LoadString(IDS_UPGRADE_PROGRESS);
	m_cUpgradeList.InsertColumn(SUBITEM_PROGRESS			, strItem, LVCFMT_CENTER, 80, -1 );				// Progress
	strItem.LoadString(IDS_PROGRESS_STATUS);
	m_cUpgradeList.InsertColumn(SUBITEM_STATUS				, strItem, LVCFMT_CENTER, 190, -1 );			// Status

	m_cUpgradeList.DeleteAllItems();



	AddData();
	AddModelToCombo();

//	CallLayoutManager();
	m_cUpgradeList.SetExtendedStyle( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );

#ifdef _DEFAULT_ID_PASSWORD_
	//{{ 2013-07-19 hkeins : 생산 부 요청으로 ID/Pasword 디폴트 값을 admin/1234로 넣었음
	m_strID       = L"admin";
	m_strPassword = L"1234";

	SetDlgItemText( IDC_UPGRADE_USERID, m_strID );
	SetDlgItemText( IDC_UPGRADE_PASSWORD, m_strPassword );
	//}}
#endif _DEFAULT_ID_PASSWORD_
	m_bInit = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CUpgradeDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}

void CUpgradeDlg::OnDestroy()
{
	CDialog::OnDestroy();
	// FIX ME: not entered here why?
}

HBRUSH CUpgradeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

LRESULT CUpgradeDlg::OnMoveWnd(WPARAM wParam, LPARAM lParam)
{
	CRect rc; GetWindowRect(&rc);
	MoveWindow(rc.left - (LONG)wParam, rc.top - (LONG)lParam, rc.Width(),rc.Height());
	return 0L;
}

void CUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UPGRADE_LIST	, m_cUpgradeList	);
	DDX_Control(pDX, IDC_OPENFILE_BTN	, m_btnOpenFile		);
	DDX_Control(pDX, IDC_UPGRADE_BTN	, m_btnUpgrade		);
	DDX_Control(pDX, IDC_CLOSE			, m_btnClose		);
	DDX_Text(pDX, IDC_OPENFILE_EDIT		, m_strFilePath		);
	DDX_Text(pDX, IDC_UPGRADE_USERID	, m_strID			);
	DDX_Text(pDX, IDC_UPGRADE_PASSWORD	, m_strPassword		);
	DDX_Control(pDX, IDC_MODEL_COMBO	, m_cmbModel		);
}

void CUpgradeDlg::SetScaninfo( int iCount, SCAN_INFO* pScanInfo)
{
	m_nUpgradeCnt	= iCount;
	m_pScanInfo		= pScanInfo;
}


void CUpgradeDlg::AddData()
{
	CString strTemp;

	//UpgradeList에 데이터 입력
	for(int i=0;i<m_nUpgradeCnt;i++)
	{
		// 이미 복사되어 있으므로 복사하지 않음
		//memcpy((char*)m_pScanInfo, m_pBufScanInfo+sizeof(SCAN_INFO)*i,sizeof(SCAN_INFO));

		// add information into UI
		LV_ITEM item;
		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = SUBITEM_IPADDRESS;
		item.pszText = m_pScanInfo[i].szAddr;
		item.lParam = (LPARAM)&m_pScanInfo[i];
		m_cUpgradeList.InsertItem(&item);

//		TRACE(L"\n###### index = %d, m_pScanInfo[i].nExtraFieldCount = %d\n",i, m_pScanInfo[i].nExtraFieldCount);
//		TRACE(L"###### m_pScanInfo[i]._ReadValues() = %s\n",m_pScanInfo[i]._ReadValues());
		// extended infomation printout

		if(m_pScanInfo[i].nExtraFieldCount)
		{
			strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");
//			TRACE(L"###### Model Type::strTemp = %s\n",strTemp);
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_MODEL;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cUpgradeList.SetItem(&item);

			strTemp = m_pScanInfo[i]._ReadValue(L"Firmware Version");
//			TRACE(L"###### Firmware Version::strTemp = %s\n\n",strTemp);
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_FIRMWAREVERSION_FROM;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cUpgradeList.SetItem(&item);
		}

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_ID;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cUpgradeList.SetItem(&item);
		m_cUpgradeList.CreateEdit(i, SUBITEM_ID, FALSE );

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_PASSWORD;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cUpgradeList.SetItem(&item);
		m_cUpgradeList.CreateEdit(i, SUBITEM_PASSWORD, TRUE );

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_PROGRESS;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cUpgradeList.SetItem(&item);
		m_cUpgradeList.CreateProgress(i, SUBITEM_PROGRESS);

		strTemp = L"";
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = SUBITEM_STATUS;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cUpgradeList.SetItem(&item);
	}
	
	return;
}



void CUpgradeDlg::AddModelToCombo()
{
	CString strTemp;
	m_strModel = L"All";
	m_nModelCnt = 0;

	//Model Combo에 Model 추가
	m_cmbModel.ResetContent();
	m_cmbModel.AddString(L"All");

	for(int i=0;i<m_nUpgradeCnt;i++)
	{
		//memcpy((char*)m_pScanInfo, m_pBufScanInfo+sizeof(SCAN_INFO)*i,sizeof(SCAN_INFO));

		// extended infomation printout
		if(m_pScanInfo[i].nExtraFieldCount)
		{
			strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");

			if(m_strModel.Find(strTemp) == -1)
			{
//				TRACE(L"###### m_strModel = %s, strTemp = %s\n",m_strModel, strTemp);
				m_cmbModel.AddString(strTemp);
				m_strModel += L"&";
				m_strModel += strTemp;
				m_nModelCnt++;
			}
		}
	}

	m_cmbModel.SetCurSel(0);
}

void CUpgradeDlg::OnSelchangeCmbModel()
{
	CString strTemp;
	CString strSelected;
	int     i = 0;
	int     j = 0;
	LV_ITEM item;
	CString strID;
	CString strPass;

	m_strModel.Find('&', m_cmbModel.GetCurSel());
	GetDlgItemText( IDC_UPGRADE_USERID, m_strID );
	GetDlgItemText( IDC_UPGRADE_PASSWORD, m_strPassword );


	int     nItemCount = m_cUpgradeList.GetItemCount();
	m_cUpgradeList.DeleteAllItems();

	if(m_cmbModel.GetCurSel() == 0)
	{
		for(i=0; i< nItemCount; i++)
		{
			m_cUpgradeList.DeleteEdit(i,     SUBITEM_ID			);   // memory leak fix
			m_cUpgradeList.DeleteEdit(i,     SUBITEM_PASSWORD	);
			m_cUpgradeList.DeleteProgress(i, SUBITEM_PROGRESS	);
		}

		for(i = 0; i < m_nUpgradeCnt; i++)
		{
			//memcpy((char*)m_pScanInfo, m_pBufScanInfo+sizeof(SCAN_INFO)*i,sizeof(SCAN_INFO));
			memset(&item, 0, sizeof(item));

			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = i;
			item.iSubItem = SUBITEM_IPADDRESS;
			item.pszText = m_pScanInfo[i].szAddr;
			item.lParam = (LPARAM)&m_pScanInfo[i];
			m_cUpgradeList.InsertItem(&item);

			if(m_pScanInfo[i].nExtraFieldCount)
			{
				strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");	
				item.mask = LVIF_TEXT;
				item.iItem = i;
				item.iSubItem = SUBITEM_MODEL;
				item.pszText = (LPTSTR)(LPCTSTR)strTemp;
				m_cUpgradeList.SetItem(&item);

				strTemp = m_pScanInfo[i]._ReadValue(L"Firmware Version");
				item.mask = LVIF_TEXT;
				item.iItem = i;
				item.iSubItem = SUBITEM_FIRMWAREVERSION_FROM;
				item.pszText = (LPTSTR)(LPCTSTR)strTemp;
				m_cUpgradeList.SetItem(&item);
			}

			strTemp = L"";
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_ID;
			item.pszText = (LPTSTR)(LPCTSTR)m_strID;
			m_cUpgradeList.SetItem(&item);
			m_cUpgradeList.CreateEdit(i, SUBITEM_ID, FALSE);
			m_cUpgradeList.SetEditText(i, SUBITEM_ID, m_strID);

			strTemp = L"";
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_PASSWORD;
			item.pszText = (LPTSTR)(LPCTSTR)m_strPassword;
			m_cUpgradeList.SetItem(&item);
			m_cUpgradeList.CreateEdit(i, SUBITEM_PASSWORD, TRUE);
			m_cUpgradeList.SetEditText(i, SUBITEM_PASSWORD, m_strPassword);

			strTemp = L"";
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_PROGRESS;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cUpgradeList.SetItem(&item);
			m_cUpgradeList.CreateProgress(i, SUBITEM_PROGRESS);

			strTemp = L"";
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_STATUS;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cUpgradeList.SetItem(&item);
		}
	}
	else
	{
		j = 0;
		for(i=0; i< nItemCount; i++)
		{
			m_cUpgradeList.DeleteEdit(i,     SUBITEM_ID			);  // memory leak fix
			m_cUpgradeList.DeleteEdit(i,     SUBITEM_PASSWORD	);
			m_cUpgradeList.DeleteProgress(i, SUBITEM_PROGRESS	);
		}

		m_cmbModel.GetLBText(m_cmbModel.GetCurSel(), strSelected);

		for(i=0;i<m_nUpgradeCnt;i++)
		{
			//memcpy((char*)m_pScanInfo, m_pBufScanInfo+sizeof(SCAN_INFO)*i,sizeof(SCAN_INFO));
			memset(&item, 0, sizeof(item));
			strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");

			if(strSelected.Compare(strTemp) == 0)
			{
				item.mask = LVIF_TEXT | LVIF_PARAM;
				item.iItem = j;
				item.iSubItem = SUBITEM_IPADDRESS;
				item.pszText = m_pScanInfo[i].szAddr;
				item.lParam = (LPARAM)&m_pScanInfo[i];
				m_cUpgradeList.InsertItem(&item);

				strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");	
				item.mask = LVIF_TEXT;
				item.iItem = j;
				item.iSubItem = SUBITEM_MODEL;
				item.pszText = (LPTSTR)(LPCTSTR)strTemp;
				m_cUpgradeList.SetItem(&item);

				strTemp = m_pScanInfo[i]._ReadValue(L"Firmware Version");
				item.mask = LVIF_TEXT;
				item.iItem = j;
				item.iSubItem = SUBITEM_FIRMWAREVERSION_FROM;
				item.pszText = (LPTSTR)(LPCTSTR)strTemp;
				m_cUpgradeList.SetItem(&item);

				strTemp = L"";
				item.mask = LVIF_TEXT;
				item.iItem = j;
				item.iSubItem = SUBITEM_ID;
				item.pszText = (LPTSTR)(LPCTSTR)m_strID;
				m_cUpgradeList.SetItem(&item);
				m_cUpgradeList.CreateEdit(j, SUBITEM_ID, FALSE);
				m_cUpgradeList.SetEditText(j, SUBITEM_ID, m_strID);

				strTemp = L"";
				item.mask = LVIF_TEXT;
				item.iItem = j;
				item.iSubItem = SUBITEM_PASSWORD;
				item.pszText = (LPTSTR)(LPCTSTR)m_strPassword;
				m_cUpgradeList.SetItem(&item);
				m_cUpgradeList.CreateEdit(j, SUBITEM_PASSWORD, TRUE);
				m_cUpgradeList.SetEditText(j, SUBITEM_PASSWORD, m_strPassword);

				strTemp = L"";
				item.mask = LVIF_TEXT;
				item.iItem = j;
				item.iSubItem = SUBITEM_PROGRESS;
				item.pszText = (LPTSTR)(LPCTSTR)strTemp;
				m_cUpgradeList.SetItem(&item);
				m_cUpgradeList.CreateProgress(j, SUBITEM_PROGRESS);

				strTemp = L"";
				item.mask = LVIF_TEXT;
				item.iItem = j;
				item.iSubItem = SUBITEM_STATUS;
				item.pszText = (LPTSTR)(LPCTSTR)strTemp;
				m_cUpgradeList.SetItem(&item);

				j++;
			}
		}
	}
}

void CUpgradeDlg::OnBnClickedOpenFile()
{
	CFileDialog dlg(TRUE); // TRUE --> open, FALSE --> save
	if(dlg.DoModal() == IDOK)
	{
		m_strFilePath = dlg.GetPathName();
		m_strFileName = dlg.GetFileName();
		if(_CheckUpgradeFileType(m_strFilePath.GetBuffer()) > 0)
		{
			SetDlgItemText(IDC_OPENFILE_EDIT,	m_strFilePath);
		}
		else
		{
			MessageBox(L"Invalid package file", NULL, MB_ICONHAND | MB_OK);
		}
		m_strFilePath.ReleaseBuffer();
	}
}

void CUpgradeDlg::OnBnClickedUpgrade()
{
	CString	login;
	CString	upgrade_step;
	CString	sRequest;
	CString strTemp;
	CString strSelected;
	int		iUpgradePort;
	int		i;
	int     nCurrent = 0;
	sRequest		= L"/cgi-bin/setup_upgrade.cgi";
	upgrade_step	= L"/xml/upgrading_step";

	if(m_strFilePath.GetLength() == 0)
	{
		AfxMessageBox(_T("Please select a file..."), MB_ICONWARNING);
		GetDlgItem(IDC_OPENFILE_BTN)->SetFocus();
		return;
	}
	if(_CheckUpgradeFileType(m_strFilePath.GetBuffer()) <= 0)
	{
		m_strFilePath.ReleaseBuffer();
		ASSERT(0); // 시나리오상 여기에 올 수 없음
		MessageBox(L"Invalid package file", NULL, MB_ICONHAND | MB_OK);
		return;
	}

	m_strFilePath.ReleaseBuffer();

	if(m_cmbModel.GetCurSel() == 0)
	{
		strSelected = L"";
	}
	else
	{
		m_cmbModel.GetLBText(m_cmbModel.GetCurSel(), strSelected);
	}

	// ID, Password 체크
	CString	strID;
	CString strPass;
	GetDlgItemText( IDC_UPGRADE_USERID,   m_strID );
	GetDlgItemText( IDC_UPGRADE_PASSWORD, m_strPassword );
	if(m_strID.IsEmpty())
	{
		AfxMessageBox(_T("Please insert ID..."), MB_ICONWARNING);
		GetDlgItem(IDC_UPGRADE_USERID)->SetFocus();
		return;
	}
	if(m_strPassword.IsEmpty())
	{
		AfxMessageBox(_T("Please insert Password..."), MB_ICONWARNING);
		GetDlgItem(IDC_UPGRADE_PASSWORD)->SetFocus();
		return;
	}

	// 각 항목의 ID, Password 체크
	SCAN_INFO* pScanInfo = NULL;

	for(i = 0; i < m_cUpgradeList.GetItemCount(); i++)
	{
		strID	= m_cUpgradeList.GetEditText(i, SUBITEM_ID		);
		strPass = m_cUpgradeList.GetEditText(i, SUBITEM_PASSWORD);
		pScanInfo = (SCAN_INFO*)m_cUpgradeList.GetItemData(i);
		ASSERT(pScanInfo != NULL);

		strTemp = pScanInfo->_ReadValue(L"Model Type");
		if(strSelected.IsEmpty() || strSelected.Compare(strTemp) == 0)
		{
			if(strID.GetLength() == 0)
			{
				AfxMessageBox(_T("Please insert ID..."), MB_ICONWARNING);
				//GetDlgItem(IDC_UPGRADE_USERID)->SetFocus();
				m_cUpgradeList.SetFocusEdit(i, SUBITEM_ID);
				return;
			}

			if(strPass.GetLength() == 0)
			{
				AfxMessageBox(_T("Please insert Password..."), MB_ICONWARNING);
				//GetDlgItem(IDC_UPGRADE_PASSWORD)->SetFocus();
				m_cUpgradeList.SetFocusEdit(i, SUBITEM_PASSWORD);
				return;
			}
		}
	}

	for(i = 0;i < m_aUpgradeThread.GetCount(); i++)
	{
		if( TYPE_TCP == m_aUpgradeThread[i]->iUpgradeType )
		{
			if( m_aUpgradeThread[i]->pUpgradeThreadTcp )
				delete m_aUpgradeThread[i]->pUpgradeThreadTcp;

			m_aUpgradeThread[i]->pUpgradeThreadTcp	= NULL;
		}
		else if( TYPE_HTTP == m_aUpgradeThread[i]->iUpgradeType )
		{
			if( m_aUpgradeThread[i]->pUpgradeThreadHttp )
			{
				m_aUpgradeThread[i]->pUpgradeThreadHttp->StopUpgrade();
				delete m_aUpgradeThread[i]->pUpgradeThreadHttp;
			}

			m_aUpgradeThread[i]->pUpgradeThreadHttp	= NULL;
		}

		if(m_aUpgradeThread[i])
		{
			delete m_aUpgradeThread[i];
			m_aUpgradeThread[i] = NULL;
		}
	}

	m_btnOpenFile.EnableWindow(FALSE);
	m_cmbModel.EnableWindow(FALSE);
	m_btnUpgrade.EnableWindow(FALSE);
	
		
	GetDlgItem(IDC_UPGRADE_USERID)->EnableWindow(FALSE);
	GetDlgItem(IDC_UPGRADE_PASSWORD)->EnableWindow(FALSE);

	//CUpgradeThreadTcp*	pUpgradeThreadTcp	= NULL;
	//CUpgradeThreadHttp*	pUpgradeThreadHttp	= NULL;
	UPGRADETHREAD*			pUpgradeThread			= NULL;

	DWORD					dwReadSize;
	DWORD					dwFileTotalSize; 
	HANDLE					hFile;
	int						iMagicCode				= 0;
//	SW_PACKAGE_FILE_INFO	UpgradeFileInfo;
	BOOL					bMemoryClearVerion		= FALSE;

//	memset( &UpgradeFileInfo, 0, sizeof(UpgradeFileInfo) );

	hFile	= CreateFile(	m_strFilePath,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL ,
							NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		return ;
	}

	dwFileTotalSize	= GetFileSize( hFile, 0 );


	SetFilePointer( hFile, 0, 0, FILE_BEGIN );
	
	ReadFile( hFile, &iMagicCode, sizeof(iMagicCode), &dwReadSize, NULL );

	CloseHandle(hFile);
	hFile	= NULL;

	if(m_cmbModel.GetCurSel() == 0)
	{
		for(i = 0; i < m_nUpgradeCnt; i++)
		{
			strID	= m_cUpgradeList.GetEditText(i, SUBITEM_ID		);
			strPass = m_cUpgradeList.GetEditText(i, SUBITEM_PASSWORD);

			if( iMagicCode == 0x7667 ||
				iMagicCode == 0x7668 )
			{
				iUpgradePort = 0;
				if( 0 < m_pScanInfo[i]._ReadValue(L"Upgrade Port").GetLength() )
					iUpgradePort = _wtoi( m_pScanInfo[i]._ReadValue(L"Upgrade Port") );

				pUpgradeThread	= new UPGRADETHREAD();
				pUpgradeThread->iUpgradeType	= TYPE_TCP;
				pUpgradeThread->pUpgradeThreadTcp = new CUpgradeThreadTcp(	this->GetSafeHwnd(), 
																			m_aUpgradeThread.GetCount(), 
																			m_pScanInfo[i].szAddr, 
																			iUpgradePort,
																			strID, 
																			strPass, 
																			m_strFilePath, 
																			m_strFileName);
				pUpgradeThread->pUpgradeThreadTcp->StartUpgrade();
			}
			else 
			{
				//			memcpy((char*)m_pScanInfo, m_pBufScanInfo+sizeof(SCAN_INFO)*i,sizeof(SCAN_INFO));
				//			TRACE(L"###### START::HttpPostFile\n");
				//			TRACE(L"###### i = %d, stServerInfo.szAddr = %s, stServerInfo.nUpgradePort = %d, login = %s\n", i, stServerInfo.szAddr, stServerInfo.nUpgradePort, login);
				//			TRACE(L"###### sRequest = %s, upgrade_step = %s, strID = %s, strPass = %s, m_strFilePath = %s\n", sRequest, upgrade_step, strID, strPass, m_strFilePath);

				if( 0 < m_pScanInfo[i]._ReadValue(L"Upgrade Port").GetLength() )
					bMemoryClearVerion		= TRUE;
				else
					bMemoryClearVerion		= FALSE;

				login.Format(L"cgi-bin/auth.cgi?mode=login&id=%s&pass=%s", strID, strPass);

				pUpgradeThread	= new UPGRADETHREAD();
				pUpgradeThread->iUpgradeType	= TYPE_HTTP;
				pUpgradeThread->pUpgradeThreadHttp = new CUpgradeThreadHttp(this->GetSafeHwnd(), 
																			m_aUpgradeThread.GetCount(), 
																			m_pScanInfo[i].szAddr, 
																			m_pScanInfo[i].nHTTPPort, 
																			strID, 
																			strPass, 
																			m_strFilePath, 
																			m_strFileName,
																			bMemoryClearVerion );
				pUpgradeThread->pUpgradeThreadHttp->StartUpgrade();
			}

			m_cUpgradeList.SetEditEnable( i, SUBITEM_ID			, FALSE );
			m_cUpgradeList.SetEditEnable( i, SUBITEM_PASSWORD	, FALSE );

			m_aUpgradeThread[i] = pUpgradeThread;
			Sleep(100);
		}
	}
	else
	{
		nCurrent = 0;
		m_cmbModel.GetLBText(m_cmbModel.GetCurSel(), strSelected);

		for(i=0; i<m_nUpgradeCnt; i++)
		{
			strID	= m_cUpgradeList.GetEditText(nCurrent, SUBITEM_ID		);
			strPass = m_cUpgradeList.GetEditText(nCurrent, SUBITEM_PASSWORD	);

			strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");	

			if(strSelected.Compare(strTemp) == 0)
			{
				if( iMagicCode == 0x7667 ||
					iMagicCode == 0x7668 )
				{
					iUpgradePort = 0;
					if( 0 < m_pScanInfo[i]._ReadValue(L"Upgrade Port").GetLength() )
						iUpgradePort = _wtoi( m_pScanInfo[i]._ReadValue(L"Upgrade Port") );

					pUpgradeThread						= new UPGRADETHREAD();
					pUpgradeThread->iUpgradeType		= TYPE_TCP;
					pUpgradeThread->pUpgradeThreadTcp	= new CUpgradeThreadTcp(this->GetSafeHwnd(), 
																				m_aUpgradeThread.GetCount(), 
																				m_pScanInfo[i].szAddr,
																				iUpgradePort,
																				strID, 
																				strPass, 
																				m_strFilePath, 
																				m_strFileName);
					pUpgradeThread->pUpgradeThreadTcp->StartUpgrade();
				}
				else
				{
					if( 0 < m_pScanInfo[i]._ReadValue(L"Upgrade Port").GetLength() )
						bMemoryClearVerion		= TRUE;
					else
						bMemoryClearVerion		= FALSE;

					login.Format(L"cgi-bin/auth.cgi?mode=login&id=%s&pass=%s", strID, strPass);

					pUpgradeThread						= new UPGRADETHREAD();
					pUpgradeThread->iUpgradeType		= TYPE_HTTP;
					pUpgradeThread->pUpgradeThreadHttp = new CUpgradeThreadHttp(this->GetSafeHwnd(), 
																				m_aUpgradeThread.GetCount(), 
																				m_pScanInfo[i].szAddr, 
																				m_pScanInfo[i].nHTTPPort, 
																				strID, 
																				strPass, 
																				m_strFilePath, 
																				m_strFileName,
																				bMemoryClearVerion );
					pUpgradeThread->pUpgradeThreadHttp->StartUpgrade();
				}

				m_cUpgradeList.SetEditEnable( nCurrent, SUBITEM_ID		, FALSE );
				m_cUpgradeList.SetEditEnable( nCurrent, SUBITEM_PASSWORD, FALSE );
				m_aUpgradeThread[i] = pUpgradeThread;
				nCurrent++;

				Sleep(100);
			}
		}
	}
}

void CUpgradeDlg::OnBnClickedClose()
{
	for(int i=0;i<m_aUpgradeThread.GetCount();i++)
	{
		if( NULL != m_aUpgradeThread[i] )
		{
			if( TYPE_TCP == m_aUpgradeThread[i]->iUpgradeType )
			{
				if( m_aUpgradeThread[i]->pUpgradeThreadTcp )
					delete m_aUpgradeThread[i]->pUpgradeThreadTcp;

				m_aUpgradeThread[i]->pUpgradeThreadTcp	= NULL;
			}
			else if( TYPE_HTTP == m_aUpgradeThread[i]->iUpgradeType )
			{
				if( m_aUpgradeThread[i]->pUpgradeThreadHttp )
				{
					m_aUpgradeThread[i]->pUpgradeThreadHttp->StopUpgrade();
					delete m_aUpgradeThread[i]->pUpgradeThreadHttp;
				}

				m_aUpgradeThread[i]->pUpgradeThreadHttp	= NULL;
			}

			if(m_aUpgradeThread[i])
			{
				delete m_aUpgradeThread[i];
				m_aUpgradeThread[i] = NULL;
			}
		}
	}

	m_aUpgradeThread.RemoveAll();

	OnCancel();
}

void CUpgradeDlg::CallLayoutManager()
{
	if(!m_bInit)
		return ;

	CRect rcClientRect;
	CRect rcControlRect;
	CRect rcNewRect;
	CRect rcOriginateRect;
	int   nCaptionSize = GetSystemMetrics(SM_CYCAPTION);
	GetWindowRect(&rcClientRect);
	ScreenToClient(&rcClientRect);

	CWnd* pServerList       = GetDlgItem(IDC_UPGRADE_LIST);
	ScreenToClient(&rcOriginateRect);
	pServerList->GetWindowRect(&rcControlRect);
	ScreenToClient(&rcControlRect);

	rcNewRect		= rcControlRect;

	rcNewRect.left	= 11;
	rcNewRect.top	= 107;
	rcNewRect.right	= rcClientRect.right - 17;
	rcNewRect.bottom= rcClientRect.bottom - 50;
	pServerList->MoveWindow(&rcNewRect);

	CWnd* pClose            = GetDlgItem(IDC_CLOSE);
	pClose->GetWindowRect(&rcControlRect);
	ScreenToClient(&rcControlRect);
	rcNewRect = rcControlRect;
	rcNewRect.right = rcClientRect.right - 14;
	rcNewRect.left  = rcNewRect.right - rcControlRect.Width();
	rcNewRect.bottom= rcClientRect.bottom - 14;
	rcNewRect.top   = rcNewRect.bottom    - rcControlRect.Height();
	pClose->MoveWindow(&rcNewRect);

	CWnd* pUpgrade       = GetDlgItem(IDC_UPGRADE_BTN);

	rcNewRect.left	= rcNewRect.left - 90;
	rcNewRect.right	= rcNewRect.right - 90;
	pUpgrade->MoveWindow(rcNewRect);

}

void CUpgradeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if( GetSafeHwnd() && m_cUpgradeList.GetSafeHwnd() && m_btnUpgrade.GetSafeHwnd() && m_btnClose.GetSafeHwnd() )
	{
		m_cUpgradeList.MoveWindow( 10, 110, cx-20, cy-10-110-30 );
		m_btnUpgrade.MoveWindow( cx/2-80, cy-21-10, 73, 21 );
		m_btnClose.MoveWindow( cx/2+3, cy-21-10, 73, 21 );
	}

//	CallLayoutManager();
}

void CUpgradeDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = UPGRADE_WINDOW_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = UPGRADE_WINDOW_MIN_HEIGHT;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CUpgradeDlg::OnEnChangeUpgradeUserid()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.
	//UpdateData(TRUE);
	GetDlgItemText( IDC_UPGRADE_USERID, m_strID );

	for(int i=0;i<m_nUpgradeCnt;i++)
	{
		m_cUpgradeList.SetEditText( i, SUBITEM_ID		, m_strID );
	}								   
}

void CUpgradeDlg::OnEnChangeUpgradePassword()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	//UpdateData(TRUE);
	GetDlgItemText( IDC_UPGRADE_PASSWORD, m_strPassword );

	for(int i=0;i<m_nUpgradeCnt;i++)
	{
		m_cUpgradeList.SetEditText( i, SUBITEM_PASSWORD, m_strPassword );
	}
}


void CUpgradeDlg::OnNMRClickUpgradeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.


	CMenu  Menu;
	CMenu* pSubMenu		= NULL;

	Menu.LoadMenu(IDR_POPUP_MENU_UPGRADE);
	
	pSubMenu	= Menu.GetSubMenu(0);
	POINT		ptAction;
	ptAction = pNMItemActivate->ptAction;
	
	m_cUpgradeList.ClientToScreen(&ptAction);

	m_nRetryUpgradeID	= pNMItemActivate->iItem;
	pSubMenu->TrackPopupMenu( TPM_LEFTALIGN , ptAction.x, ptAction.y, this );

	*pResult = 0;
}

void CUpgradeDlg::OnRetryUpgrade()
{
	PostMessage( WM_RETRY_UPGRADE_MSG, m_nRetryUpgradeID, 0 );
}

void CUpgradeDlg::OnLvnItemchangedUpgradeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	*pResult = 0;
}


int  CUpgradeDlg::_CheckUpgradeFileType(WCHAR* szUpgradeFileName)
{
	int						nFileType		= UNKNOWN_FILE;
	FILE*					fp				= NULL;
	int						iMagicCode		= 0;
	SW_PACKAGE_FILE_INFO	UpgradeFileInfo	= { 0 };
	LV_ITEM					item			= { 0 };
	WCHAR					szSrcBuffer[256]= { 0 };

	//memset(&UpgradeFileInfo, 0, sizeof(UpgradeFileInfo));
	_wfopen_s(&fp, szUpgradeFileName, L"rb");
	if(fp == NULL)
	{
		TRACE(L"CUpgradeDlg::_CheckUpgradeFileType() File module open failed\n");
		return FILE_OPEN_ERROR;
	}

	if(fread(&UpgradeFileInfo, sizeof(UpgradeFileInfo), 1, fp) != 1)
	{
		fclose(fp);
		fp = NULL;
		TRACE(L"CUpgradeDlg::_CheckUpgradeFileType() : data read failed Unknown file\n");

		return UNKNOWN_FILE;
	}

	fclose(fp);
	fp = NULL;

	if( UpgradeFileInfo.magic_code == 0x7667 ||
		UpgradeFileInfo.magic_code == 0x7668 )
	{
		TRACE(L"CUpgradeDlg::_CheckUpgradeFileType() : TCP upgrade file\n");
		nFileType = TCP_UPGRADE_FILE;

		::MultiByteToWideChar(CP_ACP, 0, UpgradeFileInfo.szSWVersion, lstrlenA(UpgradeFileInfo.szSWVersion)+1, szSrcBuffer, sizeof(szSrcBuffer)/sizeof(szSrcBuffer[0]));

		for( int i=0;i<m_nUpgradeCnt; i++ )
		{
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = SUBITEM_FIRMWAREVERSION_TO;
			item.pszText = (LPTSTR)(LPCTSTR)szSrcBuffer;
			m_cUpgradeList.SetItem(&item);
		}
	}
	else if( (UpgradeFileInfo.magic_code & 0x0000FFFF) == 0x00008B1F )
	{
		TRACE(L"CUpgradeDlg::_CheckUpgradeFileType() : HTTP upgrade file\n");
		nFileType = HTTP_UPGRADE_FILE;
	}

	return nFileType;
}