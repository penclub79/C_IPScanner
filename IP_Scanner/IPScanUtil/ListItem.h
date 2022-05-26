#pragma once

typedef struct tagListItem 
{
	tagListItem *	pNext;
	LPVOID			pData;
	tagListItem()
	{
		pData = NULL; 
		pNext = NULL;
	}
} LISTITEM, FAR *PLISTITEM;

class CListItem
{
public:
	CListItem(void);
	~CListItem(void);

private:
	int			m_nCount;
	PLISTITEM	m_pHead;
	PLISTITEM	m_pTail;
public:
	int			GetCount() { return m_nCount; }
	void		DeleteAllItems(void);
	int			Insert(PLISTITEM pItem);
	int			Delete(int nCount);
	PLISTITEM	GetList() { return m_pHead; };
};
