#pragma once

#include "NetScanBase.h"

#pragma pack(push, 1)

// Protocol Header
typedef struct _PACKET_HEADER
{
	unsigned int uiCommand;
	unsigned int uiReserved1;
	unsigned int uiReserved2;

}PACKET_HEADER;

// User Info
typedef struct _USER_INFO
{
	char aszUser_id[32];
	char aszUser_pw[32];

}USER_INFO;

// Version Info
typedef struct _VER_INFO {
	char szMajor;
	char szMinor;
	char szRevision;
}VER_INFO;

// Network Info
typedef struct _NET_INFO {
	unsigned char	szNetwork_type; // 1:Static, 2:DHCP
	char			szMac_address[32];
	unsigned char	aszIp[4];
	unsigned char	aszSubnet[4];
	unsigned char	aszGateway[4];
	unsigned int	uiHttp_port;
	unsigned int	uiBase_port;

}NET_INFO;

// Device Info
typedef struct _DEVICE_INFO
{
	unsigned char	szDevice_type;	// 2: Camera, 6: NVR
	unsigned char	szMax_channel;
	char			aszModel_name[32];
	NET_INFO		stNetwork_info; //53
	VER_INFO		stSw_version; //3
	VER_INFO		stHw_version;
	unsigned int	uiReserved1;
	unsigned int	uiReserved2;

}DEVICE_INFO;


typedef struct _HEADER_BODY
{
	_PACKET_HEADER	stPacket;
	_DEVICE_INFO	stDevInfo;
}HEADER_BODY;

#pragma pack(pop)



class CNetScanMarkIn : public NetScanBase
{
public:
	CNetScanMarkIn();
	~CNetScanMarkIn(void);

	//////////////////////////////////////////////////////////// Function
	virtual BOOL StartScan();
	virtual BOOL SendScanRequest();

	//////////////////////////////////////////////////////////// ---------/
		
protected:
	static DWORD thrMarkInScanThread(LPVOID pParam);
	void	thrMarkInReceiver();

private:

};