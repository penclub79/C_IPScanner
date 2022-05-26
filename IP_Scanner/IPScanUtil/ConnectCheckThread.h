//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: ip camera connection check class
// 2013-01-30 hkeins : connection check thread test
// 
//
#pragma once

#define MAX_PORT_COUNTS 10

class CConnectCheckThread
{
public:
	CConnectCheckThread(void);
	~CConnectCheckThread(void);

	void Start(HWND hNotifyWindow, UINT uMsg, int nIndex, WCHAR *szTargetIP, int* nPorts, int nPortCounts);
	void Stop();

	void CheckThread(); // do no call directly this is thread calle
protected:
	inline void _Lock();
	inline void _Unlock();
	inline void _Stop();
	inline void _PostMessage(int nErrorCode, int nPort); // 0 success, 1 failed

	CRITICAL_SECTION m_crit;
	HWND   m_hNotifyHwnd;
	UINT   m_uNotifyMsg;
	int    m_nIndex;
	char   m_szAddr[1024];
	int    m_nPorts[MAX_PORT_COUNTS]; // 최대 10개까지?
	int    m_nPortsCounts;

	HANDLE m_hThread;
	HANDLE m_hThreadWait;
	DWORD  m_dwThread;
	SOCKET m_socket[MAX_PORT_COUNTS];
	BOOL   m_bCancelled;
	BOOL   m_bShutdowned;
};
