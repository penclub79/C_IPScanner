//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: interface of live activex class
// Date: 2013-05-08 hkeins ports from IPScanUtil, made by jwjang
//
#pragma once

//#include "resource.h"
#include <afxinet.h>
#include "UpgradeDefine.h"
//#include "NetUtil.h"
//#include "ProgressListCtrl.h"
//#include "UpgradeThread.h"


class CUpgradeThreadHttp
{
public:
	//  CString sAuth, CString sRequest, CString sStepXML,
	CUpgradeThreadHttp(HWND pParent, int nIndex, CString sIPAddress, int nPort, CString sUserName, CString sPassword,CString sFilePath,CString sFileName, BOOL bMemoryClearVerion=TRUE);
	~CUpgradeThreadHttp(void);

	BOOL StartUpgrade();
	BOOL StopUpgrade();
	BOOL HttpPostFile();

	BOOL	m_bThreadStop;
	BOOL	m_bTransfering;

protected:
	int m_nIndex;

	BOOL	m_bMemoryClearVerion;
	CString m_sIPAddress;
	int m_nPort;

	CString m_sAuth;
	CString m_sRequest;
	CString m_sStepXML;
	CString m_sUserName;
	CString m_sPassword;
	CString m_sFilePath;
	CString m_sFileName;

	//CProgressListCtrl* m_pUpgradeList;

	HANDLE m_hUpgradeThread;
	HANDLE m_hExitWaitEvent;
	DWORD  m_dwUpgradeThreadID;
	HWND   m_hwndParent;

private:
	CString MakePreFileData(CString& strBoundary, CString& strFileName);
	CString MakePostFileData(CString& strBoundary);
	CString MakeRequestHeaders(CString& strBoundary, CString &strHostIP);
	int     SendCommand(CString& strCMD, CString& strResponse);

	CInternetSession*		m_pSession;
	CHttpConnection*		m_pHttpConnect;
//	CHttpFile*				m_pHttpFileAuth;
	CHttpFile*				m_pHttpFile;
	CHttpFile*				m_pXMLFile;
	char					m_szTempBuffer[2048];
};
