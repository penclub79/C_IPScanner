#include "StdAfx.h"
#include "ListItem.h"



// CListItem
CListItem::CListItem()
: m_nCount(0)
{
	m_pHead = m_pTail = NULL;
}
CListItem::~CListItem()
{
	DeleteAllItems();
}
void CListItem::DeleteAllItems()
{
	PLISTITEM pItem = m_pHead;
	while ( pItem ) {
		m_pHead = m_pHead->pNext;
		delete pItem;
		pItem = m_pHead;
	}
	m_pHead = m_pTail = NULL;
	m_nCount = 0;
}

int CListItem::Insert(PLISTITEM pItem)
{
	if ( pItem == NULL )
		return -1;

	if ( m_pHead == NULL ) 
	{
		m_pHead = m_pTail = pItem;
	} 
	else
	{
		m_pTail->pNext = pItem;
		m_pTail = pItem;
	}

	m_nCount++;
	return (m_nCount-1);
}

int CListItem::Delete(int nCount)
{
	PLISTITEM pItem = m_pHead;
	while ( pItem ) {
		delete pItem;
		nCount--; m_nCount--;
		if ( !nCount ) break;
		pItem = m_pHead = m_pHead->pNext;
	}
	return (m_nCount-1);
}