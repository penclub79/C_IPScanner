//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: Network Define
// Date: 2010-04-20
// Author: dcyoon
//
#pragma once

enum MODE_T {
	MODE_NONE	= 0x00,
	MODE_LIVE	= 0x01,
	MODE_PB		= 0x02,
	MODE_PBFAST	= 0x04,
	MODE_BACKUP	= 0x08,
	MODE_SETUP	= 0x10,
	MODE_FILE	= 0x20,
	MODE_AUDIO	= 0x40,
	MODE_EMAP	= 0x80
};
enum CMD_T {
	CMD_NONE					= 0x00,
	CMD_INFO					= 0x01,
	CMD_LOGIN					= 0x02,
	CMD_LIVE					= 0x03,
	CMD_STATUS_RECORD			= 0x04,
	CMD_OPEN					= 0x06,
	CMD_CLOSE					= 0x07,
	CMD_PAUSE					= 0x08,
	CMD_NVRSERVER_INFO 			= 0x09,
	CMD_EVENT_INFO				= 0x0a,
	CMD_NVRSERVER_SETUP_DATA	= 0x0b, // 2013-02-15 hkeins : Setup load/save add
	CMD_NVRSERVER_SAVE_RESULT	= 0x0c,
	//{{ 2013-06-03 hkeins : ported from ActiveX
	CMD_ABSOLUTEMOVE_COMPLETE	= 0x0d,
	CMD_NET_FPS					= 0x0e,              // FPS test MSG
	CMD_NET_CONNECTED			= 0x0f,         // Connected MSG
	//}}
	CMD_PLAYBACK				= 0x11,
	CMD_RSEARCH					= 0x12,
	CMD_ESEARCH					= 0x13,
	//
	CMD_BACKUP					= 0x21,
	CMD_SETUP					= 0x31,
	CMD_UPGRADE					= 0x51,
	CMD_USERDEF					= 0x91,
	CMD_FINISH					= 0x99,
	CMD_ERROR					= 0xEE
};

enum ACT_T {
	ACT_NONE	= 0x00,
	ACT_RUN		= 0x01,
	//
	ACT_OPEN	= 0x03,
	ACT_CLOSE	= 0x04,
	//
	ACT_AUDIO	= 0x10,
	ACT_PLAY	= 0x11,
	ACT_STEP	= 0x12,
	ACT_FF		= 0x13,
	ACT_STOP	= 0x14,
	ACT_PAUSE	= 0x15,
	ACT_BACKPLAY= 0x16,
	ACT_BACKSTEP= 0x17,
	ACT_REW		= 0x18,
	ACT_STATUS	= 0x19,
	//
	ACT_INFO	= 0x21,
	ACT_ERROR	= 0x22,
	//
	ACT_LEFT	= 0x31,
	ACT_RIGHT	= 0x32,
	ACT_UP		= 0x33,
	ACT_DOWN	= 0x34,
	ACT_ZOOMIN	= 0x35,
	ACT_ZOOMOUT	= 0x36,
	ACT_FOCUSFAR= 0x37,
	ACT_FOCUSNEAR= 0x38,
	ACT_RESET	= 0x39,
	//
	ACT_UPGRADE	= 0x51,	// F/W UPGRADE
	ACT_SOS		= 0x52,	// Send file size
	ACT_SOF		= 0x53,	// Send file
	ACT_EOF		= 0x54,	// Complete sending file
	//
	ACT_RECON	= 0x71,
	ACT_RECOFF	= 0x72,
	//
	ACT_ACK		= 0x91,
	ACT_NACK	= 0x92
};

enum PTZ_T {
	IRIS_OPEN      = 0,
	IRIS_CLOSE     = 1,
	ZOOM_TELE      = 2,
	ZOOM_WIDE      = 3,
	FOCUS_NEAR     = 4,
	FOCUS_FAR      = 5,
	FOCUS_STOP     = 6,
	MOVE_STOP      = 7,
	MOVE_UP        = 8,
	MOVE_DOWN      = 9,
	MOVE_LEFT      =10,
	MOVE_RIGHT     =11,
	MOVE_LEFTUP    =12,
	MOVE_LEFTDOWN  =13,
	MOVE_RIGHTUP   =14,
	MOVE_RIGHTDOWN =15,
	AUTO_FOCUS     =16, // 2012-09-07 hkeins : Auto focus 버튼 추가
	PRESET_GOTO    =17,
	PRESET_SET     =18,
	PRESET_DEL     =19,
	RELAY_CTRL     =20,
	MOVE_ABSOLUTE  =21, // 2013-06-03 hkeins : port from ActiveX
	MOVE_RELATIVE  =22,
	GET_PTZPOSITION=23,
};

enum NETERR_T {
	ERR_NONE				= 0x00,
	ERR_MAGIC_CODE			= 0x01,
	ERR_BODY_SIZE			= 0x02,
	ERR_CONNECT_FAIL		= 0x03,
	ERR_DISCONNECTED		= 0x04,
	ERR_TIMEOUT				= 0x05,
	ERR_LOGIN_FAIL			= 0x06,
	ERR_STREAM				= 0x07,
	ERR_PROTOCOL_TYPE		= 0x08,
	ERR_PROTOCOL_MODE		= 0x09, // 110621-hkeins : Connect status upgrade
	ERR_CONNECTING			= 0x0A, // connecting
	ERR_CONNECTED			= 0x0B, // connected
	ERR_RECONNECTING		= 0x0C, // reconnecting
	ERR_RECONNECTED			= 0x0D, // reconnected
	ERR_RECONNECT_FAILED	= 0x0E,	// reconnect failed
	ERR_RECONNECT_STREAM	= 0x0F,	// 20110829-hkeins : reconnect stream
	ERR_STREAM_STOPPED		= 0x10,	// 2012-08-28 hkeins : Stream stopped
	ERR_STREAM_RESTART		= 0x11,	// 2012-08-28 hkeins : Stream restart
	ERR_PRESET_ADD_OK		= 0xe0, // 110608-hkeins : ONVIF preset add success/add fail value add
	ERR_PRESET_ADD_FAIL		= 0xe1,
	ERR_PRESET_DEL_OK		= 0xe2,
	ERR_PRESET_DEL_FAIL		= 0xe3,
	ERR_NO_PERMISSION		= 0xe4,
	ERR_LOGIN_MAX			= 0xe5,
};

enum RES_T {
	RES_NONE	= 0x00,
	RES_HEADER	= 0x01,
	RES_BODY	= 0x02
};

const USHORT MAGIC_CODE	= 0x5471;
// 2010-09-07 hkeins : AxLive에서 해상도를 늘렸을 경우 화면이 멈추는 문제 때문에 64 --> 512Kb로 네트워크 버퍼 크기를 늘림
const UINT NET_BUF_SIZE = 1024 * 1024; // 2012-08-09 hkeins : 3M 및 저조도에서 프레임 크기가 커질 경우를 대비하기 위해 네트워크 버퍼 크기를 512 KB -> 1024 KB(1 MB)로 변경
const UINT RTSP_BUF_SIZE = 4 * 1024;
const UINT AUD_LEN = 512;

#pragma pack(push, 1)

typedef struct tagHEADER {
	WORD	magic;
	BYTE	byCmd;
	BYTE	byMode;
	UINT	nSize;
} HEADER, FAR *PHEADER;

typedef struct tagNETBUF {
	BYTE	byCmd;
	BYTE	byMode;
	BYTE	byType;
	BYTE	byError;
	UINT	nBodySize;
	UINT	nSize;
	UINT	nPos;
	BYTE	buf[NET_BUF_SIZE];
} NETBUF, FAR *PNETBUF;

typedef struct tagREQ {
	UINT	nSize;
	BYTE*	pPacket;
	tagREQ*	pNext;
	tagREQ() 
	{
		nSize = 0; pPacket = NULL; pNext = NULL;
	}

	~tagREQ() 
	{
		//if(pPacket){free(pPacket);pPacket=NULL;}
		if(pPacket)
		{
			delete[] pPacket;
			pPacket=NULL;
		}
	}
} REQ, FAR *PREQ;

typedef struct tagRES 
{
	BYTE byCmd;
	BYTE* pBuf;

	tagRES(BYTE _byCmd, BYTE* _pBuf = NULL)
	{
		byCmd	= _byCmd, 
		pBuf	= _pBuf;
	}

	~tagRES()
	{
		if(pBuf)
		{ 
			delete[] pBuf;
			pBuf=NULL;
		}
	}
} RES, FAR *PRES;

#pragma pack(pop)

const UINT HEADER_SIZE	= (UINT)sizeof(HEADER);

class CReqQueue
{
public:
	CReqQueue();
	virtual ~CReqQueue();
private:
	PREQ m_pHead, m_pTail;
	int  m_nCount;
public:
	void Clear(void);
	void Push(PREQ pItem, BOOL bFirst = FALSE);
	PREQ Pop(void);
	int  Size();  // 2012-04-20 hkeins : 아이템 개수 리턴
};
