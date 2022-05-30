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

#pragma pack(push, 1)

// Protocol Header
typedef struct _PACKET_HEADER
{
	unsigned int uiCommand;
	unsigned int uiReserved1;
	unsigned int uiReserved2;
}PACKET_HEADER;

// User Info
typedef struct _USER_INFO{	char user_id[32];
	char user_pw[32];

}USER_INFO;

// Version Info
typedef struct _VER_INFO {
	char szMajor;
	char szMinor;
	char szRevision;
}VER_INFO;

// Network Info
typedef struct _NET_INFO {
	unsigned char	uszNetwork_type; // 1:Static, 2:DHCP
	char			szMac_address[32];
	unsigned char	uszIp[4];
	unsigned char	uszSubnet[4];
	unsigned char	uszGateway[4];
	unsigned int	uiHttp_port;
	unsigned int	uiBase_port;

}NET_INFO;
// Device Info
typedef struct _DEVICE_INFO
{
	unsigned char	uszDevice_type;	// 2: Camera, 6: NVR
	unsigned char	uszMax_channel;
	char			szModel_name[32];
	NET_INFO		stNetwork_info;
	VER_INFO		stSw_version;
	VER_INFO		stHw_version;
	unsigned int	uiReserved1;
	unsigned int	uiReserved2;

}DEVICE_INFO;

#pragma pack(pop)

typedef struct _HEADER_BODY
{
	_PACKET_HEADER stPacket;
	_DEVICE_INFO   stDevInfo;
}HEADER_BODY;

class CNetScanMarkIn
{

public:
	CNetScanMarkIn();
	~CNetScanMarkIn(void);

	BOOL StartScan();
	BOOL StopScan();
	void thrMarkInReceiver();
	void SetBindAddress(ULONG _ulBindAddress);
	BOOL SendScanRequest();

protected:
	

	
private:
	HANDLE	m_hScanThread;		// Thread Handle
	DWORD	m_dwScanThreadID;	// Tread ID
	SOCKET	m_hSockReceive;
	HWND	m_hNotifyWnd;
	HWND	m_hCloseMsgRecvWnd;
	LONG	m_lNotifyMsg;
	LONG	m_lCloseMsg;
	ULONG	m_ulBindAddress;
	BOOL	m_bUserCancel;
	char*	m_pReceiverBuff;

	void ToBigEndian(HEADER_BODY* _pstReceiveData); // Little -> Big Endian
};