//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: Server scanning class
// Date: 2010-08-26 hkeins first release
//       2011-10-12 hkeins Scan 정보 확장
// Author: hkeins
//
#pragma once

//enum VERSION_PROTOCAL {
//	VERSION_1 = 0x00,
//	VERSION_2
//};
//
//const UINT32 SCAN_INFO_m_pReceive_buffer_SIZE = 64 * 1024; // 64 Kbytes temp buffer for receive temporary data
//const UINT32 SCAN_ERR_NONE = 0x00000000;
//const UINT32 SCAN_ERR_SOCKET_OPT = 0x00000001;
//const UINT32 SCAN_ERR_BIND = 0x00000002;
//const UINT32 SCAN_ERR_MEMORY = 0x00000003;
//const UINT32 SCAN_ERR_RECV = 0x00000004;

#include "NetScanBase.h"

typedef struct tagCAPTION_HEADER
{
	char aszCaption[32];
	int  nDataLen;
}CAPTION_HEADER, *LPCAPTION_HEADER;

class CNetScanVision : public NetScanBase
{

public:
	CNetScanVision();
	~CNetScanVision(void);
	
	virtual BOOL StartScan();
	virtual BOOL SendScanRequest();

	//// 2010-08-27 hkeins : change IP configuration
	//static BOOL RequestIPChange(WCHAR* strTargetServerMAC, WCHAR* strNewIP, WCHAR* strNewGateWay, int nStreamPort = 2700, int nHTTPPort = 80);
	//static BOOL RequestIPChange2(WCHAR* strTargetServerMAC, WCHAR* strNewIP, WCHAR* strNewGateWay, int nStreamPort = 2700, int nHTTPPort = 80, int cIsDHCP = 0, WCHAR* strNewsubnetMask = L"255.255.255.0", WCHAR* szID = L"", WCHAR* szPass = L"", int nEncMode = 0);

	//BOOL	SendScanRequest();
	//BOOL SendScanRequestExt(); // 확장 scan 정보 전송
protected:
	void thrReceiver();
	static DWORD thrScanThread(LPVOID pParam);

};
