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
	BOOL				bIsSuccessBind		= FALSE;	
	SOCKADDR			stSockAddr;


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
				pReceive->stPacket.uiCommand = htonl(pReceive->stPacket.uiCommand);
				pReceive->stDevInfo.stNetwork_info.uiHttp_port = htonl(pReceive->stDevInfo.stNetwork_info.uiHttp_port);
				pReceive->stDevInfo.stNetwork_info.uiBase_port = htonl(pReceive->stDevInfo.stNetwork_info.uiBase_port);

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
								sprintf_s(aszModelName, 30, "%s", pReceive->stDevInfo.aszModel_name);
								//ConversionModelName(pReceive->stDevInfo.aszModel_name, &aszModelName[0]);

							//// IP - info
							if (0 < pReceive->stDevInfo.stNetwork_info.aszIp[0])
							{
								sprintf_s(aszIpAddress, 32, "%d.%d.%d.%d",
									pReceive->stDevInfo.stNetwork_info.aszIp[0],
									pReceive->stDevInfo.stNetwork_info.aszIp[1],
									pReceive->stDevInfo.stNetwork_info.aszIp[2],
									pReceive->stDevInfo.stNetwork_info.aszIp[3]);
							}	
							else
								wsprintf(pScanInfo->szAddr, _T("N/A"));

							//// Gateway
							if (0 < pReceive->stDevInfo.stNetwork_info.aszGateway[0])
							{
								//ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.aszGateway, &aszGateWay[0]);
								sprintf_s(aszGateWay, 32, "%d.%d.%d.%d",
									pReceive->stDevInfo.stNetwork_info.aszGateway[0],
									pReceive->stDevInfo.stNetwork_info.aszGateway[1],
									pReceive->stDevInfo.stNetwork_info.aszGateway[2],
									pReceive->stDevInfo.stNetwork_info.aszGateway[3]);
							}
							else
								wsprintf(pScanInfo->szGateWay, _T("N/A"));

							//// MAC
							if (0 < strlen(pReceive->stDevInfo.stNetwork_info.szMac_address))
							{
								//ConversionMac(pReceive->stDevInfo.stNetwork_info.szMac_address, &aszMacAdrs[0]);

								int iMacLen = strlen(pReceive->stDevInfo.stNetwork_info.szMac_address);
								int iValIdx = 2;
								for (int i = 0; i < iMacLen / 2; i++)
								{
									// 2글자씩 복사
									memcpy(&aszMacAdrs[i * 3], &pReceive->stDevInfo.stNetwork_info.szMac_address[i * 2], 2);
									aszMacAdrs[iValIdx] = ':';

									// xx:xx:xx:xx:xx 4개의 콜론
									if (4 > i)
										iValIdx += 3;
								}
							}
							else
								wsprintf(pScanInfo->szMAC, _T("N/A"));

							//// SW Version
							if (0 < pReceive->stDevInfo.stSw_version.szMajor)
							{
								//ConversionVersion(&pReceive->stDevInfo.stSw_version, &aszVersion[0]);
								sprintf_s(aszVersion, 30, "%d.%d.%d", pReceive->stDevInfo.stSw_version.szMajor, pReceive->stDevInfo.stSw_version.szMinor, pReceive->stDevInfo.stSw_version.szRevision);
							}
							else
								wsprintf(pScanInfo->szSwVersion, _T("N/A"));

							this->WideCopyStringFromAnsi(pScanInfo->szAddr, 32, aszIpAddress);
							this->WideCopyStringFromAnsi(pScanInfo->szGateWay, 32, aszGateWay);
							this->WideCopyStringFromAnsi(pScanInfo->szMAC, 30, aszMacAdrs);
							this->WideCopyStringFromAnsi(pScanInfo->szSwVersion, 30, aszVersion);
							this->WideCopyStringFromAnsi(pScanInfo->szModelName, 30, aszModelName);
							pScanInfo->nHTTPPort = pReceive->stDevInfo.stNetwork_info.uiHttp_port;
							pScanInfo->iBasePort = pReceive->stDevInfo.stNetwork_info.uiBase_port;
							pScanInfo->iVideoCnt = pReceive->stDevInfo.szMax_channel;

							if (this->m_hNotifyWnd)
								::PostMessage(this->m_hNotifyWnd, this->m_lNotifyMsg, (WPARAM)pScanInfo, 0);

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

// Send Packet Set
BOOL CNetScanMarkIn::SendScanRequest()
{
	char send_buffer[255] = { 0 };
	sockaddr_in MarkInSendSock;
	BOOL bEnable = TRUE;
	SOCKET hSendSock = NULL;
	PACKET_HEADER* pSendHeader = NULL;

	TRACE(_T("Send Broadcast Ping Request\n"));
	hSendSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (SOCKET_ERROR == setsockopt(hSendSock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)))
	{
		TRACE(_T("SetSocket Error = %d\n", WSAGetLastError()));
		return FALSE;
	}

	MarkInSendSock.sin_family = AF_INET;
	MarkInSendSock.sin_port = htons(MK_UDP_REQ_PORT);
	MarkInSendSock.sin_addr.s_addr = INADDR_BROADCAST; // FIX ME : TEST

	pSendHeader = (PACKET_HEADER*)send_buffer;
	pSendHeader->uiCommand = MARKIN_PACKET_REQ_DEVICEINFO;

	if (SOCKET_ERROR == sendto(hSendSock, send_buffer, sizeof(PACKET_HEADER), 0, (SOCKADDR*)&MarkInSendSock, sizeof(sockaddr_in)))
	{
		TRACE("Vision sendto to error = %d\n", WSAGetLastError());
		return FALSE;
	}
	
	return TRUE;
}
