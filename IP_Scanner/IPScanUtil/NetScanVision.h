//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: Server scanning class
// Date: 2010-08-26 hkeins first release
//       2011-10-12 hkeins Scan 정보 확장
// Author: hkeins
//
#pragma once

//enum VERSION_PROTOCAL {
//	VERSION_1 = 0x00,
//	VERSION_2
//};
//
//const UINT32 SCAN_INFO_m_pReceive_buffer_SIZE = 64 * 1024; // 64 Kbytes temp buffer for receive temporary data
//const UINT32 SCAN_ERR_NONE = 0x00000000;
//const UINT32 SCAN_ERR_SOCKET_OPT = 0x00000001;
//const UINT32 SCAN_ERR_BIND = 0x00000002;
//const UINT32 SCAN_ERR_MEMORY = 0x00000003;
//const UINT32 SCAN_ERR_RECV = 0x00000004;

// 2012-08-07 hkeins : tagSCAN_EXT_INFO_STRUCT = operation 시 메모리 삭제 버그 수정
typedef struct tagSCAN_EXT_INFO_STRUCT
{
	WCHAR szCaption[32];
	WCHAR *lpszValue;
	int   nValueLen;
	tagSCAN_EXT_INFO_STRUCT()
	{
		memset(szCaption, 0, sizeof(szCaption));
		lpszValue = NULL;
		nValueLen = 0;
	}
	~tagSCAN_EXT_INFO_STRUCT()
	{
		if(lpszValue)
		{
			delete[] lpszValue;
			lpszValue = 0;
		}
	}
	tagSCAN_EXT_INFO_STRUCT& operator = (tagSCAN_EXT_INFO_STRUCT& src)
	{
		wcscpy_s(this->szCaption, 32, src.szCaption);
		if(this->lpszValue)
		{
			delete[] lpszValue;
			lpszValue = NULL;
		}
		this->nValueLen = src.nValueLen;
		ASSERT(src.nValueLen > 0);
		if(src.lpszValue && src.nValueLen > 0)
		{
			this->lpszValue = new WCHAR[this->nValueLen]; // NULL terminate include length
			if(this->lpszValue)
				wcscpy_s(this->lpszValue, this->nValueLen, src.lpszValue);
			else
				this->nValueLen = 0;
		}
		return *this;
	}
}SCAN_EXT_INFO, *LPSCAN_EXT_INFO;

typedef struct tagSCAN_STRUCT
{
	WCHAR	szAddr[30];
	WCHAR	szMAC[30];
	WCHAR	szGateWay[30];
	int		nStreamPort;
	int		nHTTPPort;
	int		version;
	char	cIsDHCP; // 0 - static, 1 - DHCP
	WCHAR	szSubnetMask[30];
	int		nExtraFieldCount;
	time_t	tReceiveTime;
	SCAN_EXT_INFO* pExtScanInfos;

	tagSCAN_STRUCT()
	{
		memset(this, 0, sizeof(tagSCAN_STRUCT));
		version				= 1;
		nExtraFieldCount	= 0;
		pExtScanInfos		= NULL;
	}
	~tagSCAN_STRUCT()
	{
		SAFE_DELETEA(pExtScanInfos); // 2012-01-02 : hkeins memory delete error fix. DELETE vs DELETEA 동작 차이에 의해 메모리 해제시 오류가 발생
	}

	tagSCAN_STRUCT& operator = (tagSCAN_STRUCT& src)
	{
		wcscpy_s(this->szAddr, 30, src.szAddr);
		wcscpy_s(this->szMAC,  30, src.szMAC);
		wcscpy_s(this->szGateWay,  30, src.szGateWay);
		wcscpy_s(this->szSubnetMask,  30, src.szSubnetMask);
		this->nStreamPort	= src.nStreamPort;
		this->nHTTPPort		= src.nHTTPPort;
		this->version		= src.version;
		this->cIsDHCP		= src.cIsDHCP;
		this->tReceiveTime	= src.tReceiveTime;

		SAFE_DELETEA(this->pExtScanInfos);
		this->nExtraFieldCount = src.nExtraFieldCount;
		if(src.nExtraFieldCount > 0 && src.pExtScanInfos)
		{
			this->pExtScanInfos = new SCAN_EXT_INFO[src.nExtraFieldCount];
			if(this->pExtScanInfos)
			{
				for(int i = 0; i < src.nExtraFieldCount; i++)
				{
					this->pExtScanInfos[i] = src.pExtScanInfos[i]; // copy values
				}
			}
			else
			{
				this->nExtraFieldCount = 0;
			}
		}
		return *this;
	}


	BOOL operator == (tagSCAN_STRUCT& src)
	{
		if( 0 != wcscmp(this->szMAC			, src.szMAC			) )			return FALSE;
		if( 0 != wcscmp(this->szGateWay		, src.szGateWay		) )			return FALSE;
		if( 0 != wcscmp(this->szSubnetMask	, src.szSubnetMask	) )			return FALSE;

		if( this->nStreamPort			!= src.nStreamPort		)			return FALSE;
		if( this->nHTTPPort				!= src.nHTTPPort		)			return FALSE;
		if( this->version				!= src.version			)			return FALSE;
		if( this->cIsDHCP				!= src.cIsDHCP			)			return FALSE;
		if( this->nExtraFieldCount		!= src.nExtraFieldCount	)			return FALSE;

		for(int i = 0; i < src.nExtraFieldCount; i++)
		{
			if( this->pExtScanInfos[i].nValueLen		!= src.pExtScanInfos[i].nValueLen	)			return FALSE;
			if( this->pExtScanInfos[i].nValueLen )
			{
				if( 0 != wcscmp(this->pExtScanInfos[i].szCaption	, src.pExtScanInfos[i].szCaption) )		return FALSE;
				if( 0 != wcscmp(this->pExtScanInfos[i].lpszValue	, src.pExtScanInfos[i].lpszValue	) )		return FALSE;
			}
		}

		return TRUE;
	}

	void	SetReceiveTime();
	int     _PrintValues();
	CString _ReadValue(WCHAR* szCaption);
	CString _ReadValues(); // 모든 값을 읽여서 Caption : Value 값으로 리턴해준다

	// FIX ME : List control Display 항목이 바뀌면 다음 함수도 변경하세요
	// ----------------------------------------------------------------------------------------------------
	//   0           1             2             3            4             5              6                              
	// | IP        | MAC Address | Stream Port | Http Port  | System Name | Model Type   | Firmware Version | 
	// | 7           8             9            10           11            12
	// | Resolution| Video format| AIN cnt     | AOUT cnt   | Audio in    | Audio out
	// ----------------------------------------------------------------------------------------------------
	static int     _CompareScanInfo(int nItemColumn, tagSCAN_STRUCT* pInfo1, tagSCAN_STRUCT* pInfo2);

}SCAN_INFO, *LPSCAN_INFO;


class CNetScanVision
{
public:
	CNetScanVision();
	~CNetScanVision(void);

	BOOL StartScan( );
	BOOL StopScan();
	void SetNotifyWindow(HWND hWnd, LONG msg);
	void SetCloseMsgRecvWindow(HWND hWnd, LONG msg = WM_CLOSE);

	void thrReceiver();

	// 2010-08-27 hkeins : change IP configuration
	static BOOL RequestIPChange(WCHAR* strTargetServerMAC, WCHAR* strNewIP, WCHAR* strNewGateWay, int nStreamPort = 2700, int nHTTPPort = 80);
	static BOOL RequestIPChange2(WCHAR* strTargetServerMAC, WCHAR* strNewIP, WCHAR* strNewGateWay, int nStreamPort = 2700, int nHTTPPort = 80, int cIsDHCP = 0, WCHAR* strNewsubnetMask = L"255.255.255.0", WCHAR* szID = L"", WCHAR* szPass = L"", int nEncMode = 0);
	BOOL SendScanRequest();
	BOOL SendScanRequestExt(); // 확장 scan 정보 전송
	void SetBindAddress(ULONG ulBindAddress = INADDR_ANY);
protected:
	void WideCopyStringFromAnsi(WCHAR* wszStrig, int nMaxBufferLen, char* aszString);

	char*  m_pReceive_buffer; // allocate 64 kbytes memory
	// 2010-08-26 hkeins : scanner routine
	SOCKET m_hSockReceive;
	CWnd*  m_pScanDlg; // scan wait dialog
	HANDLE m_hScanThread;
	DWORD  m_dwScanThreadID;
	int    m_nScanTimeCount; // 30,29,28....0 then timeout
	HWND   m_hReceiverWindow;
	int    m_nListItemCount;

	HWND m_hNotifyWnd;
	LONG m_lNotifyMsg;
	HWND m_hCloseMsgRecvWnd;
	LONG m_lCloseMsg;
	BOOL m_bUserCancel;
	ULONG m_ulBindAddress;
};
