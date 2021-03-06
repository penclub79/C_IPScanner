#include "StdAfx.h"
#include <afxinet.h>
#include "IPChangeThread.h"
#include "NetScanVision.h"
#include "NetCommon.h"

CIPChangeThread::CIPChangeThread()
{
	m_hThreadIPChange = NULL;
	m_dwThreadIPChangeID = 0;

	memset(&m_IPChanageThreadInfo, 0, sizeof(m_IPChanageThreadInfo));
}

CIPChangeThread::~CIPChangeThread(void)
{
	StopIPChanage();

}

BOOL CIPChangeThread::IsRunning()
{
	if (m_hThreadIPChange)
		return TRUE;

	return FALSE;
}

void CIPChangeThread::SetThreadInfo(HWND hParent, int iVersion, int iIndex, IPCHANGEITEM IPChanageInfo)
{
	if (IsRunning())
	{
		StopIPChanage();
	}

	m_IPChanageThreadInfo.iIndex = iIndex;
	m_IPChanageThreadInfo.hParent = hParent;
	m_IPChanageThreadInfo.pThis = this;
	m_IPChanageThreadInfo.iVersion = iVersion;
	memcpy(&m_IPChanageThreadInfo.IPChangeItem, &IPChanageInfo, sizeof(IPCHANGEITEM));
}

void CIPChangeThread::StartIPChanage()
{
	StopIPChanage();

	m_hThreadIPChange = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ProcThreadIPChange, &m_IPChanageThreadInfo, 0, &m_dwThreadIPChangeID);
}

void CIPChangeThread::StopIPChanage()
{
	if (m_hThreadIPChange)
	{
		m_dwThreadIPChangeID = 0;
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hThreadIPChange, 20000))
		{
			TerminateThread(m_hThreadIPChange, 0xffff1234);
		}

		CloseHandle(m_hThreadIPChange);
		m_hThreadIPChange = NULL;
	}
}

BOOL CIPChangeThread::_ValidateMacAddress(CString strMacAddress)
{
	int i;

	// 빈 경우
	if (strMacAddress.Compare(L"") == 0)
		return FALSE;

	// 자리수가 넘어간 경우
	if (strMacAddress.GetLength() != 17)
		return FALSE;

	// 허용되지 않는 문자가 들어간 경우
	WCHAR ch;

	for (i = 0; i < strMacAddress.GetLength(); i++)
	{
		ch = strMacAddress.GetAt(i);
		if ((ch == L':') ||
			(ch >= L'0' && ch <= L'9') ||
			(ch >= L'a' && ch <= L'f') ||
			(ch >= L'A' && ch <= L'F'))
		{
			// OK bypass
		}
		else
		{
			return FALSE;
		}

	}
	// 요런 포멧이 아닌 경우
	// 00:00:00:00:00:00
	int nPos = 0;
	int nColumnCount = 0;
	while (nPos >= 0)
	{
		nPos = strMacAddress.Find(L':', nPos + 1);
		if (nPos >= 0) nColumnCount++;
	}

	if (nColumnCount != 5) // : 개수가 5개가 아닌 경우
		return FALSE;

	return TRUE;
}

DWORD CIPChangeThread::ProcIPChange(LPVOID _lpParam)
{
	IPCHANGETHREADINFO* pIPChanageThreadInfo = (IPCHANGETHREADINFO*)_lpParam;
	CString				strTemp = L"";
	CString				strReceive = L"";
	BOOL				bNextSeries = FALSE;
	wchar_t*			tokenOut = NULL;
	wchar_t*			tokenIn = NULL;
	int					iCheckItemCount = 0;

	if (pIPChanageThreadInfo)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////
		// Request IP Change //////////////////////////////////////////////////////////////////////////

		TRACE(L"IPCamera Information DHCP = %d, Address = %s, Gateway = %s, Subnet = %s, HTTP Port = %d, HTTP Port = %d\n",
			pIPChanageThreadInfo->IPChangeItem.nFromIsDHCP,
			pIPChanageThreadInfo->IPChangeItem.szFromIPAddress,
			pIPChanageThreadInfo->IPChangeItem.szToGWAddress,
			pIPChanageThreadInfo->IPChangeItem.szToSubnetMask,
			pIPChanageThreadInfo->IPChangeItem.iFromHttpPort,
			pIPChanageThreadInfo->IPChangeItem.iToStreamPort);

		TRACE(L"Change Information   DHCP = %d, Address = %s, Gateway = %s, Subnet = %s, HTTP Port = %d, HTTP Port = %d\n",
			pIPChanageThreadInfo->IPChangeItem.nToIsDHCP,
			pIPChanageThreadInfo->IPChangeItem.szToIPAddress,
			pIPChanageThreadInfo->IPChangeItem.szToGWAddress,
			pIPChanageThreadInfo->IPChangeItem.szToSubnetMask,
			pIPChanageThreadInfo->IPChangeItem.iFromHttpPort,
			pIPChanageThreadInfo->IPChangeItem.iToStreamPort);

		if (0 == wcscmp(pIPChanageThreadInfo->IPChangeItem.szFromIPAddress, pIPChanageThreadInfo->IPChangeItem.szToIPAddress) &&
			0 == wcscmp(pIPChanageThreadInfo->IPChangeItem.szFromGWAddress, pIPChanageThreadInfo->IPChangeItem.szToGWAddress) &&
			0 == wcscmp(pIPChanageThreadInfo->IPChangeItem.szFromSubnetMask, pIPChanageThreadInfo->IPChangeItem.szToSubnetMask) &&
			pIPChanageThreadInfo->IPChangeItem.nFromIsDHCP == pIPChanageThreadInfo->IPChangeItem.nToIsDHCP		&&
			pIPChanageThreadInfo->IPChangeItem.iFromHttpPort == pIPChanageThreadInfo->IPChangeItem.iToHttpPort	&&
			pIPChanageThreadInfo->IPChangeItem.iFromStreamPort == pIPChanageThreadInfo->IPChangeItem.iToStreamPort)
		{
			PostMessage(pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_CHANGE_COMPLETE, pIPChanageThreadInfo->iIndex), NULL);
			pIPChanageThreadInfo->IPChangeItem.iStatus = MESSAGE_CHANGE_COMPLETE;
		}
		else
		{
			// 변경할 아이피와 이전의 아이피가 다르다면
			pIPChanageThreadInfo->IPChangeItem.tTimeOut = GetCurrentTime() + 1000 * 90;

			if (_ValidateMacAddress(pIPChanageThreadInfo->IPChangeItem.szMACAddress))
			{
				pIPChanageThreadInfo->IPChangeItem.iStatus = MESSAGE_SUCESS_MAC;
			}
			else
			{
				PostMessage(pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_FAILED_MAC, pIPChanageThreadInfo->iIndex), NULL);
				pIPChanageThreadInfo->IPChangeItem.iStatus = MESSAGE_FAILED_MAC;
			}

			//strReceive = L"";
			//if( ConnectionCheck(pIPChanageThreadInfo->IPChangeItem.szFromIPAddress, pIPChanageThreadInfo->IPChangeItem.iFromHttpPort) )
			//{
			//	PostMessage( pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_SUCESS_CONNECT, pIPChanageThreadInfo->iIndex), NULL );
			//	pIPChanageThreadInfo->IPChangeItem.iStatus	= MESSAGE_SUCESS_CONNECT;
			//}
			//else
			//{
			//	PostMessage( pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_FAILED_CONNECT, pIPChanageThreadInfo->iIndex), NULL );
			//	pIPChanageThreadInfo->IPChangeItem.iStatus	= MESSAGE_FAILED_CONNECT;
			//}


			//if( MESSAGE_SUCESS_CONNECT == pIPChanageThreadInfo->IPChangeItem.iStatus	)
			//{
			//	strTemp=L"";
			//	strTemp.Format(	L"cgi-bin/auth.cgi?mode=login&id=%s&pass=%s", 
			//		pIPChanageThreadInfo->IPChangeItem.szUserID, 
			//		pIPChanageThreadInfo->IPChangeItem.szUserPW);

			//	CallHttpRequest(pIPChanageThreadInfo->IPChangeItem.szFromIPAddress, 
			//		pIPChanageThreadInfo->IPChangeItem.iFromHttpPort, 
			//		strTemp, 
			//		strReceive);

			//	if(strReceive.Find(L"OK") != -1)
			//	{
			//		PostMessage( pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_SUCESS_LOGIN, pIPChanageThreadInfo->iIndex), NULL);
			//		pIPChanageThreadInfo->IPChangeItem.iStatus	= MESSAGE_SUCESS_LOGIN;
			//	}
			//	else
			//	{
			//		PostMessage( pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_FAILED_LOGIN, pIPChanageThreadInfo->iIndex), NULL);
			//		pIPChanageThreadInfo->IPChangeItem.iStatus	= MESSAGE_FAILED_LOGIN;
			//	}
			//}

			if (MESSAGE_SUCESS_MAC == pIPChanageThreadInfo->IPChangeItem.iStatus)
			{
				//if (pIPChanageThreadInfo->iVersion == VERSION_1)
				//{
				//	CNetScanVision::RequestIPChange((LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szMACAddress,
				//		(LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szToIPAddress,
				//		(LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szToGWAddress,
				//		pIPChanageThreadInfo->IPChangeItem.iToStreamPort,
				//		pIPChanageThreadInfo->IPChangeItem.iToHttpPort);

				//	TRACE(L"\n###### i = %d, m_strIPAddress = %s, m_strGatewayAddress = %s, m_strSubnetMask = %s\n", pIPChanageThreadInfo->iIndex,
				//		pIPChanageThreadInfo->IPChangeItem.szToIPAddress,
				//		pIPChanageThreadInfo->IPChangeItem.szToGWAddress,
				//		pIPChanageThreadInfo->IPChangeItem.szToSubnetMask);

				//	TRACE(L"###### i = %d, m_nStreamPort = %d, m_nHTTPPort = %d\n\n", pIPChanageThreadInfo->iIndex,
				//		pIPChanageThreadInfo->IPChangeItem.iToStreamPort,
				//		pIPChanageThreadInfo->IPChangeItem.iToHttpPort);

				//	PostMessage(pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_REQUEST_IPCHANGE, pIPChanageThreadInfo->iIndex), NULL);
				//	pIPChanageThreadInfo->IPChangeItem.iStatus = MESSAGE_REQUEST_IPCHANGE;
				//}
				//else if (pIPChanageThreadInfo->iVersion == VERSION_2)
				//{
				//	CNetScanVision::RequestIPChange2((LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szMACAddress,
				//		(LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szToIPAddress,
				//		(LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szToGWAddress,
				//		pIPChanageThreadInfo->IPChangeItem.iToStreamPort,
				//		pIPChanageThreadInfo->IPChangeItem.iToHttpPort,
				//		pIPChanageThreadInfo->IPChangeItem.nToIsDHCP,
				//		(LPTSTR)(LPCTSTR)pIPChanageThreadInfo->IPChangeItem.szToSubnetMask,
				//		pIPChanageThreadInfo->IPChangeItem.szUserID,
				//		pIPChanageThreadInfo->IPChangeItem.szUserPW,
				//		0);

				//	TRACE(L"\n###### i = %d, m_strIPAddress = %s, m_strGatewayAddress = %s, m_strSubnetMask = %s\n", pIPChanageThreadInfo->iIndex,
				//		pIPChanageThreadInfo->IPChangeItem.szToIPAddress,
				//		pIPChanageThreadInfo->IPChangeItem.szToGWAddress,
				//		pIPChanageThreadInfo->IPChangeItem.szToSubnetMask);

				//	TRACE(L"###### i = %d, m_nStreamPort = %d, m_nHTTPPort = %d\n\n", pIPChanageThreadInfo->iIndex,
				//		pIPChanageThreadInfo->IPChangeItem.iToStreamPort,
				//		pIPChanageThreadInfo->IPChangeItem.iToHttpPort);

				//	PostMessage(pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_REQUEST_IPCHANGE, pIPChanageThreadInfo->iIndex), NULL);
				//	pIPChanageThreadInfo->IPChangeItem.iStatus = MESSAGE_REQUEST_IPCHANGE;
				//}
				//else
				//{
				//	PostMessage(pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_FAILED_REQUEST_IPCHANGE, pIPChanageThreadInfo->iIndex), NULL);
				//	pIPChanageThreadInfo->IPChangeItem.iStatus = MESSAGE_FAILED_REQUEST_IPCHANGE;
				//}
			}

			// Request IP Change //////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////////////////////


			/////////////////////////////////////////////////////////////////////////////////////////////////
			//// Check IP Change ////////////////////////////////////////////////////////////////////////////

			if (MESSAGE_REQUEST_IPCHANGE == pIPChanageThreadInfo->IPChangeItem.iStatus)
			{
				PostMessage(pIPChanageThreadInfo->hParent, WM_IPCHANGE_MESSAGE, MAKEWPARAM(MESSAGE_SCAN_START, pIPChanageThreadInfo->iIndex), NULL);

				while (m_dwThreadIPChangeID)
				{
					PostMessage(pIPChanageThreadInfo->hParent,
						WM_IPCHANGE_MESSAGE,
						MAKEWPARAM(MESSAGE_CHECK_IPCHANGE, pIPChanageThreadInfo->iIndex),
						(LPARAM)&pIPChanageThreadInfo->IPChangeItem);

					iCheckItemCount = 0;
					if (MESSAGE_REQUEST_IPCHANGE == pIPChanageThreadInfo->IPChangeItem.iStatus)
					{
						iCheckItemCount++;
					}
					else
					{
						break;
					}

					Sleep(1000);
				}
			}
		}

		//// Check IP Change ////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////
	}

	return 0;
}

DWORD WINAPI CIPChangeThread::ProcThreadIPChange(LPVOID _lpParam)
{
	IPCHANGETHREADINFO* pIPChanageThreadInfo = (IPCHANGETHREADINFO*)_lpParam;

	if (NULL != pIPChanageThreadInfo->pThis)
	{
		((CIPChangeThread*)pIPChanageThreadInfo->pThis)->ProcIPChange(_lpParam);
	}
	return 0;
}

int CIPChangeThread::CallHttpRequest(CString ipaddr, int port, CString query, CString &response)
{
	CInternetSession*    pSession = NULL;
	CHttpConnection*    pHttpConnect = NULL;
	CHttpFile*            pHttpFile = NULL;
	CString                url = query;
	DWORD HttpRequestFlags;

	try{

		HttpRequestFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;

		// HTTP 메소드별로 사용 가능
		pSession = new CInternetSession;

		pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000);
		pSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);
		pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);
		pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5000);

		pHttpConnect = pSession->GetHttpConnection(ipaddr, (INTERNET_PORT)port);
#if 0
		pHttpFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, url);
#else
		pHttpFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			url.GetBuffer(1024), NULL, 1, NULL, (LPCTSTR)"1.0", HttpRequestFlags); url.ReleaseBuffer();
#endif

		// Use direct write to posting field!
		CString strHeaders = L"Accept: text/*\r\n";
		strHeaders += L"User-Agent: HttpCall\r\n";
		strHeaders += L"Accept-Language: UFT-8\r\n";

		pHttpFile->AddRequestHeaders(strHeaders);

		pHttpFile->SendRequest();
	}
	catch (CInternetException *m_pEx)
	{
		//		m_pEx->ReportError();
		m_pEx->Delete();
		return 0;
	}

	// result값 확인
	DWORD m_dwStatusCode;
	pHttpFile->QueryInfoStatusCode(m_dwStatusCode);

	// Read Data
	CString strRetBufLen;
	pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, strRetBufLen);

	CString szResult = L"";
	char c[1024 * 64];
	while (pHttpFile->ReadString((LPTSTR)c, 1024) != FALSE)
	{
		response += c;
	}

	pHttpFile->Flush();

	// 객체 delete 부분
	if (pHttpFile)
	{
		pHttpFile->Close();
		delete pHttpFile;
		pHttpFile = NULL;
	}
	if (pHttpConnect)
	{
		pHttpConnect->Close();
		delete pHttpConnect;
		pHttpConnect = NULL;
	}
	if (pSession)
	{
		pSession->Close();
		delete pSession;
		pSession = NULL;
	}

	return 0;
}

BOOL CIPChangeThread::ConnectionCheck(CString ipaddr, int port)
{
	BOOL bConnect = TRUE;
	int nPort = port;

	struct sockaddr_in	toaddr;
	int					on = 1;
	int					status = 0;
	int					nRet = 0;
	int					flags = 0;
	int					n = 0;
	int					error = 0;
	fd_set				rset;
	fd_set				wset;

	USES_CONVERSION;
	LPCSTR  ip = T2CA(ipaddr);

	// Client
	int sockfd = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int nonblocking = 1;
	ioctlsocket(sockfd, FIONBIO, (unsigned long*)&nonblocking);

	if (sockfd < 0)
	{
		bConnect = FALSE;
	}

	memset(&toaddr, 0x00, sizeof(toaddr));
	toaddr.sin_family = AF_INET;
	toaddr.sin_port = htons(nPort);
	toaddr.sin_addr.s_addr = inet_addr(ip);
	int conResult = 0;

	//int errno

	if (connect(sockfd, (struct sockaddr *)&toaddr, sizeof(toaddr)) < 0)
	{
		conResult = WSAGetLastError();

		if (conResult != WSAEINPROGRESS && conResult != WSAEWOULDBLOCK)
		{
			//			TRACE("%d \n", errno);
		}
		else
		{
			//			TRACE("%d \n", errno);
		}
	}
	else
	{
		bConnect = TRUE;
	}

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;

	struct timeval tval;
	tval.tv_sec = 1;
	tval.tv_usec = 0;

	if ((n = select(sockfd + 1, &rset, &wset, NULL, ((tval.tv_sec>0) || (tval.tv_usec>0)) ? &tval : NULL)) == 0)
	{
		closesocket(sockfd);        /* timeout */
		errno = WSAETIMEDOUT;

		bConnect = FALSE;
	}

	if (!(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)))
	{
		bConnect = FALSE;
	}

	nonblocking = 0;
	ioctlsocket(sockfd, FIONBIO, (unsigned long*)&nonblocking);

	if (error)
	{
		closesocket(sockfd);        /* just in case */
		errno = error;
		bConnect = FALSE;
	}

	CString kk = L"";

	if (bConnect)
	{
		bConnect = TRUE;
	}
	else
	{
		//kk.Format(L"연결실패 !!! 다시 연결해주세요");
		//AfxMessageBox(kk, MB_OK);
		bConnect = FALSE;
	}

	closesocket(sockfd);

	return bConnect;
}