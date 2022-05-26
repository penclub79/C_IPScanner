#pragma once

#include <afxinet.h>

#define		UM_STATUS_MESSAGE			WM_USER+100

typedef struct tagSETUP_OSDINFO
{
	BOOL		bUsed;
	CString		strOverlayText;
	int			iDisplayPosition;

}SETUP_OSDINFO, *LPSETUP_OSDINFO;

class COSDChangeThread
{
public:
	COSDChangeThread( HWND		pParent		,
					 int		nIndex		,
					 CString	strIPAddress,
					 int		nPort		,
					 CString	strUserName	,
					 CString	strPassword ,
					 SETUP_OSDINFO	setupOSDInfo);

	virtual ~COSDChangeThread(void);
	enum {	POSITION_TOPLEFT				= 0 ,
			POSITION_TOPRIGHT				};

	enum {	MESSAGE_ID_OSD_SET_START		= 0	,
			MESSAGE_ID_OSD_SET_END			,
			MESSAGE_ID_OSD_SET_ERROR		,
			MESSAGE_ID_OSD_SET_ERROR_AUTH	};

private:
	static DWORD WINAPI ProcOSDChange( LPVOID _lpParam );
	DWORD	OSDChange();
	int		CallHttpRequest(CString ipaddr, int port, CString query, CString strUserID, CString strPassword, CString &response);

	HWND		m_pParent		;
	int			m_nIndex		;
	CString		m_strIPAddress	;
	int			m_nPort			;
	CString		m_strUserName	;	
	CString		m_strPassword	;

	HANDLE		m_hThreadOSDChange;
	DWORD		m_dwThreadOSDChange;

	SETUP_OSDINFO	m_setupOSDInfo;
};
