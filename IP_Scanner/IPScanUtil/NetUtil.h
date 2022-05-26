//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: Network Utility Declaration
// Date: 2010-04-20
// Author: dcyoon
//
#pragma once

#include "NetDef.h"
#include <vector>
//#ifndef _ONVIF_ENABLED_
//#include "onvif_client_defines.h" // 2013-08-05 hkeins : ONVIF disable for DB test. moved from ../onviflib1/inc/onvif_client_defines.h
//#else
//#include "../onviflib1/inc/onvif_client_defines.h"
//#endif  _ONVIF_ENABLED_

#pragma pack(push, 1)

typedef struct tagVERINFO { // Verion
	DWORD	dwFilesys;	// filesystem info
	DWORD	dwFirmware;	// firmware info
	DWORD	dwChip;		// chip info
	DWORD	dwReserved;
} VERINFO, FAR *PVERINFO;

typedef struct tagSYSINFO { // System
	BYTE	byCamCount;
	BYTE	byAudioCount;
	BYTE	byTimezone;
	BYTE	byDaylight;
	DWORD	dwLocalTime;
	DWORD	dwReserved;
} SYSINFO, FAR *PSYSINFO;

typedef struct tagHDDINFO { // HDD
	BYTE	byHddCount;
	BYTE	byHddType;
	BYTE	byCD_RW;
	BYTE	byReserved;
	BYTE	pszHddName[8][4];
} HDDINFO, FAR *PHDDINFO;

typedef struct tagSTSINFO { // Status
	WORD	wLoss;
	WORD	wAlarm;
	WORD	wMotion;
	WORD	wRecord;
	BYTE	byNetwork;
	BYTE	byPlayback;
	BYTE	bySetup;
	BYTE	byBackup;
	BYTE	byHdd;
	BYTE	byFlow;
} STSINFO, FAR *PSTSINFO;
const UINT STSINFO_LEN	= sizeof(STSINFO);

////{{ 2011-03-17 hkeins : ONVIF information
//typedef struct tagONVIF_INFO
//{
//	char    szEndPointRef[255]; // uuid of device
//	char    szRtspAddress[1024];
//	char    szDeviceAddress[1024];
//	char    szDeviceIOAddress[1024];
//	char    szPTZAddress[1024];
//	std::vector<ONVIF_PROFILE_FULL*>*  pvcONVIF_PROFILES;     // std::vector<ONVIF_PROFILE_FULL*>
//	std::vector<ONVIF_PRESET_ITEM*>*   pvcONVIF_PRESET_ITEMS; // std::vector<ONVIF_PRESET_ITEM*>
//	std::vector<ONVIF_MEDIA_INFO*>*    pvcONVIF_MEDIAS;       // std::vector<ONVIF_MEDIA_INFO*>
//	std::vector<ONVIF_RELAY_INFO_ITEM*>* pvcONVIF_RELAY_ITEMS;
//
//	tagONVIF_INFO()
//	{
//		memset(szEndPointRef, 0, sizeof(szEndPointRef));
//		memset(szRtspAddress, 0, sizeof(szRtspAddress));
//		memset(szDeviceAddress, 0, sizeof(szDeviceAddress));
//		memset(szDeviceIOAddress, 0, sizeof(szDeviceAddress));
//		memset(szPTZAddress, 0, sizeof(szPTZAddress));
//		pvcONVIF_PROFILES     = NULL;
//		pvcONVIF_PRESET_ITEMS = NULL;
//		pvcONVIF_MEDIAS       = NULL;
//		pvcONVIF_RELAY_ITEMS  = NULL;
//	}
//} ONVIF_INFO, FAR *PONVIF_INFO;
////}}

typedef struct tagREQLOGIN {
	BYTE	byLevel;
	BYTE	bySpeed;
	BYTE	byCodec;	// 0 - First, 1 - Second stream
	BYTE	byReserved;
	BYTE	szID[8];
	BYTE	szPW[8];
} REQLOGIN, FAR *PREQLOGIN;

typedef struct tagRESLOGIN {
	BYTE	byLogin;
	VERINFO	ver;
	SYSINFO	sys;
	HDDINFO	hdd;
	STSINFO	sts;
	//ONVIF_INFO onvif_info; // 2011-03-17 hkeins : ONVIF information
} RESLOGIN, FAR *PRESLOGIN;

typedef struct tagREQCMD {
	DWORD	dwChannel;
	DWORD	dwRepTime;
	BYTE	byControl;
	BYTE	byLayout;
	BYTE	bySpeed;
	BYTE	byMode;
} REQCMD, FAR *PREQCMD;

typedef struct tagSTMHDR {
// 110224-dcyoon STMHDR modify
	BYTE byGOPID;   // AUDIO  : channels
	BYTE byCodec; 
	BYTE byFourCC;
	BYTE byStreamEncType;
	WORD nWd; 
	WORD nHi;// AUDIO nWd = bits per sample, nHi =  samples per sec
	UINT nSize;
	BYTE byFps;
} STMHDR, FAR *PSTMHDR;

const UINT STMHDR_LEN	= sizeof(STMHDR);

typedef struct tagSTREAM {
	STMHDR hdr;
	time_t tTime;
	BYTE* pData;
	tagSTREAM()
	{
		pData = NULL; 
		tTime = 0i64;
		ZeroMemory(&hdr,STMHDR_LEN);
	}
	~tagSTREAM() 
	{
		if(pData)
		{ 
			delete []pData;
			pData = NULL; 
		} // memory deallocator mismatch bug fix.
	}

	inline tagSTREAM& operator = (tagSTREAM& src)
	{
		memcpy(&(this->hdr),&src.hdr,STMHDR_LEN);
		if ( src.hdr.nSize ) 
		{
			this->pData = new BYTE[src.hdr.nSize + 8];
			if ( this->pData )
				memcpy(this->pData, src.pData, src.hdr.nSize);
		}
		return *this;
	}
} STREAM, FAR *PSTREAM;

typedef struct tagSTREAMQ {
	BYTE nType;
	PSTREAM pStream;
	tagSTREAMQ * pNext;
	tagSTREAMQ(PSTREAM pstream = NULL, BYTE ntype = 0)
	{
		pStream = pstream; nType = ntype; pNext = NULL;
	}
} STREAMQ, FAR *PSTREAMQ;

//// for RTSP support
//typedef struct tagRTSPBUF {
//	UINT	nSize, nPos, nErr;
//	BYTE	buf[RTSP_BUF_SIZE];
//	tagRTSPBUF()
//	{
//		Clear();
//	}
//	inline void Clear()
//	{
//		nSize = nPos = nErr = 0;
//		ZeroMemory(buf,RTSP_BUF_SIZE);
//	}
//} RTSPBUF, FAR *PRTSPBUF;
//
//typedef struct tagMediaSession {
//	int rtpsk, rtcpsk, timeout;
//	struct in_addr saddr; // server address
//	unsigned short sport, rtpport, rtcpport; // server / client port
//	unsigned char rtpcid, rtcpcid;//, payload;
//	CHAR *pszCtrl;
//	tagMediaSession()
//	{
//		rtpsk = rtcpsk = timeout = 0;
//		saddr.s_addr = 0;
//		sport = rtpport = rtcpport = 0;
//		rtpcid = rtcpcid = 0x00;//payload = 0x00;
//		pszCtrl = NULL;
//	}
//	~tagMediaSession()
//	{
//		if(pszCtrl){free(pszCtrl);pszCtrl=NULL;}
//	}
//} MSN, FAR *PMSN;
//
//typedef struct tagRTSPQ {
//	UINT nSize;
//	BYTE *pBuf;
//	tagRTSPQ * pNext;
//	tagRTSPQ(BYTE* buf = NULL, UINT size = 0)
//	{
//		pBuf = buf; nSize = size; pNext = NULL;
//	}
//} RTSPQ, FAR *PRTSPQ;

#pragma pack(pop)

class CStreamQ 
{
public:
	CStreamQ();
	virtual ~CStreamQ();
private:
	HANDLE m_hLock;
	PSTREAMQ m_pHead, m_pTail;
	int    m_nCount; // 2011-07-18 hkeins : performance update
public:
	void Clear(void);
	void Push(PSTREAM pStream, BYTE nType = 0);
	PSTREAM Pop(BYTE &nType);
	int   GetCount();
};


enum STREAM_ENC_T {
	STREAM_ENC_NONE			= 0x00,
	STREAM_ENC_BLOWFISH_32	= 0x01,
	STREAM_ENC_BLOWFISH_160	= 0x02
};

enum STREAM_T {
	STREAM_NONE = 0x00,
	STREAM_VIDEO = 0x01,
	STREAM_AUDIO = 0x02,
	STREAM_VIDEO2= 0x03, // NVR stream video
	STREAM_AUDIO2= 0x04, // NVR stream audio
};

//enum MSN_T { // RTSP Media Session
//	VID_S = 0x00,
//	AUD_S,
//};
//const UINT MAX_CMD		= 1024; // Rtsp Command

const UINT HDR_SIZE		= 40;
const UINT FSH_START	= 0x000001FF;

const BYTE SCALE_EXTEND = 0x00; // 2011-06-27 hkeins : extended resolution
const BYTE SCALE_HD1080	= 0x01;	// 1920 * 1080 (1080p)
const BYTE SCALE_SXGA	= 0x02;	// 1280 * 1024
const BYTE SCALE_HD720	= 0x03;	// 1280 * 720
const BYTE SCALE_D1		= 0x04;	// 720 * 480 (720 join)
const BYTE SCALE_D1P	= 0x14;	// 720 * 480 (720p)
const BYTE SCALE_HD1	= 0x05;	// 720 * 240
const BYTE SCALE_CIF	= 0x06;	// 352 * 240
const BYTE SCALE_QCIF	= 0x07;	// 176 * 144
const BYTE SCALE_VGA	= 0x08;	// 640 * 480 (640p)
const BYTE SCALE_QVGA	= 0x09;	// 320 * 240
const BYTE SCALE_QQVGA	= 0x0a;	// 160 * 120
const BYTE SCALE_2CIF	= 0x0c;	// 720 * 240 2 CIF Mode, SDVR22xxAM V4 Format
const BYTE SCALE_4CIF	= 0x0e;	// 720 * 480 4 CIF Mode, SDVR2204AL V1 Format
const BYTE SCALE_1600x1200 = 0x0f; // 1600x1200 4:3 mode

const BYTE CODEC_JPEG	= 0x00;
const BYTE CODEC_H264	= 0x01;
const BYTE CODEC_MPEG4  = 0x02;

const BYTE CODEC_MULAW   = 0x10;
const BYTE CODEC_ALAW    = 0x11;
const BYTE CODEC_G726    = 0x12; // 110809-hkeins : audio format added for ONVIF support
const BYTE CODEC_AAC     = 0x13;

const BYTE VFM_NTSC		= 0x00;
const BYTE VFM_PAL		= 0x01;

//const BYTE VRS_D1		= 0x02;
//const BYTE VRS_CIF	= 0x01;
//const BYTE VRS_HD1	= 0x00;

const BYTE VOP_P		= 0x00;
const BYTE VOP_I		= 0x01;

//const USHORT RTSP_PORT	= 554;
const USHORT SEARCH_PORT= 64554;
//const USHORT ONVIF_PORT = 80; // ONVIF use HTTP

const UINT WM_SOCKET				= WM_USER + 201;
const UINT WM_NET_MSG_DEVICE_STREAM	= WM_USER + 202;
const UINT WM_NET_ERR_DEVICE_STREAM	= WM_USER + 203;
const UINT WM_NET_MSG_DEVICE_EVENT	= WM_USER + 204;
const UINT WM_NET_ERR_DEVICE_EVENT	= WM_USER + 205;
const UINT WM_TWOWAYC_MSG			= WM_USER + 206;
const UINT WM_NET_REOPEN			= WM_USER + 207;

//const UINT WM_NET_ONVIF_CONNECTION	= WM_USER + 206;

// 2012-12-21 hkeins : NVR CS Client adding
const UINT WM_NET_MSG_SERVER		= WM_USER + 207;
const UINT WM_NET_ERR_SERVER		= WM_USER + 208;

////{{ 0609-hkeins : ONVIF preset add/remove
//// WM_COPY_DATA structure data defines
//const UINT  CPDATA_PRESET_ADD = 0x0300;
//const UINT  CPDATA_PRESET_DEL = 0x0301;

typedef struct tagST_PRESET_OP_INFO
{
	int   nPresetIndex;
	WCHAR szSvrName[33];
	WCHAR szPresetName[66];
	WCHAR szPresetToken[66];

	tagST_PRESET_OP_INFO()
	{
		Clear();
	}
	~tagST_PRESET_OP_INFO()
	{
		Clear();
	}
	void Clear()
	{
		nPresetIndex = 0;

		memset(szSvrName, 0, sizeof(szSvrName));
		memset(szPresetName, 0, sizeof(szPresetName));
		memset(szPresetToken,0, sizeof(szPresetToken));
	}
}ST_PRESET_OP_INFO, *LPST_PRESET_OP_INFO;
//}}

#define SEND_MSG(p,m,w,l) { if((p)&&(p)->GetSafeHwnd()){(p)->SendMessage(m,(WPARAM)(w),(LPARAM)(l));} }
#define POST_MSG(p,m,w,l) { if((p)&&(p)->GetSafeHwnd()){(p)->PostMessage(m,(WPARAM)(w),(LPARAM)(l));} }
typedef void (*stream_cb_func)(void* object, unsigned char type, void* data);

const u_char BIG_ENDIAN		= 0x01;
const u_char LITTLE_ENDIAN	= 0x00;
extern "C" {
	void check_tom();

	u_int	nswap(u_int dw);
	u_long	dwswap(u_long dw);
	u_short	wswap(u_short w);

	void e2null(u_char *buf, u_int *loc, u_int len);
	void d2null(u_char *buf, u_int *loc, u_int len);
	void e2char(u_char *buf, u_int *loc, u_char val);
	void d2char(u_char *buf, u_int *loc, u_char *pval);
	void e2int(u_char *buf, u_int *loc, u_int val);
	void d2int(u_char *buf, u_int *loc, u_int *pval);
	void e2long(u_char *buf, u_int *loc, u_long val);
	void d2long(u_char *buf, u_int *loc, u_long *pval);
	void e2short(u_char *buf, u_int *loc, u_short val);
	void d2short(u_char *buf, u_int *loc, u_short *pval);
	void e2float(u_char *buf, u_int *loc, float val);
	void d2float(u_char *buf, u_int *loc, float *pval);
	void e2double(u_char *buf, u_int *loc, double val);
	void d2double(u_char *buf, u_int *loc, double *pval);
	void e2str(u_char *buf, u_int *loc, u_char *pval, int len);
	void d2str(u_char *buf, u_int *loc, u_char *pval, int len);
	void e2tm(u_char *buf, u_int *loc, struct tm *pval);
	void d2tm(u_char *buf, u_int *loc, struct tm *pval);
}
