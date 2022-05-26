//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: TCP Firmware upgrade support
//
// History:
//    2013-05-07 made by cgkim and source copy by hkeins
//
#pragma once

#include "resource.h"
#include <afxinet.h>
#include "UpgradeDefine.h"
#include "MessageQueue.h"

#define MAX_WAIT_CHECK_COUNT		90	// 리붓이 될때 까지 대기 루프를 도는 횟수
#define MAX_REBOOT_CHECK_COUNT		120	// 대기가 끝나고 접속시도 횟수

class CUpgradeThreadTcp
{
public:
	CUpgradeThreadTcp( HWND pParent, int nIndex, CString sIPAddress, int nPort, CString sUserName, CString sPassword,CString sFilePath,CString sFileName);
	~CUpgradeThreadTcp(void);

	BOOL StartUpgrade();
	BOOL RetryUpgrade();

protected:
	BOOL ConnectForStart();
	BOOL ConnectForCheckComplete();
	BOOL ConnectForSWUpgradeWithoutMCU();
	BOOL StopUpgrade();


	int		m_nIndex;

	CString m_sIPAddress;
	int		m_nUpgradePort;

	CString m_sUserName;
	CString m_sPassword;
	CString m_sFilePath;
	CString m_sFileName;
	WCHAR	m_szDisplayMessage[256];
	int		m_iPercentage;

	HWND				m_hwndParent;
	CMessageQueue		m_MessageQueue;

	HANDLE	m_hThreadUpgrade;
	DWORD	m_dwThreadUpgradeID;
	HANDLE	m_hThreadUpgradeEvent;

	HANDLE	m_hThreadSocket;
	DWORD	m_dwThreadSocketID;
	SOCKET	m_pSockUpgrade;
	HANDLE  m_hDisocnnectLock;

	int		m_iProtocalVersion;

	int		m_iVersionCheckCount;
	int		m_iConnectCheckCount;
	int		m_iLastError;

	long					m_lSWPackageSize;
	SW_PACKAGE_FILE_INFO	m_SwPackageFileInfo;
	short					m_wMCUInfoCount;
	MCU_PACKAGE_FILE_INFO*	m_pMCUPackageFileInfo;
	int						m_iSelectMCUIndex;


private:
	BOOL			SendProtocolVersion();
	BOOL			SendAuth();
	BOOL			UpgradeStart(int iUpgradeMode);
	BOOL			SendUpgradeStart();
	BOOL			SendUpgradeReStart();
	BOOL			SendReqSWVersion();
	BOOL			SendUpgradeFile(int iReqIndex);
	BOOL			SendStartMCUUpgrade( int iMCU_Model	, int iMCU_Version );
	BOOL			SendMCUUpgradeFile(int iMCUIndex, int iReqIndex);
	BOOL			SendStopMcuUpgrade();

	INT				SendData       ( SOCKET _sock, char* _pbtSendPacket, int _iBytesToSend );

	void			Disconnect();
	BOOL			funcUpgrade();
	BOOL			funcReceive(int	iSocketMode);
	ULONG			GetIPAddress( LPCTSTR strHostName );

	static DWORD WINAPI ProcReceiveSocket	( LPVOID _lpParam );
	static DWORD WINAPI ProcUpgrade			( LPVOID _lpParam );
};
