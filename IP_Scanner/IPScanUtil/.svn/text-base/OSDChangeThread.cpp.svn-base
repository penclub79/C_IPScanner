#include "StdAfx.h"
#include "OSDChangeThread.h"

COSDChangeThread::COSDChangeThread(	HWND		pParent		, 
									int			nIndex		, 
									CString		strIPAddress, 
									int			nPort		,
									CString		strUserName	,
									CString		strPassword	,
									SETUP_OSDINFO	setupOSDInfo)
{
	m_pParent					= pParent		;
	m_nIndex					= nIndex		;
	m_strIPAddress				= strIPAddress	;
	m_nPort						= nPort			;
	m_strUserName				= strUserName	;	
	m_strPassword				= strPassword	;

	m_setupOSDInfo				= setupOSDInfo;

	m_hThreadOSDChange	= NULL;
	m_dwThreadOSDChange	= 0;

	m_hThreadOSDChange	= ::CreateThread(	NULL, 
													0, 
													(LPTHREAD_START_ROUTINE)ProcOSDChange, 
													this, 
													0, 
													&m_dwThreadOSDChange );
}

COSDChangeThread::~COSDChangeThread(void)
{
	if( m_hThreadOSDChange )
	{
		m_dwThreadOSDChange	= 0;
		WaitForSingleObject( m_hThreadOSDChange, INFINITE );

		CloseHandle(m_hThreadOSDChange);
		m_hThreadOSDChange	= NULL;
	}
}

DWORD WINAPI COSDChangeThread::ProcOSDChange( LPVOID _lpParam )
{
	COSDChangeThread*	pThis	= (COSDChangeThread*)_lpParam;

	if( pThis )
		return pThis->OSDChange();

	return 0;
}

DWORD COSDChangeThread::OSDChange()
{
	CString strCmd;
	CString strResponse;

	PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_OSD_SET_START, m_nIndex );

	if( TRUE ==  m_setupOSDInfo.bUsed )
	{
		strCmd.Format(L"cgi-bin/param.cgi?action=update&Image.Text.String.Enabled=yes&Image.Text.String.Position=%d&Image.Text.String.Value=", 
			m_setupOSDInfo.iDisplayPosition );
		strCmd.Append(m_setupOSDInfo.strOverlayText );
	}
	else
	{
		strCmd.Format(L"cgi-bin/param.cgi?action=update&Image.Text.String.Enabled=no");
	}

	CallHttpRequest(m_strIPAddress, 
					m_nPort, 
					strCmd, 
					m_strUserName,
					m_strPassword,
					strResponse );

	strResponse.Trim();

	if( -1 != strResponse.Find(L"OK") )
	{
		PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_OSD_SET_END, m_nIndex );
	}
	else
	{
		strResponse.MakeUpper();
		if( -1 != strResponse.Find(L"AUTH") &&
			-1 != strResponse.Find(L"FAIL") )
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_OSD_SET_ERROR_AUTH, m_nIndex );
		}
		else
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_OSD_SET_ERROR, m_nIndex );
		}
	}

	return 0;
}

int COSDChangeThread::CallHttpRequest(CString ipaddr, int port, CString query, CString strUserID, CString strPassword, CString &response)
{
	CInternetSession*	pSession		= NULL;
	CHttpConnection*	pHttpConnect	= NULL;
	CHttpFile*			pHttpFileAuth	= NULL;
	CHttpFile*			pHttpFile		= NULL;
	CString				strAuth			= L"";
	CString				url				= query;
	DWORD				HttpRequestFlags= 0;
	DWORD				dwStatusCode	= 0;

	try
	{
		HttpRequestFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;

		// HTTP 메소드별로 사용 가능
		pSession = new CInternetSession;

		pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT	, 5000); 
		pSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES	, 1); 
		pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT	, 5000);
		pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT	, 5000);

		pHttpConnect = pSession->GetHttpConnection( ipaddr, (INTERNET_PORT)port );

	}
	catch(CInternetException *m_pEx)
	{
//		m_pEx->ReportError();
		m_pEx->Delete();
		return 0;
	}

	// Use direct write to posting field!
	CString strRetBufLen;
	CString szResult	= L"";
	char	c[1024*64]	= {0};
	CString strHeaders	= L"Accept: text/*\r\n";

	strHeaders += L"User-Agent: HttpCall\r\n";
	strHeaders += L"Accept-Language: UFT-8\r\n";

	
	strAuth.Format( L"cgi-bin/auth.cgi?mode=login&id=%s&pass=%s", strUserID, strPassword );
	pHttpFileAuth	=  pHttpConnect->OpenRequest(	CHttpConnection::HTTP_VERB_GET,
													strAuth.GetBuffer(1024), 
													NULL, 
													1, 
													NULL, 
													(LPCTSTR)"1.0", 
													HttpRequestFlags);
	strAuth.ReleaseBuffer();


	pHttpFileAuth->AddRequestHeaders(strHeaders);
	pHttpFileAuth->SendRequest();

	// result값 확인
	pHttpFileAuth->QueryInfoStatusCode(dwStatusCode);

	// Read Data
	pHttpFileAuth->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, strRetBufLen);

	while(pHttpFileAuth->ReadString((LPTSTR)c,1024)!=FALSE)
	{
		response += c;
	}

	pHttpFileAuth->Flush();

	response.Trim();
	if( 0 == response.Compare(L"OK") )
	{
		response	= L"";
		pHttpFile = pHttpConnect->OpenRequest(	CHttpConnection::HTTP_VERB_GET,
												url.GetBuffer(1024), 
												NULL, 
												1, 
												NULL, 
												(LPCTSTR)"1.0", 
												HttpRequestFlags);
		url.ReleaseBuffer();

		pHttpFile->AddRequestHeaders(strHeaders);
		pHttpFile->SendRequest();

		// result값 확인
		pHttpFile->QueryInfoStatusCode(dwStatusCode);

		// Read Data
		pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, strRetBufLen);

		while(pHttpFile->ReadString((LPTSTR)c,1024)!=FALSE)
		{
			response += c;
		}

		response.Trim();
		pHttpFile->Flush();
	}
	else
	{
		response	= L"Fail Auth";
	}

	// 객체 delete 부분
	if(pHttpFileAuth)
	{
		pHttpFileAuth->Close();
		delete pHttpFileAuth;
		pHttpFileAuth	= NULL;
	}

	if(pHttpFile)
	{
		pHttpFile->Close();
		delete pHttpFile;
		pHttpFile = NULL;
	}

	if(pHttpConnect)
	{
		pHttpConnect->Close();
		delete pHttpConnect;
		pHttpConnect = NULL;
	}
	if(pSession)
	{
		pSession->Close();
		delete pSession;
		pSession = NULL;
	}

	return 0;
}

