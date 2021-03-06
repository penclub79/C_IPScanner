//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: Network Utility Implementation 
// Date: 2010-04-27
// Author: dcyoon
//

#include "stdafx.h"

CStreamQ::CStreamQ()
{
	m_pHead = m_pTail = NULL;
	m_hLock = CreateMutex(NULL, FALSE, NULL);
	m_nCount = 0; // 2011-07-18 hkeins : 프레임이 10개 이상밀리면 drop하도록 적용. performance update
}
CStreamQ::~CStreamQ()
{
	Clear();
	if ( m_hLock ) {
		ReleaseMutex(m_hLock);
		CloseHandle(m_hLock);
		m_hLock = NULL;
	}
}
void CStreamQ::Clear()
{
	WaitForSingleObject(m_hLock, INFINITE);
	if ( m_pHead ) {
		PSTREAMQ pItem = m_pHead;
		while ( pItem ) {
			m_pHead = m_pHead->pNext;
			SAFE_DELETE(pItem->pStream);
			delete pItem;
			pItem = m_pHead;
		}
		m_pHead = m_pTail = NULL;
	}
	m_nCount = 0;  // 2011-07-18 hkeins : 프레임이 10개 이상밀리면 drop하도록 적용. performance update
	ReleaseMutex(m_hLock);
}

void CStreamQ::Push(PSTREAM pStream, BYTE nType/*=0*/)
{
	WaitForSingleObject(m_hLock, INFINITE);
	if ( pStream )
	{
		m_nCount++;  // 2011-07-18 hkeins : 프레임이 10개 이상밀리면 drop하도록 적용. performance update
		PSTREAMQ pItem = new STREAMQ(pStream, nType);
		if ( pItem )
		{
			if ( m_pHead == NULL ) 
			{
				// first
				m_pHead = m_pTail = pItem;
			} 
			else 
			{
				m_pTail->pNext = pItem;
				m_pTail = pItem;
			}
		}
	}
	ReleaseMutex(m_hLock);
}
PSTREAM CStreamQ::Pop(BYTE &nType)
{
	WaitForSingleObject(m_hLock, INFINITE);
	PSTREAM pStream = NULL;
	if ( m_pHead ) {
		m_nCount--;  // 2011-07-18 hkeins : 프레임이 10개 이상밀리면 drop하도록 적용. performance update
		PSTREAMQ pItem = m_pHead;
		nType = pItem->nType;
		pStream = pItem->pStream;
		m_pHead = pItem->pNext;
		delete pItem;
	}
	ReleaseMutex(m_hLock);
	return pStream;	
}

int   CStreamQ::GetCount()
{
	return m_nCount;  // 2011-07-18 hkeins : 프레임이 10개 이상밀리면 drop하도록 적용. performance update
}

static u_char MACHINE_TYPE = BIG_ENDIAN;
void check_tom()
{
	u_long val = 0x01;
	if (val == htonl(val)) MACHINE_TYPE = BIG_ENDIAN;
	else MACHINE_TYPE = LITTLE_ENDIAN;
}

u_int nswap(u_int dw)
{
    union { 
        u_char b[4];   
        u_int  w;
    } s, d; 
    s.w = dw;
    d.b[0] = s.b[3]; 
    d.b[1] = s.b[2]; 
    d.b[2] = s.b[1]; 
    d.b[3] = s.b[0]; 
    return d.w;
}

u_long dwswap(u_long dw)
{
    union { 
        u_char b[4];   
        u_long w;
    } s, d; 
    s.w = dw;
    d.b[0] = s.b[3]; 
    d.b[1] = s.b[2]; 
    d.b[2] = s.b[1]; 
    d.b[3] = s.b[0]; 
    return d.w;
}

/* half word swapping byte odering */
u_short wswap(u_short w)
{
    union { 
        u_char  b[2];   
        u_short w;
    } s, d; 
    s.w = w;
    d.b[0] = s.b[1]; 
    d.b[1] = s.b[0]; 
    return d.w;
}

void e2null(u_char *buf, u_int *loc, u_int len)
{
	memset(buf+*loc, 0, len);
	*loc = *loc + len;
}
void d2null(u_char *buf, u_int *loc, u_int len)
{
	*loc = *loc + len;
}

void e2char(u_char *buf, u_int *loc, u_char val)
{
	memcpy(buf+*loc, &val, 1);
	*loc = *loc + 1;
}
void d2char(u_char *buf, u_int *loc, u_char *pval)
{
	memcpy(pval, buf+*loc, 1);
	*loc = *loc + 1;
}

void e2int(u_char *buf, u_int *loc, u_int val)
{
	u_int nt_val;
	if (MACHINE_TYPE == BIG_ENDIAN) nt_val = val;
	else nt_val = nswap(val);
	memcpy(buf+*loc, &nt_val, 4);
	*loc = *loc + 4;
}
void d2int(u_char *buf, u_int *loc, u_int *pval)
{
	u_int nt_val;
	memcpy(&nt_val, buf+*loc, 4);
	if (MACHINE_TYPE == BIG_ENDIAN) *pval = nt_val;
	else *pval = nswap(nt_val);
	*loc = *loc + 4;
}

void e2long(u_char *buf, u_int *loc, u_long val)
{
	u_long nt_val;
	if (MACHINE_TYPE == BIG_ENDIAN) nt_val = val;
	else nt_val = dwswap(val);
	memcpy(buf+*loc, &nt_val, 4);
	*loc = *loc + 4;
}
void d2long(u_char *buf, u_int *loc, u_long *pval)
{
	u_long nt_val;
	memcpy(&nt_val, buf+*loc, 4);
	if (MACHINE_TYPE == BIG_ENDIAN) *pval = nt_val;
	else *pval = dwswap(nt_val);
	*loc = *loc + 4;
}

void e2short(u_char *buf, u_int *loc, u_short val)
{
	u_short nt_val;
	if (MACHINE_TYPE == BIG_ENDIAN) nt_val = val;
	else nt_val = wswap(val);
	memcpy(buf+*loc, &nt_val, 2);
	*loc = *loc + 2;
}
void d2short(u_char *buf, u_int *loc, u_short *pval)
{
	u_short nt_val;
	memcpy(&nt_val, buf+*loc, 2);
	if (MACHINE_TYPE == BIG_ENDIAN) *pval = nt_val;
	else *pval = wswap(nt_val);
	*loc = *loc + 2;
}

void e2float(u_char *buf, u_int *loc, float val)
{
	u_int nt_val;
	memcpy(&nt_val, &val, 4);
	e2int(buf, loc, nt_val);
}
void d2float(u_char *buf, u_int *loc, float *pval)
{
	u_int nt_val;
	d2int(buf, loc, &nt_val);
	memcpy(pval, &nt_val, 4);
}

void e2double(u_char *buf, u_int *loc, double val)
{
	u_char *pval = (u_char *)&val;
	u_int hi, lo;
	/* encoding double into buffer */
	memcpy(&lo, pval, 4);
	memcpy(&hi, pval+4, 4);
	/* ajust webeye network ordering */
	if (MACHINE_TYPE == BIG_ENDIAN) { // no chage of ordering
		e2int(buf, loc, lo);
		e2int(buf, loc, hi);
	} else { // change of ordering
		e2int(buf, loc, hi);
		e2int(buf, loc, lo);
	}
}
void d2double(u_char *buf, u_int *loc, double *pval)
{
	u_char *cp;
	u_int hi, lo;
	/* ajust notwork ordering */
	if (MACHINE_TYPE == BIG_ENDIAN) { // no chage of ordering
		d2int(buf, loc, &lo);
		d2int(buf, loc, &hi);
	} else { // change of ordering
		d2int(buf, loc, &hi);
		d2int(buf, loc, &lo);
	}
	cp = (UCHAR *)pval;
	memcpy(cp, &lo, 4);
	memcpy(cp+4, &hi, 4);
}

void e2str(u_char *buf, u_int *loc, u_char *pval, int len) 
{
	memcpy(buf+*loc, pval, len);
	*loc = *loc + len;
}
void d2str(u_char *buf, u_int *loc, u_char *pval, int len)
{
	memcpy(pval, buf+*loc, len);
	*loc = *loc + len;
}

void e2tm(u_char *buf, u_int *loc, struct tm *pval)
{
	e2int(buf, loc, pval->tm_year);
	e2int(buf, loc, pval->tm_mon);
	e2int(buf, loc, pval->tm_mday);
	e2int(buf, loc, pval->tm_hour);
	e2int(buf, loc, pval->tm_min);
	e2int(buf, loc, pval->tm_sec);
	e2int(buf, loc, pval->tm_wday);
	e2int(buf, loc, pval->tm_yday);
	e2int(buf, loc, pval->tm_isdst);
}
void d2tm(u_char *buf, u_int *loc, struct tm *pval)
{
	d2int(buf, loc, (UINT *)&pval->tm_year);
	d2int(buf, loc, (UINT *)&pval->tm_mon);
	d2int(buf, loc, (UINT *)&pval->tm_mday);
	d2int(buf, loc, (UINT *)&pval->tm_hour);
	d2int(buf, loc, (UINT *)&pval->tm_min);
	d2int(buf, loc, (UINT *)&pval->tm_sec);
	d2int(buf, loc, (UINT *)&pval->tm_wday);
	d2int(buf, loc, (UINT *)&pval->tm_yday);
	d2int(buf, loc, (UINT *)&pval->tm_isdst);
}

//#ifndef _ONVIF_ENABLED_ // moved from $onviflib1/stdafx.cpp for compatiblility
////#include "onvif_client_defines.h"
////
////int ONVIF_VIDEO_SOURCE::m_nCount  = 0;
////int ONVIF_VIDEO_ENCODER::m_nCount = 0;
////int ONVIF_AUDIO_SOURCE::m_nCount = 0;
////int ONVIF_AUDIO_OUTPUT::m_nCount = 0;
////int ONVIF_AUDIO_ENCODER::m_nCount = 0;
////int ONVIF_PTZ::m_nCount = 0;
////int ONVIF_PROFILE::m_nCount = 0;
////int ONVIF_PROFILE_FULL::m_nCount = 0;
//#endif