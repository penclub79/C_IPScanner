//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: Network adaptor information
// History:
// 2013-07-15 hkeins : Network adaptor information (IP address, MAC address etc)
// ref. library      : Iphlpapi.lib
//
#pragma once
#include <vector>

typedef struct tagNETWORK_ADAPTOR_INFO_ipv4
{
	int		iAdapterID;
	WCHAR	szName[256*4];
	WCHAR	szDesc[128*4];
	BOOL	bDHCP;
	WCHAR	szMACAddress[30];
	WCHAR	szIPAddress[33];
	WCHAR	szMask[33];
	WCHAR	szGateWay[33];
	WCHAR	szDHCPAddress[33];

	tagNETWORK_ADAPTOR_INFO_ipv4()
	{
		iAdapterID	= 0;
		memset(szName, 0, sizeof(szName));
		memset(szDesc, 0, sizeof(szDesc));
		memset(szMACAddress, 0, sizeof(szMACAddress));
		memset(szIPAddress, 0, sizeof(szIPAddress));
		memset(szMask, 0, sizeof(szMask));
		memset(szGateWay, 0, sizeof(szGateWay));
		memset(szDHCPAddress, 0, sizeof(szDHCPAddress));
		bDHCP = FALSE;
	}
}NETWORK_ADAPTOR_INFO_ipv4, *LPNETWORK_ADAPTOR_INFO_ipv4;

class VHNetworkAdaptorInfo
{
public:
	VHNetworkAdaptorInfo(void);
	~VHNetworkAdaptorInfo(void);

	BOOL LoadInformation();
	BOOL ClearInformation();

	int  GetInformationCounts();
	NETWORK_ADAPTOR_INFO_ipv4* GetInformation(int nIndex);

protected:
	std::vector<NETWORK_ADAPTOR_INFO_ipv4*> m_vcAdaptorList;

};
