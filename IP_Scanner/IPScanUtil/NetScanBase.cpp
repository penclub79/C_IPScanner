#include "stdafx.h"
#include "NetScanBase.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"



NetScanBase::NetScanBase()
{
	m_hScanThread		= NULL;
	m_hNotifyWnd		= NULL;
	m_hCloseMsgRecvWnd	= NULL;
	m_dwScanThreadID	= NULL;
	m_hSockReceive		= NULL;
	m_hNotifyWnd		= NULL;
	m_pReceive_buffer	= NULL;
	m_pScanner			= NULL;
	m_bUserCancel		= FALSE;
	m_lNotifyMsg		= 0;
	m_ulBindAddress		= 0;
	m_iBindPort			= 0;
	m_lCloseMsg			= 0;
	
}


NetScanBase::~NetScanBase()
{
}


BOOL NetScanBase::StartScanF(LPTHREAD_START_ROUTINE _pThreadFunc)
{
	m_bUserCancel = FALSE;

	if (NULL != m_hScanThread)
	{
		return TRUE;
	}

	m_hScanThread = ::CreateThread(NULL, 0, _pThreadFunc, this, 0, &m_dwScanThreadID);

	if (NULL == m_hScanThread)
	{
		TRACE("Thread create failed\n");
		return FALSE;
	}

	return TRUE;
}

void NetScanBase::SocketBind()
{
	sockaddr_in		ReceiverAddr;
	//SOCKADDR_IN		SenderAddr;
	BOOL			bEnable			= TRUE;
	int				iDeviceType		= 0;
	int				nSenderAddrLen	= 0;

	m_hSockReceive = socket(AF_INET, SOCK_DGRAM, 0);

	// broadcast �����ϵ��� socket �ɼ� ����
	if (SOCKET_ERROR == setsockopt(m_hSockReceive, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)))
	{
		TRACE("2.setsocketopt error = %d\n", WSAGetLastError());
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT); // PostMessage to MainWindow
		}
	}

	// Receive�� �ּ� bind
	ReceiverAddr.sin_family			= AF_INET;
	ReceiverAddr.sin_port			= htons(m_iBindPort);
	ReceiverAddr.sin_addr.s_addr	= m_ulBindAddress; // htonl(m_ulBindAddress);

	m_pReceive_buffer = new char[SCAN_INFO_m_pReceive_buffer_SIZE]; // allocate 64 k bytes buffer
	memset(m_pReceive_buffer, 0, SCAN_INFO_m_pReceive_buffer_SIZE);

	if (SOCKET_ERROR == bind(m_hSockReceive, (SOCKADDR*)&ReceiverAddr, sizeof(SOCKADDR)))
	{
		TRACE("Bind error = %d\n", WSAGetLastError());
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_BIND); // PostMessage to MainWindow
		}
	}

	// ������ ������ ��ٸ���
	nSenderAddrLen = sizeof(SOCKADDR_IN);

	if (NULL == m_pReceive_buffer)
	{
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_MEMORY); // PostMessage to MainWindow
		}
	}

}

BOOL NetScanBase::SendScanRequest()
{
	sockaddr_in		stSockaddr;
	SOCKET			stSockSend		= NULL;
	char			szSendBuff		= NULL;
	int				iPackSize		= 0;
	BOOL			bIsSuccess		= FALSE;

	this->m_pScanner = new CNetScanMarkIn;

	stSockaddr.sin_family		= AF_INET;
	stSockaddr.sin_addr.s_addr	= INADDR_BROADCAST;

	// [����], [���� ��], [���� ���� ũ��], [���� ����ε� WinSock������ �׳� 0], [���� �ּ�], [���� �ּ� ����]
	if (SOCKET_ERROR == sendto(m_hSockReceive, &szSendBuff, sizeof(szSendBuff), 0, (SOCKADDR*)&stSockaddr, sizeof(sockaddr_in)))
	{
		TRACE("sendto to error = %d\n", WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

// Bind Address Set
void NetScanBase::SetBindAddress(ULONG _ulBindAddress)
{
	m_ulBindAddress = _ulBindAddress;
}

// Window Message Set
void NetScanBase::SetNotifyWindow(HWND _hWnd, LONG _msg)
{
	m_hNotifyWnd = _hWnd;
	m_lNotifyMsg = _msg;
}

// Close Message Set
void NetScanBase::SetCloseMsgRecvWindow(HWND _hWnd, LONG _msg/* = WM_CLOSE*/)
{
	m_hCloseMsgRecvWnd = _hWnd;
	m_lCloseMsg = _msg;
}

// char -> WCHAR ���� ����
void NetScanBase::WideCopyStringFromAnsi(WCHAR* _pwszString, int _iMaxBufferLen, char* _pszString)
{
	USES_CONVERSION;
	wcscpy_s(_pwszString, _iMaxBufferLen, A2W(_pszString));
}

// Dlg���� ����ϴ� Stop�Լ�
BOOL NetScanBase::StopScan()
{
	m_bUserCancel = TRUE;

	if (m_hSockReceive)
	{
		closesocket(m_hSockReceive);
		m_hSockReceive = NULL;
	}

	if (m_hScanThread)
	{
		m_dwScanThreadID = 0;
		TRACE("Vision WaitForSingleObject\n");

		if (WAIT_TIMEOUT == WaitForSingleObject(m_hScanThread, INFINITE))
		{
			TerminateThread(m_hScanThread, 0xffffffff);
		}

		CloseHandle(m_hScanThread);
		m_hScanThread = NULL;
	}


	return TRUE;
}

// child�� ���̴� close�Լ�
void NetScanBase::ThreadExit()
{
	closesocket(m_hSockReceive);
	m_hSockReceive = NULL;

	if (m_pReceive_buffer)
	{
		delete[] m_pReceive_buffer;
		m_pReceive_buffer = NULL;
	}

	if (m_bUserCancel && m_hCloseMsgRecvWnd && ::IsWindow(m_hCloseMsgRecvWnd))
	{
		::PostMessage(m_hCloseMsgRecvWnd, m_lCloseMsg, 0, 0);
		TRACE("Vision Thread Exit\n");
		m_bUserCancel = FALSE;
	}
}