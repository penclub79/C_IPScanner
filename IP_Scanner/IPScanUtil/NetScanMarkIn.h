#pragma once

//enum MARKIN_VERSION_PROTOCAL {
//	MARKIN_VERSION_1 = 0x00,
//	MARKIN_VERSION_2
//};
//
//const UINT32 SCAN_INFO_m_pReceive_buffer_SIZE = 64 * 1024; // 64 Kbytes temp buffer for receive temporary data
//const UINT32 SCAN_ERR_NONE = 0x00000000;
//const UINT32 SCAN_ERR_SOCKET_OPT = 0x00000001;
//const UINT32 SCAN_ERR_BIND = 0x00000002;
//const UINT32 SCAN_ERR_MEMORY = 0x00000003;
//const UINT32 SCAN_ERR_RECV = 0x00000004;

// pragma pack(push, 1) ~ pragma pack(pop) 범위까지 최소 단위는 1바이트로 표현된다.
// 컴파일러에 의해서 다르지만 __attribute__((packed)) 이랑 같은 문법이다.


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

#pragma pack(pop)

typedef struct _HEADER_BODY
{
	_PACKET_HEADER	stPacket;
	_DEVICE_INFO	stDevInfo;
}HEADER_BODY;

class CNetScanMarkIn : public NetScanBase
{
private:
	SOCKET	m_hSockReceive;
	char*	m_pszPacketBuff;
	void	ToBigEndian(HEADER_BODY* _pstReceiveData); // Little -> Big Endian
	void	ConversionNetInfo(unsigned char* _upszIp, char* _pszVal);
	void	ConversionMac(char* _pszMac, char* _pszVal);
	void	ConversionVersion(VER_INFO* _pszVer, char* _pszVal);
	void	ConversionModelName(char* _pszModel, char* _pszVal);
	
protected:
	static DWORD thrMarkInScanThread(LPVOID pParam);

public:
	CNetScanMarkIn();
	~CNetScanMarkIn(void);

	//////////////////////////////////////////////////////////// Function
	virtual BOOL StartScan();
	virtual BOOL SendScanRequest();
	virtual BOOL StopScan();

	void	thrMarkInReceiver();
	void	SetBindAddress(ULONG _ulBindAddress);
	void	SetNotifyWindow(HWND hWnd, LONG msg);
	void	SetCloseMsgRecvWindow(HWND hWnd, LONG msg/* = WM_CLOSE*/);
	
	void	SocketBind();
	void	SetScanPort();
	//////////////////////////////////////////////////////////// ---------/
};