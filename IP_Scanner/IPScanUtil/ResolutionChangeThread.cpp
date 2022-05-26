#include "StdAfx.h"
#include "ResolutionChangeThread.h"

CResolutionChangeThread::CResolutionChangeThread(HWND		pParent		, 
												 int		nIndex		, 
												 CString	strIPAddress, 
												 int		nPort		,
												 CString	strUserName	,
												 CString	strPassword	,
												 SETUP_STREAMINFO SetupStreamInfo)
{
	m_pParent					= pParent		;
	m_nIndex					= nIndex		;
	m_strIPAddress				= strIPAddress	;
	m_nPort						= nPort			;
	m_strUserName				= strUserName	;	
	m_strPassword				= strPassword	;

	m_SetupStreamInfo			= SetupStreamInfo;

	m_hThreadResolutionChange	= NULL;
	m_dwThreadResolutionChange	= 0;

	m_hThreadResolutionChange	= ::CreateThread(	NULL, 
													0, 
													(LPTHREAD_START_ROUTINE)ProcResolutionChange, 
													this, 
													0, 
													&m_dwThreadResolutionChange );
}

CResolutionChangeThread::~CResolutionChangeThread(void)
{
	if( m_hThreadResolutionChange )
	{
		m_dwThreadResolutionChange	= 0;
		WaitForSingleObject( m_hThreadResolutionChange, INFINITE );

		CloseHandle(m_hThreadResolutionChange);
		m_hThreadResolutionChange	= NULL;
	}
}

DWORD WINAPI CResolutionChangeThread::ProcResolutionChange( LPVOID _lpParam )
{
	CResolutionChangeThread*	pThis	= (CResolutionChangeThread*)_lpParam;

	if( pThis )
		return pThis->ResolutionChange();

	return 0;
}

DWORD CResolutionChangeThread::ResolutionChange()
{
	CString strCmd;
	CString strResponse;


	PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_START, m_nIndex );

	strCmd.Format(L"cgi-bin/param.cgi?action=update");

	strCmd.AppendFormat(L"&Image.NumofStream=%d", m_SetupStreamInfo.iStreamCount);

	//// first
	if( 1 <= m_SetupStreamInfo.iStreamCount )
	{
		if( 0 == wcscmp( L"H.264", m_SetupStreamInfo.aStreamItems[0].szCodec ) )
		{
			strCmd.AppendFormat(L"&Image.I0.Codec=%s",                     m_SetupStreamInfo.aStreamItems[0].szCodec);
			strCmd.AppendFormat(L"&Image.I0.Appearance.Resolution=%s",     m_SetupStreamInfo.aStreamItems[0].szResolution );
			strCmd.AppendFormat(L"&Image.I0.RateControl.Mode=%s",          m_SetupStreamInfo.aStreamItems[0].szRateControl);			
			strCmd.AppendFormat(L"&Image.I0.RateControl.TargetBitrate=%d", m_SetupStreamInfo.aStreamItems[0].lBitrate);
			strCmd.AppendFormat(L"&Image.I0.RateControl.FrameRate=%d",     m_SetupStreamInfo.aStreamItems[0].lFrameRate);
			strCmd.AppendFormat(L"&Image.I0.RateControl.GOP=%d",           m_SetupStreamInfo.aStreamItems[0].lGOP);
		}
		else
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_ERROR, m_nIndex );
			return 0;
		}
	}

	if( 2 <= m_SetupStreamInfo.iStreamCount )
	{
		if( 0 == wcscmp( L"H.264", m_SetupStreamInfo.aStreamItems[1].szCodec ) )
		{
			strCmd.AppendFormat(L"&Image.I1.Codec=%s",                     m_SetupStreamInfo.aStreamItems[1].szCodec);
			strCmd.AppendFormat(L"&Image.I1.Appearance.Resolution=%s",     m_SetupStreamInfo.aStreamItems[1].szResolution );
			strCmd.AppendFormat(L"&Image.I1.RateControl.Mode=%s",          m_SetupStreamInfo.aStreamItems[1].szRateControl);			
			strCmd.AppendFormat(L"&Image.I1.RateControl.TargetBitrate=%d", m_SetupStreamInfo.aStreamItems[1].lBitrate);
			strCmd.AppendFormat(L"&Image.I1.RateControl.FrameRate=%d",     m_SetupStreamInfo.aStreamItems[1].lFrameRate);
			strCmd.AppendFormat(L"&Image.I1.RateControl.GOP=%d",           m_SetupStreamInfo.aStreamItems[1].lGOP);
		}
		else if( 0 == wcscmp( L"MJPEG", m_SetupStreamInfo.aStreamItems[1].szCodec ) )
		{
			strCmd.AppendFormat(L"&Image.I1.Codec=%s",                     m_SetupStreamInfo.aStreamItems[1].szCodec);
			strCmd.AppendFormat(L"&Image.I1.Appearance.Resolution=%s",     m_SetupStreamInfo.aStreamItems[1].szResolution );
			strCmd.AppendFormat(L"&Image.I1.RateControl.TargetBitrate=%d", m_SetupStreamInfo.aStreamItems[1].lQuality);
			strCmd.AppendFormat(L"&Image.I1.RateControl.FrameRate=%d",     m_SetupStreamInfo.aStreamItems[1].lFrameRate);
		}
		else
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_ERROR, m_nIndex );
			return 0;
		}
	}

	if( 3 <= m_SetupStreamInfo.iStreamCount )
	{
		if( 0 == wcscmp( L"H.264", m_SetupStreamInfo.aStreamItems[2].szCodec ) )
		{
			strCmd.AppendFormat(L"&Image.I2.Codec=%s",                     m_SetupStreamInfo.aStreamItems[2].szCodec);
			strCmd.AppendFormat(L"&Image.I2.Appearance.Resolution=%s",     m_SetupStreamInfo.aStreamItems[2].szResolution );
			strCmd.AppendFormat(L"&Image.I2.RateControl.Mode=%s",          m_SetupStreamInfo.aStreamItems[2].szRateControl);			
			strCmd.AppendFormat(L"&Image.I2.RateControl.TargetBitrate=%d", m_SetupStreamInfo.aStreamItems[2].lBitrate);
			strCmd.AppendFormat(L"&Image.I2.RateControl.FrameRate=%d",     m_SetupStreamInfo.aStreamItems[2].lFrameRate);
			strCmd.AppendFormat(L"&Image.I2.RateControl.GOP=%d",           m_SetupStreamInfo.aStreamItems[2].lGOP);
		}
		else
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_ERROR, m_nIndex );
			return 0;
		}
	}

	TRACE( strCmd );
	TRACE( L"\n" );

	CallHttpRequest(m_strIPAddress, 
					m_nPort, 
					strCmd, 
					m_strUserName,
					m_strPassword,
					strResponse );

	strResponse.Trim();

	if( -1 != strResponse.Find(L"OK") )
	{
		PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_END, m_nIndex );
	}
	else
	{
		strResponse.MakeUpper();
		if( -1 != strResponse.Find(L"AUTH") &&
			-1 != strResponse.Find(L"FAIL") )
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_ERROR_AUTH, m_nIndex );
		}
		else
		{
			PostMessage( m_pParent, UM_STATUS_MESSAGE, MESSAGE_ID_RESOLUTION_SET_ERROR, m_nIndex );
		}
	}

	return 0;
}

int CResolutionChangeThread::CallHttpRequest(CString ipaddr, int port, CString query, CString strUserID, CString strPassword, CString &response)
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

