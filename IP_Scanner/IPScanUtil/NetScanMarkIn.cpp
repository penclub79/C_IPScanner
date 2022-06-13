#include "StdAfx.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"
#include "NetDef_IPUtil.h"
#include <atlconv.h>

CNetScanMarkIn::CNetScanMarkIn()
{
	m_hScanThread = NULL;
	m_dwScanThreadID = 0;
	m_bUserCancel = FALSE;
	m_hNotifyWnd = NULL;
	m_lNotifyMsg = 0;
	m_hCloseMsgRecvWnd = NULL;
	m_pReceiverBuff = NULL;
}

CNetScanMarkIn::~CNetScanMarkIn(void)
{
	if (NULL != m_pReceiverBuff)
	{
		delete[] m_pReceiverBuff;
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
	DWORD dwExitCode;
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
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hScanThread, INFINITE))
		{
			TerminateThread(m_hScanThread, 0xffffffff);
		}

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
	// Local ----------------------------------------------------------
	sockaddr_in			ReceiverAddr;
	CString				strConver;
	CString				strVal;
	BOOL				bEnable				= TRUE;
	int					iSenderAddrLen		= 0;
	HEADER_BODY*		pReceive			= NULL;
	SCAN_INFO*			pScanInfo			= NULL;
	char				aszModelName[30]	= { 0 };
	int					iDeviceType			= 0;
	TCHAR*				pwszDeviceType		= NULL;
	char				aszIpAddress[32]	= { 0 };
	char				aszSubnet[32]		= { 0 };
	char				aszGateWay[32]		= { 0 };
	char				aszMacAdrs[32]		= { 0 };
	char				aszVersion[30]		= { 0 };
	// ----------------------------------------------------------------
	strVal.Format(_T("N/A"));

	// IPv4, UDP 
	m_hSockReceive = socket(AF_INET, SOCK_DGRAM, 0);

	// Socket BroadCast Setting
	if (setsockopt(m_hSockReceive, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)) == SOCKET_ERROR)
	{
		TRACE("2.setsocketopt error = %d\n", WSAGetLastError());
		if (m_hNotifyWnd)
			::SendMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT);
		
		goto EXIT_LOOP;
	}

	// ���� �ּ����� �ʱ�ȭ
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(MK_UDP_RSP_PORT);
	ReceiverAddr.sin_addr.s_addr = m_ulBindAddress;

	// ���� �ּ����� �Ҵ�
	if (bind(m_hSockReceive, (SOCKADDR*)&ReceiverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		if (m_hNotifyWnd)
			::SendMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_BIND);

		goto EXIT_LOOP;
	}

	// This Wait Server Response 
	SOCKADDR_IN SenderAddr;
	iSenderAddrLen = sizeof(SOCKADDR_IN);

	m_pReceiverBuff = new char[SCAN_INFO_m_pReceive_buffer_SIZE];					// �޸� �Ҵ�
	if (NULL == m_pReceiverBuff)													// �޸� �Ҵ� �Ǿ����� üũ
	{
		if (m_hNotifyWnd)
			::SendMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_MEMORY);
		
		goto EXIT_LOOP;
	}

	pReceive = (HEADER_BODY*)m_pReceiverBuff;										// �Ҵ� �޸� ũ��� ����ü ���
	memset(m_pReceiverBuff, 0, sizeof(char)* SCAN_INFO_m_pReceive_buffer_SIZE);		// �ʱ�ȭ

	// Recev Data Thread live
	while (m_dwScanThreadID)
	{
		if (SOCKET_ERROR == recvfrom(m_hSockReceive, m_pReceiverBuff, sizeof(PACKET_HEADER) + sizeof(DEVICE_INFO), 0, (SOCKADDR*)&SenderAddr, &iSenderAddrLen))
		{
			DWORD dwLastError = WSAGetLastError();
			TRACE("recvfrom error = %d\n", dwLastError);
			if (m_hNotifyWnd && dwLastError != 10004)
				::SendMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_RECV);
			
			goto EXIT_LOOP;
		}

		if (m_pReceiverBuff)
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
						if ( 0 < strlen(pReceive->stDevInfo.aszModel_name) )
							ConversionModelName(pReceive->stDevInfo.aszModel_name, &aszModelName[0]);

						//// IP - info
						if ( 0 < &pReceive->stDevInfo.stNetwork_info.aszIp[0]) // �޸� �ּҷ� ip�� ��Ÿ��
							ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.aszIp, &aszIpAddress[0]);
						else
							wsprintf(pScanInfo->szAddr, strVal);
						
						//// Gateway
						if (0 < &pReceive->stDevInfo.stNetwork_info.aszGateway[0]) // �޸� �ּҷ� ip�� ��Ÿ��
							ConversionNetInfo(pReceive->stDevInfo.stNetwork_info.aszGateway, &aszGateWay[0]);
						else
							wsprintf(pScanInfo->szGateWay, strVal);

						//// MAC
						if ( 0 < strlen(pReceive->stDevInfo.stNetwork_info.szMac_address))
							ConversionMac(pReceive->stDevInfo.stNetwork_info.szMac_address, &aszMacAdrs[0]);
						else
							wsprintf(pScanInfo->szMAC, strVal);

						//// SW Version
						if (0 < &pReceive->stDevInfo.stSw_version.szMajor)
							ConversionVersion(&pReceive->stDevInfo.stSw_version, &aszVersion[0]);
						else
							wsprintf(pScanInfo->szSwVersion, strVal);

						WideCopyStringFromAnsi(pScanInfo->szAddr,		30, aszIpAddress);
						WideCopyStringFromAnsi(pScanInfo->szGateWay,	30, aszGateWay);
						WideCopyStringFromAnsi(pScanInfo->szMAC,		30, aszMacAdrs);
						WideCopyStringFromAnsi(pScanInfo->szSwVersion,	30, aszVersion);
						WideCopyStringFromAnsi(pScanInfo->szModelName,	30, aszModelName);

						pScanInfo->nHTTPPort = pReceive->stDevInfo.stNetwork_info.uiHttp_port;
						pScanInfo->iBasePort = pReceive->stDevInfo.stNetwork_info.uiBase_port;
						pScanInfo->iVideoCnt = pReceive->stDevInfo.szMax_channel;

						if (m_hNotifyWnd)
							::SendMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0);
					}
				}
			}
		}
	}

	// Thread Close
EXIT_LOOP:
	m_hSockReceive = NULL;
	closesocket(m_hSockReceive);

	if (m_pReceiverBuff)
	{
		delete[] m_pReceiverBuff;
		m_pReceiverBuff = NULL;
	}

	if (m_bUserCancel && m_hCloseMsgRecvWnd && ::IsWindow(m_hCloseMsgRecvWnd))
	{
		//SendMessage(m_hCloseMsgRecvWnd, m_lCloseMsg, 0, 0);
		m_bUserCancel = FALSE;
	}

}

// int �迭 -> TCHAR �迭 ����
void CNetScanMarkIn::ConversionNetInfo(unsigned char* _upszIp, char* _pszVal)
{
	// ���� �����÷� ������ ���ؼ� stprintf�� _s�� ���� ��ũ�� �Լ��� ���� ���� ����.
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

// MAC �ּ� �������� ��ȯ �Լ�
void CNetScanMarkIn::ConversionMac(char* _pszMac, char* _pszVal)
{
	char	aszStr[30] = { 0 };
	int		iMacLen = 0;
	int		iValIdx = 2;

	iMacLen = strlen(_pszMac);
	
	for (int i = 0; i < iMacLen/2; i++)
	{
		// 2���ھ� ����
		memcpy(&_pszVal[i * 3], &_pszMac[i * 2], 2);
		_pszVal[iValIdx] = ':';

		// x:x:x:x:x 4���� �ݷ�
		if (4 > i)
		{
			iValIdx += 3;
		}	
	}
}

// ��Ŷ Ŭ�󿡰� ������
BOOL CNetScanMarkIn::SendScanRequest()
{
	sockaddr_in		stSockaddr;
	SOCKET			stSockSend = NULL;;
	char			szSendBuff[12] = { 0 };
	int				aiByte[4] = { 0 };
	BOOL			bEnable = FALSE;
	PACKET_HEADER*	pSender = NULL;

	stSockaddr.sin_family = AF_INET;
	stSockaddr.sin_port = htons(MK_UDP_REQ_PORT);
	stSockaddr.sin_addr.s_addr = INADDR_BROADCAST;

	memset(szSendBuff, 0, sizeof(szSendBuff));
	pSender = (PACKET_HEADER*)szSendBuff;

	pSender->uiCommand = MARKIN_PACKET_REQ_DEVICEINFO;

	// [����], [���� ��], [���� ���� ũ��], [���� ����ε� WinSock������ �׳� 0], [���� �ּ�], [���� �ּ� ����]
	if (SOCKET_ERROR == sendto(m_hSockReceive, szSendBuff, sizeof(PACKET_HEADER), 0, (SOCKADDR*)&stSockaddr, sizeof(sockaddr_in)))
	{
		TRACE("sendto to error = %d\n", WSAGetLastError());
		return FALSE;
	}

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

// �򿣵�� ġȯ �Լ�
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