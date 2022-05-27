#include "StdAfx.h"
#include "NetCommon.h"
#include "NetScanMarkIn.h"
#include "NetDef_IPUtil.h"
#include <atlconv.h>

CNetScanMarkIn::CNetScanMarkIn()
{
	m_hScanThread		= NULL;
	m_dwScanThreadID	= 0;
	m_bUserCancel		= FALSE;
	m_hNotifyWnd		= NULL;
	m_lNotifyMsg		= 0;
	m_hCloseMsgRecvWnd	= NULL;
	m_pReceiverBuff		= NULL;
}

CNetScanMarkIn::~CNetScanMarkIn(void)
{
	if (NULL != m_pReceiverBuff)
	{
		delete m_pReceiverBuff;
		m_pReceiverBuff = NULL;
	}
}

// Thread
DWORD thrMarkInScanThread(LPVOID pParam)
{
	CNetScanMarkIn* pThis = (CNetScanMarkIn*)pParam;
	if (NULL == pThis)
		return 0;

	pThis->thrMarkInReceiver();

	return 0;
}

// Scanning Start
BOOL CNetScanMarkIn::StartScan()
{
	if (m_hScanThread != NULL)
		return TRUE;

	// Thread Create
	m_hScanThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrMarkInScanThread, this, 0, &m_dwScanThreadID);
	if (m_hScanThread == NULL)
	{
		TRACE("Thread create failed\n");
		return FALSE;
	}

	return TRUE;
}

// Scanning Stop
BOOL CNetScanMarkIn::StopScan()
{
	m_bUserCancel = TRUE;
	// end scanning

	if (m_hSockReceive)
	{
		closesocket(m_hSockReceive);
		m_hSockReceive = NULL;
	}

	if (m_hScanThread)
	{
		m_dwScanThreadID = 0;
		WaitForSingleObject(m_hScanThread, INFINITE);
		CloseHandle(m_hScanThread);
		m_hScanThread = NULL;
	}

	return TRUE;
}

// Thread -> MarkIn Data Receiver
void CNetScanMarkIn::thrMarkInReceiver()
{
	sockaddr_in		ReceiverAddr;
	BOOL			bEnable			= TRUE;
	int				iSenderAddrLen	= 0;
	HEADER_BODY*	pReceive = NULL;
	IPUTIL_INFO*	pInfo			= NULL;
	IPUTIL_INFO2*	pInfo2			= NULL;

	// IPv4, UDP 
	m_hSockReceive = socket(AF_INET, SOCK_DGRAM, 0);

	// Socket BroadCast Setting
	if (setsockopt(m_hSockReceive, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)) == SOCKET_ERROR)
	{
		TRACE("2.setsocketopt error = %d\n", WSAGetLastError());
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT);
			goto EXIT_LOOP;
		}
	}

	// ���� �ּ����� �ʱ�ȭ
	ReceiverAddr.sin_family			= AF_INET;
	ReceiverAddr.sin_port			= htons(9011);
	ReceiverAddr.sin_addr.s_addr	= m_ulBindAddress;

	// ���� �ּ����� �Ҵ�
	if (bind(m_hSockReceive, (SOCKADDR*)&ReceiverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		if (m_hNotifyWnd)
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_BIND);

		goto EXIT_LOOP;
	}

	// This Wait Server Response 
	SOCKADDR_IN SenderAddr;
	iSenderAddrLen = sizeof(SOCKADDR_IN);

	if (NULL == m_pReceiverBuff)
	{
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_MEMORY);
		}
	}
	
	m_pReceiverBuff = new char[sizeof(PACKET_HEADER) + sizeof(DEVICE_INFO)];
	memset(m_pReceiverBuff, 0, sizeof(char)* sizeof(PACKET_HEADER) + sizeof(DEVICE_INFO));

	pReceive = (HEADER_BODY*)m_pReceiverBuff;
	//pReceive->stPacket.uiCommand = 0x00001001;

	// Recev Data Thread live
	while (m_dwScanThreadID)
	{
		if (recvfrom(m_hSockReceive, m_pReceiverBuff, sizeof(PACKET_HEADER) + sizeof(DEVICE_INFO), 0, (SOCKADDR*)&SenderAddr, &iSenderAddrLen) == SOCKET_ERROR)
		{
			DWORD dwLastError = WSAGetLastError();
			TRACE("recvfrom error = %d\n", dwLastError);
			if (m_hNotifyWnd && dwLastError != 10004)
			{
				::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_RECV);
			}
			
			goto EXIT_LOOP;
		}


		if (m_pReceiverBuff)
		{
			TRACE("%s\n", m_pReceiverBuff);
		}

	}





	// Thread Close
EXIT_LOOP:
	m_hSockReceive = NULL;
	closesocket(m_hSockReceive);
	
	if (m_bUserCancel && m_hCloseMsgRecvWnd && ::IsWindow(m_hCloseMsgRecvWnd))
	{
		PostMessage(m_hCloseMsgRecvWnd, m_lCloseMsg, 0, 0);
		m_bUserCancel = FALSE;
	}

}

// ��Ŷ Ŭ�󿡰� ������
BOOL CNetScanMarkIn::SendScanRequest()
{
	//char szSendBuff[256] = { 0 };
	//sockaddr_in stSockaddr = { 0 };
	//BOOL bEnable = FALSE;
	//SOCKET stSock = { 0 };

	//stSockaddr.sin_family = AF_INET;
	//stSockaddr.sin_port = htons(9010);

	return TRUE;
}



void CNetScanMarkIn::SetBindAddress(ULONG _ulBindAddress)
{
	m_ulBindAddress = _ulBindAddress;
}
