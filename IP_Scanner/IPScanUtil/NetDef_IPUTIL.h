//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: IP scanning and setup protocol
//
// Date: 2010-08-24
// Author: hkeins
//
#pragma once

#include "NetDef2.h" // HEADER2 defined
//#pragma pack(push, 1)
//typedef struct  tagVH_HEADER
//{
//	WORD	magic;
//	BYTE	protocol_type;
//	DWORD	protocol_mode;
//	DWORD	body_size;
//}VH_HEADER, *LPVH_HEADER;
//
//#pragma pack(pop)


const BYTE PROTOCOL_TYPE_IPUTILITY				= 0x01;

const DWORD PROTOCOL_MODE_REQ_GET_IPINFO		= 0x01;
const DWORD PROTOCOL_MODE_REQ_SET_IPINFO		= 0x02;
const DWORD PROTOCOL_MODE_REQ_GET_IPINFO_EXT	= 0x03; // IP util extension

const DWORD PROTOCOL_MODE_RSP_GET_IPINFO		= 0x100;
const DWORD PROTOCOL_MODE_RSP_GET_IPINFO_EXT	= 0x101;// IP util extension

#pragma pack(push, 1)
typedef struct tagIPUTIL_INFO
{
	char	szIPAddress[16];
	char	szGatewayIP[16];
	char	szMACAddress[20];
	DWORD   dwStreamPort;
	DWORD	dwHTTPPort;
}IPUTIL_INFO, *LPIPUTIL_INFO;

typedef struct tagIPUTIL_INFO2
{
	char	szIPAddress[16];
	char	szGatewayIP[16];
	char	szMACAddress[20];
	DWORD   dwStreamPort;
	DWORD	dwHTTPPort;
	char	cIsDHCP;              // 0 static, 1 DHCP, IPUTIL INFO version 2
	char	szSubnetmask[16];
}IPUTIL_INFO2, *LPIPUTIL_INFO2;

typedef struct tagIPUTIL_AUTH
{
	char    ID[32];			// User ID
	char	Password[32];	// Password
	SHORT   EncMode;		// Encription mode(default 0 : no encription)
}IPUTIL_AUTH, *LPIPUTIL_AUTH;

#pragma pack(pop)