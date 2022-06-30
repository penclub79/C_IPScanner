
// IPScanUtilDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "IPScanUtil.h"
#include "IPScanUtilDlg.h"
#include "IPChangeDlg.h"
#include "UpgradeDlg.h"
#include "IPChangeDlg2.h"
#include "FactoryDefaultDlg.h"
#include "ResolutionChangeDlg.h"
#include "OSDChangeDlg.h"
#include <locale.h>					// for using hangule in MFC TRACE macro
#include "xmlite\XMLite.h"
#include "NetCommon.h"				// 공통적인 변수 사용


#pragma comment(lib, "version.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const UINT IPSCAN_WINDOW_MIN_WIDTH  = 890;
const UINT IPSCAN_WINDOW_MIN_HEIGHT = 491;

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	BOOL GetVerInfo(CString& strVer);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	CString strVerInfo;
	CString strProgrameInfo;

	if( GetVerInfo(strVerInfo) )
	{
		strProgrameInfo = L"IPScan Utility ( Ver ";
		strProgrameInfo.Append( strVerInfo );
		strProgrameInfo.Append( L" )" );

		SetDlgItemText(IDC_STATIC_PROGRAME_INFO, strProgrameInfo);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CAboutDlg::GetVerInfo(CString& strVer)
{
	BOOL bResult = FALSE;
	HMODULE hRes = AfxGetResourceHandle();
	if ( hRes == NULL ) return FALSE;
	HRSRC hVer = FindResource(hRes, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if ( hVer == NULL ) return FALSE;
	HGLOBAL hBuf = LoadResource(hRes, hVer);
	if ( hBuf == NULL ) return FALSE;
	LPVOID lpBuf = LockResource(hBuf);
	if ( lpBuf ) {
		UINT nLen = 0;
		LPVOID lpVer = NULL;
		WCHAR szKey[256] = { L"\\VarFileInfo\\Translation" };
		bResult = VerQueryValue(lpBuf,szKey,&lpVer,(UINT*)&nLen);
		if ( bResult && nLen == 4 ) {
			BYTE * pLangD = (BYTE*)lpVer;
			swprintf_s(szKey,256,L"\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
				pLangD[1],pLangD[0],pLangD[3],pLangD[2]);
			bResult = VerQueryValue(lpBuf,szKey,&lpVer,(UINT*)&nLen);
			if ( bResult ) strVer = (WCHAR*)lpVer;
		}
		UnlockResource(hBuf);
	}
	FreeResource(hBuf);
	return bResult;
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CIPScanUtilDlg 대화 상자
CIPScanUtilDlg::CIPScanUtilDlg(CWnd* pParent /*=NULL*/)
: CDialog(CIPScanUtilDlg::IDD, pParent)
, m_bScanning(FALSE)
, m_nListItemCount(0)
, m_nScanAniCount(0)
, m_iSelectVersion(VERSION_2)
, m_nCurSvrListSel(-1)
, m_bInit(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	InitializeCriticalSection(&m_mt);

	m_bSortAscending = FALSE;
	m_nSortOrient = SUBITEM_IPADDRESS; // IP
	//memset(m_szTempValue, 0, sizeof(m_szTempValue));
	m_ulAcceptAddress = 0;
}

void CIPScanUtilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SVR_LIST		, m_cSvrList);
	DDX_Control(pDX, IDC_SCAN_BTN		, m_btnScan);
	DDX_Control(pDX, IDC_CLEAR_BTN		, m_btnClearList);
	DDX_Control(pDX, IDC_CHANGEIP_BTN	, m_btnChangeIP);
	//DDX_Control(pDX, IDC_UPGRADE_BTN	, m_btnUpgrade);
	//DDX_Control(pDX, IDC_PROTOCAL_COMBO	, m_cmbProtocol);
	DDX_Control(pDX, IDC_ADAPTOR_CMB	, m_cmbNetAdaptor);
}

BEGIN_MESSAGE_MAP(CIPScanUtilDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SCAN_MSG, &CIPScanUtilDlg::OnScanMsg)
	ON_MESSAGE(WM_SCAN_CLOSE_DLG_MSG, &CIPScanUtilDlg::OnScanCloseDlgMsg)
	ON_MESSAGE(WM_CONNECT_CHECK, &CIPScanUtilDlg::OnConnectCheck)
	ON_MESSAGE(WM_SORT_REQUEST,  &CIPScanUtilDlg::OnSortRequest)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_COMMAND(ID_SETUP_SETUP,					&CIPScanUtilDlg::OnSetupResolution)
	ON_COMMAND(ID_SETUP_OSDSETUP,				&CIPScanUtilDlg::OnSetupOSD)
	ON_NOTIFY(NM_RCLICK, IDC_SVR_LIST,			&CIPScanUtilDlg::OnNMRClickSvrList)
	ON_NOTIFY(NM_CLICK, IDC_SVR_LIST,			&CIPScanUtilDlg::OnNMClickSvrList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SVR_LIST,	&CIPScanUtilDlg::OnLvnItemchangedSvrList)
	ON_NOTIFY(NM_DBLCLK, IDC_SVR_LIST,			&CIPScanUtilDlg::OnNMDblclkSvrList3)
	//ON_CBN_SELCHANGE(IDC_PROTOCAL_COMBO,		&CIPScanUtilDlg::OnCbnSelchangeProtocalCombo)
	ON_BN_CLICKED(IDOK,							&CIPScanUtilDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL,						&CIPScanUtilDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SCAN_BTN,					&CIPScanUtilDlg::OnBnClickedScanBtn)
	ON_BN_CLICKED(IDC_CHANGEIP_BTN,				&CIPScanUtilDlg::OnBnClickedChangeipBtn2)
	ON_BN_CLICKED(IDC_CLOSE,					&CIPScanUtilDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_CLEAR_BTN,				&CIPScanUtilDlg::OnBnClickedClearBtn)
	//ON_BN_CLICKED(IDC_TEST_BTN,					&CIPScanUtilDlg::OnBnClickedTestBtn)
	//ON_BN_CLICKED(IDC_TEST_BTN2,				&CIPScanUtilDlg::OnBnClickedTestBtn2)
	//ON_BN_CLICKED(IDC_UPGRADE_BTN,				&CIPScanUtilDlg::OnBnClickedUpgradeBtn)
	ON_BN_CLICKED(IDC_FACTORY_BTN,				&CIPScanUtilDlg::OnBnClickedFactoryBtn)
	ON_CBN_SELCHANGE(IDC_ADAPTOR_CMB,			&CIPScanUtilDlg::OnCbnSelchangeAdaptorCmb)
	//ON_COMMAND(IDC_PROTOCAL_COMBO, &CIPScanUtilDlg::OnProtocalCombo)
	ON_BN_CLICKED(IDC_OPEN_XML, &CIPScanUtilDlg::OnClickedOpenXml)
END_MESSAGE_MAP()


// CIPScanUtilDlg 메시지 처리기

BOOL CIPScanUtilDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// for using hangule in MFC TRACE macro
#ifdef _DEBUG
	_tsetlocale(LC_ALL, _T("korean"));
#endif
	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	//////////////////////////////////////////////////////////////////////////
	// 2010-08-26 hkeins server scan
	// server list column initialize
	// ----------------------------------------------------------------------
	// 0    1             2       3                  4             5
	// IP | MAC Address | Model | Firmware version | Stream Port | Http Port 
	// ----------------------------------------------------------------------
	// --> extension field added, column change
	// ----------------------------------------------------------------------------------------------------
	//   0           1             2          3             4            5             6                              
	// | IP        | MAC Address | Model    | Stream Port | Http Port  | Server Name | Firmware Version| | 
	// | 7           8             9          10           11           12
	// | Resolution| Video format| AIN cnt  | AOUT cnt    | Audio in   | Audio out
	// ----------------------------------------------------------------------------------------------------
	m_cSvrList.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

	//m_cSvrList.ModifyStyle(0, LVS_SINGLESEL);
	// FIX ME : display 할 항목 변경 시 밑에 있는 CompareScanInfo 함수도 같이 변경 할 것
	CString str;
	CString strItem; 
	
	str.LoadString(IDS_ADDRESS);
	strItem = L"IP " + str;
	m_cSvrList.InsertColumn(SUBITEM_IPADDRESS, strItem, LVCFMT_CENTER, 138, 0 );
	strItem = L"MAC " + str;
	m_cSvrList.InsertColumn(SUBITEM_MACADDRESS, strItem, LVCFMT_CENTER, 138, 0 );

	strItem.LoadString(IDS_IP_TYPE);
	m_cSvrList.InsertColumn(SUBITEM_IS_DHCP, strItem, LVCFMT_CENTER, 138, 0 );

	str.LoadString(IDS_PORT); 
	strItem.LoadString(IDS_STREAM);
	strItem += L" ";
	strItem += str;
	//m_cSvrList.InsertColumn(SUBITEM_PORTSTREAM		, strItem, LVCFMT_CENTER, 40, 0 );

	strItem = L"HTTP "; 
	strItem += str;
	m_cSvrList.InsertColumn(SUBITEM_PORTHTTP, strItem, LVCFMT_CENTER, 138, 0 );

	strItem = L"Upgrade ";
	strItem += str;
	m_cSvrList.InsertColumn(SUBITEM_PORTUPGRADE, strItem, LVCFMT_CENTER, 138, 0 );

	strItem.LoadString(IDS_BASE_PORT);
	m_cSvrList.InsertColumn(SUBITEM_PORTBASE, strItem, LVCFMT_CENTER, 138, 0);

	//strItem.LoadString(IDS_SERVER_NAME);
	//m_cSvrList.InsertColumn(SUBITEM_SYSTEMNAME		, strItem, LVCFMT_CENTER, 150, 0 );

	strItem.LoadString(IDS_MODEL);
	m_cSvrList.InsertColumn(SUBITEM_MODELTYPE		, strItem, LVCFMT_CENTER, 138, 0 );
	// InsertColumn(컬럼생성 순서, 컬럼 이름, 글씨 정렬, width값);

	strItem.LoadString(IDS_FIRMWARE_VERSION);
	m_cSvrList.InsertColumn(SUBITEM_FIRMWAREVERSION	, strItem, LVCFMT_CENTER, 138, 0 );

	strItem.LoadString(IDS_SW_VERSION);
	m_cSvrList.InsertColumn(SUBITEM_SW_VERSION, strItem, LVCFMT_CENTER, 138, 0);

	strItem.LoadString(IDS_VIDEO_COUNT);
	m_cSvrList.InsertColumn(SUBITEM_VIDEOCOUNT, strItem, LVCFMT_CENTER, 138, 0);

	m_cSvrList.Init();

	m_apScanner[0] = new CNetScanMarkIn();
	m_apScanner[1] = new CNetScanVision();
	
	m_btnChangeIP.EnableWindow(TRUE);

	m_bInit = TRUE;
	m_cPopupMenu.LoadMenu(IDR_POPUP_MENU);

	//AddProtocolToCombo();
	CallLayoutManager();
	_LoadNetworkAdaptorInformation();
	
	// CreateFont CFont 초기화한다.
	m_DISP_FONT.CreateFont(30, 0, 0, 0, FW_THIN, FALSE, FALSE, 0, ARABIC_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, _T("MS Shell Dlg"));
	m_btnScan.SetFont(&m_DISP_FONT);

	/*
		임시로 버튼을 숨김.(2022-06-08)
	*/
	(GetDlgItem(IDC_UPGRADE_BTN))->ShowWindow(FALSE);
	(GetDlgItem(IDC_FACTORY_BTN))->ShowWindow(FALSE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

//void CIPScanUtilDlg::AddProtocolToCombo()
//{
//	CString strTemp;
//	m_iSelectVersion	= VERSION_2;
//
//	//Model Combo에 Model 추가
//	//m_cmbProtocol.ResetContent();
//	/*m_cmbProtocol.AddString(L"Version1");*/
//	m_cmbProtocol.AddString(L"Version2");
//
//	m_cmbProtocol.SetCurSel(m_iSelectVersion);
//}

void CIPScanUtilDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CIPScanUtilDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CIPScanUtilDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CIPScanUtilDlg::OnBnClickedOk()
{
	OnOK();
}

void CIPScanUtilDlg::OnBnClickedCancel()
{
	
	OnCancel();
}

void CIPScanUtilDlg::OnBnClickedScanBtn()
{
	CString msg;

	if(!m_bScanning) // SCAN을 눌렀을 때
	{
		_ReadBindAddress();

		m_bScanning = TRUE;

		msg.LoadString(IDS_STOP);
		m_btnScan.SetWindowText(msg);

		// view status
		msg.LoadString(IDS_STATUS_SCANNING);
		SetStatusMsg(msg);

		// start set

		for (int i = 0; i < COUNT_SCAN_CLIENT; i++)
		{
			if (m_apScanner[i])
			{
				m_apScanner[i]->SetBindAddress(m_ulAcceptAddress);
				m_apScanner[i]->SetNotifyWindow(m_hWnd, WM_SCAN_MSG);
				m_apScanner[i]->SetCloseMsgRecvWindow(m_hWnd, WM_SCAN_CLOSE_DLG_MSG);
				m_apScanner[i]->StartScan();
			}
		}

		m_nScanAniCount = 0;
		SetTimer(TM_SCANNING_ANI	, 1000		, NULL);
	}
	else // Stop을 눌렀을 때
	{
		KillTimer(TM_SCANNING_ANI);

		m_bScanning = FALSE;

		msg.LoadString(IDS_SCAN);
		m_btnScan.SetWindowText(msg);

		// view status
		msg.LoadString(IDS_STATUS_STOP);
		SetStatusMsg(msg);

		// stop
		for (int i = 0; i < COUNT_SCAN_CLIENT; i++)
		{
			if (m_apScanner[i])
			{
				m_apScanner[i]->StopScan();
			}
		}
	}
}

void CIPScanUtilDlg::OnBnClickedChangeipBtn()
{
	CIPChangeDlg dlg;
	int i;
	SCAN_INFO* pInfo = NULL;
	if(m_nCurSvrListSel >= 0) pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(m_nCurSvrListSel);

	if(pInfo)
	{
		// data setting before show dialog
		dlg.m_strMACAddress		= pInfo->szMAC;
		dlg.m_strIPAddress		= pInfo->szAddr;
		dlg.m_strGatewayAddress = pInfo->szGateWay;
		//dlg.m_nStreamPort		= pInfo->nStreamPort;
		
		dlg.m_nHTTPPort			= pInfo->nHTTPPort;
		dlg.m_nVersion			= pInfo->version;
		dlg.m_strSubnetMask		= pInfo->szSubnetMask;
		dlg.m_nIsDHCP			= pInfo->cIsDHCP;
	}
	else
	{
		dlg.m_nStreamPort		= 2700;
		dlg.m_nHTTPPort			= 80;
		dlg.m_nIsDHCP			= TRUE;
		dlg.m_strSubnetMask		= L"0.0.0.0";
		dlg.m_nVersion			= VERSION_2;
	}

	if(dlg.DoModal() == IDOK)
	{
		_Lock();
		// delete current item for validation
		SCAN_INFO* pInfo = NULL;

		// MAC 주소를 리스트에서 찾아 있으면 삭제해준다
		for(i = 0; i < m_cSvrList.GetItemCount(); i++)
		{
			pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(i);
			if(pInfo && _wcsicmp(pInfo->szMAC, dlg.m_strMACAddress.GetBuffer()) == 0)
			{
				if(m_nCurSvrListSel == i)
					m_nCurSvrListSel = -1;

				delete pInfo;
				m_cSvrList.DeleteItem(i);

				m_nListItemCount--;
				if(m_nListItemCount < 0)
					m_nListItemCount = 0;
				SetCountMsg(m_nListItemCount);
				break;
			}
		}
		_Unlock();
	}
}

void CIPScanUtilDlg::OnBnClickedClose()
{
	KillTimer(TM_SCANNING_ANI);

	// Stop scan before close dialog
	for (int i = 0; i < COUNT_SCAN_CLIENT; i++)
	{
		if (m_apScanner[i])
		{
			m_apScanner[i]->StopScan();
			SAFE_DELETE(m_apScanner[i]);
		}
	}

	ClearScanList();

	OnCancel();
}

void CIPScanUtilDlg::SetStatusMsg(WCHAR* szMsg)
{
	if(szMsg == NULL)
		return;

	SetDlgItemText(IDC_STATIC_STATUES, szMsg);
}

void CIPScanUtilDlg::SetStatusMsg(CString& strMsg)
{
	SetDlgItemText(IDC_STATIC_STATUES, strMsg);
}

BOOL CIPScanUtilDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_ESCAPE) // by pass ESC key
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CIPScanUtilDlg::OnClose()
{
	KillTimer(TM_SCANNING_ANI);

	for (int i = 0; i < COUNT_SCAN_CLIENT; i++)
	{
		if (m_apScanner[i])
		{
			m_apScanner[i]->StopScan();
			SAFE_DELETE(m_apScanner[i]);
		}
	}

	ClearScanList();

	CDialog::OnClose();
}

void CIPScanUtilDlg::OnBnClickedClearBtn()
{
	_Lock();
	//ASSERT(m_bScanning != TRUE); // scanning중에는 진입 불가
	m_nListItemCount = 0;
	m_nCurSvrListSel = -1;
	ClearScanList();
	SetCountMsg(m_nListItemCount);
	_Unlock();
}

LRESULT CIPScanUtilDlg::OnScanMsg(WPARAM wParam, LPARAM lParam)
{
	CString		strTemp;
	SCAN_INFO*	pScanInfo		= NULL;
	SCAN_INFO*	pOldScanInfo	= NULL;
	int			nCurrentItem	= -1;
	int			i				= 0;


	pScanInfo = (SCAN_INFO*)wParam;
	if(wParam == NULL)
	{
		OnBnClickedScanBtn();
		_GetScanErrorMessage(lParam, strTemp);
		AfxMessageBox(strTemp, MB_ICONWARNING);
		return 0L;
	}

	//if( m_iSelectVersion != pScanInfo->version )
	//{
	//	// 선택되어 있는 버젼과 받은 버젼이 틀리다면 삭제한다. 
	//	delete pScanInfo; 
	//	return 0L;
	//}
	_Lock();

	if(pScanInfo)
	{
		//////////////////////////////////////////
		//// 응답없는 아이템 리스트에서 삭제 /////
		//time_t		tCurrentTime	= GetCurrentTime();
		//for(i = m_cSvrList.GetItemCount()-1; i>=0 ; i--)
		//{
		//	pOldScanInfo	= (SCAN_INFO*)m_cSvrList.GetItemData(i);
		//	if( 30000 < tCurrentTime - pOldScanInfo->tReceiveTime )
		//	{
		//		delete pOldScanInfo;
		//		m_cSvrList.DeleteItem(i);
		//		m_nListItemCount--;
		//		if(m_nListItemCount < 0)
		//			m_nListItemCount = 0;
		//		SetCountMsg(m_nListItemCount);
		//	}
		//}
		//// 응답없는 아이템 리스트에서 삭제 /////
		//////////////////////////////////////////


		////////////////////////////////////////
		//{{ 중복제거 //////////////////////////

		// 20110208-hkeins : BUG fix. find by MAC address(old : find by ip address)
		// 시나리오상 IP가 중복될 수도 있음

		for(i = 0; i < m_cSvrList.GetItemCount(); i++)
		{
			pOldScanInfo	= (SCAN_INFO*)m_cSvrList.GetItemData(i);
			if(wcscmp(pOldScanInfo->szMAC, pScanInfo->szMAC) == 0) // if founded then
			{
				pOldScanInfo->SetReceiveTime();
				
				if(	*pOldScanInfo == *pScanInfo	)
				{
					delete pScanInfo; 
					pScanInfo = NULL;
					_Unlock();
					return 0;
				}
				else
				{
					// 이전 아이피 정보와 현재의 데이터가 다르다면 업데이트 시킨다. 
					pScanInfo->SetReceiveTime();
					m_cSvrList.SetItemData(i, (DWORD_PTR)pScanInfo);

					delete pOldScanInfo;
					nCurrentItem = i;
					break;
				}
			}
		}
		//}}
		//{{ 중복제거 //////////////////////////
		////////////////////////////////////////

#ifdef _DEBUG
		pScanInfo->_PrintValues();
#endif
		int nInsertIndex = 0;
		if(nCurrentItem < 0 ) // insert case
		{
			nInsertIndex = _GetInsertPosition(pScanInfo);
			if(nInsertIndex < 0)
				nInsertIndex = 0;
		}

		pScanInfo->SetReceiveTime();

		// add information into UI
		/*
			LV_ITEM : 리스트 컨트롤에 데이터를 입력하고자 할 때, 이 구조체를 사용.
			CListCtrl클래스의 멤버 함수 insertItem과 SetItem함수에 LV_ITEM구조체를 인자로 넘겨
			주어서 데이터를 입력할 수 있다.
		*/
		LV_ITEM item;

		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = (nCurrentItem < 0)?nInsertIndex:nCurrentItem; // Update의 경우에는 찾은 인덱스(nCurrentItem으로)
		item.iSubItem = SUBITEM_IPADDRESS;
		item.pszText = pScanInfo->szAddr;
		item.lParam = (LPARAM)pScanInfo;
		if( nCurrentItem < 0 ) // 새로운 아이템의 경우에만 insert해준다
			m_cSvrList.InsertItem(&item);

		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0)?nInsertIndex:nCurrentItem;
		item.iSubItem = SUBITEM_MACADDRESS;
		item.pszText = pScanInfo->szMAC;
		m_cSvrList.SetItem(&item);

		if(pScanInfo->cIsDHCP==0)
			strTemp.LoadString(IDS_STATIC);
		else
			strTemp.LoadString(IDS_DHCP);

		item.mask		= LVIF_TEXT;
		item.iItem		= (nCurrentItem < 0)?nInsertIndex:nCurrentItem;
		item.iSubItem	= SUBITEM_IS_DHCP;
		item.pszText	= (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		//strTemp.Format(_T("%d"), pScanInfo->nStreamPort);
		//item.mask = LVIF_TEXT;
		//item.iItem = (nCurrentItem < 0)?nInsertIndex:nCurrentItem;
		//item.iSubItem = SUBITEM_PORTSTREAM;
		//item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		//m_cSvrList.SetItem(&item);

		strTemp.Format(_T("%d"), pScanInfo->nHTTPPort);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0)?nInsertIndex:nCurrentItem;
		item.iSubItem = SUBITEM_PORTHTTP;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.Format(_T("%d"), pScanInfo->iBasePort);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
		item.iSubItem = SUBITEM_PORTBASE;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.Format(_T("%s"), pScanInfo->szSwVersion);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
		item.iSubItem = SUBITEM_SW_VERSION;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.Format(_T("%d"), pScanInfo->iVideoCnt);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
		item.iSubItem = SUBITEM_VIDEOCOUNT;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.Format(_T("%s"), pScanInfo->szModelName);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
		item.iSubItem = SUBITEM_MODELTYPE;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.LoadString(IDS_N_AND_A);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
		item.iSubItem = SUBITEM_PORTUPGRADE;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		strTemp.LoadString(IDS_N_AND_A);
		item.mask = LVIF_TEXT;
		item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
		item.iSubItem = SUBITEM_FIRMWAREVERSION;
		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		m_cSvrList.SetItem(&item);

		// extended infomation printout
		if(pScanInfo->nExtraFieldCount)
		{
			strTemp = pScanInfo->_ReadValue(L"Upgrade Port");
			item.mask = LVIF_TEXT;
			item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
			item.iSubItem = SUBITEM_PORTUPGRADE;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cSvrList.SetItem(&item);

			strTemp = pScanInfo->_ReadValue(L"Model Type");
			item.mask = LVIF_TEXT;
			item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
			item.iSubItem = SUBITEM_MODELTYPE;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cSvrList.SetItem(&item);

			strTemp = pScanInfo->_ReadValue(L"Firmware Version");
			item.mask = LVIF_TEXT;
			item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
			item.iSubItem = SUBITEM_FIRMWAREVERSION;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cSvrList.SetItem(&item);

			strTemp.LoadString(IDS_N_AND_A);
			item.mask = LVIF_TEXT;
			item.iItem = (nCurrentItem < 0) ? nInsertIndex : nCurrentItem;
			item.iSubItem = SUBITEM_SW_VERSION;
			item.pszText = (LPTSTR)(LPCTSTR)strTemp;
			m_cSvrList.SetItem(&item);
		}
		
		//if (strTemp.IsEmpty())
		//{
		//	strTemp.LoadString(IDS_N_AND_A);
		//	//for(i = SUBITEM_PORTUPGRADE; i <= SUBITEM_AUDIOOUTCOUNT; i ++)
		//	//{
		//	//	item.mask = LVIF_TEXT;
		//	//	item.iItem = (nCurrentItem < 0)?nInsertIndex:nCurrentItem;
		//	//	item.iSubItem = i;
		//	//	item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		//	//	m_cSvrList.SetItem(&item);
		//	//}
		//	for (i = SUBITEM_PORTUPGRADE; i <= SUBITEM_VIDEOCOUNT; i++)
		//	{
		//		item.mask = LVIF_TEXT;
		//		item.iItem = (nCurrentItem < 0)?nInsertIndex:nCurrentItem;
		//		item.iSubItem = i;
		//		item.pszText = (LPTSTR)(LPCTSTR)strTemp;
		//		m_cSvrList.SetItem(&item);
		//	}
		//}

		// update count;
		m_nListItemCount++;

		//iCount = m_nScanAniCount + m_nScanMarkInCount;
		SetCountMsg(m_nListItemCount);
		//delete pScanInfo;
		pScanInfo = NULL;

		// 현재 선택된 Index 앞으로 아이템을 집어 넣은 경우, 현재 선택 값에 +1을 해준다
		if(nCurrentItem < 0 && nInsertIndex <= m_nCurSvrListSel)
		{
			//OutputDebugString(L"Item++\n");
			m_nCurSvrListSel++;
		}		
	}

	_Unlock();

	return 0;
}

LRESULT CIPScanUtilDlg::OnScanCloseDlgMsg(WPARAM wParam, LPARAM lParam)
{
//	TRACE(_T("Scanning ended some error\n"));
	TRACE(_T("Close SendMessage 반환함\n"));
	
	return 0;
}

void CIPScanUtilDlg::SetCountMsg(int nCount)
{
	CString msg;
	msg.Format(_T("Count : %d"), nCount);
	SetDlgItemText(IDC_STC_SNUM, msg);
}

void CIPScanUtilDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(TM_SCANNING_ANI == nIDEvent)
	{
		if(m_bScanning)
		{
			if( 0 == m_nScanAniCount )
			{
				for (int i = 0; i < COUNT_SCAN_CLIENT; i++)
				{
					if (m_apScanner[i])
						m_apScanner[i]->SendScanRequest();
				}
			}

			int i;
			CString msg;
			msg.LoadString(IDS_STATUS_SCANNING);
	
			for(i = 0; i < m_nScanAniCount; i++)
				msg += ". ";

			SetStatusMsg(msg);

			m_nScanAniCount++;
			m_nScanAniCount = m_nScanAniCount % 10;
		}
	}

	
	CDialog::OnTimer(nIDEvent);
}

void CIPScanUtilDlg::OnLvnItemchangedSvrList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	//int nCurSel = pNMLV->iItem;
	//CString strData;

	//if(nCurSel >= 0) // && m_nCurSvrListSel != nCurSel)
	//	m_nCurSvrListSel = nCurSel;
	//else
	//	m_nCurSvrListSel = -1;

	//strData.Format(L"lButton click index(%d)\n", m_nCurSvrListSel);
	//OutputDebugString(strData);

	*pResult = 0;
}

void CIPScanUtilDlg::OnNMDblclkSvrList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	//CString str;
	//if(!m_bScanning)
	{
		SCAN_INFO* pInfo = NULL;
		CIPChangeDlg dlg;
		int i;

		//str.Format(L"lButton double click index(%d)\n", m_nCurSvrListSel);
		//OutputDebugString(str);

		if(m_nCurSvrListSel >= 0)
		{
			pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(m_nCurSvrListSel);
			if(pInfo)
			{
				// data setting before show dialog
				dlg.m_strMACAddress = pInfo->szMAC;
				dlg.m_strIPAddress  = pInfo->szAddr;
				dlg.m_strGatewayAddress = pInfo->szGateWay;
				//dlg.m_nStreamPort	= pInfo->nStreamPort;
				dlg.m_nHTTPPort		= pInfo->nHTTPPort;
				dlg.m_nVersion      = pInfo->version;
				dlg.m_strSubnetMask = pInfo->szSubnetMask;
				dlg.m_nIsDHCP       = pInfo->cIsDHCP;
			}
		} else {
			dlg.m_nStreamPort	= 2700;
			dlg.m_nHTTPPort		= 80;
			dlg.m_nIsDHCP       = TRUE;
			dlg.m_strSubnetMask = L"0.0.0.0";
			dlg.m_nVersion      = VERSION_2;
		}

		if(dlg.DoModal() == IDOK)
		{
			_Lock();
			// delete current item for validation
			// MAC 주소를 리스트에서 찾아 있으면 삭제해준다
			for(i = 0; i < m_cSvrList.GetItemCount(); i++)
			{
				pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(i);
				if(pInfo && _wcsicmp(pInfo->szMAC, dlg.m_strMACAddress.GetBuffer()) == 0)
				{
					m_cSvrList.SetItemState(m_nCurSvrListSel, 0, LVIS_SELECTED); // unselect?
					if(m_nCurSvrListSel == i)
					{
						m_nCurSvrListSel = -1;
					}

					delete pInfo;
					m_cSvrList.DeleteItem(i);
					
					m_nListItemCount--;
					if(m_nListItemCount < 0)
						m_nListItemCount = 0;

					SetCountMsg(m_nListItemCount);
//					m_btnChangeIP.EnableWindow(FALSE);
					break;
				}
			} // for(i = 0; i < m_cSvrList.GetItemCount(); i++)
			_Unlock();
		} // if(dlg.DoModal() == IDOK)
	} // if(!m_bScanning)
	*pResult = 0;
}

void CIPScanUtilDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = IPSCAN_WINDOW_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = IPSCAN_WINDOW_MIN_HEIGHT;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void  CIPScanUtilDlg::ClearScanList()
{
//	TRACE(_T("ClearScanList -->\n"));
	SCAN_INFO* pInfo = NULL;
	for(int i = 0; i < m_cSvrList.GetItemCount(); i++)
	{
		pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(i);
		if(pInfo)
		{
	//		TRACE(_T("MAC: %s\n"), pInfo->szMAC);
			delete pInfo; // 2012-08-07 : hkeins Memory leak bug fix
			pInfo = NULL;
		}
	}
	m_cSvrList.DeleteAllItems();
	m_cSvrList.ClearState(); // 2003-01-30 hkeins : list sorting function add
//	TRACE(_T("ClearScanList <--\n"));
}


int CIPScanUtilDlg::_GetScanErrorMessage(UINT32 uErrorCode, CString& strMsg)
{
	CString strTemp;
	switch(uErrorCode)
	{
	case SCAN_ERR_MEMORY:
		strTemp.LoadString(IDS_MEMORY);
		break;
	case SCAN_ERR_RECV:
		strMsg.LoadString(IDS_RECEIVE_FAILED);
		break;
	case SCAN_ERR_BIND:
		strTemp.LoadString(IDS_BIND_FAILED);
		strMsg.Format(strTemp, VH_UDP_SCAN_PORT);
		break;
	case SCAN_ERR_SOCKET_OPT:
		strMsg.LoadString(IDS_SOCKET_ERROR);
		break;
	default:
		strMsg.LoadString(IDS_UNKNOWN_ERROR);
		break;
	}
	return 0;
}

void CIPScanUtilDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CallLayoutManager();
}

BOOL CIPScanUtilDlg::CallLayoutManager()
{
	if(!m_bInit)
		return FALSE;

	CRect rcClientRect;
	CRect rcControlRect;
	CRect rcNewRect;
	CRect rcOriginateRect;
	int   nCaptionSize = GetSystemMetrics(SM_CYCAPTION);

	GetWindowRect(&rcClientRect);
	ScreenToClient(&rcClientRect);
	rcClientRect.top += nCaptionSize;

	CWnd* pScanBtn          = GetDlgItem(IDC_SCAN_BTN);
	pScanBtn->GetWindowRect(&rcControlRect);
	rcNewRect.left = rcClientRect.left + 14;
	rcNewRect.top  = rcClientRect.top  + 14;
	rcNewRect.right= rcNewRect.left    + rcControlRect.Width();
	rcNewRect.bottom=rcNewRect.top     + rcControlRect.Height();
	//pScanBtn->MoveWindow(&rcNewRect);
	
	CWnd* pChangeIPBtn      = GetDlgItem(IDC_CHANGEIP_BTN);
	rcOriginateRect = rcNewRect; // 이전 위치를 기준으로
	pChangeIPBtn->GetWindowRect(rcControlRect);
	rcNewRect.left = rcOriginateRect.right + 14;
	rcNewRect.top  = rcClientRect.left + 14;
	rcNewRect.right = rcNewRect.left + rcControlRect.Width();
	rcNewRect.bottom= rcNewRect.top  + rcControlRect.Height();
	//pChangeIPBtn->MoveWindow(&rcNewRect);

	CWnd* pUpgradeBtn         = GetDlgItem(IDC_UPGRADE_BTN);
	pUpgradeBtn->GetWindowRect(&rcControlRect);
	rcOriginateRect = rcNewRect; // 이전 위치를 기준으로
	rcNewRect.left = rcOriginateRect.right + 14;
	rcNewRect.top  = rcClientRect.top + 14;
	rcNewRect.right=rcNewRect.left + rcControlRect.Width();
	rcNewRect.bottom=rcNewRect.top + rcControlRect.Height();
	//pUpgradeBtn->MoveWindow(&rcNewRect);

	CWnd* pFactoryDefaultBtn         = GetDlgItem(IDC_FACTORY_BTN);
	pFactoryDefaultBtn->GetWindowRect(&rcControlRect);
	rcOriginateRect = rcNewRect; // 이전 위치를 기준으로
	rcNewRect.left = rcOriginateRect.right + 14;
	rcNewRect.top  = rcClientRect.top + 14;
	rcNewRect.right=rcNewRect.left + rcControlRect.Width();
	rcNewRect.bottom=rcNewRect.top + rcControlRect.Height();
	//pFactoryDefaultBtn->MoveWindow(&rcNewRect);

	CWnd* pClearBtn         = GetDlgItem(IDC_CLEAR_BTN);
	pClearBtn->GetWindowRect(&rcControlRect);
	rcOriginateRect = rcNewRect; // 이전 위치를 기준으로
	rcNewRect.left = rcOriginateRect.right + 14;
	rcNewRect.top  = rcClientRect.top + 14;
	rcNewRect.right=rcNewRect.left + rcControlRect.Width();
	rcNewRect.bottom=rcNewRect.top + rcControlRect.Height();
	//pClearBtn->MoveWindow(&rcNewRect);


	//CWnd* pVersionStatic     = GetDlgItem(IDC_STATIC_VERSION);
	//pVersionStatic->GetWindowRect(&rcControlRect);
	//rcOriginateRect = rcNewRect; // 이전 위치를 기준으로
	//rcNewRect.left = rcOriginateRect.right + 14;
	//rcNewRect.top  = rcClientRect.top + 14;
	//rcNewRect.right=rcNewRect.left + rcControlRect.Width();
	//rcNewRect.bottom=rcNewRect.top + rcControlRect.Height();
	//pVersionStatic->MoveWindow(&rcNewRect);


	//CWnd* pProtocolcombo       = GetDlgItem(IDC_PROTOCAL_COMBO);
	//pProtocolcombo->GetWindowRect(&rcControlRect);
	//rcOriginateRect = rcNewRect; // 이전 위치를 기준으로
	//rcNewRect.left = rcOriginateRect.right + 10;
	//rcNewRect.top  = rcClientRect.top + 16;
	//rcNewRect.right=rcNewRect.left + rcControlRect.Width();
	//rcNewRect.bottom=rcNewRect.top + rcControlRect.Height();
	//pProtocolcombo->MoveWindow(&rcNewRect);


	CWnd* pSTC_SERVER_COUNT = GetDlgItem(IDC_STC_SNUM);
	pSTC_SERVER_COUNT->GetWindowRect(&rcControlRect);
	ScreenToClient(&rcControlRect);
	rcNewRect = rcControlRect;
	rcNewRect.right= rcClientRect.right - 19; // item count label position move to window size relatively
	rcNewRect.left = rcNewRect.right - rcControlRect.Width();
	rcNewRect.top  = rcClientRect.top   + 14;
	rcNewRect.bottom=rcNewRect.top   + rcControlRect.Height();
	pSTC_SERVER_COUNT->MoveWindow(&rcNewRect);

	CWnd* pServerList       = GetDlgItem(IDC_SVR_LIST);
	pScanBtn->GetWindowRect(&rcOriginateRect);
	ScreenToClient(&rcOriginateRect);
	pServerList->GetWindowRect(&rcControlRect);
	ScreenToClient(&rcControlRect);
	rcNewRect = rcControlRect;
	rcNewRect.left = rcClientRect.left  + 14;
	rcNewRect.top  = rcOriginateRect.bottom + 5;
	rcNewRect.right= rcClientRect.right - 14;
	//rcNewRect.bottom= rcClientRect.bottom - 44;
	rcNewRect.bottom = rcClientRect.bottom - 56;
	//rcNewRect.top  += 16;
	
	pServerList->MoveWindow(&rcNewRect);

	CWnd* pSTC_Status       = GetDlgItem(IDC_STATIC_STATUES);
	pSTC_Status->GetWindowRect(&rcControlRect);
	ScreenToClient(&rcControlRect);
	rcNewRect = rcControlRect;
	rcNewRect.left   = rcClientRect.left + 14;
	rcNewRect.bottom = rcClientRect.bottom - 14;
	rcNewRect.top    = rcNewRect.bottom - rcControlRect.Height();
	rcNewRect.right  = rcNewRect.left   + rcControlRect.Width();
	pSTC_Status->MoveWindow(rcNewRect);

	CWnd* pClose            = GetDlgItem(IDC_CLOSE);
	pClose->GetWindowRect(&rcControlRect);
	ScreenToClient(&rcControlRect);
	rcNewRect = rcControlRect;
	rcNewRect.right = rcClientRect.right - 14;
	rcNewRect.left  = rcNewRect.right - rcControlRect.Width();
	rcNewRect.bottom= rcClientRect.bottom - 14;
	rcNewRect.top   = rcNewRect.bottom    - rcControlRect.Height();
	pClose->MoveWindow(&rcNewRect);

	return TRUE;
}

int CIPScanUtilDlg::_GetInsertPosition(SCAN_INFO* pInfo)
{
	int			i			= 0;
	int			nInsertPos	= 0;
	int			nResult		= 0;
	SCAN_INFO* pCurrentInfo = NULL;
	
	if(pInfo == NULL)
	{
		ASSERT(0);
		return -1;
	}

	//{{ older case : extra가 없으면 제일 마지막에, 그리고 그외에는 Model Type을 기준으로 아이템을 삽입
	if(pInfo->nExtraFieldCount == 0) // insert할 놈이 Model 정보가 없다면, Model 정보가 없는 아이템을 찾아서 주소가 크면 집어넣는다
	{
		nInsertPos = m_cSvrList.GetItemCount(); // 위치 못찾으면 마지막에 집어넣는다
	}
	else if(pInfo->nExtraFieldCount > 0)
	{
		nInsertPos = m_cSvrList.GetItemCount(); // 위치 못찾으면 마지막에 집어넣는다;
		for(i = 0; i < m_cSvrList.GetItemCount(); i++)
		{
			pCurrentInfo = (SCAN_INFO*)m_cSvrList.GetItemData(i);

			if(pCurrentInfo == NULL)
			{
				ASSERT(0); // abnormal case
				continue;
			}

			nResult	= CompareScanInfo( m_nSortOrient, pCurrentInfo, pInfo );
			if(m_bSortAscending)
			{
				nResult = nResult*-1;
			}

			if( 0 < nResult )
			{
				nInsertPos	= i;
				break;
			}
		}
	}
	else
	{
		ASSERT(0);
		return -1;
	}
	//}} older case : extra가 없으면 제일 마지막에, 그리고 그외에는 Model Type을 기준으로 아이템을 삽입
	// FIX ME : new order 적용

	return nInsertPos;
}

void CIPScanUtilDlg::_Lock()
{
	// Critical Section에 진입.
	EnterCriticalSection(&m_mt);
}

void CIPScanUtilDlg::_Unlock()
{
	// Critical Section에서 빠져나옴
	LeaveCriticalSection(&m_mt);
}

void CIPScanUtilDlg::OnDestroy()
{
	_ClearAdaptorInfos();
	m_DISP_FONT.DeleteObject();
	CDialog::OnDestroy();
	// CRITICAL_SECTION 객체를 소멸시킨다.
	DeleteCriticalSection(&m_mt);
}

void CIPScanUtilDlg::OnNMRClickSvrList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if(pNMItemActivate->iItem >= 0)
	{
		m_nCurSvrListSel = pNMItemActivate->iItem;

		POINT point;
		point.x = 0;
		point.y = 0;
		GetCursorPos(&point);
		
		CMenu* pMenu = m_cPopupMenu.GetSubMenu(0);
		if(pMenu) pMenu->TrackPopupMenu(TPM_TOPALIGN, point.x, point.y, this, 0);
	}

	*pResult = 0;
}

void CIPScanUtilDlg::OnSetupOSD()
{
	COSDChangeDlg	OSDChangeDlg;
	SCAN_INFO*		pScanInfo		= NULL;
	int				nItem			= 0; 
	int				iSelectCount	= m_cSvrList.GetSelectedCount();
	int				iScanInfoCount	= 0;

	if(iSelectCount <= 0)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}

	POSITION pos = m_cSvrList.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}
	else
	{
		pScanInfo		= new SCAN_INFO[iSelectCount];

		_Lock();
		if( pScanInfo )
		{
			while (pos)
			{
				nItem = m_cSvrList.GetNextSelectedItem(pos);
				TRACE(_T("Item %d was selected!\n"), nItem);

				// you could do your own processing on nItem here
				pScanInfo[iScanInfoCount] = *(SCAN_INFO*)m_cSvrList.GetItemData(nItem);
				iScanInfoCount++;
			}
		}
		_Unlock();

		CString strTemp;

		NETWORK_ADAPTOR_INFO_ipv4* pAdpatorInfo = NULL;

		pAdpatorInfo = (NETWORK_ADAPTOR_INFO_ipv4*)m_cmbNetAdaptor.GetItemData(m_cmbNetAdaptor.GetCurSel());

		if( TRUE == OSDChangeDlg.SetScaninfo(iScanInfoCount, pScanInfo, pAdpatorInfo->iAdapterID ) )
		{
			if(OSDChangeDlg.DoModal() == IDOK)
			{
			}
		}

		if( 0 == OSDChangeDlg.GetSelectScanInfoCount() )
		{
			AfxMessageBox(_T("This model is not supported."), MB_ICONWARNING);
		}

		if( pScanInfo )
		{
			delete [] pScanInfo;
			pScanInfo	= NULL;
		}
	}
}

void CIPScanUtilDlg::OnSetupResolution()
{
	CResolutionChangeDlg	ResolutionChangeDlg;
	SCAN_INFO*	pScanInfo		= NULL;
	int			nItem			= 0; 
	int			iSelectCount	= m_cSvrList.GetSelectedCount();
	int			iScanInfoCount	= 0;

	if(iSelectCount <= 0)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}

	POSITION pos = m_cSvrList.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}
	else
	{
		pScanInfo		= new SCAN_INFO[iSelectCount];

		_Lock();
		if( pScanInfo )
		{
			while (pos)
			{
				nItem = m_cSvrList.GetNextSelectedItem(pos);
				TRACE(_T("Item %d was selected!\n"), nItem);

				// you could do your own processing on nItem here
				pScanInfo[iScanInfoCount] = *(SCAN_INFO*)m_cSvrList.GetItemData(nItem);
				iScanInfoCount++;
			}
		}
		_Unlock();

		CString strTemp;

		NETWORK_ADAPTOR_INFO_ipv4* pAdpatorInfo = NULL;
		 
		pAdpatorInfo = (NETWORK_ADAPTOR_INFO_ipv4*)m_cmbNetAdaptor.GetItemData(m_cmbNetAdaptor.GetCurSel());

		if( TRUE == ResolutionChangeDlg.SetScaninfo(iScanInfoCount, pScanInfo, pAdpatorInfo->iAdapterID ) )
		{
			if(ResolutionChangeDlg.DoModal() == IDOK)
			{
			}
		}

		if( 0 == ResolutionChangeDlg.GetSelectScanInfoCount() )
		{
			AfxMessageBox(_T("This model is not supported."), MB_ICONWARNING);
		}

		if( pScanInfo )
		{
			delete [] pScanInfo;
			pScanInfo	= NULL;
		}
	}

	//CString szHttpAddress;
	//_Lock();
	//SCAN_INFO* pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(m_nCurSvrListSel);
	//if(pInfo)
	//{
	//	szHttpAddress.Format(L"http://%s:%d", pInfo->szAddr, pInfo->nHTTPPort);
	//	ShellExecute(NULL, L"Open", szHttpAddress, NULL, NULL, SW_SHOW);			
	//_Unlock();
}

void CIPScanUtilDlg::OnNMClickSvrList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

//	TRACE(L"CIPScanUtilDlg::OnNMClickSvrList called\n");
	if(pNMItemActivate->iItem >= 0)
	{
//		m_btnChangeIP.EnableWindow(TRUE);
		m_nCurSvrListSel = pNMItemActivate->iItem;
	}
	else
	{
//		m_btnChangeIP.EnableWindow(FALSE);
		m_nCurSvrListSel = -1;
	}

	*pResult = 0;
}

// 잠시 업그레이드 버튼 Hide를 위한 주석처리(2022-06-08)
//void CIPScanUtilDlg::OnBnClickedUpgradeBtn()
//{
//	int			i			= 0;
//	int			nUpgradeCnt = 0;
//	int			nCurPos		= 0;
//	SCAN_INFO*	pScanInfo	= NULL;
//	BOOL		bScanning	= m_bScanning;
//
//	for(i = 0; i < m_cSvrList.GetItemCount(); i++)
//	{
//		if(m_cSvrList.GetCheck(i))
//		{
//			nUpgradeCnt++;
//		}
//	}
//
//	if(nUpgradeCnt <= 0)
//	{
//		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
//		return;
//	}
//
//	pScanInfo = new SCAN_INFO[nUpgradeCnt]; // SCAN_INFO[] 배열을 UpgradeDlg에 파라메터로 넘겨준다
//	if(pScanInfo == NULL)
//	{
//		AfxMessageBox(_T("Out of memory"), MB_ICONWARNING);
//		return;
//	}
//	nCurPos = 0;
//
//	_Lock();
//	for(i = 0; i < m_cSvrList.GetItemCount(); i++)
//	{
//		if(m_cSvrList.GetCheck(i))
//		{
//			pScanInfo[nCurPos++] = *((SCAN_INFO*)(m_cSvrList.GetItemData(i)));// 배열의 각각 요소를 복사
//		}
//	}
//	_Unlock();
//
//	if( TRUE == bScanning )
//	{
//		// 스캔중이 었다면 업그레이드 하기 이전에 종료 시킨다. 
//		OnBnClickedScanBtn();
//	}
//
//
//	CUpgradeDlg dlg;
//	dlg.SetScaninfo ( nUpgradeCnt, pScanInfo ); // SCAN_INFO[] 배열을 UpgradeDlg에 파라메터로 넘겨준다
//
//	if( dlg.DoModal() == IDOK)
//	{
//		//m_iSelectVersion	= m_cmbProtocol.GetCurSel();
//		OnBnClickedClearBtn();
//	}
//	else
//	{
//		//m_iSelectVersion	= m_cmbProtocol.GetCurSel();
//		OnBnClickedClearBtn();
//	}
//
//	if(	pScanInfo )
//	{
//		delete[] pScanInfo;
//		pScanInfo = NULL;
//	}
//
//	if( TRUE == bScanning )
//	{
//		OnBnClickedScanBtn();
//	}
//}

void CIPScanUtilDlg::OnBnClickedChangeipBtn2()
{
	int				i					= 0;
	int				nSelScanInfoCount	= 0;
	SCAN_INFO*		pSelScanInfo		= NULL;
	int				nScanInfoCount		= 0;
	SCAN_INFO*		pScanInfo			= NULL;
	CIPChangeDlg2*	pDlg				= NULL;
	int				nTotalItemCnt		= 0;
	int				nCurPos				= 0;
	BOOL			bScanning			= m_bScanning;

	for(i = 0; i < m_cSvrList.GetItemCount(); i++)
	{
		if(m_cSvrList.GetCheck(i))
		{
			nSelScanInfoCount++;
		}

		nScanInfoCount++;
	}

	if(nSelScanInfoCount <= 0)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}

	pSelScanInfo = new SCAN_INFO[nSelScanInfoCount]; // SCAN_INFO[] 배열을 IP change dialog에 파라메터로 넘겨준다
	if(pSelScanInfo == NULL)
	{
		AfxMessageBox(_T("Out of memory"), MB_ICONWARNING);
		return;
	}

	pScanInfo		= new SCAN_INFO[nScanInfoCount];
	if(pScanInfo == NULL)
	{
		AfxMessageBox(_T("Out of memory"), MB_ICONWARNING);
		return;
	}

	nCurPos = 0;
	_Lock();
	for(i = 0; i < m_cSvrList.GetItemCount(); i++)
	{
		if(m_cSvrList.GetCheck(i))
		{
			pSelScanInfo[nCurPos++] = *((SCAN_INFO*)m_cSvrList.GetItemData(i)); // copy values
		}

		pScanInfo[i] = *((SCAN_INFO*)m_cSvrList.GetItemData(i)); // copy values
	}
	_Unlock();

	if( TRUE == bScanning )
	{
		// 스캔중이 었다면 아이피 변경 하기 이전에 종료 시킨다. 
		OnBnClickedScanBtn();
	}

	pDlg = new CIPChangeDlg2;
	pDlg->SetScanInfo( nScanInfoCount, pScanInfo, nSelScanInfoCount, pSelScanInfo);
	//pDlg->SetScanner(m_pScannerVision);


	if(pDlg->DoModal() == IDOK)
	{
		//m_iSelectVersion	= m_cmbProtocol.GetCurSel();
		OnBnClickedClearBtn();
	}
	else
	{
		//m_iSelectVersion	= m_cmbProtocol.GetCurSel();
		OnBnClickedClearBtn();
	}

	if (NULL != pDlg)
	{
		delete pDlg;
		pDlg = NULL;
	}

	if( TRUE == bScanning )
	{
		OnBnClickedScanBtn();
	}

	if (NULL != pScanInfo)
	{
		delete[] pScanInfo;
		pScanInfo = NULL;
	}
	
	if (NULL != pSelScanInfo)
	{
		delete[] pSelScanInfo;
		pSelScanInfo = NULL;
	}
}

void CIPScanUtilDlg::OnNMDblclkSvrList2(NMHDR *pNMHDR, LRESULT *pResult)
{
/*	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int i;
	int nSelectedCnt = 0;
	int nTotalItemCnt = 0;
	SCAN_INFO* pSelectedInfo = NULL;
	CIPChangeDlg2 dlg;

	// count checked items
	for(i = 0; i < m_cSvrList.GetItemCount(); i++)
	{
		if(m_cSvrList.GetCheck(i))
		{
			nSelectedCnt++;
		}
	}

	if(nSelectedCnt == 0)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}
	pSelectedInfo = new SCAN_INFO[nSelectedCnt]; // SCAN_INFO[] 배열을 CIPChangeDlg2 파라메터로 넘겨준다.
	if(pSelectedInfo == NULL)
	{
		AfxMessageBox(_T("Out of memory"), MB_ICONWARNING);
		return;
	}

	for(i = 0; i < nSelectedCnt; i++)
	{
		if(m_cSvrList.GetCheck(i))
		{
			pSelectedInfo[i] = *((SCAN_INFO*)m_cSvrList.GetItemData(i));
		}
	}

	dlg.m_nSelectedCnt = nSelectedCnt;
	dlg.m_pScanInfo = pSelectedInfo;

	if(dlg.DoModal() == IDOK)
	{
		pSelectedInfo = NULL;
		_Lock();
		// delete current item for validation
		nTotalItemCnt = m_cSvrList.GetItemCount();

		for(i = 0; m_cSvrList.GetItemCount() > nTotalItemCnt - nSelectedCnt; i++)
		{
			pSelectedInfo = (SCAN_INFO*)m_cSvrList.GetItemData(i);
			if(m_cSvrList.GetCheck(i)<0)
				break;

			if(m_cSvrList.GetCheck(i))
			{
				m_cSvrList.SetItemState(m_nCurSvrListSel, 0, LVIS_SELECTED); // unselect?
				if(m_nCurSvrListSel == i)
				{
					m_nCurSvrListSel = -1;
				}

				delete pSelectedInfo;
				m_cSvrList.DeleteItem(i);

				i--;

				m_nListItemCount--;
				if(m_nListItemCount < 0)
					m_nListItemCount = 0;
			}
		} // for(i = 0; i < m_cSvrList.GetItemCount(); i++)
		SetCountMsg(nTotalItemCnt - nSelectedCnt);

		_Unlock();
	}
*/
	*pResult = 0;
}

void CIPScanUtilDlg::OnNMDblclkSvrList3(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 리스트 컨트롤에서 데이터가 있는 영역의 마우스 우 클릭 팝업 메뉴 띄우기
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// 2012-08-07 hkeins : 더블 클릭 시 Setup 화면을 실행하도록 적용
	if(m_nCurSvrListSel >= 0)
	{
		CString szHttpAddress;
		_Lock();
		// 선택한 아이템의 인덱스를 얻는다.
		SCAN_INFO* pInfo = (SCAN_INFO*)m_cSvrList.GetItemData(m_nCurSvrListSel);
		if(pInfo)
		{
			szHttpAddress.Format(L"http://%s:%d", pInfo->szAddr, pInfo->nHTTPPort);
			ShellExecute(NULL, L"Open", szHttpAddress, NULL, NULL, SW_SHOW);			
		}
		_Unlock();
	}

	*pResult = 0;
}

void CIPScanUtilDlg::OnBnClickedFactoryBtn()
{
	int			i				= 0;
	int			nSelectedCnt	= 0;
	int			nTotalItemCnt	= 0;
	SCAN_INFO*	pSelectedInfo	= NULL;
	int			nCurPos			= 0;
	BOOL		bScanning		= m_bScanning;


	
	// count checked items
	for(i = 0; i < m_cSvrList.GetItemCount(); i++)
	{
		if(m_cSvrList.GetCheck(i))
		{
			nSelectedCnt++;
		}
	}

	if(nSelectedCnt <= 0)
	{
		AfxMessageBox(_T("Please select one or more ipcameras..."), MB_ICONWARNING);
		return;
	}

	pSelectedInfo = new SCAN_INFO[nSelectedCnt]; // SCAN_INFO를 CFactoryDefaultDlg의 파라메터로 전달
	if(pSelectedInfo == NULL)
	{
		AfxMessageBox(_T("Out of memory"), MB_ICONWARNING);
		return;
	}
	nCurPos = 0;
	_Lock();
	for(i = 0; i <  m_cSvrList.GetItemCount(); i++)
	{
		if(m_cSvrList.GetCheck(i))
		{
			pSelectedInfo[nCurPos++] = *((SCAN_INFO*)m_cSvrList.GetItemData(i));	
		}
	}
	_Unlock();

	if( TRUE == bScanning )
	{
		// 스캔중이 었다면 팩토리 디폴트 하기 이전에 종료 시킨다. 
		OnBnClickedScanBtn();
	}

	CFactoryDefaultDlg dlg;
	dlg.m_nSelectedCnt = nSelectedCnt;
	dlg.m_pScanInfo = pSelectedInfo;

	if(dlg.DoModal() == IDOK)
	{
		//m_iSelectVersion	= m_cmbProtocol.GetCurSel();
		OnBnClickedClearBtn();
	}
	else
	{
		//m_iSelectVersion	= m_cmbProtocol.GetCurSel();
		OnBnClickedClearBtn();
	}

	if( TRUE == bScanning )
	{
		OnBnClickedScanBtn();
	}
}

LRESULT CIPScanUtilDlg::OnConnectCheck(WPARAM wParam, LPARAM lParam)
{
	int nErrorCode = (int)wParam;
	int nIndex = LOWORD(lParam);
	int nPort  = HIWORD(lParam);

//	TRACE(L"CIPScanUtilDlg::OnConnectCheck called, errorCode = %d, Index = %d port = %d\n", 
//		nErrorCode, nIndex, nPort);

////////////
	SCAN_INFO* pScanInfo = (SCAN_INFO*)m_cSvrList.GetItemData(nIndex);
	if(pScanInfo == NULL)
	{
		ASSERT(0); // 이런 경우가?
		return 0L;
	}
	LV_ITEM item;
	memset(&item, 0, sizeof(item));

	item.mask     = LVIF_TEXT;
	item.iItem    = nIndex;
	//item.iSubItem = (pScanInfo->nStreamPort == nPort)?7:8;
	item.pszText  = (nErrorCode == 0)?L"OK":L"FAIL";
	m_cSvrList.SetItem(&item);
////////////

	return 0;
}

// 2013-01-30 hkeins : List sorting function add
//WM_SORT_REQUEST
// ----------------------------------------------------------------------------------------------------
//   0           1             2             3            4             5              6                              
// | IP        | MAC Address | Stream Port | Http Port  | System Name | Model Type   | Firmware Version | 
// | 7           8             9            10           11            12
// | Resolution| Video format| AIN cnt     | AOUT cnt   | Audio in    | Audio out
// ----------------------------------------------------------------------------------------------------
typedef struct tagCheckCompareParam
{
	LPVOID	pThis;
	BOOL	bAscending;
	int		nOrient;
}ST_CHECK_COMPARE_PARAM, *LPST_CHECK_COMPARE_PARAM;

int  CIPScanUtilDlg::CompareScanInfo(int nItemColumn, tagSCAN_STRUCT* pInfo1, tagSCAN_STRUCT* pInfo2)
{
	int nResult = 0;
	switch(nItemColumn)
	{
	case CIPScanUtilDlg::SUBITEM_IPADDRESS: // IP 0
		nResult = _CompareIP(pInfo1->szAddr, pInfo2->szAddr);
		break;
	case CIPScanUtilDlg::SUBITEM_MACADDRESS: // MAC 1
		nResult = wcscmp(pInfo1->szMAC, pInfo2->szMAC);
		break;
	case CIPScanUtilDlg::SUBITEM_IS_DHCP:    // DHCP 2
		nResult = (pInfo1->cIsDHCP - pInfo2->cIsDHCP);
		break;
	//case CIPScanUtilDlg::SUBITEM_PORTSTREAM: // Stream Port 3
	//	nResult = pInfo1->nStreamPort - pInfo2->nStreamPort;
	//	break;
	case CIPScanUtilDlg::SUBITEM_PORTHTTP:   // Http Port 4
		nResult = pInfo1->nHTTPPort - pInfo2->nHTTPPort;
		break;
	case CIPScanUtilDlg::SUBITEM_PORTUPGRADE: // upgrade port 5
		nResult = pInfo1->_ReadValue(L"Upgrade Port") - pInfo2->_ReadValue(L"Upgrade Port");
		break;
	//case CIPScanUtilDlg::SUBITEM_SYSTEMNAME: // Server Name 6
	//	nResult = pInfo1->_ReadValue(L"System Name").Compare(pInfo2->_ReadValue(L"System Name"));
	//	break;
	case CIPScanUtilDlg::SUBITEM_MODELTYPE: // Model 7
		nResult = pInfo1->_ReadValue(L"Model Type").Compare(pInfo2->_ReadValue(L"Model Type"));
		break;
	case CIPScanUtilDlg::SUBITEM_FIRMWAREVERSION: // Firmware Version 8
		nResult = pInfo1->_ReadValue(L"Firmware Version").Compare(pInfo2->_ReadValue(L"Firmware Version"));
		break;
	case CIPScanUtilDlg::SUBITEM_SW_VERSION: // SW Version 8
		nResult = pInfo1->_ReadValue(L"S/W Version").Compare(pInfo2->_ReadValue(L"S/W Version"));
		break;
	case CIPScanUtilDlg::SUBITEM_VIDEOCOUNT: // Video Count 9
		nResult = pInfo1->_ReadValue(L"Video Count").Compare(pInfo2->_ReadValue(L"Video Count"));
		break;
	//case CIPScanUtilDlg::SUBITEM_SUPPORTRESOLUTION: // Resolution 9
	//	nResult = pInfo1->_ReadValue(L"Support Resolution").Compare(pInfo2->_ReadValue(L"Support Resolution"));
	//	break;
	//case CIPScanUtilDlg::SUBITEM_VIDEOFORMAT:   // Video format 10
	//	nResult = pInfo1->_ReadValue(L"Video Format").Compare(pInfo2->_ReadValue(L"Video Format"));
	//	break;
	//case CIPScanUtilDlg::SUBITEM_ALARMINCOUNT: // Alarm in count 11
	//	nResult = pInfo1->_ReadValue(L"Alarm In Count").Compare(pInfo2->_ReadValue(L"Alarm In Count"));
	//	break;
	//case CIPScanUtilDlg::SUBITEM_ALARMOUTCOUNT:// Alarm out count 12
	//	nResult = pInfo1->_ReadValue(L"Alarm Out Count").Compare(pInfo2->_ReadValue(L"Alarm Out Count"));
	//	break;
	//case CIPScanUtilDlg::SUBITEM_AUDIOINCOUNT: // Audio in count 13
	//	nResult = pInfo1->_ReadValue(L"Audio In Count").Compare(pInfo2->_ReadValue(L"Audio In Count"));
	//	break;
	//case CIPScanUtilDlg::SUBITEM_AUDIOOUTCOUNT: // Audio out count 14
	//	nResult = pInfo1->_ReadValue(L"Audio Out Count").Compare(pInfo2->_ReadValue(L"Audio Out Count"));
	//	break;
	default:
		ASSERT(0); // column added?
		nResult = 0;
	}
	return nResult;
}

static int CALLBACK CheckCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	SCAN_INFO* pInfo1 = (SCAN_INFO*)lParam1;
	SCAN_INFO* pInfo2 = (SCAN_INFO*)lParam2;
	LPST_CHECK_COMPARE_PARAM pParam = (LPST_CHECK_COMPARE_PARAM)lParam3;
	int        nCompareResult = 0; // no swap
	ASSERT(pParam != NULL);
	nCompareResult = ((CIPScanUtilDlg*)pParam->pThis)->CompareScanInfo(pParam->nOrient, pInfo1, pInfo2);
	if(pParam->bAscending)
	{
		nCompareResult = -1 * nCompareResult;
	}
	return nCompareResult;
}

LRESULT CIPScanUtilDlg::OnSortRequest(WPARAM wParam, LPARAM lParam)
{
	_Lock();
//	TRACE(L"Sort request received wParam=%d, lParam=%d\n");
	ST_CHECK_COMPARE_PARAM stParam;
	BOOL bSortProcess	= FALSE;
	stParam.nOrient		= wParam;
	stParam.bAscending	= lParam;

	// Store sorting orient and ascending order
	m_nSortOrient    = stParam.nOrient;
	m_bSortAscending = stParam.bAscending;

	// 일부 항목에 대해서만 sorting 지원 -->
	// 모든 항목에 대해 sorting 지원하도록 적용
	//switch(wParam)
	//{
	//case 0: // IP
	//case 1: // MAC
	//case 4: // System Name
	//case 5: // Model Type
	//case 6: // Firmware Version
	//	bSortProcess  = TRUE;
	//	break;
	//default:
	//	TRACE(L"Ignore column sorting");
	//	bSortProcess = FALSE;
	//}
	//if(bSortProcess)

	
	m_cSvrList.SortItems(CheckCompareFunc, (DWORD_PTR)&stParam);

	_Unlock();

	return 0L;
}
//
//void CIPScanUtilDlg::_ClearThreadList()
//{
//	CConnectCheckThread* pItem = NULL;
//	std::vector<CConnectCheckThread*>::iterator it = m_CheckThreadList.begin();
//
//	while(it != m_CheckThreadList.end())
//	{
//		pItem = *it;
//		if(pItem)
//		{
//			delete pItem;
//		}
//		it++;
//	}
//	m_CheckThreadList.clear();
//}


// read accept address from UI
void   CIPScanUtilDlg::_ReadBindAddress()
{
	NETWORK_ADAPTOR_INFO_ipv4* pAdpatorInfo = NULL;
	m_ulAcceptAddress = 0;
	USES_CONVERSION;

	if(m_cmbNetAdaptor.GetCount() > 0 && m_cmbNetAdaptor.GetCurSel() >= 0)
	{
		pAdpatorInfo = (NETWORK_ADAPTOR_INFO_ipv4*)m_cmbNetAdaptor.GetItemData(m_cmbNetAdaptor.GetCurSel());
		if(pAdpatorInfo)
		{
			TRACE(L"Selected Interface : %s IPAddress : %s\n", pAdpatorInfo->szDesc, pAdpatorInfo->szIPAddress);
			m_ulAcceptAddress = inet_addr(W2A(pAdpatorInfo->szIPAddress));
		}
		else
		{
			m_ulAcceptAddress = 0;
		}
	}
}

//WCHAR* CIPScanUtilDlg::_mbcs2widechar(char* szSrcBuffer)
//{
//	// MBCS를 유니코드로 변환
//	::ZeroMemory(m_szTempValue, sizeof(m_szTempValue)); // sizeof(wszBuffer)은 256임.
//	::MultiByteToWideChar(CP_ACP, 0, szSrcBuffer, lstrlenA(szSrcBuffer)+1, m_szTempValue, sizeof(m_szTempValue)/sizeof(m_szTempValue[0]));
//	
//	return m_szTempValue;
//}

void  CIPScanUtilDlg::_LoadNetworkAdaptorInformation()
{
	NETWORK_ADAPTOR_INFO_ipv4* pAdaptorInfo = NULL;
	int                        nInsertedPos = 0;
	int						   i = 0;
	_ClearAdaptorInfos();
	/////////////////////////
	// Get interface informations
	/////////////////////////
	m_netInfo.LoadInformation();

	for(i = 0; i < m_netInfo.GetInformationCounts(); i++)
	{
		pAdaptorInfo = m_netInfo.GetInformation(i);
		if(pAdaptorInfo)
		{
			nInsertedPos = m_cmbNetAdaptor.AddString(pAdaptorInfo->szDesc);
			m_cmbNetAdaptor.SetItemData(nInsertedPos, (DWORD_PTR)pAdaptorInfo);
		}
	}

	m_cmbNetAdaptor.SetCurSel(0);
	//////////////////////////
}

void CIPScanUtilDlg::_ClearAdaptorInfos()
{
	int i = 0;
	m_netInfo.ClearInformation();

	m_cmbNetAdaptor.ResetContent();
	//m_cmbNetAdaptor.AddString(L"-- Auto select --");
	//m_cmbNetAdaptor.SetCurSel(0);
}

void CIPScanUtilDlg::OnCbnSelchangeAdaptorCmb()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnBnClickedClearBtn();
}


//void CIPScanUtilDlg::OnCbnSelchangeProtocalCombo()
//{
//	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
//		//if( m_iSelectVersion != m_cmbProtocol.GetCurSel() )
//		//{
//		//	if( TRUE == m_bScanning )
//		//	{
//		//		OnBnClickedScanBtn();
//		//	}
//	
//	
//		//	m_iSelectVersion	= m_cmbProtocol.GetCurSel();
//		//	OnBnClickedClearBtn();
//		//	OnBnClickedScanBtn();
//		//}
//}

void CIPScanUtilDlg::OnClickedOpenXml()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	char*	pszBuff			= NULL;
	FILE*	pFile			= NULL;
	DWORD	dwFileSize		= 0;
	LPXNode lpHeader		= NULL;
	LPXNode lpBody			= NULL;
	LPXNode lpBodyCommon	= NULL;
	LPXNode lpItemData		= NULL;
	XNode	stNode;
	


	pFile = fopen("D:\\C_IPScanner\\IP_Scanner\\ProbeMatches.txt", "rb");
	fseek(pFile, 0, SEEK_END);
	dwFileSize = ftell(pFile);
	
	pszBuff = new char[sizeof(char) * dwFileSize];
	memset(pszBuff, 0, dwFileSize);

	fseek(pFile, 0, SEEK_SET);
	fread(pszBuff, 1, dwFileSize, pFile);

	stNode.Load(pszBuff);

	lpBody = stNode.GetChildArg("s:Body", NULL);
	lpBodyCommon = lpBody->GetChild("d:ProbeMatches")->GetChild("d:ProbeMatch");

	lpHeader = stNode.GetChildArg("wsa:MessageID",NULL);
	TRACE("%s\n", lpHeader->value);

	lpHeader = stNode.GetChildArg("wsa:RelatesTo", NULL);
	TRACE("%s\n", lpHeader->value);

	lpHeader = stNode.GetChildArg("wsa:To", NULL);
	TRACE("%s\n", lpHeader->value);

	lpHeader = stNode.GetChildArg("wsa:Action", NULL);
	TRACE("%s\n", lpHeader->value);

	lpItemData = lpBodyCommon->GetChild("wsa:EndpointReference")->GetChild("wsa:Address");
	TRACE("%s\n", lpItemData->value);

	lpBody = lpBodyCommon->GetChild("d:Types");
	TRACE("%s\n", lpBody->value);

	lpBody = lpBodyCommon->GetChild("d:Scopes");
	TRACE("%s\n", lpBody->value);

	lpBody = lpBodyCommon->GetChild("d:XAddrs");
	TRACE("%s\n", lpBody->value);

	lpBody = lpBodyCommon->GetChild("d:MetadataVersion");
	TRACE("%s\n", lpBody->value);


	if (NULL != pszBuff)
	{
		delete[] pszBuff;
		pszBuff = NULL;
	}
	
	if (NULL != pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	
	
}
