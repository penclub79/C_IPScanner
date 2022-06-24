#include "StdAfx.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"
#include "NetDef_IPUtil.h"
#include <atlconv.h>

CNetScanMarkIn::CNetScanMarkIn()
{
	m_bUserCancel		= FALSE;
	//m_pszPacketBuff = NULL;
}

CNetScanMarkIn::~CNetScanMarkIn(void)
{
	//if (NULL != m_pszPacketBuff)
	//{
	//	delete[] m_pszPacketBuff;
	//	m_pszPacketBuff = NULL;
	//}
}

// static
DWORD CNetScanMarkIn::thrMarkInScanThread(LPVOID pParam)
{
	CNetScanMarkIn* pThis = (CNetScanMarkIn*)pParam;
	if (NULL == pThis)
		return 0;

	pThis->thrMarkInReceiver();

	return 0;
}

BOOL CNetScanMarkIn::StartScan()
{
	m_iRevPort = MK_UDP_RSP_PORT;
	this->StartScanF((LPTHREAD_START_ROUTINE)thrMarkInScanThread);
	return TRUE;
}

// Thread -> MarkIn Data Receiver
void CNetScanMarkIn::thrMarkInReceiver()
{
	// Local ----------------------------------------------------------
	//SOCKADDR_IN			ReceiverAddr;
	int					iSenderAddrLen		= 0;
	HEADER_BODY*		pReceive			= NULL;
	SCAN_INFO*			pScanInfo			= NULL;
	char				aszModelName[30]	= { 0 };
	char				aszIpAddress[32]	= { 0 };
	char				aszGateWay[32]		= { 0 };
	char				aszMacAdrs[32]		= { 0 };
	char				aszVersion[30]		= { 0 };
	DWORD				dwLastError			= 0;
	BOOL				bIsSuccessBind = FALSE;	
	SOCKADDR			stSockAddr;


	//--------------------------------------------------------- ----------------------------------------------------------------------------
	//SOCKADDR_IN			ReceiverAddr;
	//this->m_hReceiveSock = socket(AF_INET, SOCK_DGRAM, 0);

	//// broadcast 
	//if (SOCKET_ERROR == setsockopt(this->m_hReceiveSock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)))
	//{
	//	TRACE("2.setsocketopt error = %d\n", WSAGetLastError());
	//	if (this->m_hNotifyWnd)
	//		::PostMessage(this->m_hNotifyWnd, this->m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT); // PostMessage to MainWindow
	//	
	//	this->ThreadExit();
	//	return;
	//}

	//m_stSockAddr.sin_family = AF_INET;
	//m_stSockAddr.sin_port = htons(MK_UDP_RSP_PORT);
	//m_stSockAddr.sin_addr.s_addr = this->m_ulBindAddress; // htonl(m_ulBindAddress);

	//if (SOCKET_ERROR == bind(this->m_hReceiveSock, (SOCKADDR*)&m_stSockAddr, sizeof(SOCKADDR)))
	//{
	//	TRACE("Bind error = %d\n", WSAGetLastError());
	//	if (this->m_hNotifyWnd)
	//		::PostMessage(this->m_hNotifyWnd, this->m_lNotifyMsg, 0, SCAN_ERR_BIND); // PostMessage to MainWindow
	//	
	//	this->ThreadExit();
	//	return;
	//}
	//-------------------------------------------------------------------------------------------------------------------------------------

	bIsSuccessBind = SocketBind();

	if (bIsSuccessBind)
	{
		iSenderAddrLen = sizeof(SOCKADDR_IN);

		m_pReceive_buffer = new char[SCAN_INFO_m_pReceive_buffer_SIZE];
		pReceive = (HEADER_BODY*)m_pReceive_buffer; // 할당 메모리 크기로 구조체 사용
		memset(m_pReceive_buffer, 0, sizeof(char)* SCAN_INFO_m_pReceive_buffer_SIZE);		// 초기화

		// Recev Data Thread live
		while (this->m_dwScanThreadID)
		{
			if (SOCKET_ERROR == recvfrom(this->m_hReceiveSock, m_pReceive_buffer, sizeof(PACKET_HEADER)+sizeof(DEVICE_INFO), 0, (SOCKADDR*)&stSockAddr, &iSenderAddrLen))
			{
				dwLastError = WSAGetLastError();
				TRACE("MarkIn recvfrom error = %d\n", dwLastError);
				if (this->m_hNotifyWnd && dwLastError != 10004)
					::PostMessage(this->m_hNotifyWnd, this->m_lNotifyMsg, 0, SCAN_ERR_RECV);

				this->ThreadExit();
				break;
			}

			if (pReceive)
			{
				// Data Little Endian -> Big Endian all Change
				ToBigEndian(pReceive);

				// Data Respose Success
				if (MARKIN_PACKET_RSP_DEVICEINFO == pReceive->stPacket.uiCommand)
				{
					if (pReceive)
					{
						pScanInfo = new SCAN_INFO;
						if (pScanInfo)
						{
							memset(pScanInfo, 0, sizeof(SCAN_INFO));

							pScanInfo->iScanType = 2;
							//// Model Name
							if (0 < strlen(pReceive->stDevInfo.aszModel_name))
								ConversionModelName(pReceive->stDevInfo.aszModel_name, &aszModelName[0]);

							//// IP - info
							if (0 < pReceive->stDevInfo.stNetwork_info.aszIp[0])
								ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.aszIp, &aszIpAddress[0]);
							else
								wsprintf(pScanInfo->szAddr, _T("N/A"));

							//// Gateway
							if (0 < pReceive->stDevInfo.stNetwork_info.aszGateway[0])
								ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.aszGateway, &aszGateWay[0]);
							else
								wsprintf(pScanInfo->szGateWay, _T("N/A"));

							//// MAC
							if (0 < strlen(pReceive->stDevInfo.stNetwork_info.szMac_address))
								ConversionMac(pReceive->stDevInfo.stNetwork_info.szMac_address, &aszMacAdrs[0]);
							else
								wsprintf(pScanInfo->szMAC, _T("N/A"));

							//// SW Version
							if (0 < pReceive->stDevInfo.stSw_version.szMajor)
								ConversionVersion(&pReceive->stDevInfo.stSw_version, &aszVersion[0]);
							else
								wsprintf(pScanInfo->szSwVersion, _T("N/A"));

							this->WideCopyStringFromAnsi(pScanInfo->szAddr, 30, aszIpAddress);
							this->WideCopyStringFromAnsi(pScanInfo->szGateWay, 30, aszGateWay);
							this->WideCopyStringFromAnsi(pScanInfo->szMAC, 30, aszMacAdrs);
							this->WideCopyStringFromAnsi(pScanInfo->szSwVersion, 30, aszVersion);
							this->WideCopyStringFromAnsi(pScanInfo->szModelName, 30, aszModelName);

							pScanInfo->nHTTPPort = pReceive->stDevInfo.stNetwork_info.uiHttp_port;
							pScanInfo->iBasePort = pReceive->stDevInfo.stNetwork_info.uiBase_port;
							pScanInfo->iVideoCnt = pReceive->stDevInfo.szMax_channel;

							TRACE("<< MarkIn SendMessage\n");

							if (this->m_hNotifyWnd)
								::PostMessage(this->m_hNotifyWnd, this->m_lNotifyMsg, (WPARAM)pScanInfo, 0);

							TRACE("MarkIn SendMessage >> \n");
						}
					}
				}
			}
		}
	}
	else
	{
		TRACE("Bind Fail = %d\n", WSAGetLastError());
		return;
	}
	
	return;
}

// int 배열 -> TCHAR 배열 복사
void CNetScanMarkIn::ConversionNetInfo(unsigned char* _upszIp, char* _pszVal)
{
	// 버퍼 오버플로 방지를 위해서 stprintf에 _s가 붙은 매크로 함수를 쓰는 것을 권장.
	sprintf_s(_pszVal, 32, "%d.%d.%d.%d", _upszIp[0], _upszIp[1], _upszIp[2], _upszIp[3]);
}

void CNetScanMarkIn::ConversionModelName(char* _pszModel, char* _pszVal)
{
	sprintf_s(_pszVal, 30, "%s", _pszModel);
}

void CNetScanMarkIn::ConversionVersion(VER_INFO* _pszVer, char* _pszVal)
{
	sprintf_s(_pszVal, 30, "%d.%d.%d", _pszVer->szMajor, _pszVer->szMinor, _pszVer->szRevision);
}

// MAC 주소 포맷으로 변환 함수
void CNetScanMarkIn::ConversionMac(char* _pszMac, char* _pszVal)
{
	char	aszStr[30]	= { 0 };
	int		iMacLen		= 0;
	int		iValIdx		= 2;

	iMacLen = strlen(_pszMac);
	
	for (int i = 0; i < iMacLen / 2; i++)
	{
		// 2글자씩 복사
		memcpy(&_pszVal[i * 3], &_pszMac[i * 2], 2);
		_pszVal[iValIdx] = ':';

		// xx:xx:xx:xx:xx 4개의 콜론
		if (4 > i)
			iValIdx += 3;
	}
}

// Send Packet Set
BOOL CNetScanMarkIn::SendScanRequest()
{
	BOOL bResult			= FALSE;
	//PACKET_HEADER* pSender	= NULL;

	//pSender = (PACKET_HEADER*)this->m_apszSendBuff;
	//memset(this->m_apszSendBuff, 0, sizeof(char)* sizeof(this->m_apszSendBuff));
	//pSender->uiCommand = MARKIN_PACKET_REQ_DEVICEINFO;
	//
	//bResult = this->SendScanRequestF(MK_UDP_REQ_PORT);
	return bResult;
}


// 빅엔디언 치환 함수
void CNetScanMarkIn::ToBigEndian(HEADER_BODY* _pstReceiveData)
{
	HEADER_BODY*	pstHeaderBody = NULL;
	int				aiByte[4] = { 0 };
	int				iCommandVal = 0;

	pstHeaderBody = _pstReceiveData;
	
	aiByte[0] = ((pstHeaderBody->stPacket.uiCommand >> 24) & 0xff);
	aiByte[1] = ((pstHeaderBody->stPacket.uiCommand >> 16) & 0xff);
	aiByte[2] = ((pstHeaderBody->stPacket.uiCommand >> 8) & 0xff);
	aiByte[3] = ((pstHeaderBody->stPacket.uiCommand >> 0) & 0xff);

	pstHeaderBody->stPacket.uiCommand = ((unsigned int)aiByte[0]) |
		((unsigned int)aiByte[1] << 8) |
		((unsigned int)aiByte[2] << 16) |
		((unsigned int)aiByte[3] << 24);

	aiByte[0] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 24) & 0xff);
	aiByte[1] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 16) & 0xff);
	aiByte[2] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 8) & 0xff);
	aiByte[3] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port >> 0) & 0xff);

	pstHeaderBody->stDevInfo.stNetwork_info.uiHttp_port = ((unsigned int)aiByte[0]) |
		((unsigned int)aiByte[1] << 8) |
		((unsigned int)aiByte[2] << 16) |
		((unsigned int)aiByte[3] << 24);

	aiByte[0] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 24) & 0xff);
	aiByte[1] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 16) & 0xff);
	aiByte[2] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 8) & 0xff);
	aiByte[3] = ((pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port >> 0) & 0xff);

	pstHeaderBody->stDevInfo.stNetwork_info.uiBase_port = ((unsigned int)aiByte[0]) |
		((unsigned int)aiByte[1] << 8) |
		((unsigned int)aiByte[2] << 16) |
		((unsigned int)aiByte[3] << 24);
}