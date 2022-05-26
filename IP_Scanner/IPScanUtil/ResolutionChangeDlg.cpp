// ResolutionChangeDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ARP.h"
#include "ResolutionChangeDlg.h"

// CResolutionChangeDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CResolutionChangeDlg, CDialog)

CResolutionChangeDlg::CResolutionChangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResolutionChangeDlg::IDD, pParent)
{
	m_nSelectScanInfoCnt	= 0;
	m_nScanInfoCnt			= 0;
	m_pScanInfo				= NULL;
	m_pSession				= NULL;
	m_pHttpConnect			= NULL;
	m_pHttpFile				= NULL;
	m_iAdapterID			= 0;


	m_iSelectStreamType		= STREAMTYPE_SINGLE;
	m_iSelectCodec			= CODEC_SINGLE_H264;
	m_iSelectResolution		= RESOL_S_H_D1;

	m_iBitrateStream1		= 0;
	m_iBitrateStream2		= 0;

	m_hThreadLensInit		= NULL;
	m_dwThreadLensInitID	= 0;

	memset( &m_SetupStreamInfo, 0, sizeof(m_SetupStreamInfo) );
}

CResolutionChangeDlg::~CResolutionChangeDlg()
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

void CResolutionChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_MODELTYPE	, m_ctlComboModelType	);
	DDX_Control(pDX, IDC_COMBO_STREAMTYPE	, m_ctlComboStreamType	);
	DDX_Control(pDX, IDC_COMBO_CODECTYPE	, m_ctlComboCodec		);
	DDX_Control(pDX, IDC_COMBO_RESOLUTION	, m_ctlComboResolution	);
	DDX_Control(pDX, IDC_COMBO_RATECONTROL	, m_ctlComboRateControl	);
	DDX_Control(pDX, IDC_SLIDER_BITRATE1	, m_ctlSliderBitrate1	);
	DDX_Control(pDX, IDC_EDIT_BITRATE1		, m_ctlEditBitrate1		);
	DDX_Control(pDX, IDC_COMBO_FPS1			, m_ctlComboFPS1		);
	DDX_Control(pDX, IDC_COMBO_GOP1			, m_ctlComboGOP1		);
	DDX_Control(pDX, IDC_SLIDER_BITRATE2	, m_ctlSliderBitrate2	);
	DDX_Control(pDX, IDC_EDIT_BITRATE2		, m_ctlEditBitrate2		);
	DDX_Control(pDX, IDC_COMBO_FPS2			, m_ctlComboFPS2		);
	DDX_Control(pDX, IDC_COMBO_GOP2			, m_ctlComboGOP2		);
	DDX_Control(pDX, IDC_COMBO_MJPEG_QUALITY, m_ctlComboMjpegQuality);
	DDX_Control(pDX, IDC_COMBO_MJPEG_FPS	, m_ctlComboMjpegFPS	);
	DDX_Control(pDX, IDC_DEVICE_LIST		, m_ctlListDevice		);
	DDX_Control(pDX, IDC_BUTTON_LENS_INIT	, m_ctlButtonLensInit);
	DDX_Control(pDX, IDC_BUTTON_APPLY, m_ctlButtonApply);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_ctlButtonCancel);
}


BEGIN_MESSAGE_MAP(CResolutionChangeDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_MODELTYPE, &CResolutionChangeDlg::OnCbnSelchangeComboModeltype)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAMTYPE, &CResolutionChangeDlg::OnCbnSelchangeComboStreamtype)
	ON_CBN_SELCHANGE(IDC_COMBO_RESOLUTION, &CResolutionChangeDlg::OnCbnSelchangeComboResolution)
	ON_CBN_SELCHANGE(IDC_COMBO_RATECONTROL, &CResolutionChangeDlg::OnCbnSelchangeComboRatecontrol)
	ON_CBN_SELCHANGE(IDC_COMBO_CODECTYPE, &CResolutionChangeDlg::OnCbnSelchangeComboCodectype)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BITRATE1, &CResolutionChangeDlg::OnNMCustomdrawSliderBitrate1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BITRATE2, &CResolutionChangeDlg::OnNMCustomdrawSliderBitrate2)
	ON_BN_CLICKED(IDC_BUTTON_LENS_INIT, &CResolutionChangeDlg::OnBnClickedButtonLensInit)
	ON_EN_CHANGE(IDC_USERID, &CResolutionChangeDlg::OnEnChangeUserid)
	ON_EN_CHANGE(IDC_PASSWORD, &CResolutionChangeDlg::OnEnChangePassword)
	ON_MESSAGE( UM_STATUS_MESSAGE, &CResolutionChangeDlg::OnMessageStatus)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT_BITRATE1, &CResolutionChangeDlg::OnEnChangeEditBitrate1)
	ON_EN_CHANGE(IDC_EDIT_BITRATE2, &CResolutionChangeDlg::OnEnChangeEditBitrate2)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CResolutionChangeDlg::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CResolutionChangeDlg::OnBnClickedButtonCancel)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CResolutionChangeDlg 메시지 처리기입니다.

BOOL CResolutionChangeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	m_ctlButtonLensInit.EnableWindow(FALSE);

	CString strItem; 
	CString str; 

	m_ctlListDevice.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);


	strItem.LoadString(IDS_MODEL);
	m_ctlListDevice.InsertColumn(SUBITEM_MODEL				, strItem, LVCFMT_CENTER, 110, 0 );	

	str.LoadString(IDS_ADDRESS);
	strItem = L"MAC " + str;
	m_ctlListDevice.InsertColumn(SUBITEM_MACADDRESS			, strItem, LVCFMT_CENTER, 110, 0 );	

	strItem = L"IP" + str;
	m_ctlListDevice.InsertColumn(SUBITEM_IPADDRESS			, strItem, LVCFMT_CENTER, 110, 0 );	

	strItem.LoadString(IDS_USER_ID);
	m_ctlListDevice.InsertColumn(SUBITEM_USER_ID			, strItem, LVCFMT_CENTER, 50, 0 );	

	strItem.LoadString(IDS_USER_PW);
	m_ctlListDevice.InsertColumn(SUBITEM_USER_PW			, strItem, LVCFMT_CENTER, 50, 0 );	

	strItem.LoadString(IDS_PROGRESS_STATUS);
	m_ctlListDevice.InsertColumn(SUBITEM_STATUS				, strItem, LVCFMT_CENTER, 300, 0 );	


	m_ctlComboRateControl.ResetContent();
	m_ctlComboRateControl.AddString(L"CVBR");
	m_ctlComboRateControl.AddString(L"CBR");
	m_ctlComboRateControl.SetCurSel(RATECONTROL_CVBR);


	m_ctlComboMjpegQuality.ResetContent();
	m_ctlComboMjpegQuality.AddString(L"High");
	m_ctlComboMjpegQuality.AddString(L"Normal");
	m_ctlComboMjpegQuality.AddString(L"Low");
	m_ctlComboMjpegQuality.SetCurSel(0);

	m_ctlComboFPS1.ResetContent();
	m_ctlComboFPS2.ResetContent();
	m_ctlComboMjpegFPS.ResetContent();

	m_ctlSliderBitrate1.SetRange( 0, 8000000 );
	m_ctlSliderBitrate1.SetPos( 4000000 );
	m_ctlEditBitrate1.SetWindowText( L"4000000" );
	m_iBitrateStream1	= 4000000;


	m_ctlSliderBitrate2.SetRange( 0, 8000000 );
	m_ctlSliderBitrate2.SetPos( 4000000 );
	m_ctlEditBitrate2.SetWindowText( L"4000000" );
	m_iBitrateStream2	= 4000000;

	WCHAR	szTemp[32]	= {0};

	for( int i=0; i<64; i++ )
	{
		swprintf_s(szTemp, L"02", i+1 );

		if( 30 > i )
		{
			m_ctlComboFPS1.AddString(szTemp);
			m_ctlComboFPS2.AddString(szTemp);
			m_ctlComboMjpegFPS.AddString(szTemp);
		}

		m_ctlComboGOP1.AddString(szTemp);
		m_ctlComboGOP2.AddString(szTemp);
	}

	m_ctlComboFPS1.SetCurSel(29);
	m_ctlComboFPS2.SetCurSel(29);
	m_ctlComboMjpegFPS.SetCurSel(29);

	m_ctlComboGOP1.SetCurSel(29);
	m_ctlComboGOP2.SetCurSel(29);

	InitModelCombo();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

int	CResolutionChangeDlg::GetSelectScanInfoCount()	
{
	return 	m_nSelectScanInfoCnt;
}

BOOL CResolutionChangeDlg::SetScaninfo( int iCount, SCAN_INFO* pScanInfo, int iAdapterID)
{
	if( m_pScanInfo )
	{
		delete [] m_pScanInfo;
		m_pScanInfo	= NULL;
	}

	m_iAdapterID	= iAdapterID;
	m_nScanInfoCnt	= iCount;
	m_pScanInfo		= new SCAN_INFO[iCount];

	for( int i=0; i<iCount; i++ )
	{
		m_pScanInfo[i] = pScanInfo[i];
	}

	return TRUE;
}

void CResolutionChangeDlg::CheckArp(int iAdapterID)
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

int CResolutionChangeDlg::CallHttpRequest(CString ipaddr, int port, CString query, CString &response)
{
	CInternetSession*	pSession		= NULL;
	CHttpConnection*	pHttpConnect	= NULL;
	CHttpFile*			pHttpFile		= NULL;
	CString				url				= query;
	DWORD				HttpRequestFlags;

	try{

		HttpRequestFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;

		// HTTP 메소드별로 사용 가능
		pSession = new CInternetSession;

		pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000); 
		pSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES,1); 
		pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);
		pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5000);

		pHttpConnect = pSession->GetHttpConnection(ipaddr, (INTERNET_PORT)port );
#if 0
		pHttpFile = pHttpConnect->OpenRequest(	CHttpConnection::HTTP_VERB_GET, url);
#else
		pHttpFile = pHttpConnect->OpenRequest(	CHttpConnection::HTTP_VERB_GET,
												url.GetBuffer(1024), 
												NULL, 
												1, 
												NULL, 
												(LPCTSTR)"1.0", 
												HttpRequestFlags);
		url.ReleaseBuffer();
#endif

		// Use direct write to posting field!
		CString strHeaders = L"Accept: text/*\r\n";
		strHeaders += L"User-Agent: HttpCall\r\n";
		strHeaders += L"Accept-Language: UFT-8\r\n";

		pHttpFile->AddRequestHeaders(strHeaders);

		pHttpFile->SendRequest();
	}
	catch(CInternetException *m_pEx)
	{
//		m_pEx->ReportError();
		m_pEx->Delete();
		return 0;
	}

	// result값 확인
	DWORD m_dwStatusCode;
	pHttpFile->QueryInfoStatusCode(m_dwStatusCode);

	// Read Data
	CString strRetBufLen;
	pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, strRetBufLen);

	CString szResult = L"";
	char c[1024*64];
	while(pHttpFile->ReadString((LPTSTR)c,1024)!=FALSE)
	{
		response += c;
	}

	pHttpFile->Flush();

	// 객체 delete 부분
	if(pHttpFile)
	{
		pHttpFile->Close();
		delete pHttpFile;
		pHttpFile = NULL;
	}
	if(pHttpConnect)
	{
		pHttpConnect->Close();
		delete pHttpConnect;
		pHttpConnect = NULL;
	}
	if(pSession)
	{
		pSession->Close();
		delete pSession;
		pSession = NULL;
	}

	return 0;
}

void CResolutionChangeDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	if( m_hThreadLensInit )
	{
		m_dwThreadLensInitID	= 0;
		WaitForSingleObject( m_hThreadLensInit, INFINITE );

		CloseHandle(m_hThreadLensInit);
		m_hThreadLensInit	= NULL;
	}

	for( int iItem=0; iItem<m_aSettingThread.GetCount(); iItem++ )
	{
		delete m_aSettingThread[iItem];
		m_aSettingThread[iItem]	= NULL;
	}

	m_aSettingThread.RemoveAll();
}


void CResolutionChangeDlg::InitModelCombo()
{
	CString strModelType;
	CString strTemp;
	int		iIndex = 0;

	//Model Combo에 Model 추가
	m_ctlComboModelType.ResetContent();

	for(int i=0;i<m_nScanInfoCnt;i++)
	{
		// extended infomation printout
		if(m_pScanInfo[i].nExtraFieldCount)
		{
			strModelType = m_pScanInfo[i]._ReadValue(L"Model Type");

			if( 0 <= strModelType.Find(L"Mi") 	|| 
				0 <= strModelType.Find(L"M2i") 	|| 
				0 <= strModelType.Find(L"M3i") 	|| 
				0 <= strModelType.Find(L" i") 	|| 
				0 <= strModelType.Find(L"NVS102") )
			{
				// Next Chip 시리즈는 기능 적용시키지 않는다. 
			}
			else if( 0 <= strModelType.Find(L"NVS202") )
			{
				// TI 이지만 당분간 NVS202 는 지원하지 않는다.
			}
			else
			{
				int iIndex = m_ctlComboModelType.FindString(0, strModelType);
				if( iIndex == -1 )
				{
					m_ctlComboModelType.AddString(strModelType);
				}
				else
				{
					m_ctlComboModelType.GetLBText(iIndex, strTemp);
					if( 0 != strTemp.Compare(strModelType) )
					{
						m_ctlComboModelType.AddString(strModelType);
					}
				}
			}
		}
	}

	if( m_ctlComboModelType.GetCount() )
	{
		m_ctlComboModelType.SetCurSel(0);
		OnCbnSelchangeComboModeltype();
	}
	else
	{
		CDialog::OnCancel();
	}
}

void CResolutionChangeDlg::OnCbnSelchangeComboModeltype()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CString		strTemp;
	LV_ITEM		item;
	int			iItemIndex	= 0;
	int			iItemCount	= m_ctlListDevice.GetItemCount();

	m_ctlComboModelType.GetLBText(m_ctlComboModelType.GetCurSel(), m_strSelectedModel);

	for(iItemIndex=0; iItemIndex< iItemCount; iItemIndex++)
	{
		m_ctlListDevice.DeleteEdit(iItemIndex,     SUBITEM_USER_ID);   // memory leak fix
		m_ctlListDevice.DeleteEdit(iItemIndex,     SUBITEM_USER_PW);
	}

	m_ctlListDevice.DeleteAllItems();

	///////////////////////////////////////////////////////////////////////////
	// strSelected 모델과 같은 모델들만 리스트에 추가 한다. ///////////////////

	m_nSelectScanInfoCnt	= 0;
	iItemIndex				= 0;
	for(int i=0;i<m_nScanInfoCnt;i++)
	{
		memset(&item, 0, sizeof(item));
		strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");

		if(m_strSelectedModel.Compare(strTemp) == 0)
		{
			strTemp = m_pScanInfo[i]._ReadValue(L"Model Type");	
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = iItemIndex;
			item.iSubItem = SUBITEM_MODEL;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			item.lParam = (LPARAM)&m_pScanInfo[i];
			m_ctlListDevice.InsertItem(&item);

			strTemp = m_pScanInfo[i].szMAC;	
			item.mask = LVIF_TEXT;
			item.iItem = iItemIndex;
			item.iSubItem = SUBITEM_MACADDRESS;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_ctlListDevice.SetItem(&item);

			strTemp = m_pScanInfo[i].szAddr;
			item.mask = LVIF_TEXT;
			item.iItem = iItemIndex;
			item.iSubItem = SUBITEM_IPADDRESS;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_ctlListDevice.SetItem(&item);

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

			m_ctlListDevice.SetItemData( iItemIndex, (DWORD_PTR)&m_pScanInfo[i] );

			iItemIndex++;
			m_nSelectScanInfoCnt++;
		}
	}

	// strSelected 모델과 같은 모델들만 리스트에 추가 한다. ///////////////////
	///////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////
	// 리스트에 추가된 항목이 하나라도 있다면 Stream Type 콤보를 초기화 한다.//

	m_ctlComboStreamType.ResetContent();

	if( 0 < iItemIndex )
	{
		m_ctlComboStreamType.AddString(L"Single");	
		m_ctlComboStreamType.AddString(L"Dual");	
		m_ctlComboStreamType.AddString(L"Triple");	

		m_ctlComboStreamType.SetCurSel(STREAMTYPE_SINGLE);
		OnCbnSelchangeComboStreamtype();
	}

	// 리스트에 추가된 항목이 하나라도 있다면 Stream Type 콤보를 초기화 한다.//
	///////////////////////////////////////////////////////////////////////////


	if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) ||
		0 <= m_strSelectedModel.Find( L"VN7xi" ) )
	{
		m_ctlButtonLensInit.EnableWindow(TRUE);
	}
	else
	{
		m_ctlButtonLensInit.EnableWindow(FALSE);
	}
}

void CResolutionChangeDlg::OnCbnSelchangeComboStreamtype()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_iSelectStreamType	= m_ctlComboStreamType.GetCurSel();

	m_ctlComboCodec.ResetContent();
	int		iAddIndex	= 0;

	switch( m_iSelectStreamType )
	{
	case STREAMTYPE_SINGLE	:
		iAddIndex	= m_ctlComboCodec.AddString(L"H.264");
		m_ctlComboCodec.SetItemData(iAddIndex, CODEC_SINGLE_H264);

		EnableControlH264Stream1(TRUE);
		EnableControlH264Stream2(FALSE);
		EnableControlMJPEGStream(FALSE);
		break;
	case STREAMTYPE_DUAL	:
		iAddIndex	= m_ctlComboCodec.AddString(L"H.264 + H.264");
		m_ctlComboCodec.SetItemData(iAddIndex, CODEC_DUAL_H264_H264);

		iAddIndex	= m_ctlComboCodec.AddString(L"H.264 + MJPEG");
		m_ctlComboCodec.SetItemData(iAddIndex, CODEC_DUAL_H264_MJPEG);

		EnableControlH264Stream1(TRUE);
		EnableControlH264Stream2(TRUE);
		EnableControlMJPEGStream(FALSE);
		break;
	case STREAMTYPE_TRIPLE	:
		iAddIndex	= m_ctlComboCodec.AddString(L"H.264 + MJPEG + H.264");
		m_ctlComboCodec.SetItemData(iAddIndex, CODEC_TRIPLE_H264_MJPEG_H264);

		EnableControlH264Stream1(TRUE);
		EnableControlH264Stream2(TRUE);
		EnableControlMJPEGStream(TRUE);
		break;
	}

	if( m_ctlComboCodec.GetCount() )
	{
		m_ctlComboCodec.SetCurSel(0);
		OnCbnSelchangeComboCodectype();
	}
}

void CResolutionChangeDlg::EnableControlMain( BOOL bEnable )
{
	GetDlgItem(IDC_STATIC_MODELTYPE			)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_MODELTYPE			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_GROUP_STREAMFORMAT)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_STREAMTYPE		)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_STREAMTYPE			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_CODEC				)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_CODECTYPE			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_RESOLUTION		)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_RESOLUTION			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_RATECONTROL		)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_RATECONTROL		)->EnableWindow(bEnable);
}

void CResolutionChangeDlg::EnableControlFunction( BOOL bEnable )
{
	GetDlgItem(IDC_STC_GROUP_FUNCTION		)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_LENS_INIT			)->EnableWindow(bEnable);
}

void CResolutionChangeDlg::EnableControlUserInfo( BOOL bEnable )
{
	GetDlgItem(IDC_STC_GROUP_LOGIN	)->EnableWindow(bEnable);
	GetDlgItem(IDC_STC_USERID		)->EnableWindow(bEnable);
	GetDlgItem(IDC_STC_PASSWORD		)->EnableWindow(bEnable);
	GetDlgItem(IDC_USERID			)->EnableWindow(bEnable);
	GetDlgItem(IDC_PASSWORD			)->EnableWindow(bEnable);
}

void CResolutionChangeDlg::EnableControlH264Stream1( BOOL bEnable )
{
	GetDlgItem(IDC_STATIC_GROUP_STREAM1	)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_BITRATE1		)->EnableWindow(bEnable);
	GetDlgItem(IDC_SLIDER_BITRATE1		)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_BITRATE1		)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_FRAMERATE1	)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_FPS1			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_GOP1			)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_GOP1			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_FPS1			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_BPS1			)->EnableWindow(bEnable);

	if( TRUE == bEnable )
	{
		// 컨트롤 초기화 시킨다. 
	}
}

void CResolutionChangeDlg::EnableControlH264Stream2( BOOL bEnable )
{
	GetDlgItem(IDC_STATIC_GROUP_STREAM2	)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_BITRATE2		)->EnableWindow(bEnable);
	GetDlgItem(IDC_SLIDER_BITRATE2		)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_BITRATE2		)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_FRAMERATE2	)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_FPS2			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_GOP2			)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_GOP2			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_FPS2			)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_BPS2			)->EnableWindow(bEnable);
	

	if( TRUE == bEnable )
	{
		// 컨트롤 초기화 시킨다. 
	}
	
}

void CResolutionChangeDlg::EnableControlMJPEGStream( BOOL bEnable )
{
	GetDlgItem(IDC_STATIC_GROUP_MJPEG	)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_QUALITY		)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_MJPEG_QUALITY	)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_FRAMERATE		)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMBO_MJPEG_FPS		)->EnableWindow(bEnable);

	if( TRUE == bEnable )
	{
		// 컨트롤 초기화 시킨다. 
		m_ctlComboMjpegQuality.SetCurSel(0);
		m_ctlComboMjpegFPS.SetCurSel(29);
	}

}

void CResolutionChangeDlg::InitResolutionComboForStreamSingle()
{
	m_ctlComboResolution.ResetContent();

	if( 0 <= m_strSelectedModel.Find( L"NVS202" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:960*576" );		// RESOL_S_H_960_576
		m_ctlComboResolution.SetItemData(0, RESOL_S_H_960_576 );

		m_ctlComboResolution.AddString( L"H.264:720*576" );		// RESOL_S_H_720_576
		m_ctlComboResolution.SetItemData(1, RESOL_S_H_720_576 );

		m_ctlComboResolution.AddString( L"H.264:960*288" );		// RESOL_S_H_960_288
		m_ctlComboResolution.SetItemData(2, RESOL_S_H_960_288 );

		m_ctlComboResolution.AddString( L"H.264:720*288" );		// RESOL_S_H_720_288
		m_ctlComboResolution.SetItemData(3, RESOL_S_H_720_288 );

		m_ctlComboResolution.AddString( L"H.264:480*288" );		// RESOL_S_H_480_288
		m_ctlComboResolution.SetItemData(4, RESOL_S_H_480_288 );

		m_ctlComboResolution.AddString( L"H.264:352*288" );		// RESOL_S_H_352_288
		m_ctlComboResolution.SetItemData(5, RESOL_S_H_352_288 );
	}
	//else if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	//{
	//	m_ctlComboResolution.AddString( L"H.264:D1  " );		// RESOL_S_H_D1	
	//	m_ctlComboResolution.SetItemData(0, RESOL_S_H_D1 );

	//	m_ctlComboResolution.AddString( L"H.264:HD  " );		// RESOL_S_H_HD	
	//	m_ctlComboResolution.SetItemData(1, RESOL_S_H_HD );

	//	m_ctlComboResolution.AddString( L"H.264:FHD " );		// RESOL_S_H_FHD
	//	m_ctlComboResolution.SetItemData(2, RESOL_S_H_FHD );

	//	m_ctlComboResolution.AddString( L"H.264:QXGA" );		// RESOL_S_H_QXGA
	//	m_ctlComboResolution.SetItemData(3, RESOL_S_H_QXGA );
	//}
	else if( 0 <= m_strSelectedModel.Find( L"M2TiA" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1  " );		// RESOL_S_H_D1	
		m_ctlComboResolution.SetItemData(0, RESOL_S_H_D1 );

		m_ctlComboResolution.AddString( L"H.264:HD  " );		// RESOL_S_H_HD	
		m_ctlComboResolution.SetItemData(1, RESOL_S_H_HD );

		m_ctlComboResolution.AddString( L"H.264:SXGA" );		// RESOL_S_H_SXGA
		m_ctlComboResolution.SetItemData(2, RESOL_S_H_SXGA );

		m_ctlComboResolution.AddString( L"H.264:FHD " );		// RESOL_S_H_FHD
		m_ctlComboResolution.SetItemData(3, RESOL_S_H_FHD );
	}
	else if( 0 <= m_strSelectedModel.Find( L"M2Ti" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1  " );		// RESOL_S_H_D1	
		m_ctlComboResolution.SetItemData(0, RESOL_S_H_D1 );

		m_ctlComboResolution.AddString( L"H.264:HD  " );		// RESOL_S_H_HD	
		m_ctlComboResolution.SetItemData(1, RESOL_S_H_HD );

		m_ctlComboResolution.AddString( L"H.264:FHD " );		// RESOL_S_H_FHD
		m_ctlComboResolution.SetItemData(2, RESOL_S_H_FHD );
	}
	else if( 0 <= m_strSelectedModel.Find( L"MTi" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1  " );		// RESOL_S_H_D1	
		m_ctlComboResolution.SetItemData(0, RESOL_S_H_D1 );

		m_ctlComboResolution.AddString( L"H.264:HD  " );		// RESOL_S_H_HD	
		m_ctlComboResolution.SetItemData(1, RESOL_S_H_HD );

		m_ctlComboResolution.AddString( L"H.264:SXGA" );		// RESOL_S_H_SXGA
		m_ctlComboResolution.SetItemData(2, RESOL_S_H_SXGA );
	}
	else if( 0 <= m_strSelectedModel.Find( L"VD102i" ) ||
			 0 <= m_strSelectedModel.Find( L"VN200i" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1  " );		// RESOL_S_H_D1	
		m_ctlComboResolution.SetItemData(0, RESOL_S_H_D1 );

		m_ctlComboResolution.AddString( L"H.264:HD  " );		// RESOL_S_H_HD	
		m_ctlComboResolution.SetItemData(1, RESOL_S_H_HD );

		m_ctlComboResolution.AddString( L"H.264:SXGA" );		// RESOL_S_H_SXGA
		m_ctlComboResolution.SetItemData(2, RESOL_S_H_SXGA );

		m_ctlComboResolution.AddString( L"H.264:FHD " );		// RESOL_S_H_FHD
		m_ctlComboResolution.SetItemData(3, RESOL_S_H_FHD );
	}
	//else if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	else if( 0 <= m_strSelectedModel.Find( L"VN7xi" ) ||
			 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1  " );		// RESOL_S_H_D1	
		m_ctlComboResolution.SetItemData(0, RESOL_S_H_D1 );

		m_ctlComboResolution.AddString( L"H.264:HD  " );		// RESOL_S_H_HD	
		m_ctlComboResolution.SetItemData(1, RESOL_S_H_HD );

		m_ctlComboResolution.AddString( L"H.264:FHD " );		// RESOL_S_H_FHD
		m_ctlComboResolution.SetItemData(2, RESOL_S_H_FHD );

		m_ctlComboResolution.AddString( L"H.264:QXGA" );		// RESOL_S_H_QXGA
		m_ctlComboResolution.SetItemData(3, RESOL_S_H_QXGA );
	}
}

void CResolutionChangeDlg::InitResolutionComboForStreamDualHH()
{
	m_ctlComboResolution.ResetContent();

	if( 0 <= m_strSelectedModel.Find( L"NVS202" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:352*288 + H.264:352*288" );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HH_352X288_352X288 );

		m_ctlComboResolution.AddString( L"H.264:480*288 + H.264:480*288" );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HH_480X288_480X288 );
		
		m_ctlComboResolution.AddString( L"H.264:720*288 + H.264:352*288" );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HH_720X288_352X288 );
		
		m_ctlComboResolution.AddString( L"H.264:720*288 + H.264:720*288" );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HH_720X288_720X288 );
		
		m_ctlComboResolution.AddString( L"H.264:960*288 + H.264:480*288" );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HH_960X288_480X288 );
		
		m_ctlComboResolution.AddString( L"H.264:960*288 + H.264:960*288" );
		m_ctlComboResolution.SetItemData(5, RESOL_D_HH_960X288_960X288 );
		
		m_ctlComboResolution.AddString( L"H.264:720*576 + H.264:352*288" );
		m_ctlComboResolution.SetItemData(6, RESOL_D_HH_720X576_352X288 );
		
		m_ctlComboResolution.AddString( L"H.264:720*576 + H.264:720*288" );
		m_ctlComboResolution.SetItemData(7, RESOL_D_HH_720X576_720X288 );
		
		m_ctlComboResolution.AddString( L"H.264:720*576 + H.264:720*576" );
		m_ctlComboResolution.SetItemData(8, RESOL_D_HH_720X576_720X576 );
		
		m_ctlComboResolution.AddString( L"H.264:960*576 + H.264:480*288" );
		m_ctlComboResolution.SetItemData(9, RESOL_D_HH_960X576_480X288 );
		
		m_ctlComboResolution.AddString( L"H.264:960*576 + H.264:960*288" );
		m_ctlComboResolution.SetItemData(10, RESOL_D_HH_960X576_960X288 );
		
		m_ctlComboResolution.AddString( L"H.264:960*576 + H.264:960*576" );
		m_ctlComboResolution.SetItemData(11, RESOL_D_HH_960X576_960X576 );
	}
	//else if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	//{
	//	m_ctlComboResolution.AddString( L"H.264:D1 + H.264:D1  " );
	//	m_ctlComboResolution.SetItemData(0, RESOL_D_HH______D1______D1 );

	//	m_ctlComboResolution.AddString( L"H.264:HD + H.264:CIF " );
	//	m_ctlComboResolution.SetItemData(1, RESOL_D_HH______HD_____CIF );
	//}
	else if( 0 <= m_strSelectedModel.Find( L"M2TiA" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + H.264:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HH______D1______D1 );

		m_ctlComboResolution.AddString( L"H.264:HD + H.264:CIF " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HH______HD_____CIF );

		m_ctlComboResolution.AddString( L"H.264:SXGA + H.264:D1  " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HH____SXGA______D1 );

		m_ctlComboResolution.AddString( L"H.264:FHD + H.264:CIF  " );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HH_____FHD_____CIF );

		m_ctlComboResolution.AddString( L"H.264:FHD + H.264:VGA  " );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HH_____FHD_____VGA );
	}
	else if( 0 <= m_strSelectedModel.Find( L"M2Ti" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + H.264:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HH______D1______D1 );

		m_ctlComboResolution.AddString( L"H.264:HD + H.264:CIF " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HH______HD_____CIF );
	}
	else if( 0 <= m_strSelectedModel.Find( L"MTi" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + H.264:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HH______D1______D1 );

		m_ctlComboResolution.AddString( L"H.264:HD + H.264:QVGA" );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HH______HD____QVGA );

		m_ctlComboResolution.AddString( L"H.264:SXGA + H.264:D1  " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HH____SXGA______D1 );
	}
	else if( 0 <= m_strSelectedModel.Find( L"VD102i" ) ||
			 0 <= m_strSelectedModel.Find( L"VN200i" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + H.264:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HH______D1______D1 );

		m_ctlComboResolution.AddString( L"H.264:HD + H.264:CIF " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HH______HD_____CIF );

		m_ctlComboResolution.AddString( L"H.264:SXGA + H.264:D1  " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HH____SXGA______D1 );

		m_ctlComboResolution.AddString( L"H.264:FHD + H.264:CIF " );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HH_____FHD_____CIF );

		m_ctlComboResolution.AddString( L"H.264:FHD + H.264:VGA " );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HH_____FHD_____VGA );
	}
	//else if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	else if( 0 <= m_strSelectedModel.Find( L"VN7xi" ) ||
			 0 <= m_strSelectedModel.Find( L"M3Ti")	)
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + H.264:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HH______D1______D1 );

		m_ctlComboResolution.AddString( L"H.264:HD + H.264:CIF " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HH______HD_____CIF );

		m_ctlComboResolution.AddString( L"H.264:FHD + H.264:CIF " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HH_____FHD_____CIF );

		m_ctlComboResolution.AddString( L"H.264:FHD + H.264:VGA " );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HH_____FHD_____VGA );
	}
}

void CResolutionChangeDlg::InitResolutionComboForStreamDualHM()
{
	m_ctlComboResolution.ResetContent();

	if( 0 <= m_strSelectedModel.Find( L"NVS202" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:352*288 + MJPEG:352*288" );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HM_352X288_352X288 );
		m_ctlComboResolution.AddString( L"H.264:480*288 + MJPEG:480*288" );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HM_480X288_480X288 );
		m_ctlComboResolution.AddString( L"H.264:720*288 + MJPEG:352*288" );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HM_720X288_352X288 );
		m_ctlComboResolution.AddString( L"H.264:720*288 + MJPEG:720*288" );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HM_720X288_720X288 );
		m_ctlComboResolution.AddString( L"H.264:960*288 + MJPEG:480*288" );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HM_960X288_480X288 );
		m_ctlComboResolution.AddString( L"H.264:960*288 + MJPEG:960*288" );
		m_ctlComboResolution.SetItemData(5, RESOL_D_HM_960X288_960X288 );
		m_ctlComboResolution.AddString( L"H.264:720*576 + MJPEG:352*288" );
		m_ctlComboResolution.SetItemData(6, RESOL_D_HM_720X576_352X288 );
		m_ctlComboResolution.AddString( L"H.264:720*576 + MJPEG:720*288" );
		m_ctlComboResolution.SetItemData(7, RESOL_D_HM_720X576_720X288 );
		m_ctlComboResolution.AddString( L"H.264:720*576 + MJPEG:720*576" );
		m_ctlComboResolution.SetItemData(8, RESOL_D_HM_720X576_720X576 );
		m_ctlComboResolution.AddString( L"H.264:960*576 + MJPEG:480*288" );
		m_ctlComboResolution.SetItemData(9, RESOL_D_HM_960X576_480X288 );
		m_ctlComboResolution.AddString( L"H.264:960*576 + MJPEG:960*288" );
		m_ctlComboResolution.SetItemData(10, RESOL_D_HM_960X576_960X288 );
		m_ctlComboResolution.AddString( L"H.264:960*576 + MJPEG:960*576" );
		m_ctlComboResolution.SetItemData(11, RESOL_D_HM_960X576_960X576 );
	}
	//else if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	//{
	//	m_ctlComboResolution.AddString( L"H.264:D1 + MJPEG:D1  " );
	//	m_ctlComboResolution.SetItemData(0, RESOL_D_HM______D1______D1 );
	//	m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA " );
	//	m_ctlComboResolution.SetItemData(1, RESOL_D_HM______HD_____VGA );
	//	m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:QVGA" );
	//	m_ctlComboResolution.SetItemData(2, RESOL_D_HM_____FHD____QVGA );
	//	m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:VGA " );
	//	m_ctlComboResolution.SetItemData(3, RESOL_D_HM_____FHD_____VGA );
	//	m_ctlComboResolution.AddString( L"H.264:QXGA + MJPEG:QVGA" );
	//	m_ctlComboResolution.SetItemData(4, RESOL_D_HM____QXGA____QVGA );
	//	m_ctlComboResolution.AddString( L"H.264:QXGA + MJPEG:VGA " );
	//	m_ctlComboResolution.SetItemData(5, RESOL_D_HM____QXGA_____VGA );
	//}
	else if( 0 <= m_strSelectedModel.Find( L"M2TiA" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + MJPEG:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HM______D1______D1 );
		m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HM______HD_____VGA );
		m_ctlComboResolution.AddString( L"H.264:SXGA + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HM____SXGA_____VGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:QVGA" );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HM_____FHD____QVGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HM_____FHD_____VGA );
	}
	else if( 0 <= m_strSelectedModel.Find( L"M2Ti" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + MJPEG:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HM______D1______D1 );
		m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HM______HD_____VGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:QVGA" );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HM_____FHD____QVGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HM_____FHD_____VGA );
	}
	else if( 0 <= m_strSelectedModel.Find( L"MTi" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + MJPEG:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HM______D1______D1 );
		m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HM______HD_____VGA );
		m_ctlComboResolution.AddString( L"H.264:SXGA + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HM____SXGA_____VGA );
	}
	else if( 0 <= m_strSelectedModel.Find( L"VD102i" ) ||
			 0 <= m_strSelectedModel.Find( L"VN200i" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + MJPEG:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HM______D1______D1 );
		m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HM______HD_____VGA );
		m_ctlComboResolution.AddString( L"H.264:SXGA + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HM____SXGA_____VGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:QVGA" );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HM_____FHD____QVGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HM_____FHD_____VGA );
	}
	//else if( 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	else if( 0 <= m_strSelectedModel.Find( L"VN7xi" ) ||
			 0 <= m_strSelectedModel.Find( L"M3Ti" ) )
	{
		m_ctlComboResolution.AddString( L"H.264:D1 + MJPEG:D1  " );
		m_ctlComboResolution.SetItemData(0, RESOL_D_HM______D1______D1 );
		m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(1, RESOL_D_HM______HD_____VGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:QVGA" );
		m_ctlComboResolution.SetItemData(2, RESOL_D_HM_____FHD____QVGA );
		m_ctlComboResolution.AddString( L"H.264:FHD + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(3, RESOL_D_HM_____FHD_____VGA );
		m_ctlComboResolution.AddString( L"H.264:QXGA + MJPEG:QVGA" );
		m_ctlComboResolution.SetItemData(4, RESOL_D_HM____QXGA____QVGA );
		m_ctlComboResolution.AddString( L"H.264:QXGA + MJPEG:VGA " );
		m_ctlComboResolution.SetItemData(5, RESOL_D_HM____QXGA_____VGA );
	}
}

void CResolutionChangeDlg::InitResolutionComboForStreamTriple()
{
	m_ctlComboResolution.ResetContent();
	m_ctlComboResolution.AddString( L"H.264:HD + MJPEG:VGA + H.264:CIF" );
	m_ctlComboResolution.SetItemData(0, RESOL_T_HMH___HD__VGA__CIF );
}

void CResolutionChangeDlg::OnCbnSelchangeComboCodectype()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iSelectCodec	= m_ctlComboCodec.GetItemData(m_ctlComboCodec.GetCurSel());

	//SetDlgItemText( IDC_USERID	, L"" );
	//SetDlgItemText( IDC_PASSWORD, L"" );

	switch( m_iSelectStreamType )
	{
	case STREAMTYPE_SINGLE	:
		if( CODEC_SINGLE_H264 == m_iSelectCodec )
		{
			EnableControlH264Stream1(TRUE);
			EnableControlH264Stream2(FALSE);
			EnableControlMJPEGStream(FALSE);
			InitResolutionComboForStreamSingle();
		}
		break;
	case STREAMTYPE_DUAL	:
		if( CODEC_DUAL_H264_H264 == m_iSelectCodec )
		{
			EnableControlH264Stream1(TRUE);
			EnableControlH264Stream2(TRUE);
			EnableControlMJPEGStream(FALSE);

			InitResolutionComboForStreamDualHH();
		}
		else if( CODEC_DUAL_H264_MJPEG == m_iSelectCodec )
		{
			EnableControlH264Stream1(TRUE);
			EnableControlH264Stream2(FALSE);
			EnableControlMJPEGStream(TRUE);

			InitResolutionComboForStreamDualHM();
		}
		break;
	case STREAMTYPE_TRIPLE	:
		if( CODEC_TRIPLE_H264_MJPEG_H264 == m_iSelectCodec )
		{
			EnableControlH264Stream1(TRUE);
			EnableControlH264Stream2(TRUE);
			EnableControlMJPEGStream(TRUE);

			InitResolutionComboForStreamTriple();
		}
		break;
	}

	if( m_ctlComboResolution.GetCount() )
	{
		m_ctlComboResolution.SetCurSel(0);
		OnCbnSelchangeComboResolution();
	}
}

void CResolutionChangeDlg::OnCbnSelchangeComboResolution()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int		iResolution	=0;
	WCHAR	szTemp[32]	= {0};

	m_iSelectResolution	= m_ctlComboResolution.GetCurSel();
	iResolution			= m_ctlComboResolution.GetItemData(m_iSelectResolution);

	if( RESOL_S_H_QXGA				== iResolution ||
		RESOL_D_HH____QXGA____QVGA	== iResolution ||
		RESOL_D_HH____QXGA_____VGA	== iResolution ||
		RESOL_D_HM____QXGA____QVGA	== iResolution ||
		RESOL_D_HM____QXGA_____VGA	== iResolution )
	{
		m_ctlComboFPS1.ResetContent();
		m_ctlComboFPS2.ResetContent();
		m_ctlComboMjpegFPS.ResetContent();

		for( int i=0; i<20; i++ )
		{
			swprintf_s(szTemp, L"%02d", i+1 );
			m_ctlComboFPS1.AddString(szTemp);
			m_ctlComboFPS2.AddString(szTemp);
			m_ctlComboMjpegFPS.AddString(szTemp);
		}

		m_ctlComboFPS1.SetCurSel(19);
		m_ctlComboFPS2.SetCurSel(19);
		m_ctlComboMjpegFPS.SetCurSel(19);
	}
	else
	{
		m_ctlComboFPS1.ResetContent();
		m_ctlComboFPS2.ResetContent();
		m_ctlComboMjpegFPS.ResetContent();

		for( int i=0; i<30; i++ )
		{
			swprintf_s(szTemp, L"%02d", i+1 );
			m_ctlComboFPS1.AddString(szTemp);
			m_ctlComboFPS2.AddString(szTemp);
			m_ctlComboMjpegFPS.AddString(szTemp);
		}

		m_ctlComboFPS1.SetCurSel(29);
		m_ctlComboFPS2.SetCurSel(29);
		m_ctlComboMjpegFPS.SetCurSel(29);
	}

	//////////////////////////////////////////
	// H.264 First Stream Bitrate 설정 ///////

	switch( iResolution )
	{
	case RESOL_S_H_FHD				:			
	case RESOL_S_H_QXGA				:			
	case RESOL_D_HH_____FHD_____CIF	:			
	case RESOL_D_HH_____FHD____QVGA	:			
	case RESOL_D_HH_____FHD_____VGA	:			
	case RESOL_D_HH____QXGA____QVGA	:			
	case RESOL_D_HH____QXGA_____VGA	:			
	case RESOL_D_HM_____FHD_____CIF	:			
	case RESOL_D_HM_____FHD____QVGA	:			
	case RESOL_D_HM_____FHD_____VGA	:			
	case RESOL_D_HM____QXGA____QVGA	:			
	case RESOL_D_HM____QXGA_____VGA	:			
		{
			m_ctlSliderBitrate1.SetRange( 100000, 8000000 );	
			m_ctlSliderBitrate1.SetPos( 4000000 );
			m_ctlEditBitrate1.SetWindowText( L"4000000" );
		}
		break;
	case RESOL_S_H_HD				:
	case RESOL_S_H_SXGA				:
	case RESOL_D_HH____SXGA_____VGA :
	case RESOL_D_HH____SXGA______D1 :
	case RESOL_D_HH______HD_____CIF	:
	case RESOL_D_HH______HD____QVGA	:
	case RESOL_D_HM______HD_____CIF	:
	case RESOL_D_HM______HD____QVGA	:
	case RESOL_D_HM______HD_____VGA	:
	case RESOL_D_HM____SXGA_____VGA	:
	case RESOL_D_HM____SXGA______D1	:
	case RESOL_T_HMH___HD__VGA__CIF	:
		m_ctlSliderBitrate1.SetRange( 100000, 4000000 );	
		m_ctlSliderBitrate1.SetPos( 2000000 );
		m_ctlEditBitrate1.SetWindowText( L"2000000" );
		break;
	case RESOL_S_H_D1				:			
	case RESOL_S_H_960_576			:			
	case RESOL_S_H_720_576			:		
	case RESOL_D_HH______D1______D1	:			
	case RESOL_D_HH_960X480_960X480	:			
	case RESOL_D_HH_960X480_960X240	:			
	case RESOL_D_HH_960X480_480X240	:			
	case RESOL_D_HH_720X480_720X480	:			
	case RESOL_D_HH_720X480_720X240	:			
	case RESOL_D_HH_720X480_352X240	:			
	case RESOL_D_HH_960X576_960X576	:
	case RESOL_D_HH_960X576_960X288	:
	case RESOL_D_HH_960X576_480X288	:
	case RESOL_D_HH_720X576_720X576	:
	case RESOL_D_HH_720X576_720X288	:
	case RESOL_D_HH_720X576_352X288	:
	case RESOL_D_HM______D1______D1	:
	case RESOL_D_HM_960X480_960X480	:
	case RESOL_D_HM_960X480_960X240	:
	case RESOL_D_HM_960X480_480X240	:
	case RESOL_D_HM_720X480_720X480	:
	case RESOL_D_HM_720X480_720X240	:
	case RESOL_D_HM_720X480_352X240	:
	case RESOL_D_HM_960X576_960X576	:
	case RESOL_D_HM_960X576_960X288	:
	case RESOL_D_HM_960X576_480X288	:
	case RESOL_D_HM_720X576_720X576	:
	case RESOL_D_HM_720X576_720X288	:
	case RESOL_D_HM_720X576_352X288	:
		m_ctlSliderBitrate1.SetRange( 100000, 2000000 );	
		m_ctlSliderBitrate1.SetPos( 1000000 );
		m_ctlEditBitrate1.SetWindowText( L"1000000" );
		break;
	case RESOL_S_H_960_288			:
	case RESOL_S_H_720_288			:
	case RESOL_D_HH_960X240_960X240	:
	case RESOL_D_HH_960X240_480X240	:
	case RESOL_D_HH_720X240_720X240	:
	case RESOL_D_HH_720X240_352X240	:
	case RESOL_D_HH_960X288_960X288	:
	case RESOL_D_HH_960X288_480X288	:
	case RESOL_D_HH_720X288_720X288	:
	case RESOL_D_HH_720X288_352X288	:
	case RESOL_D_HM_960X240_960X240	:
	case RESOL_D_HM_960X240_480X240	:
	case RESOL_D_HM_720X240_720X240	:
	case RESOL_D_HM_720X240_352X240	:
	case RESOL_D_HM_960X288_960X288	:
	case RESOL_D_HM_960X288_480X288	:
	case RESOL_D_HM_720X288_720X288	:
	case RESOL_D_HM_720X288_352X288	:
		m_ctlSliderBitrate1.SetRange( 100000, 1000000 );	
		m_ctlSliderBitrate1.SetPos( 500000 );
		m_ctlEditBitrate1.SetWindowText( L"500000" );
		break;
	case RESOL_S_H_480_288			:
	case RESOL_S_H_352_288			:
	case RESOL_D_HH_480X240_480X240	:
	case RESOL_D_HH_352X240_352X240	:
	case RESOL_D_HH_480X288_480X288	:
	case RESOL_D_HH_352X288_352X288	:
	case RESOL_D_HM_480X240_480X240	:
	case RESOL_D_HM_352X240_352X240	:
	case RESOL_D_HM_480X288_480X288	:
	case RESOL_D_HM_352X288_352X288	:
		m_ctlSliderBitrate1.SetRange( 100000,  500000 );	
		m_ctlSliderBitrate1.SetPos( 300000 );
		m_ctlEditBitrate1.SetWindowText( L"500000" );
		break;
	}

	// H.264 First Stream Bitrate 설정 ///////
	//////////////////////////////////////////


	//////////////////////////////////////////
	// H.264 Second Stream Bitrate 설정 //////

	switch( m_iSelectStreamType )
	{
	case STREAMTYPE_DUAL	:
		if( CODEC_DUAL_H264_H264 == m_iSelectCodec )
		{
			switch( iResolution )
			{
			case RESOL_D_HH______D1______D1:
			case RESOL_D_HH______HD____QVGA:
			case RESOL_D_HH____SXGA_____VGA:
			case RESOL_D_HH____SXGA______D1:
			case RESOL_D_HH_____FHD____QVGA:
			case RESOL_D_HH_____FHD_____VGA:
			case RESOL_D_HH____QXGA____QVGA:
			case RESOL_D_HH____QXGA_____VGA:
			case RESOL_D_HH_960X480_960X480:
			case RESOL_D_HH_960X576_960X576:
			case RESOL_D_HH_960X480_960X240:
			case RESOL_D_HH_960X576_960X288:
			case RESOL_D_HH_960X480_480X240:
			case RESOL_D_HH_960X576_480X288:
			case RESOL_D_HH_720X480_720X480:
			case RESOL_D_HH_720X576_720X576:
			case RESOL_D_HH_720X480_720X240:
			case RESOL_D_HH_720X576_720X288:
			case RESOL_D_HH_960X240_960X240:
			case RESOL_D_HH_960X288_960X288:
			case RESOL_D_HH_960X240_480X240:
			case RESOL_D_HH_960X288_480X288:
			case RESOL_D_HH_720X240_720X240:
			case RESOL_D_HH_720X288_720X288:
			case RESOL_D_HH_480X240_480X240:
			case RESOL_D_HH_480X288_480X288:
				{
					m_ctlSliderBitrate2.SetRange( 100000, 1000000 );
					m_ctlSliderBitrate2.SetPos( 500000 );
					m_ctlEditBitrate2.SetWindowText( L"500000" );
				}
				break;

			case RESOL_D_HH______HD_____CIF:
			case RESOL_D_HH_____FHD_____CIF:
			case RESOL_D_HH_720X480_352X240:
			case RESOL_D_HH_720X576_352X288:
			case RESOL_D_HH_720X240_352X240:
			case RESOL_D_HH_720X288_352X288:
			case RESOL_D_HH_352X240_352X240:
			case RESOL_D_HH_352X288_352X288:
				{
					m_ctlSliderBitrate2.SetRange( 100000, 500000 );
					m_ctlSliderBitrate2.SetPos( 300000 );
					m_ctlEditBitrate2.SetWindowText( L"300000" );
				}
				break;
			}
		}
	case STREAMTYPE_TRIPLE	:
		{
			if( RESOL_T_HMH___HD__VGA__CIF == iResolution )
			{
				m_ctlSliderBitrate2.SetRange( 100000, 500000 );
			}
		}
		break;
	}

	// H.264 Second Stream Bitrate 설정 //////
	//////////////////////////////////////////
}

void CResolutionChangeDlg::OnNMCustomdrawSliderBitrate1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int		iBitrateStream1	= m_ctlSliderBitrate1.GetPos();
	WCHAR	szText[32]		= {0};

	if( m_iBitrateStream1 != iBitrateStream1 )
	{
		m_iBitrateStream1 = iBitrateStream1;
		swprintf_s( szText, L"%d", m_iBitrateStream1 );
		m_ctlEditBitrate1.SetWindowText( szText );
	}

	*pResult = 0;
}

void CResolutionChangeDlg::OnNMCustomdrawSliderBitrate2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	int		iBitrateStream2	= m_ctlSliderBitrate2.GetPos();
	WCHAR	szText[32]		= {0};

	if( m_iBitrateStream2 != iBitrateStream2 )
	{
		m_iBitrateStream2 = iBitrateStream2;
		swprintf_s( szText, L"%d", m_iBitrateStream2 );
		m_ctlEditBitrate2.SetWindowText( szText );
	}
}

int	CResolutionChangeDlg::GetResolutionTextByID( int iResolution, WCHAR* pszResolution1, WCHAR* pszResolution2, WCHAR* pszResolution3 )
{
	int		iStreamType	= STREAMTYPE_SINGLE;
	switch (iResolution)
	{
	case RESOL_S_H_D1				:iStreamType	= STREAMTYPE_SINGLE; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"D1"	);	break;
	case RESOL_S_H_HD				:iStreamType	= STREAMTYPE_SINGLE; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"HD"	);	break;
	case RESOL_S_H_SXGA				:iStreamType	= STREAMTYPE_SINGLE; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"SXGA");	break;
	case RESOL_S_H_FHD				:iStreamType	= STREAMTYPE_SINGLE; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");	break;
	case RESOL_S_H_QXGA				:iStreamType	= STREAMTYPE_SINGLE; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"QXGA");	break;
	case RESOL_S_H_960_576			:
	case RESOL_S_H_720_576			:
	case RESOL_S_H_960_288			:
	case RESOL_S_H_720_288			:
	case RESOL_S_H_480_288			:
	case RESOL_S_H_352_288			:iStreamType	= STREAMTYPE_SINGLE	; break;
	case RESOL_D_HH______D1______D1	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"D1"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"D1"	);break;
	case RESOL_D_HH______HD_____CIF	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"HD"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"CIF"	);break;
	case RESOL_D_HH______HD____QVGA	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"HD"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"QVGA");break;
	case RESOL_D_HH____SXGA_____VGA	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"SXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);break;
	case RESOL_D_HH____SXGA______D1	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"SXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"D1"	);break;
	case RESOL_D_HH_____FHD_____CIF	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");		if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"CIF"	);break;
	case RESOL_D_HH_____FHD____QVGA	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");		if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"QVGA");break;
	case RESOL_D_HH_____FHD_____VGA	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");		if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);break;
	case RESOL_D_HH____QXGA____QVGA	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"QXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"QVGA");break;
	case RESOL_D_HH____QXGA_____VGA	:iStreamType	= STREAMTYPE_DUAL	; if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"QXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);break;
	case RESOL_D_HH_960X480_960X480	:
	case RESOL_D_HH_960X480_960X240	:
	case RESOL_D_HH_960X480_480X240	:
	case RESOL_D_HH_720X480_720X480	:
	case RESOL_D_HH_720X480_720X240	:
	case RESOL_D_HH_720X480_352X240	:
	case RESOL_D_HH_960X240_960X240	:
	case RESOL_D_HH_960X240_480X240	:
	case RESOL_D_HH_720X240_720X240	:
	case RESOL_D_HH_720X240_352X240	:
	case RESOL_D_HH_480X240_480X240	:
	case RESOL_D_HH_352X240_352X240	:
	case RESOL_D_HH_960X576_960X576	:
	case RESOL_D_HH_960X576_960X288	:
	case RESOL_D_HH_960X576_480X288	:
	case RESOL_D_HH_720X576_720X576	:
	case RESOL_D_HH_720X576_720X288	:
	case RESOL_D_HH_720X576_352X288	:
	case RESOL_D_HH_960X288_960X288	:
	case RESOL_D_HH_960X288_480X288	:
	case RESOL_D_HH_720X288_720X288	:
	case RESOL_D_HH_720X288_352X288	:
	case RESOL_D_HH_480X288_480X288	:
	case RESOL_D_HH_352X288_352X288	:iStreamType	= STREAMTYPE_DUAL	; break;
	case RESOL_D_HM______D1______D1	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"D1"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"D1"	);	break;
	case RESOL_D_HM______HD_____CIF	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"HD"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"CIF"	);	break;
	case RESOL_D_HM______HD____QVGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"HD"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"QVGA");	break;
	case RESOL_D_HM______HD_____VGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"HD"	);	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);	break;
	case RESOL_D_HM____SXGA_____VGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"SXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);	break;
	case RESOL_D_HM____SXGA______D1	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"SXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"D1"	);	break;
	case RESOL_D_HM_____FHD_____CIF	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"CIF"	);	break;
	case RESOL_D_HM_____FHD____QVGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"QVGA");	break;
	case RESOL_D_HM_____FHD_____VGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"FHD");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);	break;
	case RESOL_D_HM____QXGA____QVGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"QXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"QVGA");	break;
	case RESOL_D_HM____QXGA_____VGA	:iStreamType	= STREAMTYPE_DUAL	;if( pszResolution1 ) swprintf_s( pszResolution1, 30, L"QXGA");	if( pszResolution2 ) swprintf_s( pszResolution2, 30, L"VGA"	);	break;
	case RESOL_D_HM_960X480_960X480	:
	case RESOL_D_HM_960X480_960X240	:
	case RESOL_D_HM_960X480_480X240	:
	case RESOL_D_HM_720X480_720X480	:
	case RESOL_D_HM_720X480_720X240	:
	case RESOL_D_HM_720X480_352X240	:
	case RESOL_D_HM_960X240_960X240	:
	case RESOL_D_HM_960X240_480X240	:
	case RESOL_D_HM_720X240_720X240	:
	case RESOL_D_HM_720X240_352X240	:
	case RESOL_D_HM_480X240_480X240	:
	case RESOL_D_HM_352X240_352X240	:
	case RESOL_D_HM_960X576_960X576	:
	case RESOL_D_HM_960X576_960X288	:
	case RESOL_D_HM_960X576_480X288	:
	case RESOL_D_HM_720X576_720X576	:
	case RESOL_D_HM_720X576_720X288	:
	case RESOL_D_HM_720X576_352X288	:
	case RESOL_D_HM_960X288_960X288	:
	case RESOL_D_HM_960X288_480X288	:
	case RESOL_D_HM_720X288_720X288	:
	case RESOL_D_HM_720X288_352X288	:
	case RESOL_D_HM_480X288_480X288	:
	case RESOL_D_HM_352X288_352X288	:iStreamType	= STREAMTYPE_DUAL	; break;
	case RESOL_T_HMH___HD__VGA__CIF	:	
		if( NULL != pszResolution1 )	swprintf_s( pszResolution1, 30, L"HD"	);	
		if( NULL != pszResolution2 )	swprintf_s( pszResolution2, 30, L"VGA"	);	
		if( NULL != pszResolution3 )	swprintf_s( pszResolution3, 30, L"CIF"	);	
		iStreamType	= STREAMTYPE_TRIPLE;
		break;
	}	

	return iStreamType;
}								   		

void CResolutionChangeDlg::GetSetupStreamInfo( SETUP_STREAMINFO* pSetupStreamInfo )
{
	WCHAR	szRateControl[30]	= {0};
	int		iResolution			= 0;
	int		iStreamType			= STREAMTYPE_SINGLE;

	iResolution		= m_ctlComboResolution.GetItemData(m_ctlComboResolution.GetCurSel());

	iStreamType = GetResolutionTextByID(iResolution, 
										pSetupStreamInfo->aStreamItems[0].szResolution,
										pSetupStreamInfo->aStreamItems[1].szResolution,
										pSetupStreamInfo->aStreamItems[2].szResolution);

	if( RATECONTROL_CVBR == m_ctlComboRateControl.GetCurSel() )
	{
		swprintf_s( szRateControl, L"cvbr" );
	}
	else if( RATECONTROL_CBR	== m_ctlComboRateControl.GetCurSel() )
	{
		swprintf_s( szRateControl, L"cbr" );
	}

	if(		STREAMTYPE_SINGLE == m_iSelectStreamType )
	{
		if( STREAMTYPE_SINGLE != iStreamType )
		{
			AfxMessageBox(L"Different StreamType", MB_ICONWARNING);
			return;
		}

		pSetupStreamInfo->iStreamCount	= 1;

		swprintf_s( pSetupStreamInfo->aStreamItems[0].szCodec, L"H.264" );
		swprintf_s( pSetupStreamInfo->aStreamItems[0].szRateControl, szRateControl );

		pSetupStreamInfo->aStreamItems[0].lBitrate	= m_ctlSliderBitrate1.GetPos();
		pSetupStreamInfo->aStreamItems[0].lFrameRate= m_ctlComboFPS1.GetCurSel()+1;
		pSetupStreamInfo->aStreamItems[0].lGOP		= m_ctlComboGOP1.GetCurSel()+1;

	}
	else if( STREAMTYPE_DUAL	== m_iSelectStreamType )
	{
		if( STREAMTYPE_DUAL != iStreamType )
		{
			AfxMessageBox(L"Different StreamType", MB_ICONWARNING);
			return;
		}

		pSetupStreamInfo->iStreamCount	= 2;

		swprintf_s( pSetupStreamInfo->aStreamItems[0].szCodec, L"H.264" );
		swprintf_s( pSetupStreamInfo->aStreamItems[0].szRateControl, szRateControl );

		pSetupStreamInfo->aStreamItems[0].lBitrate	= m_ctlSliderBitrate1.GetPos();
		pSetupStreamInfo->aStreamItems[0].lFrameRate= m_ctlComboFPS1.GetCurSel()+1;
		pSetupStreamInfo->aStreamItems[0].lGOP		= m_ctlComboGOP1.GetCurSel()+1;


		if( CODEC_DUAL_H264_H264 == m_iSelectCodec )
		{
			swprintf_s( pSetupStreamInfo->aStreamItems[1].szCodec, L"H.264" );
			swprintf_s( pSetupStreamInfo->aStreamItems[1].szRateControl, szRateControl );

			pSetupStreamInfo->aStreamItems[1].lBitrate	= m_ctlSliderBitrate2.GetPos();
			pSetupStreamInfo->aStreamItems[1].lFrameRate= m_ctlComboFPS2.GetCurSel()+1;
			pSetupStreamInfo->aStreamItems[1].lGOP		= m_ctlComboGOP2.GetCurSel()+1;
		}
		else if( CODEC_DUAL_H264_MJPEG == m_iSelectCodec )
		{
			swprintf_s( pSetupStreamInfo->aStreamItems[1].szCodec, L"MJPEG" );

			pSetupStreamInfo->aStreamItems[1].lBitrate	= m_ctlComboMjpegQuality.GetCurSel();
			pSetupStreamInfo->aStreamItems[1].lFrameRate= m_ctlComboMjpegFPS.GetCurSel()+1;
		}

	}
	else if( STREAMTYPE_TRIPLE	== m_iSelectStreamType )
	{
		if( STREAMTYPE_TRIPLE != iStreamType )
		{
			AfxMessageBox(L"Different StreamType", MB_ICONWARNING);
			return;
		}

		pSetupStreamInfo->iStreamCount	= 3;

		// First 
		swprintf_s( pSetupStreamInfo->aStreamItems[0].szCodec, L"H.264" );
		swprintf_s( pSetupStreamInfo->aStreamItems[0].szRateControl, szRateControl );

		pSetupStreamInfo->aStreamItems[0].lBitrate	= m_ctlSliderBitrate1.GetPos();
		pSetupStreamInfo->aStreamItems[0].lFrameRate= m_ctlComboFPS1.GetCurSel()+1;
		pSetupStreamInfo->aStreamItems[0].lGOP		= m_ctlComboGOP1.GetCurSel()+1;



		// Second
		swprintf_s( pSetupStreamInfo->aStreamItems[1].szCodec, L"MJPEG" );

		pSetupStreamInfo->aStreamItems[1].lBitrate	= m_ctlComboMjpegQuality.GetCurSel();
		pSetupStreamInfo->aStreamItems[1].lFrameRate= m_ctlComboMjpegFPS.GetCurSel()+1;



		// Third
		swprintf_s( pSetupStreamInfo->aStreamItems[2].szCodec, L"H.264" );
		swprintf_s( pSetupStreamInfo->aStreamItems[2].szRateControl, szRateControl );

		pSetupStreamInfo->aStreamItems[2].lBitrate	= m_ctlSliderBitrate2.GetPos();
		pSetupStreamInfo->aStreamItems[2].lFrameRate= m_ctlComboFPS2.GetCurSel()+1;
		pSetupStreamInfo->aStreamItems[2].lGOP		= m_ctlComboGOP2.GetCurSel()+1;
	}
}
DWORD WINAPI CResolutionChangeDlg::ProcLensInit  ( LPVOID _lpParam )
{
	CResolutionChangeDlg*	pThis	= (CResolutionChangeDlg*)_lpParam;

	CString strResponse;
	CString	strRequest;

	SCAN_INFO*	pScanInfo = NULL;

	strRequest.Format(L"/cgi-bin/param.cgi?action=update&ImageSource0.I0.Sensor.LensInit");

	for( int iItem=0; iItem<pThis->m_ctlListDevice.GetItemCount(); iItem++ )
	{
		if( 0 == pThis->m_dwThreadLensInitID )
			break;

		pScanInfo = (SCAN_INFO*)pThis->m_ctlListDevice.GetItemData(iItem);

		pThis->PostMessage(UM_STATUS_MESSAGE, (WPARAM)CResolutionChangeThread::MESSAGE_ID_LENSINIT_START, (LPARAM)iItem );

		pThis->CallHttpRequest( pScanInfo->szAddr, pScanInfo->nHTTPPort, strRequest, strResponse );
		strResponse.Trim();

		if( strResponse.Compare(L"OK") )
		{
		}

		pThis->PostMessage(UM_STATUS_MESSAGE, (WPARAM)CResolutionChangeThread::MESSAGE_ID_LENSINIT_END, (LPARAM)iItem );
	}

	pThis->PostMessage(UM_STATUS_MESSAGE, (WPARAM)CResolutionChangeThread::MESSAGE_ID_LENSINIT_COMPLETE, (LPARAM)0 );
	
	return 0;
}

void CResolutionChangeDlg::OnBnClickedButtonLensInit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	EnableControlMain(FALSE);
	EnableControlFunction(FALSE);
	EnableControlUserInfo(FALSE);
	EnableControlH264Stream1(FALSE);
	EnableControlH264Stream2(FALSE);
	EnableControlMJPEGStream(FALSE);

	for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
	{
		m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_ID		, FALSE );
		m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_PW		, FALSE );
	}			

	if( m_hThreadLensInit )
	{
		m_dwThreadLensInitID	= 0;
		WaitForSingleObject( m_hThreadLensInit, INFINITE );

		CloseHandle(m_hThreadLensInit);
		m_hThreadLensInit	= NULL;
	}

	m_hThreadLensInit	= CreateThread( NULL, 
										0, 
										(LPTHREAD_START_ROUTINE)ProcLensInit, 
										this, 
										0, 
										&m_dwThreadLensInitID );

}

void CResolutionChangeDlg::OnCbnSelchangeComboRatecontrol()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CResolutionChangeDlg::OnEnChangeUserid()
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

void CResolutionChangeDlg::OnEnChangePassword()
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

LRESULT CResolutionChangeDlg::OnMessageStatus(WPARAM wParam, LPARAM lParam)
{
	int		iMessageID			= (int)wParam;
	int		iItemIndex			= (int)lParam;
	WCHAR	szMessage[MAX_PATH]	= {0};

	switch( iMessageID )
	{
	case CResolutionChangeThread::MESSAGE_ID_LENSINIT_START				:	swprintf_s(szMessage, L"Lens Initialize started" );		break;
	case CResolutionChangeThread::MESSAGE_ID_LENSINIT_END				:	swprintf_s(szMessage, L"Lens Initialize completed" );	break;
	case CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_START		:	swprintf_s(szMessage, L"Resolution Set started" );		break;
	case CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_END			:	swprintf_s(szMessage, L"Resolution Set completed" );	break;
	case CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_ERROR		:	swprintf_s(szMessage, L"Resolution Set error !!" );		break;
	case CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_ERROR_AUTH	:	swprintf_s(szMessage, L"Resolution Set error(Auth)!!" );break;
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

	if( CResolutionChangeThread::MESSAGE_ID_LENSINIT_COMPLETE == iMessageID )
	{
		if( m_hThreadLensInit )
		{
			m_dwThreadLensInitID	= 0;
			WaitForSingleObject( m_hThreadLensInit, INFINITE );

			CloseHandle(m_hThreadLensInit);
			m_hThreadLensInit	= NULL;

			EnableControlMain(TRUE);
			EnableControlFunction(TRUE);
			EnableControlUserInfo(TRUE);

			switch( m_iSelectStreamType )
			{
			case STREAMTYPE_SINGLE	:
				if( CODEC_SINGLE_H264 == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(FALSE);
					EnableControlMJPEGStream(FALSE);
				}
				break;
			case STREAMTYPE_DUAL	:
				if( CODEC_DUAL_H264_H264 == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(TRUE);
					EnableControlMJPEGStream(FALSE);
				}
				else if( CODEC_DUAL_H264_MJPEG == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(FALSE);
					EnableControlMJPEGStream(TRUE);
				}
				break;
			case STREAMTYPE_TRIPLE	:
				if( CODEC_TRIPLE_H264_MJPEG_H264 == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(TRUE);
					EnableControlMJPEGStream(TRUE);
				}
				break;
			}

			for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
			{
				m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_ID		, TRUE );
				m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_PW		, TRUE );
			}			
		}

		return 0;
	}
	else if( CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_END			== iMessageID ||
			 CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_ERROR		== iMessageID ||
			 CResolutionChangeThread::MESSAGE_ID_RESOLUTION_SET_ERROR_AUTH	== iMessageID )
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
			EnableControlFunction(TRUE);
			EnableControlUserInfo(TRUE);

			switch( m_iSelectStreamType )
			{
			case STREAMTYPE_SINGLE	:
				if( CODEC_SINGLE_H264 == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(FALSE);
					EnableControlMJPEGStream(FALSE);
				}
				break;
			case STREAMTYPE_DUAL	:
				if( CODEC_DUAL_H264_H264 == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(TRUE);
					EnableControlMJPEGStream(FALSE);
				}
				else if( CODEC_DUAL_H264_MJPEG == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(FALSE);
					EnableControlMJPEGStream(TRUE);
				}
				break;
			case STREAMTYPE_TRIPLE	:
				if( CODEC_TRIPLE_H264_MJPEG_H264 == m_iSelectCodec )
				{
					EnableControlH264Stream1(TRUE);
					EnableControlH264Stream2(TRUE);
					EnableControlMJPEGStream(TRUE);
				}
				break;
			}

			for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
			{
				m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_ID		, TRUE );
				m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_PW		, TRUE );
			}	

			//SetDlgItemText( IDC_USERID	, L"" );
			//SetDlgItemText( IDC_PASSWORD, L"" );
		}
	}

	return 0;
}


void CResolutionChangeDlg::OnEnChangeEditBitrate1()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int		iGetSel			= m_ctlSliderBitrate1.GetPos();
	int		iEditBitrate	= 0;
	BOOL	bTranslate		= FALSE;

	iEditBitrate	= ::GetDlgItemInt( GetSafeHwnd(), IDC_EDIT_BITRATE1, &bTranslate, FALSE );

	if( iEditBitrate != iGetSel )
	{
		m_ctlSliderBitrate1.SetPos( iEditBitrate );
	}
}

void CResolutionChangeDlg::OnEnChangeEditBitrate2()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialog::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int		iGetSel			= m_ctlSliderBitrate2.GetPos();
	int		iEditBitrate	= 0;
	BOOL	bTranslate		= FALSE;

	iEditBitrate	= ::GetDlgItemInt( GetSafeHwnd(), IDC_EDIT_BITRATE2, &bTranslate, FALSE );

	if( iEditBitrate != iGetSel )
	{
		m_ctlSliderBitrate2.SetPos( iEditBitrate );
	}
}

void CResolutionChangeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	if( GetSafeHwnd() && m_ctlListDevice.GetSafeHwnd() )
	{
		m_ctlListDevice.MoveWindow( 10, 360, cx-20, cy-10-360-30 );
		m_ctlButtonApply.MoveWindow( cx/2-80, cy-21-10, 73, 21 );
		m_ctlButtonCancel.MoveWindow( cx/2+3, cy-21-10, 73, 21 );
	}
}

void CResolutionChangeDlg::OnBnClickedButtonApply()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	WCHAR	szUserID[32]		= {0};
	WCHAR	szUserPW[32]		= {0};

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
	EnableControlFunction(FALSE);
	EnableControlUserInfo(FALSE);
	EnableControlH264Stream1(FALSE);
	EnableControlH264Stream2(FALSE);
	EnableControlMJPEGStream(FALSE);

	for( int i=0; i<m_ctlListDevice.GetItemCount(); i++ )
	{
		m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_ID		, FALSE );
		m_ctlListDevice.SetEditEnable( i, SUBITEM_USER_PW		, FALSE );
	}	

	for( int iItem=0; iItem<m_aSettingThread.GetCount(); iItem++ )
	{
		delete m_aSettingThread[iItem];
		m_aSettingThread[iItem]	= NULL;
	}

	m_aSettingThread.RemoveAll();

	CheckArp( m_iAdapterID );

	memset( &m_SetupStreamInfo, 0, sizeof(m_SetupStreamInfo) );
	GetSetupStreamInfo( &m_SetupStreamInfo );

	SCAN_INFO*	pScanInfo = NULL;
	for( int iItem=0; iItem<m_ctlListDevice.GetItemCount(); iItem++ )
	{
		pScanInfo = (SCAN_INFO*)m_ctlListDevice.GetItemData(iItem);

		swprintf_s( szUserID, L"%s", m_ctlListDevice.GetEditText(iItem, SUBITEM_USER_ID) );
		swprintf_s( szUserPW, L"%s", m_ctlListDevice.GetEditText(iItem, SUBITEM_USER_PW) );

		m_aSettingThread[iItem]	= new CResolutionChangeThread(	this->GetSafeHwnd()	,
																iItem				,
																pScanInfo->szAddr	,
																pScanInfo->nHTTPPort,
																szUserID			,
																szUserPW			, 
																m_SetupStreamInfo	);
	}
}

void CResolutionChangeDlg::OnBnClickedButtonCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}

void CResolutionChangeDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	lpMMI->ptMinTrackSize.x = 785;
	lpMMI->ptMinTrackSize.y = 570;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

BOOL CResolutionChangeDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE) // by pass ESC key
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
