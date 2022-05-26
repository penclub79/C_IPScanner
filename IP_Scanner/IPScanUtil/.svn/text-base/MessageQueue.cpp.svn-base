#include "StdAfx.h"
#include "MessageQueue.h"

CMessageQueue::CMessageQueue(void)
: m_iRear(0)
, m_iFront(0)
{
	m_iRear		= 0;
	m_iFront	= 0;
	m_hMutex	= NULL;
	::ZeroMemory( &m_aMessageQueueItem[0], sizeof(m_aMessageQueueItem) );
}

CMessageQueue::~CMessageQueue(void)
{
	UnInitQueue();
}

BOOL CMessageQueue::InitQueue(void)
{
	if( NULL != m_hMutex )
	{
		::CloseHandle( m_hMutex );
		m_hMutex	= NULL;
	}

	m_hMutex	= ::CreateMutex( NULL, FALSE, NULL );
	return false;
}

BOOL CMessageQueue::UnInitQueue(void)
{
	if( m_hMutex )
	{
		::CloseHandle( m_hMutex );
		m_hMutex	= NULL;
	}
	return false;
}

BOOL CMessageQueue::PushQueue(MESSAGE_QUEUE_ITEM QueueItem)
{
	DWORD	dwExitCode	= 0x00;

//	TRACE( "<< Push Queue \n" );

	if(!IsQueueFull())  // 값이 Full 이 아니면 큐에 값을 넣는 부분이다.
	{
		dwExitCode	= ::WaitForSingleObject( m_hMutex, 5000 );

		if( dwExitCode == WAIT_OBJECT_0 )
		{
			m_aMessageQueueItem[m_iRear] = QueueItem;
			m_iRear = (m_iRear+1)%MAX_MESSAGE_QUEUE_COUNT;  // 값을 넣은 다음 m_iRear 값에 m_iRear+1 사이즈의 나머지 값을 넣는다.

			::ReleaseMutex( m_hMutex );
//			TRACE( "Push Queue >>\n" );
			return true;
		}
	}

//	TRACE( "Push Queue >>\n" );
	return false;
}

BOOL CMessageQueue::PopQueue(MESSAGE_QUEUE_ITEM* QueueItem)
{
	DWORD	dwExitCode	= 0x00;
//	TRACE( "<< PopQueue \n" );

	if(!IsEmpty())  // 값이 있다면 큐에서 값을 빼는 부분이다.
	{
		dwExitCode	= ::WaitForSingleObject( m_hMutex, 5000 );

		if( dwExitCode == WAIT_OBJECT_0 )
		{
			memcpy( QueueItem, &m_aMessageQueueItem[m_iFront], sizeof(MESSAGE_QUEUE_ITEM) );
			m_iFront = (m_iFront+1)%MAX_MESSAGE_QUEUE_COUNT;  // 값을 뺀 다음 뺀위치를 저장 하는 부분이다.

			::ReleaseMutex( m_hMutex );
//			TRACE( "PopQueue >> \n" );
			return true;
		}
	}

//	TRACE( "PopQueue >> \n" );

	return false;
}

BOOL CMessageQueue::GetFirstQueue(MESSAGE_QUEUE_ITEM* QueueItem)
{
	DWORD	dwExitCode	= 0x00;

	if(!IsEmpty())  // 값이 있다면 큐에서 값을 빼는 부분이다.
	{
		dwExitCode	= ::WaitForSingleObject( m_hMutex, 5000 );

		if( dwExitCode == WAIT_OBJECT_0 )
		{
			memcpy( QueueItem, &m_aMessageQueueItem[m_iFront], sizeof(MESSAGE_QUEUE_ITEM) );

			::ReleaseMutex( m_hMutex );
			return true;
		}
	}

	return false;
}

int CMessageQueue::GetQueueCount(void)
{
	if( m_iRear >= m_iFront )
	{
		return m_iRear - m_iFront;
	}
	else
	{
		return MAX_MESSAGE_QUEUE_COUNT-m_iFront+m_iRear;
	}

	return 0;
}


BOOL CMessageQueue::IsQueueFull(void)
{
	BOOL	bResult		= FALSE;
	DWORD	dwExitCode	= 0x00;

	dwExitCode	= ::WaitForSingleObject( m_hMutex, 5000 );

	if( dwExitCode == WAIT_OBJECT_0 )
	{
		if((m_iRear+1)%MAX_MESSAGE_QUEUE_COUNT==m_iFront)	// 현재의 위치에 +1 한 값에 사이즈를 나눈 값이 뺄 위치와 같다면 
			bResult		= TRUE;								// 값이 꽉 찼다는 뜻이다.
		else
			bResult		= FALSE;           

		::ReleaseMutex( m_hMutex );
	}

	return bResult;
}

BOOL CMessageQueue::IsEmpty(void)
{
	BOOL	bResult		= FALSE;
	DWORD	dwExitCode	= 0x00;

	dwExitCode	= ::WaitForSingleObject( m_hMutex, 5000 );

	if( dwExitCode == WAIT_OBJECT_0 )
	{
		if(m_iRear==m_iFront)								// 현재의 위치와 사이즈의 나머지 값이 뺄 위치와 같다면 값이 없다는 뜻이다
			bResult		= TRUE;				
		else
			bResult		= FALSE;        

		::ReleaseMutex( m_hMutex );
	}
	return bResult;
}