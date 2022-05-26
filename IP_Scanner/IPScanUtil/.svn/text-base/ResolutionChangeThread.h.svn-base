#pragma once

#include <afxinet.h>

#define		UM_STATUS_MESSAGE			WM_USER+100

typedef struct tagSETUP_STREAMINFO_ITEM
{
	WCHAR szCodec[30];
	WCHAR szResolution[30];
	WCHAR szRateControl[30];

	long  lBitrate;
	long  lFrameRate;
	long  lGOP;
	long  lQuality;

}SETUP_STREAMINFO_ITEM, *LPSETUP_STREAMINFO_ITEM;

typedef struct tagSETUP_STREAMINFO
{
	int						iStreamCount;
	SETUP_STREAMINFO_ITEM	aStreamItems[3];

}SETUP_STREAMINFO, *LPSETUP_STREAMINFO;

class CResolutionChangeThread
{
public:
	CResolutionChangeThread( HWND		pParent		,
							 int		nIndex		,
							 CString	strIPAddress,
							 int		nPort		,
							 CString	strUserName	,
							 CString	strPassword ,
							 SETUP_STREAMINFO SetupStreamInfo);

	virtual ~CResolutionChangeThread(void);

	enum {	MESSAGE_ID_LENSINIT_START			= 0	,
			MESSAGE_ID_LENSINIT_END				,
			MESSAGE_ID_LENSINIT_COMPLETE		,
			MESSAGE_ID_RESOLUTION_SET_START		,
			MESSAGE_ID_RESOLUTION_SET_END		,
			MESSAGE_ID_RESOLUTION_SET_ERROR		,
			MESSAGE_ID_RESOLUTION_SET_ERROR_AUTH,
	};

private:
	static DWORD WINAPI ProcResolutionChange( LPVOID _lpParam );
	DWORD	ResolutionChange();
	int		CallHttpRequest(CString ipaddr, int port, CString query, CString strUserID, CString strPassword, CString &response);

	HWND		m_pParent		;
	int			m_nIndex		;
	CString		m_strIPAddress	;
	int			m_nPort			;
	CString		m_strUserName	;	
	CString		m_strPassword	;

	HANDLE		m_hThreadResolutionChange;
	DWORD		m_dwThreadResolutionChange;

	SETUP_STREAMINFO	m_SetupStreamInfo;
};
