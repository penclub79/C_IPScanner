//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: interface of live activex class
// Date: 2013-05-08 hkeins ports from IPScanUtil, made by jwjang
//
#pragma once
#include "stdafx.h"
#include "UpgradeThreadHttp.h"
#include <atlconv.h>

CUpgradeThreadHttp::CUpgradeThreadHttp(HWND pParent, int nIndex, CString sIPAddress, int nPort, CString sUserName, CString sPassword,CString sFilePath,CString sFileName,BOOL bMemoryClearVerion)
{
//	TRACE(L"###### CUpgradeThreadHttp Start! nIndex = %d\n", nIndex);
	m_hwndParent = pParent;
	m_hUpgradeThread = NULL;
	m_dwUpgradeThreadID = 0;

	m_bMemoryClearVerion	= bMemoryClearVerion;

	m_pSession = NULL;
	m_pHttpConnect = NULL;
//	m_pHttpFileAuth = NULL;
	m_pHttpFile = NULL;
	m_pXMLFile = NULL;

	//CString sAuth, CString sRequest, CString sStepXML, 
	m_nIndex = nIndex;
	m_sIPAddress = sIPAddress;
	m_nPort = nPort;
	m_sAuth.Format(L"cgi-bin/auth.cgi?mode=login&id=%s&pass=%s", sUserName, sPassword);

	m_sRequest = L"/cgi-bin/setup_upgrade.cgi";
	m_sStepXML = L"/xml/upgrading_step";

	m_sUserName = sUserName;
	m_sPassword = sPassword;
	m_sFilePath = sFilePath;
	m_sFileName = sFileName;

	m_bThreadStop = TRUE;
	m_hExitWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	TRACE(L"###### CUpgradeThreadHttp Start! \n");
}

CUpgradeThreadHttp::~CUpgradeThreadHttp(void)
{
	m_hwndParent = NULL;
	StopUpgrade();

	// change thread termination to wait terminate
	//if(m_hUpgradeThread)
	//{
	//	this->m_bThreadStop = TRUE;
	//	SetEvent(m_hExitWaitEvent);
	//	WaitForSingleObject(m_hUpgradeThread, INFINITE);
	//	CloseHandle(m_hUpgradeThread);
	//	m_hUpgradeThread = NULL;
	//	CloseHandle(m_hExitWaitEvent);
	//}
}

DWORD thrUpgradeThread(LPVOID pParam)
{
	CUpgradeThreadHttp* pThis = (CUpgradeThreadHttp*)pParam;
	if(pThis == NULL)
		return 0;

	pThis->HttpPostFile();

	return 0;
}

BOOL CUpgradeThreadHttp::StartUpgrade()
{
	if(m_hUpgradeThread != NULL)
		return TRUE;

	m_bThreadStop = FALSE;

	m_hUpgradeThread = :: CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrUpgradeThread, this, 0, &m_dwUpgradeThreadID);
	if(m_hUpgradeThread == NULL)
	{
//		TRACE("Thread create failed\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CUpgradeThreadHttp::StopUpgrade()
{
	// end upgrading

	//if(m_pHttpFileAuth)
	//{
	//	m_pHttpFileAuth->Flush();
	//	m_pHttpFileAuth->Close();
	//}
	if(m_pXMLFile)
	{
		m_pXMLFile->Flush();
		m_pXMLFile->Close();
	}

	if(m_pHttpFile)
	{
		m_pHttpFile->Flush();
		m_pHttpFile->Close();
	}

	//if(m_pHttpConnect)
	//{
	//	m_pHttpConnect->Close();
	//}

	//if(m_pSession)
	//{
	//	m_pSession->Close();
	//}

	m_bThreadStop = TRUE;
	if(m_hUpgradeThread)
	{
		SetEvent(m_hExitWaitEvent);
		if( WAIT_TIMEOUT == WaitForSingleObject(m_hUpgradeThread, 2000) )
		{
			if(m_pHttpConnect)
			{
				m_pHttpConnect->Close();
			}

			if(m_pSession)
			{
				m_pSession->Close();
			}

			WaitForSingleObject(m_hUpgradeThread, INFINITE);
		}
		
		CloseHandle(m_hUpgradeThread);
		m_hUpgradeThread = NULL;
	}

	m_bThreadStop = FALSE;
	//if(m_pHttpFileAuth)
	//{
	//	//m_pHttpFileAuth->Flush();
	//	//m_pHttpFileAuth->Close();
	//	delete m_pHttpFileAuth;
	//	m_pHttpFileAuth = NULL;
	//}
	if(m_pXMLFile)
	{
		//m_pXMLFile->Flush();
		//m_pXMLFile->Close();
		delete m_pXMLFile;
		m_pXMLFile = NULL;
	}

	if(m_pHttpFile)
	{
		//m_pHttpFile->Flush();
		//m_pHttpFile->Close();
		delete m_pHttpFile;
		m_pHttpFile = NULL;
	}

	if(m_pHttpConnect)
	{
		m_pHttpConnect->Close();
		delete m_pHttpConnect;
		m_pHttpConnect = NULL;
	}

	if(m_pSession)
	{
		m_pSession->Close();
		delete m_pSession;
		m_pSession = NULL;
	}

	return TRUE;
}


BOOL CUpgradeThreadHttp::HttpPostFile()
{
//	TRACE(L"###### HttpPostFile Start!\n");
	BOOL bReturn = FALSE;
	DWORD dwRet = 0;

	CString                auth = m_sAuth;
	CString                xmlrequest = m_sStepXML;
	CString                xmlresponse = L"";

	CFile	file;
	float	dwTotalRequestLength;
	int		cReadLength;
	int		cReadFileLength = 0;
	float	cWriteFileLength = 0;
	DWORD	dwChunkLength = 64*1024; 
	void*	pBuffer = malloc(dwChunkLength);
	int		nProgress = 0;
	BOOL	bTransferring = FALSE;
	CString strHeaders;
	
	m_pSession = new CInternetSession(L"UpgradeAgent");
	
	m_pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT		, 6000000); 
	m_pSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES		, 1000); 
	m_pSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT		, 6000000);
	m_pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT	, 6000000);
	m_pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT		, 600000*15);
	m_pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT			, 600000*15);

	try
	{   
		if(file.Open(m_sFilePath,CFile::modeRead|CFile::shareDenyNone) == FALSE)
		{
//			TRACE(L"###### FILE %s Open ERROR!, Index = %d, m_sIPAddress = %s\n", m_sFileName, m_nIndex, m_sIPAddress);
			//AfxMessageBox(_T("Selected file not found..."), MB_ICONWARNING);
			if(m_hwndParent && ::IsWindow(m_hwndParent))
			{
				PostMessage(m_hwndParent, 
					WM_UPGRADE_MSG, 
					MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
					(LPARAM)_UPGRADE_ERROR_OPEN_ERROR );
			}

			bReturn = FALSE;
			goto EXIT_LOOP;
		}

		CString strHTTPBoundary = _T("---------------------------7dc266391062a");       
		CString strPreFileData = MakePreFileData(strHTTPBoundary, m_sFileName);
		CString strPostFileData = MakePostFileData(strHTTPBoundary);
		CString strRequestHeaders = MakeRequestHeaders(strHTTPBoundary, m_sIPAddress);
		dwTotalRequestLength = (float)strPreFileData.GetLength() + (float)strPostFileData.GetLength() + (float)file.GetLength() - 1; 
		m_pHttpConnect = m_pSession->GetHttpConnection(m_sIPAddress,0, m_nPort, m_sUserName, m_sPassword); 

		DWORD dwHttpRequestFlags =  INTERNET_FLAG_RELOAD		| 
									INTERNET_FLAG_PRAGMA_NOCACHE| 
									INTERNET_FLAG_DONT_CACHE	;

		CString	strTemp = L"";

		if(m_hwndParent && ::IsWindow(m_hwndParent))
		{
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
				(LPARAM)_UPGRADE_STATUS_CONNECTING );
		}

/*		m_pHttpFileAuth = m_pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			auth.GetBuffer(1024), NULL, 1, NULL, (LPCTSTR)"1.0", dwHttpRequestFlags);
		auth.ReleaseBuffer();

		CString strHeaders = L"Accept: text/*\r\n";
		strHeaders += L"User-Agent: HttpCall\r\n";
		strHeaders += L"Accept-Language: UFT-8\r\n";

		m_pHttpFileAuth->AddRequestHeaders(strHeaders);

		m_pHttpFileAuth->SendRequest();

		CString szAuthResult = L"";
		char c[1024*64];
		while(m_pHttpFileAuth->ReadString((LPTSTR)c,1024)!=FALSE)
		{
			szAuthResult += c;
			if(m_bThreadStop == TRUE)
			{
				bReturn = FALSE;
				goto EXIT_LOOP;
			}
		}

//		TRACE(L"###### szAuthResult = %s\n",szAuthResult);
		if(szAuthResult.Find(L"FAIL") != -1)
		{
			strTemp = L"Login Fail";

			if(m_pHttpFileAuth)
			{
				m_pHttpFileAuth->Flush();
				m_pHttpFileAuth->Close();
				delete m_pHttpFileAuth;
				m_pHttpFileAuth = NULL;
			}
			if(m_hwndParent && ::IsWindow(m_hwndParent))
			{
				PostMessage(m_hwndParent, 
					WM_UPGRADE_MSG, 
					MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
					(LPARAM)_UPGRADE_ERROR_AUTH_FAILED );
			}

			bReturn = FALSE;
			goto EXIT_LOOP;
		}

		if(m_pHttpFileAuth)
		{
			m_pHttpFileAuth->Flush();
			m_pHttpFileAuth->Close();
			delete m_pHttpFileAuth;
			m_pHttpFileAuth = NULL;
		}
*/

		CString szAuthResult = L""; 
		if(SendCommand( auth, szAuthResult) < 0)
		{
			bReturn = FALSE;

			goto EXIT_LOOP;
		}

		// 다른곳에서 이미 진행 시 오류 처리
		if(-1 != szAuthResult.Find(L"FAIL"))
		{
			if(m_hwndParent && ::IsWindow(m_hwndParent))
			{
				PostMessage(m_hwndParent, 
					WM_UPGRADE_MSG, 
					MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
					(LPARAM)_UPGRADE_ERROR_AUTH_FAILED );
			}

			bReturn = FALSE;
			goto EXIT_LOOP;
		}

		if( TRUE == m_bMemoryClearVerion )
		{

			//{{ ------------- client shutdown && memory clear -------------
			// memory clear
			// shutdown
			CString szClientStop = L""; 
			if( SendCommand( CString(L"cgi-bin/vsconfig.cgi?mode=63"), szClientStop) < 0 )
			{
				bReturn = FALSE;

				goto EXIT_LOOP;
			}

			TRACE( L"szClientStop = %s\n", szClientStop );

			// 다른곳에서 이미 진행 시 오류 처리
			if(0 != szClientStop.Compare(L"1"))
			{
				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
						(LPARAM)_UPGRADE_ERROR_UPGRADE_IS_WORKING_IN_OTHER_PLACE );
				}
				bReturn = FALSE;
				goto EXIT_LOOP;
			}

			CString szMemoryClear = L"";
			if( SendCommand(CString(L"cgi-bin/vsconfig.cgi?mode=64"), szMemoryClear) < 0)
			{
				bReturn = FALSE;

				goto EXIT_LOOP;
			}

			TRACE( L"szMemoryClear = %s\n", szMemoryClear );
		}

		//////////////////////////////////////////////////////////////////////////////
		///////CHttpConnection::HTTP_VERB_POST for Upload Method/////////////////////
//		TRACE(L"###### File Transfer Start!\n");
		TRACE(L"###### m_sRequest = %s\n",m_sRequest);
		bTransferring = TRUE;
		m_pHttpFile = m_pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST, m_sRequest, NULL, 1, NULL, (LPCTSTR)"1.0", dwHttpRequestFlags);
		m_pHttpFile->AddRequestHeaders(strRequestHeaders); 
		TRACE( L"%s \n", strRequestHeaders );
		m_pHttpFile->SendRequestEx((DWORD)dwTotalRequestLength, HSR_INITIATE|HSR_SYNC);  
		TRACE( L"%d \n", dwTotalRequestLength );

		USES_CONVERSION; 
		LPCSTR  lpaszTemp1 = T2CA(strPreFileData); 

		TRACE( L"%s \n", lpaszTemp1 );
		m_pHttpFile->Write((LPSTR)(LPCSTR)lpaszTemp1, strPreFileData.GetLength()); 

		strTemp = L"File Transfer Start";

		nProgress = 0;
		if(m_hwndParent && ::IsWindow(m_hwndParent))
		{
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
				(LPARAM)_UPGRADE_STATUS_FILE_TRANSFERING);
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
				(LPARAM)nProgress );
		}
		cReadLength = -1; 

		while (cReadLength!=0)
		{
			cReadLength = file.Read(pBuffer, dwChunkLength);
			if (0 != cReadLength)
			{
				m_pHttpFile->Write(pBuffer,cReadLength);
				memset(pBuffer, 0, dwChunkLength);
				cWriteFileLength += (float)cReadLength;
				strTemp = L"File Transfering";

				//if(nProgress != (int)(cWriteFileLength/dwTotalRequestLength)*10)
				//{
					nProgress = (int)(((double)cWriteFileLength/dwTotalRequestLength)*10);
					TRACE( L"nProgress = %d\n", nProgress );

					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)nProgress);
				//}			
			}
			cReadFileLength += cReadLength;

			if(m_bThreadStop == TRUE)
			{
				bReturn = FALSE;
				goto EXIT_LOOP;
			}
		}

		file.Close();

		nProgress = 10;

		//TRACE(L"###### cReadFileLength = %d, cWriteFileLength = %f\n",cReadFileLength, cWriteFileLength);

		LPCSTR  lpaszTemp2 = T2CA(strPostFileData); 
		TRACE( L"****\n%s\n****\n", lpaszTemp2 );

		m_pHttpFile->Write((LPSTR)(LPCSTR)lpaszTemp2, strPostFileData.GetLength());

		//TRACE(L"###### File Transfered!\n");
		strTemp = L"File Transfered";

		if(m_hwndParent && ::IsWindow(m_hwndParent))
		{
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
				(LPARAM)_UPGRADE_STATUS_PREPARE_UPGRADING );
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
				(LPARAM)nProgress );
		}

		TRACE( xmlrequest );	// L"/xml/upgrading_step";
		m_pXMLFile = m_pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST,
			xmlrequest.GetBuffer(1024), NULL, 1, NULL, (LPCTSTR)"1.0", dwHttpRequestFlags);
		xmlrequest.ReleaseBuffer();

		while(m_bThreadStop == FALSE)
		{
			strHeaders = L"Accept: text/*\r\n";
			strHeaders += L"User-Agent: HttpCall\r\n";
			strHeaders += L"Accept-Language: UFT-8\r\n";

			m_pXMLFile->AddRequestHeaders(strHeaders);
			m_pXMLFile->SendRequest();
			m_pXMLFile->QueryInfoStatusCode(dwRet);

			if(dwRet == HTTP_STATUS_OK)
			{
				bTransferring = FALSE;
				bReturn = TRUE;
				break;
			} 
			else
			{
				bReturn = FALSE; 
			}

//			TRACE(L"###### Prepairing Upgrade!, m_sIPAddress = %s\n", m_sIPAddress);
			strTemp = L"Prepairing Upgrade";

			if(m_hwndParent && ::IsWindow(m_hwndParent))
			{
				PostMessage(m_hwndParent, 
					WM_UPGRADE_MSG, 
					MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
					(LPARAM)nProgress++ );
			}
			if(nProgress > 40)
			{
				nProgress = 40;
			}

			Sleep(1000);
		}

		if(m_pXMLFile)
		{
			m_pXMLFile->Flush();
			m_pXMLFile->Close();
			delete m_pXMLFile;
			m_pXMLFile = NULL;
		}

		strTemp = L"Prepairing Upgrade";
		if(m_hwndParent && ::IsWindow(m_hwndParent))
		{
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
				(LPARAM)_UPGRADE_STATUS_READY_UPGRADE );
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
				(LPARAM)40 );
		}

		if(m_bThreadStop == TRUE)
		{
			bReturn = FALSE;
			goto EXIT_LOOP;
		}

//		TRACE(L"###### Upgrade Start!\n");

		m_pXMLFile = m_pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_POST,
			xmlrequest.GetBuffer(1024), NULL, 1, NULL, (LPCTSTR)"1.0", dwHttpRequestFlags);
		
		BOOL bUpgradeOS = FALSE;
		BOOL bUpgradeApplication = FALSE;
		BOOL bUpgradeWebApplication = FALSE;
		char c[1024];
		xmlresponse = L"";
		
		strHeaders = L"Accept: text/*\r\n";
		strHeaders += L"User-Agent: HttpCall\r\n";
		strHeaders += L"Accept-Language: UFT-8\r\n";

		while(m_bThreadStop == FALSE)
		{
			TRACE(L"A waiting...\n");

			m_pXMLFile->AddRequestHeaders(strHeaders);
			m_pXMLFile->SendRequest();

			xmlresponse	= L"";

			while(m_pXMLFile->ReadString((LPTSTR)c,1024)!=FALSE)
			{
				xmlresponse += c;
				if(m_bThreadStop == TRUE)
				{
					if(m_hwndParent && ::IsWindow(m_hwndParent))
					{
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_UPGRADE_FILE_IS_NOT_AVAILABLE );
					}
					bReturn = FALSE;
					goto EXIT_LOOP;
				}

//				TRACE("xmlresponse: %s\n", xmlresponse);
				//Sleep(100);
			}

			TRACE(L"###### Checking Upgrade Process!, xmlresponse = %s\n", xmlresponse);

			if(xmlresponse.Find(L"Upgrading OS") != -1)
			{
				strTemp = L"Upgrading OS";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					if(!bUpgradeOS) // 최초 진입시 한번만 상태 변경 메시지를 보내준다
					{
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_UPGRADE_OS );

						bUpgradeOS = TRUE;
					}

					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)nProgress++ );
				}

				if(nProgress>50)
				{
					nProgress = 50;
				}
			}
			else if(xmlresponse.Find(L"Upgraded OS") != -1)
			{
				strTemp = L"Upgraded OS";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)nProgress++ );
				}
				if(nProgress > 50)
				{
					nProgress = 50;
				}
			}
			else if(xmlresponse.Find(L"Upgrading Application") != -1)
			{
				strTemp = L"Upgrading Application";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					if(!bUpgradeApplication) // 최초 진입 시 한번만 상태 업데이트
					{
						bUpgradeApplication = TRUE;
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_UPGRADE_APPLICATION );
					}
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)nProgress++ );
				}
				if(nProgress > 70)
				{
					nProgress = 70;
				}
			}
			else if(xmlresponse.Find(L"Upgraded  Application") != -1)
			{
				strTemp = L"Upgraded  Application";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)70 );
				}
			}
			else if(xmlresponse.Find(L"Upgrading Web Application") != -1)
			{
				strTemp = L"Upgrading Web Application";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					if(!bUpgradeWebApplication) // 최초 진입 시 한번만 상태 업데이트
					{
						bUpgradeWebApplication = TRUE;
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_UPGRADE_APPLICATION_WEB );
					}
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)nProgress++ );
				}
				if(nProgress>95)
				{
					nProgress = 95;
				}			
			}
			else if(xmlresponse.Find(L"Upgraded Web Application") != -1)
			{
				strTemp = L"Upgraded Web Application";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)98 );
				}
			}
			else if(xmlresponse.Find(L"Finished upgrade") != -1)
			{
				strTemp = L"Finished upgrade";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)100 );
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
						(LPARAM)_UPGRADE_STATUS_COMPLETE );
				}
				break;
			}
			else if(xmlresponse.Find(L"Upgrade file is not available") != -1)
			{
				strTemp = L"Upgrade file is not available";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)100 );
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
						(LPARAM)_UPGRADE_ERROR_UPGRADE_FILE_IS_NOT_AVAILABLE );
				}
				break;
			}
			else if(xmlresponse.Find(L"Now system is restarting...") != -1)
			{
				strTemp = L"Restarting";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)99 );
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
						(LPARAM)_UPGRADE_STATUS_RESTARTING_SW );
				}
			}
			else if( xmlresponse.Find(L"UPGRADE FAILED-NO MATCH MODEL") != -1 )
			{
				strTemp = L"No Match model";

				if(m_hwndParent && ::IsWindow(m_hwndParent))
				{
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), 
						(LPARAM)99 );
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
						(LPARAM)_UPGRADE_ERROR_NO_MATCH_MODEL );
				}
				break;
			}

			Sleep(1000);
		}

		xmlrequest.ReleaseBuffer();


		if(m_pXMLFile)
		{
			m_pXMLFile->Flush();
			m_pXMLFile->Close();
			delete m_pXMLFile;
			m_pXMLFile = NULL;
		}

		m_pHttpFile->EndRequest(HSR_SYNC);
		//////////////////////////////////////////////////////////////////////// 
		m_pHttpFile->QueryInfoStatusCode(dwRet);
		
		if(dwRet == HTTP_STATUS_OK)
		{
//			TRACE(L"###### HTTP_STATUS_OK:: dwRet = %d\n",dwRet);
			bReturn = TRUE;
		} 
		else
		{
//			TRACE(L"###### HTTP_STATUS_FAIL:: dwRet = %d\n",dwRet);
			bReturn = FALSE; 
		}
	}
	catch(CInternetException* m_pEx)
	{   
		//m_pEx->ReportError();
		m_pEx->Delete();
		m_pEx = NULL;
		bReturn = TRUE;
		m_bThreadStop = TRUE;
		if(m_hwndParent && ::IsWindow(m_hwndParent))
		{
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
				(LPARAM)_UPGRADE_ERROR_CONNECT_FAILED );
		}

		goto EXIT_LOOP;
	}

	if(m_pHttpFile)
	{
		m_pHttpFile->Flush();
		m_pHttpFile->Close();
		delete m_pHttpFile;
		m_pHttpFile = NULL;
	}

	if(m_pHttpConnect)
	{
		m_pHttpConnect->Close();
		delete m_pHttpConnect;
		m_pHttpConnect = NULL;
	}

	if(m_pSession)
	{
		m_pSession->Close();
		delete m_pSession;
		m_pSession = NULL;
	}
	m_bThreadStop = TRUE;
	return bReturn;


EXIT_LOOP:
//	TRACE(L"###### goto EXIT_LOOP\n");
	if(m_bThreadStop && bTransferring && this->m_bMemoryClearVerion)
	{
		TRACE(L"Upgrade canceled by user shutup system\n");
		// shutup system if canceled operation while tranferring file
		CString szShutUp = L"";
		if(SendCommand(CString(L"cgi-bin/vsconfig.cgi?mode=65"), szShutUp) < 0)
		{
		}
	}

	if(m_pHttpFile)
	{
		m_pHttpFile->Flush();
		m_pHttpFile->Close();
		delete m_pHttpFile;
		m_pHttpFile = NULL;
	}

	if(m_pHttpConnect)
	{
		m_pHttpConnect->Close();
		delete m_pHttpConnect;
		m_pHttpConnect = NULL;
	}

	if(m_pSession)
	{
		m_pSession->Close();
		delete m_pSession;
		m_pSession = NULL;
	}

	m_bThreadStop = TRUE;

	if( NULL != pBuffer )
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	WaitForSingleObject(m_hExitWaitEvent, INFINITE);

	return bReturn;
}

CString CUpgradeThreadHttp::MakePreFileData(CString& strBoundary, CString& strFileName)
{
	CString strFormat;
	CString strData;

	strFormat += _T("--%s");
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"mode\"");
	strFormat += _T("\r\n");
	strFormat += _T("\r\n");
	strFormat += _T("1");
	strFormat += _T("\r\n");
	strFormat += _T("--%s");
	strFormat += _T("\r\n");
	strFormat += _T("Content-Disposition: form-data; name=\"upgrade_file_name\"; filename=\"%s\"");
	strFormat += _T("\r\n");
	strFormat += _T("Content-Type: application/x-gzip-compressed");
	strFormat += _T("\r\n\r\n");

	strData.Format(strFormat, strBoundary, strBoundary, strFileName);

	return strData;
}

CString CUpgradeThreadHttp::MakePostFileData(CString& strBoundary)
{
	CString strFormat;
	CString strData = _T("");

	strFormat += _T("\r\n");
	strFormat += _T("--%s--");
	strFormat += _T("\r\n");
	strFormat += _T("\r\n");

	strData.Format(strFormat, strBoundary, strBoundary);

	return strData;
}

CString CUpgradeThreadHttp::MakeRequestHeaders(CString &strBoundary, CString &strHostIP)
{
	CString strFormat=_T("");
	CString strData =_T("");
	strFormat += _T("\r\n");
	strFormat += _T("Accept: text/*, image/jpeg, application/x-ms-application, image/gif, application/xaml+xml, image/pjpeg, application/x-ms-xbap, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*\r\n"); 
	strFormat += _T("Referer: http://%s/cgi-bin/setup_upgrade.cgi\r\n");
	strFormat += _T("User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; Tablet PC 2.0; .NET4.0E)\r\n"); 
	strFormat += _T("Accept-Language: UFT-8\r\n"); 
	strFormat += _T("Content-Type: multipart/form-data; boundary=%s\r\n"); 
	strFormat += _T("Accept-Encoding: gzip, deflate\r\n"); 
	strFormat += _T("Cache-Control: no-cache\r\n"); 
	strData.Format(strFormat, strHostIP, strBoundary);

	return strData;
}

int    CUpgradeThreadHttp::SendCommand(CString& strCMD, CString& strResponse)
{
	CHttpFile* pFile = NULL;
	int        nRet  = 0;
	DWORD dwHttpRequestFlags =  			
		INTERNET_FLAG_RELOAD			
		| INTERNET_FLAG_PRAGMA_NOCACHE
		| INTERNET_FLAG_DONT_CACHE;

	if(m_pHttpConnect == NULL)
		return -1;

	try
	{
		pFile = m_pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			strCMD.GetBuffer(1024), NULL, 1, NULL, (LPCTSTR)"1.0", dwHttpRequestFlags);
		strCMD.ReleaseBuffer();

		CString strHeaders = L"Accept: text/*\r\n";
		strHeaders += L"User-Agent: HttpCall\r\n";
		strHeaders += L"Accept-Language: UFT-8\r\n";

		pFile->AddRequestHeaders(strHeaders);

		pFile->SendRequest();

		CString szAuthResult = L"";

		memset(m_szTempBuffer, 0, sizeof(m_szTempBuffer));
		while( pFile->ReadString((LPTSTR)m_szTempBuffer,1024)!=FALSE)
		{
			szAuthResult += m_szTempBuffer;
			if(m_bThreadStop == TRUE)
			{
				nRet = -1;
				break;
			}
		}

		if(-1 != nRet)
			strResponse = szAuthResult;
	}
	catch(CInternetException* pEx)
	{   
		//m_pEx->ReportError();
		pEx->Delete();
		pEx = NULL;
		nRet = -1;
	}
	if(pFile)
	{
		pFile->Flush();
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}

	return nRet;
}