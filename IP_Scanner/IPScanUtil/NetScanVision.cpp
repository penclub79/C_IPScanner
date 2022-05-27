#include "StdAfx.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetDef_IPUtil.h"
#include <atlconv.h>

void tagSCAN_STRUCT::SetReceiveTime()
{
	tReceiveTime	= GetCurrentTime();
}

int   tagSCAN_STRUCT::_PrintValues()
{
	int i;
	CString str;

	for(i = 0 ; i < nExtraFieldCount; i++)
	{
		str.Format(L"Caption : %s  Value : %s\n", pExtScanInfos[i].szCaption, pExtScanInfos[i].lpszValue);
		OutputDebugString(str);
	}

	return 0;
}
CString tagSCAN_STRUCT::_ReadValue(WCHAR* szCaption)
{
	CString str;
	int i;
	if(szCaption == NULL)
		return str;

	for(i = 0 ; i < nExtraFieldCount; i++)
	{
		if(wcscmp(pExtScanInfos[i].szCaption, szCaption) == 0)
		{
			str = pExtScanInfos[i].lpszValue;
			break;
		}
	}
	return str;
}

CString tagSCAN_STRUCT::_ReadValues() // 모든 값을 읽여서 Caption : Value 값으로 리턴해준다
{
	CString str;
	int i;

	str.AppendFormat(L"%s\n", szAddr);
	for(i = 0 ; i < nExtraFieldCount; i++)
	{
		str.AppendFormat(L"%s : %s\n", pExtScanInfos[i].szCaption, pExtScanInfos[i].lpszValue);
	}
	return str;
}

// 2013-01-31 hkeins : 현재 display 항목
// ----------------------------------------------------------------------------------------------------
//   0           1             2             3            4             5              6                              
// | IP        | MAC Address | Stream Port | Http Port  | System Name | Model Type   | Firmware Version | 
// | 7           8             9            10           11            12
// | Resolution| Video format| AIN cnt     | AOUT cnt   | Audio in    | Audio out
// ----------------------------------------------------------------------------------------------------
// FIX ME : Display column 항목을 추가하거나 변경 시 하기 함수를 같이 변경해야한다.
// static
int  tagSCAN_STRUCT::_CompareScanInfo(int nItemColumn, tagSCAN_STRUCT* pInfo1, tagSCAN_STRUCT* pInfo2)
{
	ASSERT(0); // 쓰지마세요. main dialog에서 필드를 변경 할 수 있으므로 여기서 비교하지 않도록 적용합니다
	int nResult = 0;
	switch(nItemColumn)
	{
	case 0: // IP
		nResult = _CompareIP(pInfo1->szAddr, pInfo2->szAddr);
		break;
	case 1: // MAC
		nResult = wcscmp(pInfo1->szMAC, pInfo2->szMAC);
		break;
	case 2: // Stream Port
		nResult = pInfo1->nStreamPort - pInfo2->nStreamPort;
		break;
	case 3: // Http Port
		nResult = pInfo1->nHTTPPort - pInfo2->nHTTPPort;
		break;
	case 4: // Server Name
		nResult = pInfo1->_ReadValue(L"System Name").Compare(pInfo2->_ReadValue(L"System Name"));
		break;
	case 5: // Model
		nResult = pInfo1->_ReadValue(L"Model Type").Compare(pInfo2->_ReadValue(L"Model Type"));
		break;
	case 6: // Firmware Version
		nResult = pInfo1->_ReadValue(L"Firmware Version").Compare(pInfo2->_ReadValue(L"Firmware Version"));
		break;
	case 7: // Resolution
		nResult = pInfo1->_ReadValue(L"Support Resolution").Compare(pInfo2->_ReadValue(L"Support Resolution"));
		break;
	case 8: // Video format
		nResult = pInfo1->_ReadValue(L"Video Format").Compare(pInfo2->_ReadValue(L"Video Format"));
		break;
	case 9: // Alarm in count
		nResult = pInfo1->_ReadValue(L"Alarm In Count").Compare(pInfo2->_ReadValue(L"Alarm In Count"));
		break;
	case 10:// Alarm out count
		nResult = pInfo1->_ReadValue(L"Alarm Out Count").Compare(pInfo2->_ReadValue(L"Alarm Out Count"));
		break;
	case 11: // Audio in count
		nResult = pInfo1->_ReadValue(L"Audio In Count").Compare(pInfo2->_ReadValue(L"Audio In Count"));
		break;
	case 12: // Audio out count
		nResult = pInfo1->_ReadValue(L"Audio Out Count").Compare(pInfo2->_ReadValue(L"Audio Out Count"));
		break;
	default:
		ASSERT(0); // column added?
		nResult = 0;
	}
	return nResult;
}

DWORD thrScanThread(LPVOID pParam)
{
	CNetScanVision* pThis = (CNetScanVision*)pParam;
	if(pThis == NULL)
		return 0;

	pThis->thrReceiver();

	return 0;
}

CNetScanVision::CNetScanVision()
{
	m_hNotifyWnd		= NULL;
	m_lNotifyMsg		= 0;
	m_hCloseMsgRecvWnd	= NULL;
	m_lCloseMsg			= WM_CLOSE;
	m_hScanThread		= NULL;
	m_dwScanThreadID	= 0;
	m_hSockReceive		= 0;
	receive_buffer		= NULL;
	//m_hSockSend = 0;
	m_ulBindAddress     = 0;
}

CNetScanVision::~CNetScanVision(void)
{

}

BOOL CNetScanVision::StartScan()
{
	if(m_hScanThread != NULL)
		return TRUE;

	m_bUserCancel		= FALSE;
	m_hScanThread		= ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrScanThread, this, 0, &m_dwScanThreadID);
	if(m_hScanThread == NULL)
	{
		TRACE("Thread create failed\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CNetScanVision::StopScan()
{
	m_bUserCancel = TRUE;
	// end scanning

	if(m_hSockReceive)
	{
		closesocket(m_hSockReceive);
		m_hSockReceive = NULL;
	}

	if(m_hScanThread)
	{
		m_dwScanThreadID	= 0;
		WaitForSingleObject(m_hScanThread, INFINITE);
		CloseHandle(m_hScanThread);
		m_hScanThread = NULL;
	}

	return TRUE;
}

void CNetScanVision::SetNotifyWindow(HWND hWnd, LONG msg)
{
	m_hNotifyWnd = hWnd;
	m_lNotifyMsg = msg;
}

void CNetScanVision::SetCloseMsgRecvWindow(HWND hWnd, LONG msg/* = WM_CLOSE*/)
{
	m_hCloseMsgRecvWnd = hWnd;
	m_lCloseMsg = msg;
}

// 2010-08-26 hkeins : Camera scanning routine
// scanner logic
void CNetScanVision::thrReceiver()
{
	// initialize routine here
	// wait receive break from user or timer(timer will occur 1 minute after)
	//SOCKET m_hSockSend = NULL;

	//char send_buffer[255];
	//sockaddr_in TargetAddr;

//	//{{ TEST : server info add test
//#ifdef _DEBUG
//	SCAN_INFO* pScanInfo = NULL;
//
//	pScanInfo = new SCAN_INFO;
//	wcscpy_s(pScanInfo->szAddr, L"192.168.0.6");
//	wcscpy_s(pScanInfo->szGateWay, L"192.168.0.1");
//	wcscpy_s(pScanInfo->szMAC, L"FF:FF:FF:FA");
//	pScanInfo->nStreamPort	= 2700;
//	pScanInfo->nHTTPPort	= 80;
//	pScanInfo->version      = VERSION_1;
//	pScanInfo->nExtraFieldCount = 0;
//	pScanInfo->pExtScanInfos = NULL;
//	if(m_hNotifyWnd)
//		PostMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0);
//
//	Sleep(100);
//	pScanInfo = new SCAN_INFO;
//	wcscpy_s(pScanInfo->szAddr, L"118.46.219.170");
//	wcscpy_s(pScanInfo->szGateWay, L"118.46.219.254");
//	wcscpy_s(pScanInfo->szMAC, L"00:ee:de:23:02:01");
//	pScanInfo->nStreamPort	= 2700;
//	pScanInfo->nHTTPPort	= 80;
//	pScanInfo->version      = VERSION_2;
//	pScanInfo->cIsDHCP      = 0;
//	pScanInfo->nExtraFieldCount = 3;
//	wcscpy_s(pScanInfo->szSubnetMask, L"255.255.255.0");
//
//	pScanInfo->pExtScanInfos = new SCAN_EXT_INFO[3];
//	wcscpy_s(pScanInfo->pExtScanInfos[0].szCaption, 32, L"System Name");
//	pScanInfo->pExtScanInfos[0].lpszValue = new WCHAR[100];
//	wcscpy_s(pScanInfo->pExtScanInfos[0].lpszValue, 100, L"Vision Mi");
//	pScanInfo->pExtScanInfos[0].nValueLen = wcslen(pScanInfo->pExtScanInfos[0].lpszValue) + 1;
//
//	wcscpy_s(pScanInfo->pExtScanInfos[1].szCaption, 32, L"Model Type");
//	pScanInfo->pExtScanInfos[1].lpszValue = new WCHAR[100];
//	wcscpy_s(pScanInfo->pExtScanInfos[1].lpszValue, 100, L"Vision Mi");
//	pScanInfo->pExtScanInfos[1].nValueLen = wcslen(pScanInfo->pExtScanInfos[1].lpszValue) + 1;
//
//	wcscpy_s(pScanInfo->pExtScanInfos[2].szCaption, 32, L"Firmware Version");
//	pScanInfo->pExtScanInfos[2].lpszValue = new WCHAR[100];
//	wcscpy_s(pScanInfo->pExtScanInfos[2].lpszValue, 100, L"1.0.1.10");
//	pScanInfo->pExtScanInfos[2].nValueLen = wcslen(pScanInfo->pExtScanInfos[2].lpszValue) + 1;
//
//	if(m_hNotifyWnd)
//		PostMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0);
//
//	Sleep(100);
//
//	//pScanInfo = new SCAN_INFO;
//	//wcscpy_s(pScanInfo->szAddr, L"192.168.0.8");
//	//wcscpy_s(pScanInfo->szGateWay, L"192.168.0.1");
//	//wcscpy_s(pScanInfo->szMAC, L"FF:FF:FF:FC");
//	//pScanInfo->nStreamPort	= 2700;
//	//pScanInfo->nHTTPPort	= 80;
//	//pScanInfo->version      = VERSION_2;
//	//pScanInfo->cIsDHCP      = 1;
//	//wcscpy_s(pScanInfo->szSubnetMask, L"255.255.255.0");
//
//	//if(m_hNotifyWnd)
//	//	PostMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0);
//	return;
//#endif
	//}}
	////{{ error msg test code
	//if(m_hNotifyWnd)
	//	::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT); // PostMessage to MainWindow
	//goto EXIT_LOOP;
	////}}
	// 소켓 생성
	//m_hSockSend = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in ReceiverAddr;

	m_hSockReceive = socket(AF_INET, SOCK_DGRAM, 0);

	// broadcast 가능하도록 socket 옵션 조정
	BOOL bEnable = TRUE;
	if(setsockopt(m_hSockReceive, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)) == SOCKET_ERROR)
	{
		TRACE("2.setsocketopt error = %d\n", WSAGetLastError());
		if(m_hNotifyWnd)
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_SOCKET_OPT); // PostMessage to MainWindow
		goto EXIT_LOOP;
	}

	// Receive할 주소 bind
	ReceiverAddr.sin_family			= AF_INET;
	ReceiverAddr.sin_port			= htons(VH_UDP_SCAN_PORT);
	ReceiverAddr.sin_addr.s_addr	= m_ulBindAddress; // htonl(m_ulBindAddress);

	if(bind(m_hSockReceive, (SOCKADDR*)&ReceiverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		TRACE("Bind error = %d\n", WSAGetLastError());
		if(m_hNotifyWnd)
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_BIND); // PostMessage to MainWindow

		goto EXIT_LOOP;
	}

	// broadcast로 뿌려준다
	//sockaddr_in 
	//TargetAddr.sin_family = AF_INET;
	//TargetAddr.sin_port   = htons(VH_UDP_SCAN_PORT);
	//TargetAddr.sin_addr.s_addr = INADDR_BROADCAST; // FIX ME : TEST

	//memset(send_buffer, 0, sizeof(send_buffer));

	//HEADER2* pSendHeader = (HEADER2*)send_buffer;
	//pSendHeader->magic = MAGIC2_CODE;
	//pSendHeader->protocol_type = PROTOCOL_TYPE_IPUTILITY;
	//pSendHeader->protocol_mode = PROTOCOL_MODE_REQ_GET_IPINFO;
	//pSendHeader->body_size = 0;
	//if(sendto(hSockSend, send_buffer, sizeof(HEADER2), 0, (SOCKADDR*)&TargetAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	//{
	//	TRACE("sendto to error = %d\n", WSAGetLastError());
	//	closesocket(m_hSockReceive);
	//	closesocket(hSockSend);
	//	m_hSockReceive = NULL;
	//	return;
	//}
	//dialog측에서 10초마다 한번씩 재반복
	//SendScanRequest();

	// 서버의 응답을 기다린다
	SOCKADDR_IN SenderAddr;
	int nSenderAddrLen = sizeof(SOCKADDR_IN);

	receive_buffer = new char[SCAN_INFO_RECEIVE_BUFFER_SIZE]; // allocate 64 k bytes buffer
	if(receive_buffer == NULL)
	{
		if(m_hNotifyWnd)
			::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_MEMORY); // PostMessage to MainWindow
		goto EXIT_LOOP;
	}

	memset(receive_buffer, 0, SCAN_INFO_RECEIVE_BUFFER_SIZE);
	HEADER2*			pReceive		= (HEADER2*)receive_buffer;
	IPUTIL_INFO*		pInfo			= NULL;
	IPUTIL_INFO2*		pInfo2			= NULL;
	SCAN_INFO*			pScanInfo		= NULL;
	BYTE*				pExtField		= NULL;
	int					nToRead			= pReceive->body_size - sizeof(IPUTIL_INFO2);
	int					nItemCount		= 0;
	int					i;

	typedef struct tagCAPTION_HEADER
	{
		char szCaption[32];
		int  nDataLen;
	}CAPTION_HEADER, *LPCAPTION_HEADER;

	LPCAPTION_HEADER lpCapt;
	SCAN_EXT_INFO* pExtInfos = NULL;

	USES_CONVERSION;

	while( m_dwScanThreadID )
	{
		if(recvfrom(m_hSockReceive, receive_buffer, SCAN_INFO_RECEIVE_BUFFER_SIZE, 0, (SOCKADDR*)&SenderAddr,&nSenderAddrLen) == SOCKET_ERROR)
		{
			DWORD dwLastError = WSAGetLastError();
			TRACE("recvfrom error = %d\n", dwLastError);
			if(m_hNotifyWnd && dwLastError != 10004)
					::PostMessage(m_hNotifyWnd, m_lNotifyMsg, 0, SCAN_ERR_RECV); // PostMessage to MainWindow
			goto EXIT_LOOP;
		}

		if(pReceive->magic == MAGIC2_CODE)
		{
			// parsing and update list
			if(pReceive->protocol_mode == PROTOCOL_MODE_RSP_GET_IPINFO_EXT)
			{
				pInfo		= (IPUTIL_INFO *)(receive_buffer+sizeof(HEADER2));
				pInfo2		= (IPUTIL_INFO2*)(receive_buffer+sizeof(HEADER2));

				pScanInfo	= new SCAN_INFO;
				if ( pScanInfo ) 
				{
					//USES_CONVERSION;

					WideCopyStringFromAnsi(pScanInfo->szAddr, 30, pInfo->szIPAddress);
					WideCopyStringFromAnsi(pScanInfo->szGateWay, 30, pInfo->szGatewayIP);
					WideCopyStringFromAnsi(pScanInfo->szMAC, 30, pInfo->szMACAddress);

					pScanInfo->nStreamPort	= pInfo->dwStreamPort;
					pScanInfo->nHTTPPort	= pInfo->dwHTTPPort;
					pScanInfo->version      = VERSION_2; // IPUTIL version 1

					if(pReceive->body_size >= sizeof(IPUTIL_INFO2)) // read extend field
					{
						pScanInfo->cIsDHCP      = pInfo2->cIsDHCP;
						//WideCopyStringFromAnsi(pScanInfo->szGateWay, 30, pInfo2->szGatewayIP);
						WideCopyStringFromAnsi(pScanInfo->szSubnetMask, 30, pInfo2->szSubnetmask); // 2012-07-10 hkeins : scan utility subnet mask reading 오류 수정
					}

					pExtField = NULL;
					// read extended field
					if(pReceive->body_size > sizeof(IPUTIL_INFO2))
					{
						nToRead = pReceive->body_size - sizeof(IPUTIL_INFO2);
						nItemCount = 0;
						i = 0;
						nItemCount = 0;
						lpCapt     = NULL;
						pExtField = (BYTE*)(receive_buffer + sizeof(HEADER2) + sizeof(IPUTIL_INFO2) ); // set pointer
						nToRead = pReceive->body_size - sizeof(IPUTIL_INFO2);

						while(nToRead > 0)
						{
							lpCapt = (LPCAPTION_HEADER)pExtField;

							pExtField = pExtField + (sizeof(CAPTION_HEADER) + lpCapt->nDataLen);
							nToRead -= (sizeof(CAPTION_HEADER) + lpCapt->nDataLen);
							nItemCount++;
						}

						// read data into array
						pExtField = (BYTE*)(receive_buffer + sizeof(HEADER2) + sizeof(IPUTIL_INFO2) ); // reset pointer
						if(nItemCount > 0)
						{
							nToRead = pReceive->body_size - (sizeof(HEADER2) + sizeof(IPUTIL_INFO2));
							pExtInfos = new SCAN_EXT_INFO[nItemCount];
							if(pExtInfos)
							{
								memset(pExtInfos, 0, sizeof(SCAN_EXT_INFO) * nItemCount);
								i = 0;
								while(nToRead > 0)
								{
									lpCapt = (LPCAPTION_HEADER)pExtField;

									WideCopyStringFromAnsi(pExtInfos[i].szCaption, 32, lpCapt->szCaption);
									TRACE( pExtInfos[i].szCaption );
									TRACE( L" = " );


									pExtInfos[i].nValueLen = lpCapt->nDataLen+2; // 2012-08-07 hkeins : 데이터 길이를 복사하지 않는 버그 수정

									CHAR*	lpszTemp		= new CHAR[pExtInfos[i].nValueLen];

									memset( lpszTemp, 0, sizeof(CHAR)*(pExtInfos[i].nValueLen ) );
									memcpy( lpszTemp, (char*)(pExtField + sizeof(CAPTION_HEADER)), lpCapt->nDataLen );

									pExtInfos[i].lpszValue   = new WCHAR[pExtInfos[i].nValueLen];
									memset( pExtInfos[i].lpszValue, 0, sizeof(WCHAR)*(pExtInfos[i].nValueLen) );

									if(pExtInfos[i].lpszValue)
									{
										if( 0 == wcscmp( pExtInfos[i].szCaption, L"Upgrade Port" ) )
										{
											int it=0;
										}

										// FIX ME: A2W가 문제될 거 같은데?
										WideCopyStringFromAnsi( pExtInfos[i].lpszValue, pExtInfos[i].nValueLen, lpszTemp ); 

										TRACE( pExtInfos[i].lpszValue );
										TRACE( L"\n" );

									}

									if( lpszTemp )
									{
										delete [] lpszTemp;
										lpszTemp	= NULL;
									}

									pExtField = pExtField + (sizeof(CAPTION_HEADER) + lpCapt->nDataLen);
									nToRead -= (sizeof(CAPTION_HEADER) + lpCapt->nDataLen);
									i++;
								}

								pScanInfo->pExtScanInfos    = pExtInfos;
								pScanInfo->nExtraFieldCount = nItemCount;
							}
						}
					}

					if(m_hNotifyWnd)
						::PostMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0); // PostMessage to MainWindow
				}
			}
			else if(pReceive->protocol_mode == PROTOCOL_MODE_RSP_GET_IPINFO)
			{
				//TRACE("Response received\n");
				// 정보를 찍어준다
				pInfo   = (IPUTIL_INFO *)(receive_buffer+sizeof(HEADER2));
				pInfo2  = (IPUTIL_INFO2*)(receive_buffer+sizeof(HEADER2));

/*				TRACE("1IP  : %s\n",			pInfo->szIPAddress);
				TRACE("Gateway: %s\n",		pInfo->szGatewayIP);
				TRACE("MAC : %s\n",			pInfo->szMACAddress);
				TRACE("StreamPort: %d\n",	pInfo->dwStreamPort);
				TRACE("HttpPort: %d\n",		pInfo->dwHTTPPort);
*/
				pScanInfo = new SCAN_INFO;
				if ( pScanInfo )
				{
					//USES_CONVERSION;

					WideCopyStringFromAnsi(pScanInfo->szAddr,    30 ,pInfo->szIPAddress);
					WideCopyStringFromAnsi(pScanInfo->szGateWay, 30, pInfo->szGatewayIP);
					WideCopyStringFromAnsi(pScanInfo->szMAC,     30, pInfo->szMACAddress);

					pScanInfo->nStreamPort	= pInfo->dwStreamPort;
					pScanInfo->nHTTPPort	= pInfo->dwHTTPPort;
					pScanInfo->version      = VERSION_1; // IPUTIL version 1

					if(pReceive->body_size == sizeof(IPUTIL_INFO2)) // read extend field
					{
						pScanInfo->cIsDHCP      = pInfo2->cIsDHCP;
						WideCopyStringFromAnsi(pScanInfo->szGateWay, 30, pInfo2->szGatewayIP);
					}

					if(m_hNotifyWnd)
						::PostMessage(m_hNotifyWnd, m_lNotifyMsg, (WPARAM)pScanInfo, 0); // PostMessage to MainWindow
				}
			}
			else
			{
			}
		}
	}

EXIT_LOOP:
	closesocket(m_hSockReceive);
	//closesocket(m_hSockSend);
	m_hSockReceive = NULL;
	//m_hSockSend = NULL;

	if(receive_buffer)
	{
		delete[] receive_buffer;
		receive_buffer = NULL;
	}

	if(m_bUserCancel && m_hCloseMsgRecvWnd && ::IsWindow(m_hCloseMsgRecvWnd))
	{
		TRACE("PostMessage\n");
		PostMessage(m_hCloseMsgRecvWnd, m_lCloseMsg, 0, 0);
		m_bUserCancel = FALSE;
	}
}

BOOL CNetScanVision::RequestIPChange(WCHAR* strTargetServerMAC, WCHAR* strNewIP, WCHAR* strNewGateWay, int nStreamPort/*=2700*/, int nHTTPPort/*=80*/)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	char   send_buffer[255];

	// broadcast 가능하도록 socket 옵션 조정
	BOOL bEnable = TRUE;

	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable) == SOCKET_ERROR))
	{
		return FALSE;
	}

	SOCKADDR_IN TargetAddr;
	TargetAddr.sin_family		= AF_INET;
	TargetAddr.sin_port			= htons(VH_UDP_SCAN_PORT);
	TargetAddr.sin_addr.s_addr	= INADDR_BROADCAST; // broad casting

	memset(send_buffer, 0, sizeof(send_buffer));
	//typedef struct tagIPUTIL_INFO
	//{
	//	char	szIPAddress[16];
	//	char	szGatewayIP[16];
	//	char	szMACAddress[20];
	//	DWORD   dwStreamPort;
	//	DWORD	dwHTTPPort;
	//}IPUTIL_INFO, *LPIPUTIL_INFO;
	HEADER2    * pHeader = (HEADER2*)send_buffer;
	IPUTIL_INFO* pInfo = (IPUTIL_INFO*)(send_buffer + sizeof(HEADER2));
	// header set
	pHeader->magic = MAGIC2_CODE;
	pHeader->protocol_type = PROTOCOL_TYPE_IPUTILITY;
	pHeader->protocol_mode = PROTOCOL_MODE_REQ_SET_IPINFO;
	pHeader->body_size = sizeof(IPUTIL_INFO);
	// body set
	USES_CONVERSION;
	strcpy_s(pInfo->szIPAddress, W2A(strNewIP));
	strcpy_s(pInfo->szGatewayIP, W2A(strNewGateWay));
	strcpy_s(pInfo->szMACAddress, W2A(strTargetServerMAC));
	pInfo->dwHTTPPort	= nHTTPPort;
	pInfo->dwStreamPort	= nStreamPort;

	// send 
	if(sendto(sock, send_buffer, sizeof(HEADER2)+sizeof(IPUTIL_INFO), 0, (sockaddr*)&TargetAddr, sizeof(TargetAddr)) == SOCKET_ERROR)
	{
		closesocket(sock);
		return FALSE;
	}
	// clear temp datas
	Sleep(300); // 0.3 seconds wait
	closesocket(sock);
	// clear temp datas
	Sleep(300); // 0.3 seconds wait
	
	return TRUE;
}

BOOL CNetScanVision::RequestIPChange2(WCHAR* strTargetServerMAC, WCHAR* strNewIP, WCHAR* strNewGateWay, int nStreamPort/* = 2700*/, int nHTTPPort/* = 80*/, int cIsDHCP/* = 0*/, WCHAR* strNewsubnetMask/*=L"255.255.255.0"*/, WCHAR* szID /*= L""*/, WCHAR* szPass/* = L""*/, int nEncMode/* = 0*/)
{
	SOCKET sock;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	char   send_buffer[500];

	// broadcast 가능하도록 socket 옵션 조정
	BOOL bEnable = TRUE;

	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable) == SOCKET_ERROR))
	{
		return FALSE;
	}
	SOCKADDR_IN TargetAddr;
	TargetAddr.sin_family = AF_INET;
	TargetAddr.sin_port   = htons(VH_UDP_SCAN_PORT);
	TargetAddr.sin_addr.s_addr = INADDR_BROADCAST; // broad casting

	memset(send_buffer, 0, sizeof(send_buffer));

	//typedef struct tagIPUTIL_INFO2
	//{
	//	char	szIPAddress[16];
	//	char	szGatewayIP[16];
	//	char	szMACAddress[20];
	//	DWORD   dwStreamPort;
	//	DWORD	dwHTTPPort;
	//  char	cIPMode;              // IPUTIL INFO version 2
	//  char	szSubnetmask[16];
	//}IPUTIL_INFO2, *LPIPUTIL_INFO2;

	HEADER2    * pHeader = (HEADER2*)send_buffer;
	IPUTIL_INFO2* pInfo = (IPUTIL_INFO2*)(send_buffer + sizeof(HEADER2));
	IPUTIL_AUTH*  pAuth = (IPUTIL_AUTH*)((BYTE*)pInfo + sizeof(IPUTIL_INFO2));
	// header set
	pHeader->magic = MAGIC2_CODE;
	pHeader->protocol_type = PROTOCOL_TYPE_IPUTILITY;
	pHeader->protocol_mode = PROTOCOL_MODE_REQ_SET_IPINFO;
	pHeader->body_size = sizeof(IPUTIL_INFO2);
	int nPacketSize = sizeof(HEADER2)+sizeof(IPUTIL_INFO2);
	// body set
	USES_CONVERSION;
	strcpy_s(pInfo->szIPAddress, W2A(strNewIP));
	strcpy_s(pInfo->szGatewayIP, W2A(strNewGateWay));
	strcpy_s(pInfo->szMACAddress, W2A(strTargetServerMAC));
	pInfo->dwHTTPPort	= nHTTPPort;
	pInfo->dwStreamPort	= nStreamPort;
	pInfo->cIsDHCP = (char)cIsDHCP;
	strcpy_s(pInfo->szSubnetmask, W2A(strNewsubnetMask));
	// ID가 설정된 경우에는 Login 정보를 패킷에 함께 보낸다
	if(szID != NULL && wcscmp(szID, L"") != 0)
	{
		nPacketSize += sizeof(IPUTIL_AUTH);
		pHeader->body_size += sizeof(IPUTIL_AUTH);
		strcpy_s(pAuth->ID, 32, W2A(szID));
		strcpy_s(pAuth->Password, 32, W2A(szPass));
		pAuth->EncMode = nEncMode;
		// FIX ME: encription 적용
	}

	// send
	if(sendto(sock, send_buffer, nPacketSize, 0, (sockaddr*)&TargetAddr, sizeof(TargetAddr)) == SOCKET_ERROR)
	{
		closesocket(sock);
		return FALSE;
	}
	Sleep(300); // 0.3 seconds wait
	closesocket(sock);
	// clear temp datas
	Sleep(300); // 0.3 seconds wait

	return TRUE;
}

BOOL CNetScanVision::SendScanRequest()
{
	char send_buffer[255];
	sockaddr_in TargetAddr;

	BOOL bEnable = TRUE;
	SOCKET hSockSend = NULL;

	////TRACE(_T("Send broadcast ping request\n"));
	//hSockSend = socket(AF_INET, SOCK_DGRAM, 0);
	//if(setsockopt(hSockSend, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)) == SOCKET_ERROR)
	//{
	//	TRACE("1.setsocketopt error = %d\n", WSAGetLastError());
	//	closesocket(hSockSend);
	//	return FALSE;
	//}

	TargetAddr.sin_family = AF_INET;
	TargetAddr.sin_port   = htons(VH_UDP_SCAN_PORT);
	TargetAddr.sin_addr.s_addr = INADDR_BROADCAST; // FIX ME : TEST

	memset(send_buffer, 0, sizeof(send_buffer));

	HEADER2* pSendHeader = (HEADER2*)send_buffer;
	pSendHeader->magic = MAGIC2_CODE;
	pSendHeader->protocol_type = PROTOCOL_TYPE_IPUTILITY;
	pSendHeader->protocol_mode = PROTOCOL_MODE_REQ_GET_IPINFO;
	pSendHeader->body_size = 0;

	if(sendto(m_hSockReceive, send_buffer, sizeof(HEADER2), 0, (SOCKADDR*)&TargetAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		TRACE("sendto to error = %d\n", WSAGetLastError());
		//closesocket(hSockSend);
		return FALSE;
	}
	//// clear temp datas
	//Sleep(300); // 0.3 seconds wait
	//closesocket(hSockSend);
	//// clear temp datas
	//Sleep(300); // 0.3 seconds wait
	return TRUE;
}

BOOL CNetScanVision::SendScanRequestExt()
{
	char send_buffer[255];
	sockaddr_in TargetAddr;

	BOOL bEnable = TRUE;
	SOCKET hSockSend = NULL;

//	TRACE(_T("Send broadcast extended ping request\n"));
	//hSockSend = socket(AF_INET, SOCK_DGRAM, 0);
	//if(setsockopt(hSockSend, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable)) == SOCKET_ERROR)
	//{
	//	TRACE("SendScanRequestExt setsocketopt error = %d\n", WSAGetLastError());
	//	closesocket(hSockSend);
	//	return FALSE;
	//}

	TargetAddr.sin_family = AF_INET;
	TargetAddr.sin_port   = htons(VH_UDP_SCAN_PORT);
	TargetAddr.sin_addr.s_addr = INADDR_BROADCAST; // FIX ME : TEST

	memset(send_buffer, 0, sizeof(send_buffer));

	HEADER2* pSendHeader = (HEADER2*)send_buffer;
	pSendHeader->magic = MAGIC2_CODE;
	pSendHeader->protocol_type = PROTOCOL_TYPE_IPUTILITY;
	pSendHeader->protocol_mode = PROTOCOL_MODE_REQ_GET_IPINFO_EXT;
	pSendHeader->body_size = 0;

	if(sendto(m_hSockReceive, send_buffer, sizeof(HEADER2), 0, (SOCKADDR*)&TargetAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		TRACE("sendto to error = %d\n", WSAGetLastError());
		//closesocket(hSockSend);
		return FALSE;
	}
	// clear temp datas
	//Sleep(300); // 0.3 seconds wait
	//closesocket(hSockSend);

	return TRUE;
}

// 2012-03-22 : A2W가 Thread while문에서 실행되면 스택에 쌓이면서 오버플로가 발생하는 문제 수정을 위해 만든 함수
void CNetScanVision::WideCopyStringFromAnsi(WCHAR* wszStrig, int nMaxBufferLen, char* aszString)
{
	USES_CONVERSION;
	wcscpy_s(wszStrig, nMaxBufferLen, A2W(aszString));
}

void CNetScanVision::SetBindAddress(ULONG ulBindAddress)
{
	m_ulBindAddress = ulBindAddress;
}
