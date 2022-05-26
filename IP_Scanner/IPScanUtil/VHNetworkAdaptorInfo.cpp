//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: Network adaptor information
// History:
// 2013-07-15 hkeins : Network adaptor information (IP address, MAC address etc)
//
#include "StdAfx.h"
#include "VHNetworkAdaptorInfo.h"
#include <Iphlpapi.h>
#include <atlconv.h>

#ifndef TRACE
#define TRACE
#endif

VHNetworkAdaptorInfo::VHNetworkAdaptorInfo(void)
{
}

VHNetworkAdaptorInfo::~VHNetworkAdaptorInfo(void)
{
	ClearInformation();
}

BOOL VHNetworkAdaptorInfo::LoadInformation()
{
	ClearInformation();
	/////////////////////////
	// Get interface informations
	/////////////////////////

	USES_CONVERSION;
	IP_ADAPTER_INFO  *pAdapterInfo = NULL;
	ULONG            ulOutBufLen   = 0;
	DWORD            dwRetVal      = 0;
	NETWORK_ADAPTOR_INFO_ipv4* pAdaptorInfo = NULL;
	//
	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ulOutBufLen  = sizeof(IP_ADAPTER_INFO);
	//
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
	{
		free (pAdapterInfo);

		pAdapterInfo = (IP_ADAPTER_INFO *) malloc ( ulOutBufLen );
		if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS)
		{
			TRACE(L"GetAdaptersInfo call failed with %d\n", dwRetVal);
			if (pAdapterInfo)
				free(pAdapterInfo);

			return FALSE;
		}
	}

	if(dwRetVal == ERROR_SUCCESS)
	{
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter) 
		{
			TRACE(L"Adapter Name: %s\n", A2W(pAdapter->AdapterName));
			TRACE(L"Adapter Desc: %s\n", A2W(pAdapter->Description));
			TRACE(L"\tAdapter Addr: \t"); // MAC address
			for (UINT i = 0; i < pAdapter->AddressLength; i++) 
			{
				if (i == (pAdapter->AddressLength - 1))
					TRACE(L"%.2X\n",(int)pAdapter->Address[i]);
				else
					TRACE(L"%.2X-",(int)pAdapter->Address[i]);
			}

			TRACE(L"IP Address: %s\n",  A2W(pAdapter->IpAddressList.IpAddress.String));
			TRACE(L"IP Mask: %s\n",    A2W(pAdapter->IpAddressList.IpMask.String));
			TRACE(L"\tGateway: \t%s\n", A2W(pAdapter->GatewayList.IpAddress.String));
			TRACE(L"\t***\n");
			if (pAdapter->DhcpEnabled)
			{
				TRACE(L"\tDHCP Enabled: Yes\n");
				TRACE(L"\t\tDHCP Server: \t%s\n", A2W(pAdapter->DhcpServer.IpAddress.String));
			}
			else
				TRACE(L"\tDHCP Enabled: No\n");

			////
			if(strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				pAdaptorInfo = new NETWORK_ADAPTOR_INFO_ipv4;
				if(pAdaptorInfo)
				{
					pAdaptorInfo->iAdapterID	= pAdapter->Index;
					wcscpy_s(pAdaptorInfo->szName, 256 * 4, A2W(pAdapter->AdapterName));
					wcscpy_s(pAdaptorInfo->szDesc, 128 * 4, A2W(pAdapter->Description));
					swprintf(pAdaptorInfo->szMACAddress, L"%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
						pAdapter->Address[0], pAdapter->Address[1], pAdapter->Address[2],
						pAdapter->Address[3], pAdapter->Address[4], pAdapter->Address[5]);
					wcscpy_s(pAdaptorInfo->szIPAddress, 33, A2W(pAdapter->IpAddressList.IpAddress.String));
					wcscpy_s(pAdaptorInfo->szGateWay,   33, A2W(pAdapter->GatewayList.IpAddress.String));
					wcscpy_s(pAdaptorInfo->szMask,      33, A2W(pAdapter->IpAddressList.IpMask.String)); 
						
					pAdaptorInfo->bDHCP = pAdapter->DhcpEnabled;
					if(pAdaptorInfo->bDHCP)
					{
						wcscpy_s(pAdaptorInfo->szDHCPAddress, 33, A2W(pAdapter->DhcpServer.IpAddress.String));
					}

					m_vcAdaptorList.push_back(pAdaptorInfo);
				}
			}
			////
			pAdapter = pAdapter->Next;
		}

		if (pAdapterInfo)
			free(pAdapterInfo);
	}
	//////////////////////////
	return TRUE;
}

BOOL VHNetworkAdaptorInfo::ClearInformation()
{
	std::vector<NETWORK_ADAPTOR_INFO_ipv4*>::iterator it = m_vcAdaptorList.begin();
	while(it != m_vcAdaptorList.end())
	{
		if(*it)
		{
			delete *it;
		}
		it++;
	}
	m_vcAdaptorList.clear();
	return TRUE;
}

int  VHNetworkAdaptorInfo::GetInformationCounts()
{
	return m_vcAdaptorList.size();
}

NETWORK_ADAPTOR_INFO_ipv4* VHNetworkAdaptorInfo::GetInformation(int nIndex)
{
	if(nIndex < 0 || nIndex >= (int)m_vcAdaptorList.size())
		return NULL;

	return m_vcAdaptorList[nIndex];
}