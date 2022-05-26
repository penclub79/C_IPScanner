//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: ip camera connection check class
// 2013-01-30 hkeins : connection check thread test
// 
//
#include "StdAfx.h"
#include "ConnectCheckThread.h"
#include <atlconv.h>

static DWORD thrThreadStart(LPVOID lpParam)
{
	CConnectCheckThread* pThis = (CConnectCheckThread*)lpParam;

	pThis->CheckThread();

	return 0;
}

CConnectCheckThread::CConnectCheckThread(void)
{
	int i = 0;
	m_hThread = NULL;
	m_dwThread= 0;
	for(i = 0; i < MAX_PORT_COUNTS; i++)
	{
		m_socket[i] = NULL;
	}
	m_bCancelled = FALSE;
	m_hThreadWait = CreateEvent(NULL, FALSE, FALSE, NULL);
	memset(m_szAddr, 0, sizeof(m_szAddr));
	for(i = 0; i < MAX_PORT_COUNTS; i++)
	{
		m_nPorts[i] = 0;
	}
	m_hNotifyHwnd = NULL;
	m_uNotifyMsg  = 0;
	m_bShutdowned = FALSE;
	m_nIndex = -1;
	m_nPortsCounts = 0;
	InitializeCriticalSection(&m_crit);
}

CConnectCheckThread::~CConnectCheckThread(void)
{
	_Lock();
	m_bShutdowned = TRUE;
	_Unlock();
	Stop();
	if(m_hThreadWait)
	{
		CloseHandle(m_hThreadWait);
		m_hThreadWait = NULL;
	}
	DeleteCriticalSection(&m_crit);
}

void CConnectCheckThread::_Lock()
{
	EnterCriticalSection(&m_crit);
}

void CConnectCheckThread::_Unlock()
{
	LeaveCriticalSection(&m_crit);
}

void CConnectCheckThread::_Stop()
{
	if(m_hThread)
	{
		TRACE(L"CConnectCheckThread::_Stop() - start\n");
		m_bCancelled = TRUE;
		for(int i = 0; i < MAX_PORT_COUNTS; i++)
		{
			if(m_socket[i]) closesocket(m_socket[i]);
			m_socket[i] = NULL;
		}
		SetEvent(m_hThreadWait);
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_bCancelled = FALSE;
		m_hThread = NULL;
		m_dwThread = 0;
		m_nPortsCounts = 0;
		TRACE(L"CConnectCheckThread::_Stop() - end\n");
	}
}

void CConnectCheckThread::_PostMessage(int nErrorCode, int nPort) // 0 success, 1 failed
{
	if(m_hNotifyHwnd && m_uNotifyMsg)
	{
		::PostMessage(m_hNotifyHwnd, m_uNotifyMsg, nErrorCode, MAKELONG(m_nIndex, nPort));
	}
}

void CConnectCheckThread::Start(HWND hNotifyWindow, UINT uMsg, int nIndex, WCHAR *szTargetIP, int *nPorts, int nPortCounts)
{
	_Lock();
	if(m_bShutdowned)
	{
		_Unlock();
		return;
	}
	_Stop();
	TRACE(L"CConnectCheckThread::Start() - start\n");
	ASSERT(hNotifyWindow != NULL && uMsg != NULL && szTargetIP != NULL && nPorts != NULL && nPortCounts <= MAX_PORT_COUNTS);
	// stop and set information
	m_hNotifyHwnd = hNotifyWindow;
	m_uNotifyMsg  = uMsg; 
	m_nIndex      = nIndex;
	m_nPortsCounts = nPortCounts;
	USES_CONVERSION;
	char* pTemp = W2A(szTargetIP);
	strncpy_s(m_szAddr, pTemp, strlen(pTemp));
	for(int i = 0; i < nPortCounts; i++)
	{
		if(i < nPortCounts)
		{
			m_nPorts[i] = nPorts[i];
		}
		else
		{
			m_nPorts[i] = 0;
		}
	}

	m_hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrThreadStart, this, 0, &m_dwThread);
	TRACE(L"CConnectCheckThread::Start() - end\n");
	_Unlock();
}

void CConnectCheckThread::Stop()
{
	_Lock();
	_Stop();
	_Unlock();
}

void CConnectCheckThread::CheckThread() // do no call directly this is thread calle
{
	TRACE(L"CConnectCheckThread::CheckThread() - start\n");

	BOOL   bIsConnectSuccess = FALSE;
	struct sockaddr_in toaddr;
	int    i;

	for(i = 0; i < m_nPortsCounts; i++)
	{
		TRACE(L"- Try connect port[%d]\n", m_nPorts[i]);
		bIsConnectSuccess = FALSE;
		m_socket[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		memset(&toaddr,0x00, sizeof(toaddr));
		toaddr.sin_family = AF_INET;
		toaddr.sin_port = htons(m_nPorts[i]);
		toaddr.sin_addr.s_addr = inet_addr(m_szAddr);

		while(1)
		{
			if(connect(m_socket[i], (struct sockaddr *)&toaddr, sizeof(toaddr)) < 0)
			{
				int nLastError = WSAGetLastError();
				TRACE(L"Connect error code %d(%x)\n", nLastError, nLastError);
				break;
			}
			bIsConnectSuccess = TRUE;
			break;
		}

		// send message to window
		if(bIsConnectSuccess)
		{
			TRACE(L"- Connect OK        [%d]\n", m_nPorts[i]);
			_PostMessage(0, m_nPorts[i]);
		}
		else
		{
			TRACE(L"- Connect failed port[%d]\n", m_nPorts[i]);
			_PostMessage(1, m_nPorts[i]);
		}
		if(m_bCancelled)
			break;
	}
	if(!m_bCancelled)
	{
		for(i = 0; i < MAX_PORT_COUNTS; i++)
		{
			if(m_socket[i]) closesocket(m_socket[i]);
			m_socket[i] = NULL;
		}
	}
	WaitForSingleObject(m_hThreadWait, INFINITE);

	TRACE(L"CConnectCheckThread::CheckThread() - exit\n");
}
