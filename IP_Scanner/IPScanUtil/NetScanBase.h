#pragma once

typedef struct tagSCAN_EXT_INFO_STRUCT
{
	WCHAR aszCaption[32];
	WCHAR *lpszValue;
	int   nValueLen;
	tagSCAN_EXT_INFO_STRUCT()
	{
		memset(aszCaption, 0, sizeof(aszCaption));
		lpszValue = NULL;
		nValueLen = 0;
	}
	~tagSCAN_EXT_INFO_STRUCT()
	{
		if (lpszValue)
		{
			delete[] lpszValue;
			lpszValue = 0;
		}
	}
	tagSCAN_EXT_INFO_STRUCT& operator = (tagSCAN_EXT_INFO_STRUCT& src)
	{
		wcscpy_s(this->aszCaption, 32, src.aszCaption);
		if (this->lpszValue)
		{
			delete[] lpszValue;
			lpszValue = NULL;
		}
		this->nValueLen = src.nValueLen;
		ASSERT(src.nValueLen > 0);
		if (src.lpszValue && src.nValueLen > 0)
		{
			this->lpszValue = new WCHAR[this->nValueLen]; // NULL terminate include length
			if (this->lpszValue)
				wcscpy_s(this->lpszValue, this->nValueLen, src.lpszValue);
			else
				this->nValueLen = 0;
		}
		return *this;
	}
}SCAN_EXT_INFO, *LPSCAN_EXT_INFO;


typedef struct tagSCAN_STRUCT
{
	int		iScanType = 0; // 1 : Vision, 2 : MarkIn
	WCHAR	szAddr[30];
	WCHAR	szMAC[30];
	WCHAR	szGateWay[30];
	WCHAR	szSwVersion[30]; // +추가
	WCHAR	szModelName[30];

	//int		nStreamPort;
	int		iBasePort;
	int		nHTTPPort;
	int		version;
	char	cIsDHCP; // 0 - static, 1 - DHCP
	WCHAR	szSubnetMask[30];
	int		nExtraFieldCount;
	int		iVideoCnt; // +추가
	time_t	tReceiveTime;
	SCAN_EXT_INFO* pExtScanInfos;

	tagSCAN_STRUCT()
	{
		memset(this, 0, sizeof(tagSCAN_STRUCT));
		//version				= 1;
		nExtraFieldCount = 0;
		pExtScanInfos = NULL;
	}
	~tagSCAN_STRUCT()
	{
		SAFE_DELETEA(pExtScanInfos); // 2012-01-02 : hkeins memory delete error fix. DELETE vs DELETEA 동작 차이에 의해 메모리 해제시 오류가 발생
	}

	tagSCAN_STRUCT& operator = (tagSCAN_STRUCT& src)
	{
		wcscpy_s(this->szAddr, 30, src.szAddr);
		wcscpy_s(this->szMAC, 30, src.szMAC);
		wcscpy_s(this->szGateWay, 30, src.szGateWay);
		wcscpy_s(this->szSubnetMask, 30, src.szSubnetMask);
		wcscpy_s(this->szSwVersion, 30, src.szSwVersion);
		wcscpy_s(this->szModelName, 30, src.szModelName);

		//this->nStreamPort	= src.nStreamPort;
		this->nHTTPPort = src.nHTTPPort;
		this->version = src.version;
		this->cIsDHCP = src.cIsDHCP;
		this->tReceiveTime = src.tReceiveTime;
		this->iVideoCnt = src.iVideoCnt;

		SAFE_DELETEA(this->pExtScanInfos);
		this->nExtraFieldCount = src.nExtraFieldCount;
		if (src.nExtraFieldCount > 0 && src.pExtScanInfos)
		{
			this->pExtScanInfos = new SCAN_EXT_INFO[src.nExtraFieldCount];
			if (this->pExtScanInfos)
			{
				for (int i = 0; i < src.nExtraFieldCount; i++)
				{
					this->pExtScanInfos[i] = src.pExtScanInfos[i]; // copy values
				}
			}
			else
			{
				this->nExtraFieldCount = 0;
			}
		}

		else if (2 == iScanType)
		{
			this->iBasePort = src.iBasePort;
		}

		return *this;
	}


	BOOL operator == (tagSCAN_STRUCT& src)
	{
		if (0 != wcscmp(this->szMAC, src.szMAC))			return FALSE;
		if (0 != wcscmp(this->szGateWay, src.szGateWay))			return FALSE;
		if (0 != wcscmp(this->szSubnetMask, src.szSubnetMask))			return FALSE;
		if (0 != wcscmp(this->szSwVersion, src.szSwVersion))			return FALSE;
		if (0 != wcscmp(this->szModelName, src.szModelName))			return FALSE;

		//if( this->nStreamPort			!= src.nStreamPort		)			return FALSE;
		if (this->nHTTPPort != src.nHTTPPort)			return FALSE;
		if (this->version != src.version)			return FALSE;
		if (this->cIsDHCP != src.cIsDHCP)			return FALSE;
		if (this->nExtraFieldCount != src.nExtraFieldCount)			return FALSE;
		if (this->iBasePort != src.iBasePort)			return FALSE;
		if (this->iVideoCnt != src.iVideoCnt)			return FALSE;

		for (int i = 0; i < src.nExtraFieldCount; i++)
		{
			if (this->pExtScanInfos[i].nValueLen != src.pExtScanInfos[i].nValueLen)			return FALSE;
			if (this->pExtScanInfos[i].nValueLen)
			{
				if (0 != wcscmp(this->pExtScanInfos[i].aszCaption, src.pExtScanInfos[i].aszCaption))		return FALSE;
				if (0 != wcscmp(this->pExtScanInfos[i].lpszValue, src.pExtScanInfos[i].lpszValue))		return FALSE;
			}
		}

		return TRUE;
	}


	void	SetReceiveTime();
	int     _PrintValues();
	CString _ReadValue(WCHAR* aszCaption);
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


class NetScanBase
{
private:
	//////////////////////////////////////////////////////////// Variable
	HANDLE		m_hScanThread;		
	//////////////////////////////////////////////////////////// ---------/

protected:
	//////////////////////////////////////////////////////////// Variable
	NetScanBase*	m_pScanner;
	HWND			m_hNotifyWnd;
	LONG			m_lNotifyMsg;
	HWND			m_hCloseMsgRecvWnd; 
	LONG			m_lCloseMsg;
	SOCKET			m_hSockReceive;		
	char*			m_pReceive_buffer;
	int				m_iRevPort;
	BOOL			m_bUserCancel;
	ULONG			m_ulBindAddress;
	DWORD			m_dwScanThreadID;
	//////////////////////////////////////////////////////////// ---------/


	//////////////////////////////////////////////////////////// Function
	BOOL	StartScanF(LPTHREAD_START_ROUTINE _pThreadFunc);
	void	WideCopyStringFromAnsi(WCHAR* _pwszString, int _iMaxBufferLen, char* _pszString);
	BOOL	StopScans(SOCKET _hSocket);
	//////////////////////////////////////////////////////////// ---------/

public:
	NetScanBase();			
	virtual ~NetScanBase(); 

	//////////////////////////////////////////////////////////// Function
	virtual BOOL StartScan() = 0;
	virtual BOOL SendScanRequest() = 0;
	virtual BOOL StopScan() = 0;

	//void	SocketBind();
	void	SetBindAddress(ULONG _ulBindAddress);
	void	SetNotifyWindow(HWND _hWnd, LONG _msg);
	void	SetCloseMsgRecvWindow(HWND _hWnd, LONG _msg/* = WM_CLOSE*/);
	void	ThreadExit();
	//////////////////////////////////////////////////////////// ---------/

};
