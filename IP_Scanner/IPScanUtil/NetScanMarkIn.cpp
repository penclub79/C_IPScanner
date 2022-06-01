#include "StdAfx.h"
#include "NetCommon.h"
#include "NetScanVision.h"
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

void CNetScanMarkIn::WideCopyStringFromAnsi(WCHAR* wszStrig, int nMaxBufferLen, char* aszString)
{
	USES_CONVERSION;
	wcscpy_s(wszStrig, nMaxBufferLen, A2W(aszString));
}

// Thread -> MarkIn Data Receiver
void CNetScanMarkIn::thrMarkInReceiver()
{
	sockaddr_in		ReceiverAddr;
	BOOL			bEnable				= TRUE;
	int				iSenderAddrLen		= 0;
	unsigned int	uiPacketSize		= 0;
	HEADER_BODY*	pReceive			= NULL;
	IPUTIL_INFO*	pInfo				= NULL;
	IPUTIL_INFO2*	pInfo2				= NULL;
	SCAN_INFO*		pScanInfo			= NULL;
	TCHAR			awszIp[30]			= { 0 };
	TCHAR			awszSubnet[30]		= { 0 };
	TCHAR			awszGateway[30]		= { 0 };
	TCHAR			awszMac[40]			= { 0 };
	TCHAR			awszModelName[128]	= { 0 };
	//TCHAR*			pwszDeviceType		= NULL;
	CString			strConver;

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

	// 서버 주소정보 초기화
	ReceiverAddr.sin_family			= AF_INET;
	ReceiverAddr.sin_port			= htons(MK_UDP_RSP_PORT);
	ReceiverAddr.sin_addr.s_addr	= m_ulBindAddress;

	// 서버 주소정보 할당
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
			//goto EXIT_LOOP;
		}
		uiPacketSize = sizeof(PACKET_HEADER) + sizeof(DEVICE_INFO);

		if (m_pReceiverBuff)
		{
			// Data Little Endian -> Big Endian all Change
			ToBigEndian(pReceive);

			// Data Respose Success
			if (MARKIN_PACKET_RSP_HEADER == pReceive->stPacket.uiCommand)
			{
				pInfo =	(IPUTIL_INFO*)(m_pReceiverBuff + sizeof(HEADER2));
				pInfo2 = (IPUTIL_INFO2*)(m_pReceiverBuff + sizeof(HEADER2));
				
				if (pReceive)
				{
					// Info를 TCHAR에 담는 함수

					//// Device_type
					//UChartoTChar(&pReceive->stDevInfo.uszDevice_type, pwszDeviceType);
					//if (pReceive->stDevInfo.uszDevice_type == szDeviceType[0]) // 2
					//{
					//	pReceive->stDevInfo.uszDevice_type = NULL;
					//	strcpy((char*)pReceive->stDevInfo.uszDevice_type, "Camera");
					//}
					//else if (pReceive->stDevInfo.uszDevice_type == szDeviceType[1])
					//{
					//	pReceive->stDevInfo.uszDevice_type = NULL;
					//	strcpy((char*)pReceive->stDevInfo.uszDevice_type, "NVR");
					//}

					//// IP
					if (pReceive->stDevInfo.stNetwork_info.uszIp)
						ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.uszIp, &awszIp[0]);
					
					//// Subnet
					if (pReceive->stDevInfo.stNetwork_info.uszSubnet)
						ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.uszSubnet, &awszSubnet[0]);

					//// Gateway
					if (pReceive->stDevInfo.stNetwork_info.uszGateway)
						ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.uszGateway, &awszGateway[0]);

					//// Model Name
					if (pReceive->stDevInfo.szModel_name)
						mbstowcs(awszModelName, pReceive->stDevInfo.szModel_name, sizeof(TCHAR) * strlen(pReceive->stDevInfo.szModel_name));
					
					//// MAC
					if (pReceive->stDevInfo.stNetwork_info.szMac_address)
						ConversionMac(pReceive->stDevInfo.stNetwork_info.szMac_address, awszMac);
					
					
					/*WideCopyStringFromAnsi(pScanInfo->szGateWay, 30, pInfo->szGatewayIP);
					WideCopyStringFromAnsi(pScanInfo->szMAC, 30, pInfo->szMACAddress);*/

					/*pScanInfo->nStreamPort	= pInfo->dwStreamPort;
					pScanInfo->nHTTPPort	= pInfo->dwHTTPPort;*/
					//pScanInfo->version		= VERSION_2; // IPUTIL version 1

					/*if (m_hNotifyWnd)
						::PostMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0);*/
				}
			}

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

//void CNetScanMarkIn::UChartoTChar(unsigned char* _puszStr, TCHAR* _pwszVal)
//{
//	char aszStr[5] = { 0 };
//
//	for (int i = 0; i < strlen(aszStr); i++)
//	{
//		aszStr[i] = *_puszStr;
//	}
//
//}

// int 배열 -> TCHAR 배열 복사
void CNetScanMarkIn::ConversionNetInfo(unsigned char* _upszIp, TCHAR* _pwszVal)
{
	int aiIp[4] = { 0 };
	int iIpLen = 0;

	// 주소값을 넣는다.
	for (iIpLen = 0; iIpLen < 4; iIpLen++)
	{
		aiIp[iIpLen] = _upszIp[iIpLen];
	}

	// xxx.xxx.xxx.xxx로 변환한다.
	_stprintf(_pwszVal, _T("%d.%d.%d.%d"), aiIp[0], aiIp[1], aiIp[2], aiIp[3]);
	
	// 버퍼 오버플로 방지를 위해서 stprintf에 _s가 붙은 매크로 함수를 쓰는 것을 권장.
	//_stprintf_s(_pwszVal, sizeof(TCHAR), _T("%d.%d.%d.%d"), aiIp[0], aiIp[1], aiIp[2], aiIp[3]);
}

// MAC 주소 포맷으로 변환 함수
void CNetScanMarkIn::ConversionMac(char* _pszMac, TCHAR* _pwszVal)
{
	char aszStr[30] = { 0 };
	//char temp;
	int iMacLen = 0;

	// char 배열에 2개 단어씩 슬라이스
	iMacLen = strlen(_pszMac);
	//for (int i = 0; i < iMacLen; i++)
	//{
	//	// 짝수번째 단어 뒤에 ':' 삽입
	//	if (0 != i)
	//	{
	//		if (0 == i % 2)
	//		{	
	//			
	//		}
	//	}
	//}
	for (int i = 0; i < iMacLen; i++)
	{
		_pwszVal[i] = _pszMac[i];
	}

	// 대문자로 변환

	// AB:CD:ED:GH 포맷으로 변환
	// TCHAR 에 배열 복사
	//_stprintf(_pwszVal, _T("%s"), _pszMac);

}

// 패킷 클라에게 보내기
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

void CNetScanMarkIn::SetNotifyWindow(HWND hWnd, LONG msg)
{
	m_hNotifyWnd = hWnd;
	m_lNotifyMsg = msg;
}

void CNetScanMarkIn::SetCloseMsgRecvWindow(HWND hWnd, LONG msg/* = WM_CLOSE*/)
{
	m_hCloseMsgRecvWnd = hWnd;
	m_lCloseMsg = msg;
}


void CNetScanMarkIn::SetBindAddress(ULONG _ulBindAddress)
{
	m_ulBindAddress = _ulBindAddress;
}

// 빅엔디언 치환 함수
void CNetScanMarkIn::ToBigEndian(HEADER_BODY* _pstReceiveData)
{
	HEADER_BODY*	pstHeaderBody	= NULL;
	int				iByte[4]		= { 0 };
	int				iCommandVal		= 0;
	
	pstHeaderBody = _pstReceiveData;
	
	iByte[0] = ((pstHeaderBody->stPacket.uiCommand >> 24) & 0xff);
	iByte[1] = ((pstHeaderBody->stPacket.uiCommand >> 16) & 0xff);
	iByte[2] = ((pstHeaderBody->stPacket.uiCommand >> 8) & 0xff);
	iByte[3] = ((pstHeaderBody->stPacket.uiCommand >> 0) & 0xff);

	pstHeaderBody->stPacket.uiCommand = ((unsigned int)iByte[0]) |
		((unsigned int)iByte[1] << 8) |
		((unsigned int)iByte[2] << 16) |
		((unsigned int)iByte[3] << 24);

	iByte[0] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 24) & 0xff);
	iByte[1] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 16) & 0xff);
	iByte[2] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 8) & 0xff);
	iByte[3] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 0) & 0xff);

	pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port = ((unsigned int)iByte[0]) |
		((unsigned int)iByte[1] << 8) |
		((unsigned int)iByte[2] << 16) |
		((unsigned int)iByte[3] << 24);

	iByte[0] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 24) & 0xff);
	iByte[1] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 16) & 0xff);
	iByte[2] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 8) & 0xff);
	iByte[3] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 0) & 0xff);

	pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port = ((unsigned int)iByte[0]) |
		((unsigned int)iByte[1] << 8) |
		((unsigned int)iByte[2] << 16) |
		((unsigned int)iByte[3] << 24);
}