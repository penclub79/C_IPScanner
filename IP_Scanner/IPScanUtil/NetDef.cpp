//
// Copyright (C) Since 2010 VISIONHITECH. All rights reserved.
// 
// Description: Network Define Implementation 
// Date: 2010-04-27
// Author: dcyoon
//

#include "stdafx.h"

CReqQueue::CReqQueue()
{
	m_pHead		= NULL;
	m_pTail		= NULL;
	m_nCount	= 0;
}
CReqQueue::~CReqQueue()
{
	Clear();
}

void CReqQueue::Clear()
{
	if ( m_pHead ) 
	{
		PREQ pItem = m_pHead;
		while ( pItem )
		{
			m_pHead = m_pHead->pNext;
			delete pItem;
			pItem = m_pHead;
		}
		
		m_pHead = NULL;
		m_pTail = NULL;
	}
	m_nCount = 0;
}

void CReqQueue::Push(PREQ pItem, BOOL bFirst/*=FALSE*/)
{
	if ( pItem ) 
	{
		if ( m_pHead == NULL ) 
		{
			// first
			m_pHead = m_pTail = pItem;
		} 
		else if ( bFirst )
		{
			pItem->pNext = m_pHead;
			m_pHead = pItem;
		} 
		else
		{
			m_pTail->pNext = pItem;
			m_pTail = pItem;
		}
		m_nCount++;
	}
}

PREQ CReqQueue::Pop()
{
	PREQ pItem = m_pHead;
	if ( pItem )
	{
		m_pHead = pItem->pNext;
		m_nCount--;
	}
	
	return pItem;
}

int  CReqQueue::Size()
{
	return m_nCount;
}