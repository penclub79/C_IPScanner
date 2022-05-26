#pragma once

#define WM_IPCHANGE_MESSAGE			WM_USER	+ 300

typedef enum  _tagIPCHANGE_MESSAGE
{
	MESSAGE_CLOSE_THREAD			= 0,
	//MESSAGE_SUCESS_CONNECT			= 1,
	//MESSAGE_SUCESS_LOGIN			= 2,
	MESSAGE_SUCESS_MAC				= 3,

	MESSAGE_CHANGE_COMPLETE			= 4,

	MESSAGE_FAILED_CONNECT			= 10,
	MESSAGE_FAILED_LOGIN			= 11,
	MESSAGE_FAILED_MAC				= 12,
	MESSAGE_FAILED_TIMEOUT			= 13,
	MESSAGE_FAILED_DUPLICATIONIP	= 14,
	MESSAGE_FAILED_REQUEST_IPCHANGE	= 15,

	MESSAGE_REQUEST_IPCHANGE		= 20,
	MESSAGE_SCAN_START				= 22,
	MESSAGE_CHECK_IPCHANGE			= 23,


}IPCHANGE_MESSAGE;

typedef struct tagIPCHANGEITEM
{
	WCHAR	szMACAddress[32];
	WCHAR	szModel[32];

	// Source Device Info
	int		nFromIsDHCP;
	WCHAR	szFromIPAddress[32];
	WCHAR	szFromGWAddress[32];
	WCHAR	szFromSubnetMask[32];
	int		iFromHttpPort;
	int		iFromStreamPort;

	WCHAR	szUserID[32];
	WCHAR	szUserPW[32];

	// Destination Device Info
	int		nToIsDHCP;
	WCHAR	szToIPAddress[32];
	WCHAR	szToGWAddress[32];
	WCHAR	szToSubnetMask[32];
	int		iToHttpPort;
	int		iToStreamPort;

	int		iStatus;
	time_t	tTimeOut;

	tagIPCHANGEITEM()
	{
		memset( &szModel			, 0, sizeof(szModel ) );
		memset( &szMACAddress		, 0, sizeof(szMACAddress ) );
		memset( &szFromIPAddress	, 0, sizeof(szFromIPAddress ) );
		memset( &szFromGWAddress	, 0, sizeof(szFromGWAddress ) );
		memset( &szFromSubnetMask	, 0, sizeof(szFromSubnetMask ) );
		memset( &szUserID			, 0, sizeof(szUserID ) );
		memset( &szUserPW			, 0, sizeof(szUserPW ) );
		memset( &szToIPAddress		, 0, sizeof(szToIPAddress ) );
		memset( &szToGWAddress		, 0, sizeof(szToGWAddress ) );
		memset( &szToSubnetMask		, 0, sizeof(szToSubnetMask ) );
		

		iFromHttpPort	= 80;
		iFromStreamPort	= 2700;
		nFromIsDHCP		= FALSE;
		iToHttpPort		= 80;
		iToStreamPort	= 2700;
		nToIsDHCP		= FALSE;
		iStatus			= 0;
		tTimeOut		= 0;

	}
} IPCHANGEITEM, *PIPCHANGEITEM;


class CIPChangeThread
{
	typedef struct tagIPCHANGETHREADINFO 
	{
		int					iIndex;
		IPCHANGEITEM		IPChangeItem;
		void*				pThis;
		int					iVersion;
		HWND				hParent;

		tagIPCHANGETHREADINFO()
		{
			iIndex			= 0;
			hParent			= NULL;
			pThis			= NULL;
			iVersion		= 0;
		}

		~tagIPCHANGETHREADINFO() 
		{
		}

	} IPCHANGETHREADINFO, *PIPCHANGETHREAD;

public:
	CIPChangeThread();
	virtual ~CIPChangeThread(void);

	BOOL IsRunning();
	void SetThreadInfo(HWND hParent, int iVersion, int iIndex, IPCHANGEITEM IPChanageInfo);
	void StartIPChanage();
	void StopIPChanage();

	BOOL _ValidateMacAddress(CString strMacAddress);


private:
	int CallHttpRequest(CString ipaddr, int port, CString query, CString &response);
	BOOL ConnectionCheck(CString ipaddr, int port);

	static DWORD WINAPI ProcThreadIPChange	( LPVOID _lpParam );
	DWORD ProcIPChange	( LPVOID _lpParam );


	HANDLE				m_hThreadIPChange;
	DWORD				m_dwThreadIPChangeID;
	IPCHANGETHREADINFO	m_IPChanageThreadInfo;
};
