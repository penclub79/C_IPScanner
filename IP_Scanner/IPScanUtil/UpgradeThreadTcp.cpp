//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: Firmware upgrade by TCP class
//
// History:
//    2013-05-07 made by cgkim and source copy by hkeins
//
#include "stdafx.h"
#include "UpgradeThreadTcp.h"


CUpgradeThreadTcp::CUpgradeThreadTcp( HWND pParent, int nIndex, CString sIPAddress, int nPort, CString sUserName, CString sPassword,CString sFilePath,CString sFileName)
{
//	TRACE(L"###### CUpgradeThreadTcp Start! nIndex = %d\n", nIndex);
	m_hThreadUpgrade		= NULL;
	m_dwThreadUpgradeID		= 0;
	m_hThreadUpgradeEvent	= NULL;

	memset (&m_szDisplayMessage[0], 0, sizeof(m_szDisplayMessage) );


	m_iSelectMCUIndex		= 0;
	m_wMCUInfoCount			= 0;
	m_pMCUPackageFileInfo	= NULL;
	
	m_lSWPackageSize		= 0;
	memset (&m_SwPackageFileInfo, 0, sizeof(m_SwPackageFileInfo) );
	

	m_iPercentage			= 0;


	m_sFilePath				= sFilePath;
	m_sFileName				= sFileName;

	m_hThreadSocket			= NULL;
	m_dwThreadSocketID		= 0;

	m_pSockUpgrade			= INVALID_SOCKET;

	m_hwndParent			= pParent;
	m_nIndex				= nIndex;

	m_sIPAddress			= sIPAddress;
	m_nUpgradePort			= nPort;

	m_sUserName				= sUserName;
	m_sPassword				= sPassword;

	m_iProtocalVersion		= 0;
	m_iVersionCheckCount	= 0;
	m_iConnectCheckCount	= 0;
	m_iLastError			= 0;
	m_hDisocnnectLock       = CreateMutex(NULL, FALSE, NULL);

	


	// ==================================== Message Queue Initialize
	m_MessageQueue.InitQueue();	
	// ==================================== Message Queue Initialize

	m_hThreadUpgradeEvent	= ::CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hThreadUpgrade		= ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcUpgrade, this, 0, &this->m_dwThreadUpgradeID );	


//	TRACE(L"###### CUpgradeThreadTcp Start! \n");
}

CUpgradeThreadTcp::~CUpgradeThreadTcp(void)
{
	m_hwndParent	= NULL;
	DWORD exitCode	= 0;

	Disconnect();

	m_dwThreadUpgradeID	= 0;

	if( NULL != m_hThreadUpgrade )
	{
		if( NULL != m_hThreadUpgradeEvent )
		{
			SetEvent( m_hThreadUpgradeEvent );
		}

		if( WAIT_TIMEOUT == WaitForSingleObject( m_hThreadUpgrade, 100000 ) )
		{
			:: TerminateThread(m_hThreadUpgrade, exitCode);
		}

		if(m_hThreadUpgrade)
		{
			::CloseHandle(m_hThreadUpgrade);
			m_hThreadUpgrade = NULL;
		}

		if( m_hThreadUpgradeEvent )
		{
			::CloseHandle(m_hThreadUpgradeEvent);
			m_hThreadUpgradeEvent	= NULL;
		}
	}
	if(m_hDisocnnectLock)
	{
		ReleaseMutex(m_hDisocnnectLock);
		CloseHandle(m_hDisocnnectLock);
		m_hDisocnnectLock = NULL;
	}

	if( m_pMCUPackageFileInfo )
	{
		delete [] m_pMCUPackageFileInfo;
		m_pMCUPackageFileInfo	= NULL;
	}

	m_MessageQueue.UnInitQueue();
}

DWORD WINAPI CUpgradeThreadTcp::ProcUpgrade  ( LPVOID _lpParam )
{
	CUpgradeThreadTcp* pThis = (CUpgradeThreadTcp*)_lpParam;
	if(pThis == NULL)
		return 0;

	pThis->funcUpgrade();

	TRACE( L"Index = %d, Thread Exit ProcUpgrade ///////////////////////////////////////////\n",pThis->m_nIndex );
	return 0xAAAAAAAA;
}

BOOL CUpgradeThreadTcp::funcUpgrade()
{
	//	TRACE(L"###### funcUpgrade Start!\n");
	BOOL bReturn = FALSE;
	MESSAGE_QUEUE_ITEM		QueueItemMain							= {0};
	int  i = 0;

	while( 0 < m_dwThreadUpgradeID )
	{
		if( WAIT_TIMEOUT == WaitForSingleObject( m_hThreadUpgradeEvent, 1000 ) )
		{
			continue;
		}

		if( TRUE == m_MessageQueue.IsEmpty() )
		{
			if( 0 == m_dwThreadUpgradeID )
				break;

			Sleep(500);
			continue;
		}
		else
		{
			m_MessageQueue.PopQueue( &QueueItemMain );

			switch( QueueItemMain.iCommandID )
			{
			case MESSAGE_CONNECT_FOR_SW_UPGRADE_WITHOUT_MCU:
				{
					TRACE( L"Index = %d, MESSAGE_CONNECT_FOR_SW_UPGRADE_WITHOUT_MCU\n", m_nIndex );

					if( 1024 > m_nUpgradePort )
					{
						if( NULL != m_hwndParent )
						{
							wsprintf( m_szDisplayMessage, L"Failed(This IP camera does not support the upgrade file.)" );
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
								(LPARAM)_UPGRADE_ERROR_NOT_SUPPORT_UPGRAE_FILE );

							return FALSE;
						}
					}
					else
					{
						if( TRUE == ConnectForSWUpgradeWithoutMCU() )
						{
							if( NULL != m_hwndParent )
							{
								wsprintf( m_szDisplayMessage, L"Ready to File S/W Transfering" );
								PostMessage(m_hwndParent, 
									WM_UPGRADE_MSG, 
									MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
									(LPARAM)_UPGRADE_STATUS_READY_TO_SW_FILE_TRANSFERRING );
							}
						}
						else
						{
							if( NULL != m_hwndParent )
							{
								wsprintf( m_szDisplayMessage, L"Failed(Can not connect.)" );
								PostMessage(m_hwndParent, 
									WM_UPGRADE_MSG, 
									MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
									(LPARAM)_UPGRADE_ERROR_CONNECT_FAILED);
							}
						}
					}
				}
				break;
			case MESSAGE_CONNECT_FOR_START		:
				{
					TRACE( L"Index = %d, MESSAGE_CONNECT_FOR_START\n", m_nIndex );

					if( 1024 > m_nUpgradePort )
					{
						if( NULL != m_hwndParent )
						{
							wsprintf( m_szDisplayMessage, L"Failed(This IP camera does not support the upgrade file.)" );
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
								(LPARAM)_UPGRADE_ERROR_NOT_SUPPORT_UPGRAE_FILE );

							return FALSE;
						}
					}
					else
					{
						if( TRUE == ConnectForStart() )
						{
						}
						else
						{
							if( NULL != m_hwndParent )
							{
								wsprintf( m_szDisplayMessage, L"Failed(Can not connect.)" );
								PostMessage(m_hwndParent, 
									WM_UPGRADE_MSG, 
									MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
									(LPARAM)_UPGRADE_ERROR_CONNECT_FAILED);
							}
						}
					}
				}
				break;
			case MESSAGE_CONNECT_FOR_RESTART:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_CONNECT_FOR_RESTART\n", m_nIndex );

					if( 1024 > m_nUpgradePort )
					{
						if( NULL != m_hwndParent )
						{
							wsprintf( m_szDisplayMessage, L"Failed(This IP camera does not support the upgrade file.)" );
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
								(LPARAM)_UPGRADE_ERROR_NOT_SUPPORT_UPGRAE_FILE );

							return FALSE;
						}
					}
					else
					{
						if( FALSE == ConnectForStart() )
						{
							if( MAX_REBOOT_CHECK_COUNT > m_iConnectCheckCount )
							{
								MESSAGE_QUEUE_ITEM QueueItem	= {0};
								QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_RESTART;
								m_MessageQueue.PushQueue( QueueItem );
								SetEvent(m_hThreadUpgradeEvent);
								Sleep(500);
								TRACE( L"Index = %d, MESSAGE_CONNECT_FOR_RESTART>>>>>>>>>>\n", m_nIndex );
							}
							else
							{
								if( NULL != m_hwndParent )
								{
									wsprintf( m_szDisplayMessage, L"Failed(Can not connect.)" );
									PostMessage(m_hwndParent, 
										WM_UPGRADE_MSG, 
										MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
										(LPARAM)_UPGRADE_ERROR_CONNECT_FAILED);
								}
							}

							m_iConnectCheckCount++;
						}
						else
						{
							m_iConnectCheckCount	= 0;
						}
					}

					TRACE( L"Index = %d, MESSAGE_CONNECT_FOR_RESTART>>>>>>>>>>\n", m_nIndex );
				}
				break;

			case MESSAGE_WAIT_FOR_RETRAY_UPGRADE:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_RETRAY_UPGRADE\n", m_nIndex );
					Disconnect();

					m_iConnectCheckCount	= 1;

					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Wait for Retry Upgrade" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_ATTEMPT_RETRY );

						m_iPercentage	= 0;
						PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
					}

					for( int i=0; i<10; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_RETRAY_UPGRADE %d \n", m_nIndex, i );
						if( 0 == m_dwThreadUpgradeID )
							break;
					}

					MESSAGE_QUEUE_ITEM QueueItem	= {0};
					QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_RESTART;
					m_MessageQueue.PushQueue( QueueItem );
					SetEvent(m_hThreadUpgradeEvent);

					TRACE( L"Index = %d, MESSAGE_WAIT_FOR_RETRAY_UPGRADE>>>>>>>>>>\n", m_nIndex );
				}
				break;
			case MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST_MCU:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST_MCU\n", m_nIndex );
					Disconnect();

					m_iConnectCheckCount	= 1;

					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Reboot for the upgrade. Please wait." );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_RESTARTING_MCU );

						m_iPercentage	= 0;
						PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
					}

					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(120);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST_MCU %d \n", m_nIndex,  i );
						if( 0 == m_dwThreadUpgradeID )
							break;
					}

					MESSAGE_QUEUE_ITEM QueueItem	= {0};
					QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_RESTART;
					m_MessageQueue.PushQueue( QueueItem );
					SetEvent(m_hThreadUpgradeEvent);

					TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST_MCU>>>>>>>>>>\n", m_nIndex );
				}
				break;

			case MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST\n", m_nIndex );
					Disconnect();

					m_iConnectCheckCount	= 1;

					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(120);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST %d \n", m_nIndex,  i );
						if( 0 == m_dwThreadUpgradeID )
							break;
					}

					if( NULL != m_hwndParent )
					{
						if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
						{
							// Software package 만 있을때
							if( NULL != m_hwndParent )
							{
								wsprintf( m_szDisplayMessage, L"Reboot for the upgrade. Please wait." );
								PostMessage(m_hwndParent, 
									WM_UPGRADE_MSG, 
									MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
									(LPARAM)_UPGRADE_STATUS_RESTARTING_SW );

								m_iPercentage	= 0;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}

							MESSAGE_QUEUE_ITEM QueueItem	= {0};
							QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_RESTART;
							m_MessageQueue.PushQueue( QueueItem );
							SetEvent(m_hThreadUpgradeEvent);
						}
						else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
						{
							// MCU package 만 있을때
							// Soft Package 가 없으므로..대기 없음

							if( NULL != m_hwndParent )
							{
								wsprintf( m_szDisplayMessage, L"Reboot for the upgrade. Please wait." );
								PostMessage(m_hwndParent, 
									WM_UPGRADE_MSG, 
									MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
									(LPARAM)_UPGRADE_STATUS_RESTARTING_SW );

								m_iPercentage	= 0;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}


							MESSAGE_QUEUE_ITEM QueueItem	= {0};
							QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_RESTART;
							m_MessageQueue.PushQueue( QueueItem );
							SetEvent(m_hThreadUpgradeEvent);
						}
						else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
						{
							// Software/MCU package 있을때
							if( NULL != m_hwndParent )
							{
								wsprintf( m_szDisplayMessage, L"Reboot for the upgrade. Please wait." );
								PostMessage(m_hwndParent, 
									WM_UPGRADE_MSG, 
									MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
									(LPARAM)_UPGRADE_STATUS_RESTARTING_SW );

								m_iPercentage	= 50;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}

							MESSAGE_QUEUE_ITEM QueueItem	= {0};
							QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_SW_UPGRADE_WITHOUT_MCU;
							m_MessageQueue.PushQueue( QueueItem );
							SetEvent(m_hThreadUpgradeEvent);
						}
					}

					TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST>>>>>>>>>>\n", m_nIndex );
				}
				break;

			case MESSAGE_WAIT_FOR_REBOOT_NOT_ENOUGHMEMORY:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_REBOOT_NOT_ENOUGHMEMORY\n", m_nIndex );

					// reboot 명령 전송해야 합니다. 

					SendUpgradeReStart();
					Disconnect();

					m_iConnectCheckCount	= 1;

					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Reboot for the upgrade. Please wait." );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_RESTARTING_ERROR );
					}

					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_NOT_ENOUGHMEMORY %d \n", m_nIndex,  i );
						if( 0 == m_dwThreadUpgradeID )
							break;
					}

					MESSAGE_QUEUE_ITEM QueueItem	= {0};
					QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_RESTART;
					m_MessageQueue.PushQueue( QueueItem );
					SetEvent(m_hThreadUpgradeEvent);


					TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_NOT_ENOUGHMEMORY>>>>>>>>>>\n", m_nIndex );
				}
				break;

			case MESSAGE_CONNECT_FOR_VERIONCHECK	:
				{	
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_CONNECT_FOR_VERIONCHECK \n", m_nIndex );

					if( 1024 > m_nUpgradePort )
					{
						if( NULL != m_hwndParent )
						{
							wsprintf( m_szDisplayMessage, L"Failed(This IP camera does not support the upgrade file.)" );
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
								(LPARAM)_UPGRADE_ERROR_NOT_SUPPORT_UPGRAE_FILE  );

							return FALSE;
						}
					}
					else
					{
						if( FALSE == ConnectForCheckComplete() )
						{
							if( MAX_REBOOT_CHECK_COUNT > m_iVersionCheckCount )
							{
								MESSAGE_QUEUE_ITEM QueueItem	= {0};
								QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_VERIONCHECK;
								m_MessageQueue.PushQueue( QueueItem );
								SetEvent(m_hThreadUpgradeEvent);
								Sleep(500);
								TRACE( L"Index = %d, MESSAGE_CONNECT_FOR_VERIONCHECK>>>>>>>>>>\n", m_nIndex );

								m_iVersionCheckCount++;

								if( NULL != m_hwndParent )
								{
									wsprintf( m_szDisplayMessage, L"Attempts to connect" );
									PostMessage(m_hwndParent, 
										WM_UPGRADE_MSG, 
										MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
										(LPARAM)_UPGRADE_STATUS_ATTEMPT_REBOOT );
								}

							}
							else
							{
								if( NULL != m_hwndParent )
								{
									wsprintf( m_szDisplayMessage, L"Failed(In order to check the version, can not connect.)" );
									PostMessage(m_hwndParent, 
										WM_UPGRADE_MSG, 
										MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
										(LPARAM)_UPGRADE_ERROR_IN_ORDER_TO_VERSION_CHECKING_CAN_NOT_CONNECT );
								}
							}
						}
						else
						{
							m_iVersionCheckCount	= 0;
						}
					}

					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_CONNECT_FOR_VERIONCHECK \n", m_nIndex );
				}
				break;

			case MESSAGE_WAIT_FOR_REBOOT_VERSIONCHECK:
				{
					Disconnect();

					m_iVersionCheckCount	= 1;

					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_REBOOT_VERSIONCHECK\n", m_nIndex );

					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_VERSIONCHECK %d \n", m_nIndex, i );
						if( 0 == m_dwThreadUpgradeID )
							break;

						if( NULL != m_hwndParent )
						{
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
								(LPARAM)_UPGRADE_STATUS_WAITFOR_REBOOT );
						}
					}

					MESSAGE_QUEUE_ITEM QueueItem	= {0};
					QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_VERIONCHECK;
					m_MessageQueue.PushQueue( QueueItem );
					SetEvent(m_hThreadUpgradeEvent);

					TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_VERSIONCHECK>>>>>>>>>>\n", m_nIndex );
				}
				break;

			case MESSAGE_SEND_REQ_UPGRADE_START	:
				{
					// 인증이 끝나면 업그래이드 스타트를 한다. 
					if( TRUE ==  UpgradeStart(QueueItemMain.iArg1) )
					{
						Sleep(500);
					}
					else
					{
						Disconnect();

						if( NULL != m_hwndParent )
						{
							wsprintf( m_szDisplayMessage, L"Failed(This file can not be opened.)" );
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
								(LPARAM)_UPGRADE_ERROR_OPEN_ERROR );
						}

					}
				}
				break;
			case MESSAGE_WAIT_FOR_MCU_FILE_TRANSFERING:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_MCU_FILE_TRANSFERING\n", m_nIndex );
					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_MCU_FILE_TRANSFERING %d \n", m_nIndex,  i );
						if( 0 == m_dwThreadUpgradeID )
							break;
						if( 0 <  m_MessageQueue.GetQueueCount() )
							break;


						if( NULL != m_hwndParent )
						{
							if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
							{
								// Software package 만 있을때
								if( 10 >= m_iPercentage )
								{
									m_iPercentage = i/3;
									PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
								}
							}
							else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
							{
								// MCU package 만 있을때
								// Soft Package 가 없으므로..대기 없음
							}
							else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
							{
								// Software/MCU package 있을때
								if( 10 >= m_iPercentage )
								{
									m_iPercentage = i/3;
									PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
								}
							}

						}
					}

					if( NULL != m_hwndParent )
					{
						if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
						{
							// Software package 만 있을때
							if( 10 >= m_iPercentage )
							{
								m_iPercentage = 10;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}
						}
						else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
						{
							// MCU package 만 있을때
							// Soft Package 가 없으므로..대기 없음
						}
						else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
						{
							// Software/MCU package 있을때
							if( 10 >= m_iPercentage )
							{
								m_iPercentage = 10;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}
						}
					}
				}
				break;
			case MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING\n", m_nIndex );
					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING %d \n", m_nIndex, i );
						if( 0 == m_dwThreadUpgradeID )
							break;
						if( 0 <  m_MessageQueue.GetQueueCount() )
							break;


						if( NULL != m_hwndParent )
						{
							if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
							{
								// Software package 만 있을때
								if( 10 >= m_iPercentage )
								{
									m_iPercentage = i/3;
									PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
								}
							}
							else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
							{
								// MCU package 만 있을때
								// Soft Package 가 없으므로..대기 없음
							}
							else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
							{
								// Software/MCU package 있을때
								if( 60 >= m_iPercentage )
								{
									m_iPercentage = 50 + i/3;
									PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
								}
							}

						}
					}
					if( NULL != m_hwndParent )
					{
						if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
						{
							// Software package 만 있을때
							if( 10 >= m_iPercentage )
							{
								m_iPercentage = 10;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}
						}
						else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
						{
							// MCU package 만 있을때
							// Soft Package 가 없으므로..대기 없음
						}
						else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
						{
							// Software/MCU package 있을때
							if( 60 >= m_iPercentage )
							{
								m_iPercentage = 60;
								PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
							}
						}

					}
				}
				break;

			case MESSAGE_WAIT_FOR_UPGRADE_READY:
				{
					TRACE( L"Index = %d, <<<<<<<<< MESSAGE_WAIT_FOR_UPGRADE_READY\n", m_nIndex );
					for( int i=0; i<MAX_WAIT_CHECK_COUNT; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, MESSAGE_WAIT_FOR_UPGRADE_READY %d \n", m_nIndex, i );
						if( 0 == m_dwThreadUpgradeID )
							break;
						if( 0 <  m_MessageQueue.GetQueueCount() )
							break;

						if( NULL != m_hwndParent )
						{
							// 프로그래스 설정( 업그레이드 준비 )
							if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
							{
								// Software package 만 있을때
								if( 60 >= m_iPercentage )
								{
									m_iPercentage = 50 + i/3;
									PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
								}
							}
							else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
							{
								// MCU package 만 있을때
								// Soft Package 가 없으므로..대기 없음
							}
							else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
							{
								// Software/MCU package 있을때
								if( 80 >= m_iPercentage )
								{
									m_iPercentage = 80 + i/3;
									PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
								}
							}
						}
					}
				}
				break;

			case MESSAGE_UPGRADE_READY:
				{

				}
				break;

			case MESSAGE_SEND_UPGRADE_MCU_FILE_RESTART_WAIT:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Re-request file transfering" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_FILE_RETRANSFERING );
					}

					// 네트워크 라인에서 보내어지고 있는 데이터를 처리 할때까지 대기
					for( i=0; i<20; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, WAIT FOR ReTransfering %d \n", m_nIndex, i );
						if( 0 == m_dwThreadUpgradeID )
							break;
					}

					// 대기할 동안 보내어진 메시지를 버린다. 
					MESSAGE_QUEUE_ITEM	QueueItemTemp			= {0};
					int					iQueuecount				= m_MessageQueue.GetQueueCount();
					TRACE( L"Index = %d, Restart Queue Count = %d \n", m_nIndex, iQueuecount );

					for( i=0; i<iQueuecount; i++)
					{
						m_MessageQueue.GetFirstQueue( &QueueItemTemp );
						if( MESSAGE_SEND_UPGRADE_MCU_FILE_RESTART_WAIT == QueueItemTemp.iCommandID )
						{
							// 받은 데이터 중 제일 마지막에 받은넘을 기억한다. 
							m_MessageQueue.PopQueue( &QueueItemMain );
						}

						TRACE( L"Index = %d, Restart Queue Count = %d \n", m_nIndex, m_MessageQueue.GetQueueCount() );
					}

					iQueuecount	= m_MessageQueue.GetQueueCount();
					TRACE( L"Index = %d, Restart Queue Count = %d \n", m_nIndex, iQueuecount );

					MESSAGE_QUEUE_ITEM QueueItemUpgrade	= {0};
					QueueItemUpgrade.iCommandID		= MESSAGE_SEND_UPGRADE_MCU_FILE;
					QueueItemUpgrade.iArg1			= QueueItemMain.iArg1;
					QueueItemUpgrade.iArg2			= QueueItemMain.iArg2;
					m_MessageQueue.PushQueue( QueueItemUpgrade );
					SetEvent(m_hThreadUpgradeEvent);

					TRACE( L"Index = %d, Restart Index = %d \n", m_nIndex, QueueItemUpgrade.iArg1 );
				}
				break;

			case MESSAGE_SEND_UPGRADE_SW_FILE_RESTART_WAIT:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Re-request file transfering" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_FILE_RETRANSFERING );
					}

					// 네트워크 라인에서 보내어지고 있는 데이터를 처리 할때까지 대기
					for( i=0; i<20; i++ )
					{
						Sleep(500);
						TRACE( L"Index = %d, WAIT FOR ReTransfering %d \n", m_nIndex, i );
						if( 0 == m_dwThreadUpgradeID )
							break;
					}

					// 대기할 동안 보내어진 메시지를 버린다. 
					MESSAGE_QUEUE_ITEM	QueueItemTemp			= {0};
					int					iQueuecount				= m_MessageQueue.GetQueueCount();
					TRACE( L"Index = %d, Restart Queue Count = %d \n", m_nIndex, iQueuecount );

					for( i=0; i<iQueuecount; i++)
					{
						m_MessageQueue.GetFirstQueue( &QueueItemTemp );
						if( MESSAGE_SEND_UPGRADE_SW_FILE_RESTART_WAIT == QueueItemTemp.iCommandID )
						{
							// 받은 데이터 중 제일 마지막에 받은넘을 기억한다. 
							m_MessageQueue.PopQueue( &QueueItemMain );
						}

						TRACE( L"Index = %d, Restart Queue Count = %d \n", m_nIndex, m_MessageQueue.GetQueueCount() );
					}

					iQueuecount	= m_MessageQueue.GetQueueCount();
					TRACE( L"Index = %d, Restart Queue Count = %d \n", m_nIndex, iQueuecount );

					MESSAGE_QUEUE_ITEM QueueItemUpgrade	= {0};
					QueueItemUpgrade.iCommandID			= MESSAGE_SEND_UPGRADE_SW_FILE;
					QueueItemUpgrade.iArg1				= QueueItemMain.iArg1;
					m_MessageQueue.PushQueue( QueueItemUpgrade );
					SetEvent(m_hThreadUpgradeEvent);

					TRACE( L"Index = %d, Restart Index = %d \n", m_nIndex, QueueItemUpgrade.iArg1 );
				}
				break;

			case MESSAGE_SEND_UPGRADE_SW_FILE:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"File Transfering" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_FILE_TRANSFERING );
					}

					if(!SendUpgradeFile(QueueItemMain.iArg1))
					{
						Disconnect();
						wsprintf( m_szDisplayMessage, L"Failed (File open error)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_OPEN_ERROR  );
					}
				}
				break;

			case MESSAGE_UPGRADE_COMPLETE	:
				{
					Disconnect();

					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
						(LPARAM)_UPGRADE_STATUS_COMPLETE );
				}
				break;

			case MESSAGE_AUTHENTIFICATION_FAIL	:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Failed(Authentification)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_AUTH_FAILED );
					}

					Disconnect();
				}
				break;

			case MESSAGE_ERROR_INVALID_FIRMWARE	:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Failed(Invalid firmware)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_INVALID_FIRMWARE );
					}

					Disconnect();
				}
				break;

			case MESSAGE_ERROR_NO_MATCH_MODEL	:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Failed(No match model)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_NO_MATCH_MODEL );
					}

					Disconnect();
				}
				break;

			case MESSAGE_ERROR_NO_MATCH_VERSION	:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Failed(No match version)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_NO_MATCH_VERSION );
					}

					Disconnect();
				}
				break;

			case MESSAGE_ERROR_BAD_COMMAND		:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Upgrade is working in other places" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_UPGRADE_IS_WORKING_IN_OTHER_PLACE );
					}

					Disconnect();
				}
				break;
			case MESSAGE_ERROR_TIMEOUT:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Failed(Receive Timeout.)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_RECEIVE_TIME_OUT );
					}

					Disconnect();
				}
				break;

			case MESSAGE_SEND_REQ_UPGRADE_MCU_START:
				{
					if( TRUE == SendStartMCUUpgrade(QueueItemMain.iArg1, QueueItemMain.iArg1) )
					{
						MESSAGE_QUEUE_ITEM QueueItem	= {0};
						QueueItem.iCommandID		= MESSAGE_WAIT_FOR_MCU_FILE_TRANSFERING;
						m_MessageQueue.PushQueue( QueueItem );
						SetEvent(m_hThreadUpgradeEvent);
					}
					else
					{
						SendStopMcuUpgrade();

						wsprintf( m_szDisplayMessage, L"Failed(No match MCU model)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_NO_MATCH_MCU_MODEL );
					}
				}
				break;

			case MESSAGE_ERROR_PROTOCAL_MCU_INFO:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Failed(Don't support MCU upgrade)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_PROTOCOL_MCU_INFO_FAIL );
					}

					Disconnect();
				}
				break;
			case MESSAGE_SEND_UPGRADE_MCU_FILE:
				{
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"MCU File Transfering" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_MCU_FILE_TRANSFERING );
					}

					if(!SendMCUUpgradeFile(QueueItemMain.iArg1, QueueItemMain.iArg2))
					{
						Disconnect();
						wsprintf( m_szDisplayMessage, L"Failed (File open error)" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
							(LPARAM)_UPGRADE_ERROR_OPEN_ERROR  );
					}
				}
				break;
			case MESSAGE_ERROR_DISCONNECTED		:
				{
					if( 0 == m_iConnectCheckCount	&& 
						0 == m_iVersionCheckCount	&&
						0 == m_iLastError			)
					{
						if( NULL != m_hwndParent )
						{
							wsprintf( m_szDisplayMessage, L"Failed(Disconnected.)" );
							PostMessage(m_hwndParent, 
								WM_UPGRADE_MSG, 
								MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
								(LPARAM)_UPGRADE_ERROR_DISCONNECTED );
						}
					}

					Disconnect();
				}
				break;

			}

			if( 0 < m_MessageQueue.GetQueueCount() )
				SetEvent(m_hThreadUpgradeEvent );
		}

		Sleep( 1 );
	}

	return bReturn;
}

DWORD WINAPI CUpgradeThreadTcp::ProcReceiveSocket  ( LPVOID _lpParam )
{
	SOCKET_MODE_INFO*		pModeInfo	= (SOCKET_MODE_INFO*)_lpParam;
	CUpgradeThreadTcp*		pThis		= (CUpgradeThreadTcp*)pModeInfo->pThis;

	pThis->funcReceive(pModeInfo->iSocketMode);

	delete pModeInfo;
	pModeInfo	= NULL;

	TRACE( L"Index = %d, Thread Exit ProcReceiveSocket ///////////////////////////////////////////\n", pThis->m_nIndex );
	return 0xBBBBBBBB;
}


BOOL CUpgradeThreadTcp::funcReceive(int	iSocketMode)
{
	int							nReceiverAddrLen	= sizeof(SOCKADDR_IN);
	SOCKADDR_IN					ReceiverAddr;
	UPGRADE_HEADER				UpgradeHeader;
	UPGRADE_PROTOCOL_VERSION_RSP UpgradeVersionInfoRsp;
	UPGRADE_AUTH_RSP			UpgradeRspAuth;
	UPGRADE_FILE_STREAM_RSP		UpgradeFileStreamRsp;
	UPGRADE_NAND_WRITE_RSP		UpgradeNandWriteRsp;
	UPGRADE_SW_VERSION_RSP		UpgradeSWVersionRsp;
	UPGRADE_START_INFO_RSP		UpgradeStartInfoRsp;
	UPGRADE_MCU_INFO_RSP		UpgradeMCUInfoRsp;
	UPGRADE_RSP					UpgradeRsp;
	BOOL						bVersionReceive	= FALSE;
	int							iUpgradeStatus		= 0;

	SendProtocolVersion();

	while( 0 < m_dwThreadSocketID )
	{
		nReceiverAddrLen	= 0;
		if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof(UpgradeHeader), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
		{
			MESSAGE_QUEUE_ITEM QueueItem	= {0};
			QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
			QueueItem.iArg1				= WSAGetLastError();
			m_MessageQueue.PushQueue( QueueItem );
			SetEvent(m_hThreadUpgradeEvent);
			m_dwThreadSocketID	= 0;

			break;
		}

		if( FALSE == bVersionReceive )
		{
			if( MAGIC2_CODE				== UpgradeHeader.magic_code		&&
				PROTOCOL_TYPE_UPGRADE	== UpgradeHeader.protocol_type	)
			{
				switch ( UpgradeHeader.protocol_mode )
				{
				case _UPGRADE_RSP_PROTOCOL_VERSION:
					{
						if( sizeof(UPGRADE_PROTOCOL_VERSION_RSP) == UpgradeHeader.body_size )
						{
							if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeVersionInfoRsp, sizeof(UpgradeVersionInfoRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
							{
								MESSAGE_QUEUE_ITEM QueueItem	= {0};
								QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
								QueueItem.iArg1				= WSAGetLastError();
								m_MessageQueue.PushQueue( QueueItem );
								SetEvent(m_hThreadUpgradeEvent);
								m_dwThreadSocketID	= 0;
								break;
							}

							if( _UPGRADE_RSP_MSG_PROTOCOL_VERSION_SUCCESS == UpgradeVersionInfoRsp.messages )
							{
								bVersionReceive		= TRUE;
								m_iProtocalVersion	= UpgradeVersionInfoRsp.protocol_version;

								if( _UPGRADESOCKET_MODE_UPGRADE					== iSocketMode ||
									_UPGRADESOCKET_MODE_SWUPGRADE_WITHOUT_MCU	== iSocketMode )
								{
									SendAuth();
								}
								else 
								{
									SendReqSWVersion();
								}
							}
						}
					}
					break;
				}
			}
		}
		else
		{
			if( 1 <= m_iProtocalVersion )
			{
				if( MAGIC2_CODE				== UpgradeHeader.magic_code		&&
					PROTOCOL_TYPE_UPGRADE	== UpgradeHeader.protocol_type	)
				{
					switch ( UpgradeHeader.protocol_mode )
					{
					case _UPGRADE_RSP_AUTH			:
						{	
							if( sizeof(UPGRADE_AUTH_RSP) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeRspAuth, sizeof(UpgradeRspAuth), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								if( _UPGRADE_RSP_MSG_AUTH_SUCCESS == UpgradeRspAuth.messages )
								{
									TRACE( L"Index = %d, Authentification SUCESS , protocol_version = %d\n", m_nIndex, m_iProtocalVersion );

									if( _UPGRADESOCKET_MODE_UPGRADE					== iSocketMode ||
										_UPGRADESOCKET_MODE_SWUPGRADE_WITHOUT_MCU	== iSocketMode )
									{
										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID		= MESSAGE_SEND_REQ_UPGRADE_START;

										QueueItem.iArg1				= iSocketMode;
//										QueueItem.iArg1				= _UPGRADESOCKET_MODE_SWUPGRADE_WITHOUT_MCU;

										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

									}
								}
								else if( _UPGRADE_RSP_MSG_BADCOMMAND == UpgradeRspAuth.messages )
								{
									TRACE( L"Index = %d, Authentification Badcommand\n", m_nIndex );

									m_dwThreadSocketID	= 0;
									m_iLastError		= _UPGRADE_ERROR_UPGRADE_IS_WORKING_IN_OTHER_PLACE;

									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_BAD_COMMAND;
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
								}
								else if( _UPGRADE_RSP_MSG_NO_AUTHORITY == UpgradeRspAuth.messages )	
								{
									m_dwThreadSocketID	= 0;
									m_iLastError		= _UPGRADE_ERROR_AUTH_FAILED;

									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID	= MESSAGE_ERROR_AUTHORITY;
									QueueItem.iArg1			= 0;
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									TRACE( L"Index = %d, _UPGRADE_RSP_NO_AUTHORITY	\n", m_nIndex );	
								}
								else
								{
									TRACE( L"Index = %d, Authentification FAIL\n", m_nIndex );
									m_dwThreadSocketID	= 0;
									m_iLastError		= _UPGRADE_ERROR_AUTH_FAILED;

									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_AUTHENTIFICATION_FAIL;
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
								}
							}
						}				
						break;
					case _UPGRADE_RSP_MCU_INFO		:
						{
							if( sizeof(UpgradeMCUInfoRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeMCUInfoRsp, sizeof(UpgradeMCUInfoRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch( UpgradeMCUInfoRsp.messages )
								{
								case _UPGRADE_RSP_MSG_PROTOCAL_MCU_INFO_SUCCESS :
									{
										TRACE( L"Index = %d, Receive _UPGRADE_RSP_MCU_INFO  _UPGRADE_RSP_MSG_PROTOCAL_MCU_INFO_SUCCESS\n", m_nIndex );

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_SEND_REQ_UPGRADE_MCU_START;
										QueueItem.iArg1			= UpgradeMCUInfoRsp.mcu_model;
										QueueItem.iArg2			= UpgradeMCUInfoRsp.mcu_version;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
									}
									break;

								case _UPGRADE_RSP_MSG_PROTOCAL_MCU_INFO_FAIL :
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_PROTOCOL_MCU_INFO_FAIL;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_PROTOCAL_MCU_INFO;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

										TRACE( L"Index = %d, _UPGRADE_RSP_BADCOMMAND	\n", m_nIndex );	

										SendStopMcuUpgrade();
										m_dwThreadSocketID	= 0;
									}
									break;
								}
							}							
						}
						break;

					case _UPGRADE_RSP_MCU_FILE_INFO :
						{
							if( sizeof(UPGRADE_RSP) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeRsp, sizeof(UpgradeRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch ( UpgradeRsp.messages )
								{
								case _UPGRADE_RSP_MSG_FILE_INFO_SUCCESS		:	
									{
										TRACE( L"Index = %d, Receive _UPGRADE_RSP_MCU_FILE_INFO  _UPGRADE_RSP_MSG_FILE_INFO_SUCCESS\n",m_nIndex );

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_SEND_UPGRADE_MCU_FILE;
										QueueItem.iArg1			= m_iSelectMCUIndex;
										QueueItem.iArg2			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
									}
									break;
								case _UPGRADE_RSP_MSG_BADCOMMAND:
									{

									}
									break;
								case _UPGRADE_RSP_MSG_NO_MATCH_MODEL:
									{

									}
									break;
								case _UPGRADE_RSP_MSG_INVALID_FIRMWARE:
									{

									}
									break;
								case _UPGRADE_RSP_MSG_MEMORY_IS_NOT_ENOUGH:
									{

									}
									break;
								}
							}
						}
						break;
					case _UPGRADE_RSP_FILE_INFO		:				
						{
							if( sizeof(UpgradeStartInfoRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeStartInfoRsp, sizeof(UpgradeStartInfoRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch ( UpgradeStartInfoRsp.messages )
								{
								case _UPGRADE_RSP_MSG_FILE_INFO_SUCCESS		:	
									{
										if( _UPGRADE_STATUS_FILE_TRANSFERING != iUpgradeStatus )
										{
											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID	= MESSAGE_SEND_UPGRADE_SW_FILE;
											QueueItem.iArg1			= 0;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);


											iUpgradeStatus	= _UPGRADE_STATUS_FILE_TRANSFERING;
										}

										TRACE( L"Index = %d, _UPGRADE_RSP_FILE_INFO_SUCCESS		\n",m_nIndex );	
									}
									break;
								case _UPGRADE_RSP_MSG_BADCOMMAND			:	
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_IN_ORDER_TO_VERSION_CHECKING_CAN_NOT_CONNECT;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_BAD_COMMAND;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

										TRACE( L"Index = %d, _UPGRADE_RSP_BADCOMMAND	\n",m_nIndex );	
									}
									break;
								case _UPGRADE_RSP_MSG_NO_MATCH_MODEL		:	
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_NO_MATCH_MODEL;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_NO_MATCH_MODEL;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
										TRACE( L"Index = %d, _UPGRADE_RSP_NO_MATCH_MODEL	\n",m_nIndex );	
									}
									break;
								case _UPGRADE_RSP_MSG_INVALID_FIRMWARE		:	
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_INVALID_FIRMWARE;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_INVALID_FIRMWARE;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
										TRACE( L"Index = %d, _UPGRADE_RSP_INVALID_FIRMWARE	\n",m_nIndex );	
									}
									break;
								case _UPGRADE_RSP_MSG_MEMORY_IS_NOT_ENOUGH	:	
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_MEMORY_NOT_ENOUGH;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_WAIT_FOR_REBOOT_NOT_ENOUGHMEMORY;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
										TRACE( L"Index = %d, MESSAGE_WAIT_FOR_REBOOT_NOT_ENOUGHMEMORY	\n",m_nIndex );	
									}
									break;
								}
							}
						}				
						break;
					case _UPGRADE_RSP_MCU_FILE_STREAM:
						{
							if( sizeof(UpgradeFileStreamRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeFileStreamRsp, sizeof(UpgradeFileStreamRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch( UpgradeFileStreamRsp.messages )
								{
								case _UPGRADE_RSP_MSG_BADCOMMAND:
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_BAD_COMMAND;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_BAD_COMMAND;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

										TRACE( L"Index = %d, _UPGRADE_RSP_BADCOMMAND Index = %d	Message = %d\n" ,m_nIndex,
											UpgradeFileStreamRsp.index,
											UpgradeFileStreamRsp.messages);

									}
									break;
								case _UPGRADE_RSP_MSG_NO_MATCH_FILE_INDEX:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_NO_MATCH_FILE_INDEX Index = %d	Message = %d\n" ,m_nIndex,
											UpgradeFileStreamRsp.index,
											UpgradeFileStreamRsp.messages);

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_SEND_UPGRADE_MCU_FILE_RESTART_WAIT;
										QueueItem.iArg1			= m_iSelectMCUIndex;
										QueueItem.iArg2			= UpgradeFileStreamRsp.index;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

										TRACE( L"Index = %d, _UPGRADE_RSP_NO_MATCH_FILE_INDEX Push Queue Count = %d \n", m_nIndex, m_MessageQueue.GetQueueCount() );
									}
									break;
								case _UPGRADE_RSP_MSG_FILE_RECEIVED_OK:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_FILE_RECEIVED_OK Index = %d	Message = %d\n" ,m_nIndex,
											UpgradeFileStreamRsp.index,
											UpgradeFileStreamRsp.messages);
									}
									break;

								case _UPGRADE_RSP_MSG_TIMEOUT:
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_RECEIVE_TIME_OUT;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_TIMEOUT;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
										TRACE( L"Index = %d, _UPGRADE_RSP_TIMEOUT	\n", m_nIndex );	
									}
								}
							}
						}
						break;

					case _UPGRADE_RSP_MCU_WRITE :
						{
							if( sizeof(UpgradeNandWriteRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeNandWriteRsp, sizeof(UpgradeNandWriteRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch( UpgradeNandWriteRsp.messages )
								{
								case _UPGRADE_RSP_MSG_FILE_NOT_EXIST_MCU:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_FILE_NOT_EXIST		\n",m_nIndex );
										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID		= MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST_MCU;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
									}
									break;
								case _UPGRADE_RSP_MSG_READY_TO_UPGRADE_MCU:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_READY_TO_UPGRADE_MCU\n" , m_nIndex);
										if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
										{
											// Software package 만 있을때
										}
										else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
										{
											// MCU package 만 있을때
											if( m_iPercentage != 20 )
											{
												m_iPercentage = 20;
												PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
											}
										}
										else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
										{
											// Software/MCU package 있을때
											if( m_iPercentage != 20)
											{
												m_iPercentage = 20;
												PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
											}
										}

										wsprintf( m_szDisplayMessage, L"Upgrade MCU" );
										PostMessage(m_hwndParent, 
											WM_UPGRADE_MSG, 
											MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
											(LPARAM)_UPGRADE_STATUS_UPGRADE_MCU );
									}
									break;
								case _UPGRADE_RSP_MSG_INVALID_FIRMWARE_MCU :
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_INVALID_FIRMWARE_MCU\n",m_nIndex );
									}
									break;
								case _UPGRADE_RSP_MSG_UPGRADING_MCU :
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_UPGRADING_MCU\n",m_nIndex );

										// 프로그래스 설정( MCU 업그레이드 중.. )
										if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
										{
											// Software package 만 있을때
										}
										else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
										{
											// MCU package 만 있을때
											if( m_iPercentage != 20+( UpgradeNandWriteRsp.written_percent*4/5) )
											{
												m_iPercentage = 20+( UpgradeNandWriteRsp.written_percent*4/5);
												PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
											}
										}
										else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
										{
											// Software/MCU package 있을때
											if( m_iPercentage != 20+( UpgradeNandWriteRsp.written_percent*3/10) )
											{
												m_iPercentage = 20+( UpgradeNandWriteRsp.written_percent*3/10);
												PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
											}
										}

									}
									break;
								case _UPGRADE_RSP_MSG_UPGRADED_MCU :
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_UPGRADED_MCU\n",m_nIndex );
									}
									break;
								case _UPGRADE_RSP_MSG_FINISHED_UPGRADE_MCU :
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_FINISHED_UPGRADE_MCU\n",m_nIndex );

										if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
										{
											// Software package 만 있을때
										}
										else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
										{
											// MCU package 만 있을때
											if( NULL != m_hwndParent )
											{
												wsprintf( m_szDisplayMessage, L"Completed" );
												PostMessage(m_hwndParent, 
													WM_UPGRADE_MSG, 
													MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
													(LPARAM)_UPGRADE_STATUS_COMPLETE );
											}

											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID		= MESSAGE_UPGRADE_COMPLETE;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);
											m_dwThreadSocketID	= 0;
										}
										else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
										{
											// Software/MCU package 있을때
											if( m_iPercentage != 50 )
											{
												m_iPercentage = 50;
												PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
											}

											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID		= MESSAGE_CONNECT_FOR_SW_UPGRADE_WITHOUT_MCU;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);
											m_dwThreadSocketID	= 0;
										}
									}
									break;
								}
							}

						}
						break;

					case _UPGRADE_RSP_FILE_STREAM	:				
						{		
							if( sizeof(UpgradeFileStreamRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeFileStreamRsp, sizeof(UpgradeFileStreamRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch( UpgradeFileStreamRsp.messages )
								{
								case _UPGRADE_RSP_MSG_BADCOMMAND:
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_BAD_COMMAND;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_BAD_COMMAND;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

										TRACE( L"Index = %d, _UPGRADE_RSP_BADCOMMAND Index = %d	Message = %d\n" ,m_nIndex,
											UpgradeFileStreamRsp.index,
											UpgradeFileStreamRsp.messages);

									}
									break;
								case _UPGRADE_RSP_MSG_NO_MATCH_FILE_INDEX:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_NO_MATCH_FILE_INDEX Index = %d	Message = %d\n" ,m_nIndex,
											UpgradeFileStreamRsp.index,
											UpgradeFileStreamRsp.messages);

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_SEND_UPGRADE_SW_FILE_RESTART_WAIT;
										QueueItem.iArg1			= UpgradeFileStreamRsp.index;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);

										TRACE( L"Index = %d, _UPGRADE_RSP_NO_MATCH_FILE_INDEX Push Queue Count = %d \n", m_nIndex, m_MessageQueue.GetQueueCount() );
									}
									break;
								case _UPGRADE_RSP_MSG_FILE_RECEIVED_OK:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_FILE_RECEIVED_OK Index = %d	Message = %d\n" ,m_nIndex,
											UpgradeFileStreamRsp.index,
											UpgradeFileStreamRsp.messages);
									}
									break;

								case _UPGRADE_RSP_MSG_TIMEOUT:
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_RECEIVE_TIME_OUT;

										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID	= MESSAGE_ERROR_TIMEOUT;
										QueueItem.iArg1			= 0;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
										TRACE( L"Index = %d, _UPGRADE_RSP_TIMEOUT	\n",m_nIndex);	
									}
								}
							}
						}				
						break;
					case _UPGRADE_RSP_NAND_WRITE	:				
						{
							if( sizeof(UpgradeNandWriteRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeNandWriteRsp, sizeof(UpgradeNandWriteRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch( UpgradeNandWriteRsp.messages )
								{
								case _UPGRADE_RSP_MSG_FILE_NOT_EXIST:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_FILE_NOT_EXIST		\n",m_nIndex );
										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID		= MESSAGE_WAIT_FOR_REBOOT_FILE_NOT_EXIST;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
									}
									break;
								case _UPGRADE_RSP_MSG_READY_TO_UPGRADE:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_READY_TO_UPGRADE	\n",m_nIndex );

										if( _UPGRADE_STATUS_READY_UPGRADE != iUpgradeStatus )
										{
											if( NULL != m_hwndParent )
											{
												wsprintf( m_szDisplayMessage, L"Ready to upgrade" );
												PostMessage(m_hwndParent, 
													WM_UPGRADE_MSG, 
													MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
													(LPARAM)_UPGRADE_STATUS_READY_UPGRADE );
											}

											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID		= MESSAGE_WAIT_FOR_UPGRADE_READY;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);

											iUpgradeStatus	= _UPGRADE_STATUS_READY_UPGRADE;
										}
									}
									break;
								case _UPGRADE_RSP_MSG_NO_MATCH_MODEL:
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_NO_MATCH_MODEL;

										TRACE( L"Index = %d, _UPGRADE_RSP_NO_MATCH_MODEL		\n",m_nIndex );
										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID		= MESSAGE_ERROR_NO_MATCH_MODEL;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
									}
									break;
								case _UPGRADE_RSP_MSG_INVALID_FIRMWARE:
									{
										m_dwThreadSocketID	= 0;
										m_iLastError		= _UPGRADE_ERROR_INVALID_FIRMWARE;

										TRACE( L"Index = %d, _UPGRADE_RSP_INVALID_FIRMWARE	\n",m_nIndex );
										MESSAGE_QUEUE_ITEM QueueItem	= {0};
										QueueItem.iCommandID		= MESSAGE_ERROR_INVALID_FIRMWARE;
										m_MessageQueue.PushQueue( QueueItem );
										SetEvent(m_hThreadUpgradeEvent);
									}
									break;
								case _UPGRADE_RSP_MSG_UPGRADING_OS:
									{
//										TRACE( L"Index = %d, _UPGRADE_RSP_UPGRADING_OS		\n",m_nIndex);

										if( _UPGRADE_STATUS_UPGRADE_OS != iUpgradeStatus )
										{
											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID		= MESSAGE_UPGRADE_READY;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);

											if( NULL != m_hwndParent )
											{
												wsprintf( m_szDisplayMessage, L"Upgrade OS" );
												PostMessage(m_hwndParent, 
													WM_UPGRADE_MSG, 
													MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
													(LPARAM)_UPGRADE_STATUS_UPGRADE_OS );
											}

											iUpgradeStatus	= _UPGRADE_STATUS_UPGRADE_OS;
										}

										if( NULL != m_hwndParent )
										{
											// 프로그래스 설정( OS 업그레이드 중.. )
											if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software package 만 있을때
												if( m_iPercentage != 60+( UpgradeNandWriteRsp.written_percent/5) )
												{
													m_iPercentage = 60+( UpgradeNandWriteRsp.written_percent/5);
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
											else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
											{
												// MCU package 만 있을때
												// Software package가 없으므로 적용되지 않음
											}
											else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software/MCU package 있을때
												if( m_iPercentage != 80+( UpgradeNandWriteRsp.written_percent/10) )
												{
													m_iPercentage = 80+( UpgradeNandWriteRsp.written_percent/10);
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
										}
									}
									break;
								case _UPGRADE_RSP_MSG_UPGRADED_OS:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_UPGRADED_OS		\n",m_nIndex );

										if( NULL != m_hwndParent )
										{
											// 프로그래스 설정( OS 업그레이드 완료 )
											if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software package 만 있을때
												if( m_iPercentage != 80 )
												{
													m_iPercentage = 80;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
											else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
											{
												// MCU package 만 있을때
											}
											else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software/MCU package 있을때
												if( m_iPercentage != 90 )
												{
													m_iPercentage = 90;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
										}
									}
									break;
								case _UPGRADE_RSP_MSG_UPGRADING_APPLICATION:
									{
//										TRACE( L"Index = %d, _UPGRADE_RSP_UPGRADING_APPLICATION \n",m_nIndex );

										if( _UPGRADE_STATUS_UPGRADE_APPLICATION != iUpgradeStatus )
										{
											if( NULL != m_hwndParent )
											{
												wsprintf( m_szDisplayMessage, L"Upgrade Application" );
												PostMessage(m_hwndParent, 
													WM_UPGRADE_MSG, 
													MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
													(LPARAM)_UPGRADE_STATUS_UPGRADE_APPLICATION );
											}
											iUpgradeStatus	= _UPGRADE_STATUS_UPGRADE_APPLICATION;
										}


										if( NULL != m_hwndParent )
										{
											// 프로그래스 설정( Application 업그레이드 중.. )
											if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software package 만 있을때
												if( m_iPercentage != 80+( UpgradeNandWriteRsp.written_percent/5) )
												{
													m_iPercentage = 80+( UpgradeNandWriteRsp.written_percent/5);
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
											else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
											{
												// MCU package 만 있을때
											}
											else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software/MCU package 있을때
												if( m_iPercentage != 90+( UpgradeNandWriteRsp.written_percent/10) )
												{
													m_iPercentage = 90+( UpgradeNandWriteRsp.written_percent/10);
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
										}
									}
									break;
								case _UPGRADE_RSP_MSG_UPGRADED_APPLICATION:
									{
										if( NULL != m_hwndParent )
										{
											// 프로그래스 설정( Application 업그레이드 완료 )
											if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software package 만 있을때
												if( m_iPercentage != 100 )
												{
													m_iPercentage	= 100;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
											else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
											{
												// MCU package 만 있을때
											}
											else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software/MCU package 있을때
												if( m_iPercentage != 100 )
												{
													m_iPercentage	= 100;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
										}
										TRACE( L"Index = %d, _UPGRADE_RSP_UPGRADED_APPLICATION \n",m_nIndex );
									}
									break;
								case _UPGRADE_RSP_MSG_FINISHED_UPGRADE:
									{
										if( NULL != m_hwndParent )
										{
											// 프로그래스 설정( 업그레이드 완료 )
											if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software package 만 있을때
												if( m_iPercentage != 100 )
												{
													m_iPercentage	= 100;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
											else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
											{
												// MCU package 만 있을때
											}
											else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software/MCU package 있을때
											}
										}

										TRACE( L"Index = %d, _UPGRADE_RSP_FINISHED_UPGRADE \n",m_nIndex );
									}
									break;
								case _UPGRADE_RSP_MSG_SYSTEM_RESTARTING:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_SYSTEM_RESTARTING \n",m_nIndex );
										if( NULL != m_hwndParent )
										{
											// 프로그래스 설정( 업그레이드 완료 )
											if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software package 만 있을때
												if( m_iPercentage != 100 )
												{
													m_iPercentage	= 100;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
											else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
											{
												// MCU package 만 있을때
											}
											else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
											{
												// Software/MCU package 있을때
												if( m_iPercentage != 100 )
												{
													m_iPercentage	= 100;
													PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
												}
											}
										}

										if( _UPGRADE_STATUS_WAITFOR_REBOOT != iUpgradeStatus )
										{
											if( NULL != m_hwndParent )
											{
												wsprintf( m_szDisplayMessage, L"Wait for rebooting" );
												PostMessage(m_hwndParent, 
													WM_UPGRADE_MSG, 
													MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
													(LPARAM)_UPGRADE_STATUS_WAITFOR_REBOOT );
											}

											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID		= MESSAGE_WAIT_FOR_REBOOT_VERSIONCHECK;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);

											iUpgradeStatus = _UPGRADE_STATUS_WAITFOR_REBOOT;
											m_dwThreadSocketID	= 0;
										}
									}
									break;
								}
							}
						}				
						break;
					case _UPGRADE_RSP_SW_VERSION	:				
						{		
							if( sizeof(UpgradeSWVersionRsp) == UpgradeHeader.body_size )
							{
								if( SOCKET_ERROR == recvfrom( m_pSockUpgrade, (char*)&UpgradeSWVersionRsp, sizeof(UpgradeSWVersionRsp), 0, (SOCKADDR*)&ReceiverAddr,&nReceiverAddrLen ) )
								{
									MESSAGE_QUEUE_ITEM QueueItem	= {0};
									QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
									QueueItem.iArg1				= WSAGetLastError();
									m_MessageQueue.PushQueue( QueueItem );
									SetEvent(m_hThreadUpgradeEvent);
									m_dwThreadSocketID	= 0;
									break;
								}

								switch( UpgradeSWVersionRsp.messages )
								{
								case _UPGRADE_RSP_MSG_SW_VERSION_SUCCESS:
									{
										TRACE( L"Index = %d, _UPGRADE_RSP_MSG_SW_VERSION_SUCCESS \n", m_nIndex );

										//WCHAR	szMessage[32];
										//wsprintf( szMessage, L"%d", m_SwPackageFileInfo.modelType );
										//AfxMessageBox( szMessage );

										if( 255 == m_SwPackageFileInfo.modelType )
										{
											if( NULL != m_hwndParent )
											{
												wsprintf( m_szDisplayMessage, L"Completed" );
												PostMessage(m_hwndParent, 
													WM_UPGRADE_MSG, 
													MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
													(LPARAM)_UPGRADE_STATUS_COMPLETE );
											}

											MESSAGE_QUEUE_ITEM QueueItem	= {0};
											QueueItem.iCommandID		= MESSAGE_UPGRADE_COMPLETE;
											m_MessageQueue.PushQueue( QueueItem );
											SetEvent(m_hThreadUpgradeEvent);
											m_dwThreadSocketID	= 0;
										}
										else
										{
											if( 0 == strcmp( m_SwPackageFileInfo.szSWVersion, UpgradeSWVersionRsp.sw_version ) )
											{
												if( NULL != m_hwndParent )
												{
													wsprintf( m_szDisplayMessage, L"Completed" );
													PostMessage(m_hwndParent, 
														WM_UPGRADE_MSG, 
														MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
														(LPARAM)_UPGRADE_STATUS_COMPLETE );
												}

												MESSAGE_QUEUE_ITEM QueueItem	= {0};
												QueueItem.iCommandID		= MESSAGE_UPGRADE_COMPLETE;
												m_MessageQueue.PushQueue( QueueItem );
												SetEvent(m_hThreadUpgradeEvent);
												m_dwThreadSocketID	= 0;
											}										
											else
											{
												TRACE( L"Index = %d, _UPGRADE_RSP_MSG_SW_VERSION_SUCCESS FAIL >>>>>>>>>>>>>>>>>>>>\n", m_nIndex );
												m_dwThreadSocketID	= 0;
												m_iLastError		= _UPGRADE_ERROR_NO_MATCH_VERSION;

												MESSAGE_QUEUE_ITEM QueueItem	= {0};
												QueueItem.iCommandID		= MESSAGE_ERROR_NO_MATCH_VERSION;
												m_MessageQueue.PushQueue( QueueItem );
												SetEvent(m_hThreadUpgradeEvent);

												m_iLastError	= _UPGRADE_ERROR_NO_MATCH_VERSION;
											}
										}
									}
									break;
								}
							}
						}				
						break;
					case _UPGRADE_RSP_STATUS		:				
						{				
							TRACE( L"Index = %d, _UPGRADE_RSP_STATUS Message = %d \n", m_nIndex, UpgradeHeader.protocol_mode );
						}
						break;
					case _UPGRADE_RSP_STOP			:				
						{				
							TRACE( L"Index = %d, _UPGRADE_RSP_STOP Message = %d \n", m_nIndex, UpgradeHeader.protocol_mode );
						}				
						break;
					}

				}
			}
		}

		Sleep( 2 );
	}

	return TRUE;
}

void CUpgradeThreadTcp::Disconnect()
{
	if(m_hDisocnnectLock)
		WaitForSingleObject(m_hDisocnnectLock, INFINITE);
	DWORD exitCode	= 0;

	if( INVALID_SOCKET != m_pSockUpgrade )
	{
		closesocket( m_pSockUpgrade );
		m_pSockUpgrade = INVALID_SOCKET;
	}

	if( NULL != m_hThreadSocket )
	{
		m_dwThreadSocketID	= 0;
		if( WAIT_TIMEOUT == WaitForSingleObject( m_hThreadSocket, 10000 ) )
		{
			:: TerminateThread(m_hThreadSocket, exitCode);
		}

		if(m_hThreadSocket)
		{
			::CloseHandle(m_hThreadSocket);
			m_hThreadSocket = NULL;
		}
	}
	if(m_hDisocnnectLock)
		ReleaseMutex(m_hDisocnnectLock);
}

ULONG CUpgradeThreadTcp::GetIPAddress( LPCTSTR strHostName )
{
	LPHOSTENT lphostent;
	ULONG uAddr = INADDR_NONE;

	if ( NULL != strHostName )
	{
#ifdef _UNICODE
		char strHost[256] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, strHostName, -1, strHost, sizeof(strHost), NULL, NULL );
#else
		LPCTSTR strHost = strHostName;
#endif
		// Check for an Internet Protocol dotted address string
		uAddr = inet_addr( strHost );

		if ( (INADDR_NONE == uAddr) && (strcmp( strHost, "255.255.255.255" )) )
		{
			// It's not an address, then try to resolve it as a hostname
			if ( lphostent = gethostbyname( strHost ) )
				uAddr = *((ULONG *) lphostent->h_addr_list[0]);
		}
	}

	return ntohl( uAddr );
}

BOOL CUpgradeThreadTcp::SendMCUUpgradeFile(int iMCUIndex, int iReqIndex)
{
	HANDLE				hFile				= NULL;
	DWORD				dwReadSizeTotal		= 0;
	DWORD				dwReadSize			= 0;
	DWORD				dwFileTotalSize		= 0; 
	DWORD				dwFileRequestSize	= 0; 
	UPGRADE_FILE_STREAM	UpgradeFileStream;
	UPGRADE_HEADER		UpgradeHeader;
	WCHAR				szText[MAX_PATH]	= {0};


	if( 0 >= m_wMCUInfoCount )
	{
		return FALSE;
	}

	hFile	= CreateFile(	m_sFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL ,
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		return FALSE;
	}

	dwFileTotalSize		= sizeof(m_pMCUPackageFileInfo[iMCUIndex].magic_code)+
							sizeof(m_pMCUPackageFileInfo[iMCUIndex].mcu_model)+
							sizeof(m_pMCUPackageFileInfo[iMCUIndex].mcu_version)+
							sizeof(m_pMCUPackageFileInfo[iMCUIndex].mcu_checksumsize)+
							m_pMCUPackageFileInfo[iMCUIndex].mcu_checksumsize+
							sizeof(m_pMCUPackageFileInfo[iMCUIndex].mcu_size)+
							m_pMCUPackageFileInfo[iMCUIndex].mcu_size;

	TRACE( L"/////////////////////////////////////////////////////////////////////\n" );
	TRACE( L"Index = %d, Send SendMCUUpgradeFile iMCUIndex = %d, iReqIndex = %d, Offset = %d\n",m_nIndex, iMCUIndex, iReqIndex, m_pMCUPackageFileInfo[iMCUIndex].mcu_opset );
	TRACE( L"Index = %d, File Size = %d \n",m_nIndex, dwFileTotalSize );

	dwFileRequestSize = dwFileTotalSize-1024*iReqIndex;
	TRACE( L"Index = %d, Requset Size = %d \n",m_nIndex, dwFileRequestSize );

	LARGE_INTEGER	liMovePointerNew;
	LARGE_INTEGER	liMovePointer;

	liMovePointerNew.QuadPart	= 0;
	liMovePointer.QuadPart		= iReqIndex;
	liMovePointer.QuadPart		= m_pMCUPackageFileInfo[iMCUIndex].mcu_opset+liMovePointer.QuadPart*1024;


	SetFilePointerEx( hFile, liMovePointer , &liMovePointerNew, FILE_BEGIN );

	TRACE( L"Index = %d, Move Point %lld \n", m_nIndex, liMovePointer.QuadPart );
	TRACE( L"Index = %d, Move result Point %lld \n", m_nIndex, liMovePointerNew.QuadPart );

	int	iPercentage	= 0;
	int iIndex		= 0;
	int	iIndexCount	= (int)(dwFileTotalSize/1024);
	MESSAGE_QUEUE_ITEM		QueueItem							= {0};
	bool bReRequest	= false;
	int	iSendSize	= 0;

	TRACE( L"Index = %d, Requset index Count = %d \n", m_nIndex, iIndexCount );

	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_FILE_STREAM;
	UpgradeHeader.body_size			= sizeof(UPGRADE_FILE_STREAM);

	for( iIndex=iReqIndex; iIndex<iIndexCount;iIndex++ )
	{
		if( 0 == m_dwThreadUpgradeID )
			break;

		if( 0 < m_MessageQueue.GetQueueCount() )
		{
			bReRequest	= TRUE;
			TRACE( L"Index = %d, MessageQueue Count Error Queue Count = %d\n" , m_nIndex, m_MessageQueue.GetQueueCount());
			break;
		}


		//TRACE( L"Index = %d, MagicCode = %x, protocoltype = %d, protocolmode = %d, bodysize = %d\n",m_nIndex, 
		//																							UpgradeHeader.magic_code,
		//																							UpgradeHeader.protocol_type,
		//																							UpgradeHeader.protocol_mode,
		//																							UpgradeHeader.body_size );

		iSendSize	= SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );
		if( sizeof( UpgradeHeader ) != iSendSize )
		{
			TRACE( L"Index = %d, sizeof( UpgradeHeader ) != iSendSize", m_nIndex );

			MESSAGE_QUEUE_ITEM QueueItem	= {0};
			QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
			QueueItem.iArg1				= WSAGetLastError();
			m_MessageQueue.PushQueue( QueueItem );
			SetEvent(m_hThreadUpgradeEvent);
		}


		UpgradeFileStream.index		= iIndex;
		UpgradeFileStream.length	= sizeof(UpgradeFileStream.data);
		ReadFile( hFile, &UpgradeFileStream.data[0], UpgradeFileStream.length, &dwReadSize, NULL );
		if( dwReadSize != 1024 )
		{
			DWORD dwCurrent	= ::SetFilePointer( hFile, 0, 0, FILE_CURRENT );
			TRACE( L"Index = %d, Read Error index = %d, ReadSize = %d, Current Position = %d ",m_nIndex, iIndex, dwReadSize, dwCurrent );
		}

		dwReadSizeTotal	+= dwReadSize;

		//TRACE( L"Index = %d, Read Data Index = %02d, Length = %d, UpgradeFileStream = 0x%02x, UpgradeFileStream1 = 0x%02x, UpgradeFileStream2 = 0x%02x, UpgradeFileStream3 = 0x%02x ",
		//	m_nIndex								,
		//	iIndex									,
		//	dwReadSize								,
		//	(unsigned char)UpgradeFileStream.data[0]	,
		//	(unsigned char)UpgradeFileStream.data[1]	,
		//	(unsigned char)UpgradeFileStream.data[2]	,
		//	(unsigned char)UpgradeFileStream.data[3]	);

		iSendSize	= SendData(m_pSockUpgrade, (char*)&UpgradeFileStream, sizeof(UpgradeFileStream) );
		if( sizeof( UpgradeFileStream ) != iSendSize )
		{
			TRACE( L"Index = %d, sizeof( UpgradeFileStream ) != iSendSize", m_nIndex );

			MESSAGE_QUEUE_ITEM QueueItem	= {0};
			QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
			QueueItem.iArg1				= WSAGetLastError();
			m_MessageQueue.PushQueue( QueueItem );
			SetEvent(m_hThreadUpgradeEvent);
		}

		if( NULL != m_hwndParent )
		{
			// 프로그래스 설정( MCU 파일 전송중.. )
			if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software package 만 있을때
			}
			else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
			{
				// MCU package 만 있을때
				iPercentage	= (iIndex*100/iIndexCount)/10+10;
				if( m_iPercentage != iPercentage )
				{
					m_iPercentage = iPercentage ;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}
			else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software/MCU package 있을때
				iPercentage	= (iIndex*100/iIndexCount)/10+10;
				if( m_iPercentage != iPercentage )
				{
					m_iPercentage = iPercentage ;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}

			swprintf_s( szText, L"Index = %d, RequestIndex = %d, Index = %d, iIndexCount = %d, iPercentage = %d \n", m_nIndex, iReqIndex, iIndex, iIndexCount, iPercentage );
//			TRACE( L"Index = %d, RequestIndex = %d, Index = %d, iIndexCount = %d, iPercentage = %d \n", m_nIndex, iReqIndex, iIndex, iIndexCount, iPercentage );
		}
	}

	//TRACE( L"Index = %d, Send index Count = %d \n", m_nIndex, iIndex );
	//TRACE( L"Index = %d, bReRequest = %d \n", m_nIndex, bReRequest );
	//TRACE( L"Index = %d, Send Size = %d \n", m_nIndex, dwReadSizeTotal );


	if( 0 != m_dwThreadUpgradeID && FALSE == bReRequest )
	{
		if( dwFileRequestSize > dwReadSizeTotal )
		{
			UpgradeHeader.magic_code		= MAGIC2_CODE;
			UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
			UpgradeHeader.protocol_mode		= _UPGRADE_REQ_FILE_STREAM;
			UpgradeHeader.body_size			= sizeof(UPGRADE_FILE_STREAM);

			SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

			ReadFile( hFile, &UpgradeFileStream.data[0], dwFileRequestSize-dwReadSizeTotal ,&dwReadSize, NULL );
			dwReadSizeTotal	+= dwReadSize;

			UpgradeFileStream.index		= iIndex;
			UpgradeFileStream.length	= dwReadSize;

			SendData(m_pSockUpgrade, (char*)&UpgradeFileStream, sizeof(UpgradeFileStream) );
		}

		if( NULL != m_hwndParent )
		{
			// 프로그래스 설정( MCU 파일 전송중 완료 )
			if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software package 만 있을때
			}
			else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
			{
				// MCU package 만 있을때
				if( m_iPercentage != 20 )
				{
					m_iPercentage = 20;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}
			else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software/MCU package 있을때
				if( m_iPercentage != 20 )
				{
					m_iPercentage = 20;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}
		}
	}

	TRACE( L"Index = %d, SendMCUUpgradeFile COMPLETE \n", m_nIndex, iIndex );
	TRACE( L"Index = %d, SendMCUUpgradeFile Total Size = %d \n", m_nIndex, dwReadSizeTotal );

	CloseHandle(hFile);
	hFile	= NULL;

	return TRUE;
}

BOOL CUpgradeThreadTcp::SendUpgradeFile(int iReqIndex)
{
	HANDLE				hFile				= NULL;
	DWORD				dwReadSizeTotal		= 0;
	DWORD				dwReadSize			= 0;
	DWORD				dwFileTotalSize		= 0; 
	DWORD				dwFileRequestSize	= 0; 
	UPGRADE_FILE_STREAM	UpgradeFileStream;
	UPGRADE_HEADER		UpgradeHeader;

	hFile	= CreateFile(	m_sFilePath,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL ,
							NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		return FALSE;
	}

	LARGE_INTEGER	liMovePointerNew;
	LARGE_INTEGER	liMovePointer;
	int				iPercentage	= 0;
	int				iIndex		= 0;
	int				iIndexCount	= (int)(dwFileTotalSize/1024);
	int				iMagic_code	= 0;

	ReadFile( hFile, &iMagic_code, sizeof(iMagic_code), &dwReadSize, NULL );

	if( MAGIC_CODE_UPGRADEFILE1 == iMagic_code )
	{
		dwFileTotalSize		= GetFileSize( hFile, 0 );
		TRACE( L"Index = %d, File Size = %d \n", m_nIndex, dwFileTotalSize );
		dwFileRequestSize	= dwFileTotalSize-1024*iReqIndex;
		TRACE( L"Index = %d, Requset Size = %d \n", m_nIndex, dwFileRequestSize );

		liMovePointerNew.QuadPart	= 0;
		liMovePointer.QuadPart		= iReqIndex;
		liMovePointer.QuadPart		= liMovePointer.QuadPart*1024;

		SetFilePointerEx( hFile, liMovePointer , &liMovePointerNew, FILE_BEGIN );

		TRACE( L"Index = %d, Move Point %lld \n", m_nIndex, liMovePointer.QuadPart );
		TRACE( L"Index = %d, Move result Point %lld \n", m_nIndex, liMovePointerNew.QuadPart );

		iPercentage	= 0;
		iIndex		= 0;
		iIndexCount	= (int)(dwFileTotalSize/1024);
	}
	else if( MAGIC_CODE_UPGRADEFILE2 == iMagic_code )
	{
		dwFileTotalSize		= m_lSWPackageSize;

		TRACE( L"Index = %d, File Size = %d \n", m_nIndex, dwFileTotalSize );
		dwFileRequestSize	= dwFileTotalSize-1024*iReqIndex;
		TRACE( L"Index = %d, Requset Size = %d \n", m_nIndex, dwFileRequestSize );

		liMovePointerNew.QuadPart	= 0;
		liMovePointer.QuadPart		= iReqIndex;
		liMovePointer.QuadPart		= liMovePointer.QuadPart*1024 + sizeof(int) + sizeof(long) ;

		SetFilePointerEx( hFile, liMovePointer , &liMovePointerNew, FILE_BEGIN );

		TRACE( L"Index = %d, Move Point %lld \n", m_nIndex, liMovePointer.QuadPart );
		TRACE( L"Index = %d, Move result Point %lld \n", m_nIndex, liMovePointerNew.QuadPart );

		iPercentage	= 0;
		iIndex		= 0;
		iIndexCount	= (int)(dwFileTotalSize/1024);
	}
	else
	{
		CloseHandle(hFile);
		hFile	= NULL;

		return FALSE;
	}

	MESSAGE_QUEUE_ITEM		QueueItem							= {0};
	bool bReRequest	= false;

	TRACE( L"Index = %d, Requset index Count = %d \n", m_nIndex, iIndexCount );

	for( iIndex=iReqIndex; iIndex<iIndexCount;iIndex++ )
	{
		if( 0 == m_dwThreadUpgradeID )
			break;

		if( 0 < m_MessageQueue.GetQueueCount() )
		{
			bReRequest	= TRUE;
			TRACE( L"Index = %d, MessageQueue Count Error Queue Count = %d\n" , m_nIndex, m_MessageQueue.GetQueueCount());
			break;
		}

		UpgradeHeader.magic_code		= MAGIC2_CODE;
		UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
		UpgradeHeader.protocol_mode		= _UPGRADE_REQ_FILE_STREAM;
		UpgradeHeader.body_size			= sizeof(UPGRADE_FILE_STREAM);

		if( sizeof( UpgradeHeader ) != SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) ) )
		{
			MESSAGE_QUEUE_ITEM QueueItem	= {0};
			QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
			QueueItem.iArg1				= WSAGetLastError();
			m_MessageQueue.PushQueue( QueueItem );
			SetEvent(m_hThreadUpgradeEvent);
		}

		//TRACE( L"Index = %d, MagicCode = %x, protocoltype = %d, protocolmode = %d, bodysize = %d\n",m_nIndex, 
		//	UpgradeHeader.magic_code,
		//	UpgradeHeader.protocol_type,
		//	UpgradeHeader.protocol_mode,
		//	UpgradeHeader.body_size );

		ReadFile( hFile, &UpgradeFileStream.data[0], 1024, &dwReadSize, NULL );
		if( dwReadSize != 1024 )
		{
			DWORD dwCurrent	= ::SetFilePointer( hFile, 0, 0, FILE_CURRENT );
			TRACE( L"Index = %d, Read Error index = %d, ReadSize = %d, Current Position = %d ", m_nIndex, iIndex, dwReadSize, dwCurrent );
		}

		dwReadSizeTotal	+= dwReadSize;

		UpgradeFileStream.index		= iIndex;
		UpgradeFileStream.length	= dwReadSize;

		if( sizeof(UpgradeFileStream) != SendData(m_pSockUpgrade, (char*)&UpgradeFileStream, sizeof(UpgradeFileStream) ) )
		{
			MESSAGE_QUEUE_ITEM QueueItem	= {0};
			QueueItem.iCommandID		= MESSAGE_ERROR_DISCONNECTED;
			QueueItem.iArg1				= WSAGetLastError();
			m_MessageQueue.PushQueue( QueueItem );
			SetEvent(m_hThreadUpgradeEvent);
		}


		if( NULL != m_hwndParent )
		{
			// 프로그래스 설정( Software 파일 전송중.. )
			if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software package 만 있을때
				iPercentage	= (iIndex*100/iIndexCount)*2/5+10;
				if( m_iPercentage != iPercentage )
				{
					m_iPercentage = iPercentage ;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}
			else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
			{
				// MCU package 만 있을때
			}
			else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software/MCU package 있을때
				iPercentage	= (iIndex*100/iIndexCount)/10+60;
				if( m_iPercentage != iPercentage )
				{
					m_iPercentage = iPercentage ;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}

			//TRACE( L"Index = %d, RequestIndex = %d, Index = %d, iIndexCount = %d, iPercentage = %d \n", m_nIndex, iReqIndex, iIndex, iIndexCount, iPercentage );
		}
	}

	if( iIndex != iIndexCount )
	{
		bReRequest	= TRUE;
	}

	TRACE( L"Index = %d, Send index Count = %d \n",m_nIndex, iIndex );
	TRACE( L"Index = %d, bReRequest = %d \n", m_nIndex, bReRequest );
	TRACE( L"Index = %d, Send Size = %d \n", m_nIndex, dwReadSizeTotal );


	if( 0 != m_dwThreadUpgradeID && FALSE == bReRequest )
	{
		if( dwFileRequestSize > dwReadSizeTotal )
		{
			UpgradeHeader.magic_code		= MAGIC2_CODE;
			UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
			UpgradeHeader.protocol_mode		= _UPGRADE_REQ_FILE_STREAM;
			UpgradeHeader.body_size			= sizeof(UPGRADE_FILE_STREAM);

			SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

			ReadFile( hFile, &UpgradeFileStream.data[0], dwFileRequestSize-dwReadSizeTotal ,&dwReadSize, NULL );
			dwReadSizeTotal	+= dwReadSize;

			UpgradeFileStream.index		= iIndex;
			UpgradeFileStream.length	= dwReadSize;

			SendData(m_pSockUpgrade, (char*)&UpgradeFileStream, sizeof(UpgradeFileStream) );

			TRACE( L"Index = %d, SendUpgradeFile Last Size = %d, index = %d \n", m_nIndex, UpgradeFileStream.length, iIndex );
		}

		if( NULL != m_hwndParent )
		{
			// 프로그래스 설정( Software 파일 전송 완료 )
			if( 0 == m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software package 만 있을때
				if( m_iPercentage != 50 )
				{
					m_iPercentage = 50;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}
			else if( 0 < m_wMCUInfoCount && 0 == m_lSWPackageSize )
			{
				// MCU package 만 있을때
			}
			else if( 0 < m_wMCUInfoCount && 0 < m_lSWPackageSize )
			{
				// Software/MCU package 있을때
				if( m_iPercentage != 70 )
				{
					m_iPercentage = 70;
					PostMessage( m_hwndParent, WM_UPGRADE_MSG, MAKEWPARAM(UPGRADE_PROGRESS, m_nIndex), m_iPercentage );
				}
			}
		}
	}

	TRACE( L"Index = %d, SendUpgradeFile COMPLETE \n", m_nIndex, iIndex );
	TRACE( L"Index = %d, SendUpgradeFile Total Size = %d \n", m_nIndex, dwReadSizeTotal );

	CloseHandle(hFile);
	hFile	= NULL;

	return TRUE;
}

BOOL CUpgradeThreadTcp::SendReqSWVersion()
{
	UPGRADE_HEADER		UpgradeHeader;
	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_SW_VERSION;
	UpgradeHeader.body_size			= 0;

	SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );
	TRACE( L"Index = %d, Send _UPGRADE_REQ_SW_VERSION\n", m_nIndex );
	return TRUE;
}

BOOL CUpgradeThreadTcp::SendUpgradeReStart()
{
	UPGRADE_HEADER		UpgradeHeader;

	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_REBOOT;
	UpgradeHeader.body_size			= 0;

	SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

	TRACE( L"Index = %d, Send _UPGRADE_REQ_RESTART\n", m_nIndex );

	return TRUE;
}

BOOL CUpgradeThreadTcp::UpgradeStart(int iUpgradeMode)
{
	DWORD				dwReadSize;
	HANDLE				hFile;
	UPGRADE_HEADER		UpgradeHeader;
	int					iMagicCodeFile	= 0;
	LONG				lDistanceToMove	= 0;

	m_lSWPackageSize	= 0;
	memset( &m_SwPackageFileInfo, 0, sizeof(m_SwPackageFileInfo) );

	m_iSelectMCUIndex	= 0;
	m_wMCUInfoCount		= 0;
	if( m_pMCUPackageFileInfo )
	{
		delete []  m_pMCUPackageFileInfo;
		m_pMCUPackageFileInfo	= NULL;
	}

	hFile	= CreateFile( m_sFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL ,
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		return FALSE;
	}

	SetFilePointer( hFile, 0, 0, FILE_BEGIN );
	ReadFile( hFile, &iMagicCodeFile, sizeof(iMagicCodeFile), &dwReadSize, NULL );

	if( MAGIC_CODE_UPGRADEFILE1 == iMagicCodeFile )
	{
		SetFilePointer( hFile, 0, 0, FILE_BEGIN );
		ReadFile( hFile, &m_SwPackageFileInfo, sizeof(m_SwPackageFileInfo), &dwReadSize, NULL );

		m_lSWPackageSize	= GetFileSize( hFile, 0 );

		if( SendUpgradeStart() )
		{
			if( NULL != m_hwndParent )
			{
				wsprintf( m_szDisplayMessage, L"Ready to S/W File Transfering" );
				PostMessage(m_hwndParent, 
					WM_UPGRADE_MSG, 
					MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
					(LPARAM)_UPGRADE_STATUS_READY_TO_SW_FILE_TRANSFERRING );
			}

			MESSAGE_QUEUE_ITEM QueueItem	= {0};
			QueueItem.iCommandID		= MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING;
			m_MessageQueue.PushQueue( QueueItem );
			SetEvent(m_hThreadUpgradeEvent);
		}
	}
	else if( MAGIC_CODE_UPGRADEFILE2	== iMagicCodeFile		)
	{
		ReadFile( hFile, &m_lSWPackageSize, sizeof(m_lSWPackageSize), &dwReadSize, NULL );
		if( 0 < m_lSWPackageSize )
		{
			ReadFile( hFile, &m_SwPackageFileInfo, sizeof(m_SwPackageFileInfo), &dwReadSize, NULL );
			SetFilePointer( hFile, m_lSWPackageSize-sizeof(m_SwPackageFileInfo), 0, FILE_CURRENT );
		}

		ReadFile( hFile, &m_wMCUInfoCount, sizeof(m_wMCUInfoCount), &dwReadSize, NULL );

		if( 0 < m_wMCUInfoCount )
		{
			if( 2 <= m_iProtocalVersion )
			{
				// MCU 정보가 있다면 카메라의 MCU 정보를 요청한다. 
				m_pMCUPackageFileInfo	= new MCU_PACKAGE_FILE_INFO[m_wMCUInfoCount];

				for( int i=0; i<m_wMCUInfoCount; i++ )
				{
					m_pMCUPackageFileInfo[i].mcu_opset	= SetFilePointer( hFile, 0, 0, FILE_CURRENT );
					ReadFile( hFile, &m_pMCUPackageFileInfo[i].magic_code, sizeof(int)+sizeof(short)+sizeof(short)+sizeof(long), &dwReadSize, NULL );

					SetFilePointer( hFile, m_pMCUPackageFileInfo[i].mcu_checksumsize, 0, FILE_CURRENT );
					ReadFile( hFile, &m_pMCUPackageFileInfo[i].mcu_size, sizeof(long), &dwReadSize, NULL );

					SetFilePointer( hFile, m_pMCUPackageFileInfo[i].mcu_size, 0, FILE_CURRENT );
				}

				if( _UPGRADESOCKET_MODE_UPGRADE == iUpgradeMode )
				{
					TRACE( L"Index = %d, m_wMCUInfoCount _UPGRADESOCKET_MODE_UPGRADE \n", m_nIndex );
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Ready to MCU File Transfering" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_READY_TO_MCU_FILE_TRANSFERRING );
					}

					UpgradeHeader.magic_code		= MAGIC2_CODE;
					UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
					UpgradeHeader.protocol_mode		= _UPGRADE_REQ_MCU_INFO;
					UpgradeHeader.body_size			= 0;

					SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );
				}
				else if( _UPGRADESOCKET_MODE_SWUPGRADE_WITHOUT_MCU == iUpgradeMode )
				{
					TRACE( L"Index = %d, m_wMCUInfoCount _UPGRADESOCKET_MODE_SWUPGRADE_WITHOUT_MCU \n", m_nIndex);
					if( NULL != m_hwndParent )
					{
						wsprintf( m_szDisplayMessage, L"Ready to S/W File Transfering" );
						PostMessage(m_hwndParent, 
							WM_UPGRADE_MSG, 
							MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
							(LPARAM)_UPGRADE_STATUS_READY_TO_SW_FILE_TRANSFERRING );
					}

					// MCU 업그래이드 이후 진행하는 S/W Upgrade
					if( SendUpgradeStart() )
					{
						MESSAGE_QUEUE_ITEM QueueItem	= {0};
						QueueItem.iCommandID		= MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING;
						m_MessageQueue.PushQueue( QueueItem );
						SetEvent(m_hThreadUpgradeEvent);
					}
				}


				TRACE( L"Index = %d, Send _UPGRADE_REQ_MCU_INFO\n", m_nIndex );
			}
			else
			{
				if( NULL != m_hwndParent )
				{
					m_iLastError		= _UPGRADE_ERROR_PROTOCOL_MCU_INFO_FAIL;

					wsprintf( m_szDisplayMessage, L"Failed(This IP camera does not support the upgrade file.)" );
					PostMessage(m_hwndParent, 
						WM_UPGRADE_MSG, 
						MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
						(LPARAM)_UPGRADE_ERROR_NOT_SUPPORT_UPGRAE_FILE );

					Disconnect();
					return TRUE;
				}
			}
		}
		else
		{
			TRACE( L"Index = %d, MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING \n", m_nIndex);

			if( NULL != m_hwndParent )
			{
				wsprintf( m_szDisplayMessage, L"Ready to S/W File Transfering" );
				PostMessage(m_hwndParent, 
					WM_UPGRADE_MSG, 
					MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
					(LPARAM)_UPGRADE_STATUS_READY_TO_SW_FILE_TRANSFERRING );
			}

			// MCU 정보가 없다면 이전과 동일하게 _UPGRADE_REQ_START 를 전송한다. 
			if( SendUpgradeStart() )
			{
				MESSAGE_QUEUE_ITEM QueueItem	= {0};
				QueueItem.iCommandID		= MESSAGE_WAIT_FOR_SW_FILE_TRANSFERING;
				m_MessageQueue.PushQueue( QueueItem );
				SetEvent(m_hThreadUpgradeEvent);
			}
		}
	}

	CloseHandle(hFile);
	hFile	= NULL;

	return TRUE;
}

BOOL CUpgradeThreadTcp::SendUpgradeStart()
{
	DWORD				dwFileTotalSize; 
	HANDLE				hFile;
	UPGRADE_HEADER		UpgradeHeader;
	UPGRADE_START_INFO	UpgradeStartInfo;
	int					iMagicCodeFile	= 0;
	long				lSWPackageSize	= 0;
	LONG				lDistanceToMove	= 0;

	hFile	= CreateFile( m_sFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL ,
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		return FALSE;
	}

	dwFileTotalSize	= GetFileSize( hFile, 0 );

	::CloseHandle(hFile);
	hFile	= NULL;

	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_START;
	UpgradeHeader.body_size			= sizeof(UPGRADE_START_INFO);

	SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

	UpgradeStartInfo.model_type	= m_SwPackageFileInfo.modelType;
	UpgradeStartInfo.dsp_type	= m_SwPackageFileInfo.dsptype;
	UpgradeStartInfo.oem_type	= m_SwPackageFileInfo.oemtype;
	UpgradeStartInfo.length		= m_lSWPackageSize;
	USES_CONVERSION;
	strcpy( UpgradeStartInfo.name		, W2A(m_sFileName) );

	SendData( m_pSockUpgrade, (char*)&UpgradeStartInfo, sizeof( UpgradeStartInfo ) );
	TRACE( L"Index = %d, Send _UPGRADE_REQ_START\n", m_nIndex );

	return TRUE;
}

BOOL CUpgradeThreadTcp::SendProtocolVersion()
{
	UPGRADE_HEADER		UpgradeHeader;
	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_PROTOCOL_VERSION;
	UpgradeHeader.body_size			= 0;

	SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

	return TRUE;
}


BOOL CUpgradeThreadTcp::SendAuth()
{
	UPGRADE_HEADER		UpgradeHeader;
	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_AUTH;
	UpgradeHeader.body_size			= sizeof(UPGRADE_AUTH);

	SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

	UPGRADE_AUTH		UpgradeAuth;

	USES_CONVERSION;
	strcpy( UpgradeAuth.id			, W2A(m_sUserName.GetBuffer(255)));
	strcpy( UpgradeAuth.password	, W2A(m_sPassword.GetBuffer(255)));
	SendData( m_pSockUpgrade, (char*)&UpgradeAuth, sizeof( UpgradeAuth ) );
	m_sUserName.ReleaseBuffer();
	m_sPassword.ReleaseBuffer();
	TRACE( L"Index = %d, Send _UPGRADE_REQ_AUTH\n", m_nIndex );

	return TRUE;
}

BOOL CUpgradeThreadTcp::SendStartMCUUpgrade( int iMCU_Model	, int iMCU_Version )
{
	UPGRADE_HEADER			UpgradeHeader;
	UPGRADE_MCU_FILE_INFO	UpgradeStartMCU;
	DWORD					dwReadSize			= 0;
	DWORD					dwMCULength			= 0;
	int						iMagicCodeFile		= 0;
	long					lSWPackageSize		= 0;
	LONG					lDistanceToMove		= 0;
	short					wMCUInfoCount		= 0;

	for( int i=0; i<(int)m_wMCUInfoCount; i++ )
	{
		if( m_pMCUPackageFileInfo[i].mcu_model == iMCU_Model )
		{
			m_iSelectMCUIndex				= i;

			UpgradeHeader.magic_code		= MAGIC2_CODE;
			UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
			UpgradeHeader.protocol_mode		= _UPGRADE_REQ_START_MCU_UPGRADE;
			UpgradeHeader.body_size			= sizeof(UpgradeStartMCU);
			SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );

			UpgradeStartMCU.model_type		= iMCU_Model;
			UpgradeStartMCU.length			= sizeof(m_pMCUPackageFileInfo[i].magic_code)+
												sizeof(m_pMCUPackageFileInfo[i].mcu_model)+
												sizeof(m_pMCUPackageFileInfo[i].mcu_version)+
												sizeof(m_pMCUPackageFileInfo[i].mcu_checksumsize)+
												m_pMCUPackageFileInfo[i].mcu_checksumsize+
												sizeof(m_pMCUPackageFileInfo[i].mcu_size)+
												m_pMCUPackageFileInfo[i].mcu_size;
			USES_CONVERSION;
			strcpy( UpgradeStartMCU.name		, W2A(m_sFileName) );
			SendData( m_pSockUpgrade, (char*)&UpgradeStartMCU, sizeof( UpgradeStartMCU ) );

			TRACE( L"Index = %d, Send _UPGRADE_REQ_START_MCU_UPGRADE %x \n"	, m_nIndex, iMCU_Model );
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CUpgradeThreadTcp::SendStopMcuUpgrade()
{
	UPGRADE_HEADER		UpgradeHeader;
	UpgradeHeader.magic_code		= MAGIC2_CODE;
	UpgradeHeader.protocol_type		= PROTOCOL_TYPE_UPGRADE;
	UpgradeHeader.protocol_mode		= _UPGRADE_REQ_STOP_MCU_UPGRADE;
	UpgradeHeader.body_size			= 0;

	SendData( m_pSockUpgrade, (char*)&UpgradeHeader, sizeof( UpgradeHeader ) );
	return TRUE;
}

BOOL CUpgradeThreadTcp::ConnectForCheckComplete()
{
	Disconnect();

	if( 1024 > m_nUpgradePort )
	{
		if( NULL != m_hwndParent )
		{
			wsprintf( m_szDisplayMessage, L"Failed(This version does not support.)" );
			PostMessage(m_hwndParent, 
				WM_UPGRADE_MSG, 
				MAKEWPARAM(UPGRADE_ERROR, m_nIndex), 
				(LPARAM)_UPGRADE_ERROR_THIS_VERSION_DOES_NOT_SUPPORT  );
			return FALSE;
		}
	}


	struct sockaddr_in	server;

	server.sin_family		= AF_INET;
	server.sin_port			= htons( (int)m_nUpgradePort );
	server.sin_addr.s_addr	= htonl( GetIPAddress( m_sIPAddress ) );

	m_pSockUpgrade = socket(AF_INET, SOCK_STREAM, 0 );	// SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
	if( m_pSockUpgrade == INVALID_SOCKET) 
	{
		TRACE( L"Index = %d, Client: Error Opening socket: Error %d\n", m_nIndex, WSAGetLastError() );
		Disconnect();
		return FALSE;		
	}

	if( SOCKET_ERROR == connect( m_pSockUpgrade,(struct sockaddr*)&server, sizeof(server) ) ) 
	{
		TRACE( L"Index = %d, connect() failed: %d\n", m_nIndex, WSAGetLastError() );

		Disconnect();
		return FALSE;		
	} 

	SOCKET_MODE_INFO*	pSocketMode	= new SOCKET_MODE_INFO;
	pSocketMode->pThis			= this;
	pSocketMode->iSocketMode	= _UPGRADESOCKET_MODE_CHECKCOMPLETE;


	m_hThreadSocket = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ProcReceiveSocket, (LPVOID)pSocketMode, 0, &this->m_dwThreadSocketID );
	if( NULL == m_hThreadSocket ) 
	{
		Disconnect();
		return FALSE;		
	}

	return TRUE;
}



BOOL CUpgradeThreadTcp::RetryUpgrade()
{
	if( NULL != m_hwndParent )
	{
		wsprintf( m_szDisplayMessage, L"Wait for Retry Upgrade" );
		PostMessage(m_hwndParent, 
			WM_UPGRADE_MSG, 
			MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
			(LPARAM)_UPGRADE_STATUS_ATTEMPT_RETRY );
	}

	MESSAGE_QUEUE_ITEM QueueItemUpgrade	= {0};
	QueueItemUpgrade.iCommandID			= MESSAGE_WAIT_FOR_RETRAY_UPGRADE;
	QueueItemUpgrade.iArg1				= 0;
	m_MessageQueue.PushQueue( QueueItemUpgrade );
	SetEvent(m_hThreadUpgradeEvent);
	return TRUE;
}

BOOL CUpgradeThreadTcp::StartUpgrade()
{
	if( NULL != m_hwndParent )
	{
		wsprintf( m_szDisplayMessage, L"Attempt to connect" );
		PostMessage(m_hwndParent, 
			WM_UPGRADE_MSG, 
			MAKEWPARAM(UPGRADE_SETSTATUS, m_nIndex), 
			(LPARAM)_UPGRADE_STATUS_CONNECTING );
	}

	MESSAGE_QUEUE_ITEM QueueItemUpgrade	= {0};
	QueueItemUpgrade.iCommandID			= MESSAGE_CONNECT_FOR_START;
	QueueItemUpgrade.iArg1				= 0;
	m_MessageQueue.PushQueue( QueueItemUpgrade );
	SetEvent(m_hThreadUpgradeEvent);
	return TRUE;
}

BOOL CUpgradeThreadTcp::ConnectForSWUpgradeWithoutMCU()
{
	Disconnect();

//	Sleep(1000);

	struct sockaddr_in	server;

	server.sin_family		= AF_INET;
	server.sin_port			= htons( (int)m_nUpgradePort );
	server.sin_addr.s_addr	= htonl( GetIPAddress( m_sIPAddress ) );

	m_pSockUpgrade = socket(AF_INET, SOCK_STREAM, 0 );	// SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
	if( m_pSockUpgrade == INVALID_SOCKET) 
	{
		TRACE( L"Index = %d, Client: Error Opening socket: Error %d\n", m_nIndex, WSAGetLastError() );

		Disconnect();
		return FALSE;		
	}

	if( SOCKET_ERROR == connect( m_pSockUpgrade,(struct sockaddr*)&server, sizeof(server) ) ) 
	{
		TRACE( L"Index = %d, connect() failed: %d\n", m_nIndex, WSAGetLastError() );

		Disconnect();
		return FALSE;		
	} 

	SOCKET_MODE_INFO*	pSocketMode	= new SOCKET_MODE_INFO;
	pSocketMode->pThis			= this;
	pSocketMode->iSocketMode	= _UPGRADESOCKET_MODE_SWUPGRADE_WITHOUT_MCU;


	m_hThreadSocket = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ProcReceiveSocket, (LPVOID)pSocketMode, 0, &this->m_dwThreadSocketID );
	if( NULL == m_hThreadSocket ) 
	{
		Disconnect();
		return FALSE;		
	}

	return TRUE;
}

BOOL CUpgradeThreadTcp::ConnectForStart()
{
	Disconnect();

	struct sockaddr_in	server;

	server.sin_family		= AF_INET;
	server.sin_port			= htons( (int)m_nUpgradePort );
	server.sin_addr.s_addr	= htonl( GetIPAddress( m_sIPAddress ) );

	m_pSockUpgrade = socket(AF_INET, SOCK_STREAM, 0 );	// SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
	if( m_pSockUpgrade == INVALID_SOCKET) 
	{
		TRACE( L"Index = %d, Client: Error Opening socket: Error %d\n", m_nIndex, WSAGetLastError() );

		Disconnect();
		return FALSE;		
	}

	if( SOCKET_ERROR == connect( m_pSockUpgrade,(struct sockaddr*)&server, sizeof(server) ) ) 
	{
		TRACE( L"Index = %d, connect() failed: %d\n", m_nIndex, WSAGetLastError() );

		Disconnect();
		return FALSE;		
	} 

	SOCKET_MODE_INFO*	pSocketMode	= new SOCKET_MODE_INFO;
	pSocketMode->pThis			= this;
	pSocketMode->iSocketMode	= _UPGRADESOCKET_MODE_UPGRADE;


	m_hThreadSocket = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ProcReceiveSocket, (LPVOID)pSocketMode, 0, &this->m_dwThreadSocketID );
	if( NULL == m_hThreadSocket ) 
	{
		Disconnect();
		return FALSE;		
	}

	return TRUE;
}


INT  CUpgradeThreadTcp::SendData       ( SOCKET _sock, char* _pbtSendPacket, int _iBytesToSend )
{
	int iSendBytes   = 0;
	int iRemainBytes = 0;
	int iSend        = 0;

	iRemainBytes = _iBytesToSend ;

	while( iRemainBytes > 0 )
	{
		iSend = send(_sock, (char*)(_pbtSendPacket + iSendBytes), iRemainBytes, 0);
		if( iSend <= 0 )
		{
			break;
		}

		if( iSend != iRemainBytes )
		{
			TRACE( "Different Send Size \n" );
		}

		iSendBytes    += iSend;
		iRemainBytes  -= iSend;
	}

	return iSendBytes;
}

BOOL CUpgradeThreadTcp::StopUpgrade()
{
	// end upgrading

	Disconnect();

	return TRUE;
}

