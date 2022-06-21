#include "stdafx.h"
#include "NetScanBase.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"



NetScanBase::NetScanBase()
{
	m_hVisScanThread	= NULL;
	m_bUserCancel		= FALSE;
	m_hNotifyWnd		= NULL;
	m_lCloseMsg			= 0;
	m_hVisScanThread	= NULL;
	m_hMkScanThread		= NULL;
	m_hCloseMsgRecvWnd	= NULL;
	m_dwScanThreadID	= NULL;
	m_hSockReceive		= NULL;
	m_hNotifyWnd		= NULL;
	m_lNotifyMsg		= 0;
	m_ulBindAddress		= 0;
	m_pReceive_buffer	= NULL;
	m_pScanner = NULL;

}


NetScanBase::~NetScanBase()
{
}

DWORD ScanThread(int i_iScanType, LPVOID i_pParam)
{
	NetScanBase* pThis = (NetScanBase*)i_pParam;
	if (NULL == pThis)
		return 0;

	switch (i_iScanType)
	{
		case VISION:
			pThis->SocketBind();
			break;
		case MARKIN:
			//pThis->MkReceiver();
			break;
		case ONVIF:
			break;
		default:
			break;
	}

	return 0;
}

BOOL NetScanBase::StartScan(int i_iScanType, DWORD i_pdwThread)
{
	if (m_hVisScanThread != NULL)
		return TRUE;
	
	m_bUserCancel = FALSE;
	m_hVisScanThread = ::CreateThread(NULL, 0, *(LPTHREAD_START_ROUTINE)(ScanThread)(i_iScanType, &i_pdwThread), this, 0, &m_dwScanThreadID);
	
	if (m_hVisScanThread == NULL)
	{
		TRACE("Thread create failed\n");
		return FALSE;
	}

	return TRUE;
}

void NetScanBase::SocketBind()
{
	sockaddr_in		ReceiverAddr;
	BOOL			bEnable			= TRUE;
	int				iDeviceType		= 0;
	int				nSenderAddrLen	= 0;
	SOCKADDR_IN		SenderAddr;

	m_hSockReceive = socket(AF_INET, SOCK_DGRAM, 0);

	// broadcast 가능하도록 socket 옵션 조정
	if (SOCKET_ERROR == setsockopt(m_hSockReceive, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)))
	{
		TRACE("2.setsocketopt error = %d\n", WSAGetLastError());
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT); // PostMessage to MainWindow
		}
		ThreadExit();
	}

	// Receive할 주소 bind
	ReceiverAddr.sin_family			= AF_INET;
	ReceiverAddr.sin_port			= htons(VH_UDP_SCAN_PORT);
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
		ThreadExit();
	}

	// 서버의 응답을 기다린다
	nSenderAddrLen = sizeof(SOCKADDR_IN);

	if (NULL == m_pReceive_buffer)
	{
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_MEMORY); // PostMessage to MainWindow
		}
		ThreadExit();
	}

}

BOOL NetScanBase::SendScanRequest(int iScanType)
{
	sockaddr_in		stSockaddr;
	SOCKET			stSockSend		= NULL;
	char			szSendBuff		= NULL;
	int				iPackSize		= 0;
	BOOL			bIsSuccess		= FALSE;
	

	stSockaddr.sin_family		= AF_INET;
	stSockaddr.sin_addr.s_addr	= INADDR_BROADCAST;
	
	switch(iScanType)
	{
		case VISION:
			stSockaddr.sin_port = htons(VH_UDP_SCAN_PORT);
			//m_pScanner = new CNetScanMarkIn();
			break;

		case MARKIN:
			stSockaddr.sin_port = htons(MK_UDP_REQ_PORT);
			//bIsSuccess = ((CNetScanMarkIn*)m_pScanner)->SendPacketSet(&szSendBuff);
			
			break;

		default:
			return FALSE;
	}

	// [소켓], [보낼 값], [보낼 값의 크기], [전송 모드인데 WinSock에서는 그냥 0], [보낼 주소], [보낼 주소 길이]
	if (SOCKET_ERROR == sendto(m_hSockReceive, &szSendBuff, sizeof(szSendBuff), 0, (SOCKADDR*)&stSockaddr, sizeof(sockaddr_in)))
	{
		TRACE("sendto to error = %d\n", WSAGetLastError());
		return FALSE;
	}

	
	return TRUE;
}

void NetScanBase::SetBindAddress(ULONG ulBindAddress)
{
	m_ulBindAddress = ulBindAddress;
}

void NetScanBase::SetNotifyWindow(HWND hWnd, LONG msg)
{
	m_hNotifyWnd = hWnd;
	m_lNotifyMsg = msg;
}

void NetScanBase::SetCloseMsgRecvWindow(HWND hWnd, LONG msg/* = WM_CLOSE*/)
{
	m_hCloseMsgRecvWnd = hWnd;
	m_lCloseMsg = msg;
}

void NetScanBase::WideCopyStringFromAnsi(WCHAR* pwszString, int iMaxBufferLen, char* pszString)
{
	USES_CONVERSION;
	wcscpy_s(pwszString, iMaxBufferLen, A2W(pszString));
}

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