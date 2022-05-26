#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "OXMaskedEdit.h"
#include "NetScanVision.h"
#include "ProgressListCtrl.h"
#include "IPChangeThread.h"
#include "ListItem.h"

// CIPChangeDlg2 대화 상자입니다.


class CIPChangeDlg2 : public CDialog
{
	DECLARE_DYNAMIC(CIPChangeDlg2)

public:
	CIPChangeDlg2(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CIPChangeDlg2();

	void SetScanInfo( int iScanInfoCount, SCAN_INFO* pScanInfo, int iSelScanInfoCount, SCAN_INFO* pSelScanInfo); // SCAN_INFO[m_nSelectedCnt] array values
	void SetScanner(CNetScanVision * pScanner);


	// 대화 상자 데이터입니다.
	enum { IDD = IDD_CAM_IPCHANGE2 };
	enum {
		SUBITEM_MACADDRESS			= 0 ,
		SUBITEM_IPADDRESS			= 1 ,
		SUBITEM_TO_IPADDRESS		= 2 ,
		SUBITEM_TO_GATEWAY			= 3 ,
		SUBITEM_IS_DHCP				= 4 ,
		SUBITEM_PORTSTREAM			= 5 ,
		SUBITEM_PORTHTTP			= 6 ,
		SUBITEM_USER_ID				= 7 ,
		SUBITEM_USER_PW				= 8 ,
		SUBITEM_STATUS				= 9

		//SUBITEM_SYSTEMNAME			= 9 ,
		//SUBITEM_MODELTYPE			= 10,
		//SUBITEM_FIRMWAREVERSION		= 11,
		//SUBITEM_SUPPORTRESOLUTION	= 12,
		//SUBITEM_VIDEOFORMAT			= 13,
		//SUBITEM_ALARMINCOUNT		= 14,
		//SUBITEM_ALARMOUTCOUNT		= 15,
		//SUBITEM_AUDIOINCOUNT		= 16,
		//SUBITEM_AUDIOOUTCOUNT		= 17 
	};

private:
	void				AddData();
	void				RefreshData();

	void				SetDHCPMode(BOOL bMode);

	BOOL				ScanInfoLock();
	void				ScanInfoUnLock();

	HANDLE				m_hMutexScanInfo;

	CRect				m_rcWnd;

	CString				m_strOrgIPAddress;
	CString				m_strOrgGatewayAddress;
	CString				m_strOrgSubnetMask;
	int					m_nOrgStreamPort;
	int					m_nOrgHTTPPort;

	CString				m_strCurIPAddress;
	CString				m_strCurGatewayAddress;
	CString				m_strCurSubnetMask;
	int					m_nCurStreamPort;
	int					m_nCurHTTPPort;

	CMap < int, int, CIPChangeThread*, CIPChangeThread* > m_aSettingThread;

	int					m_nSelScanInfoCnt;
	int					m_nIPChangeEnd;
	SCAN_INFO*			m_pSelScanInfo; 

	int					m_nScanInfoCnt;
	SCAN_INFO*			m_pScanInfo; // SCAN_INFO[m_nSelectedCnt] array values

	CProgressListCtrl	m_cSvrList;
	CString				m_strMACAddress;
	CString				m_strIPAddress;
	CString				m_strGatewayAddress;
	CIPAddressCtrl		m_ctrlIPAddress;
	CIPAddressCtrl		m_ctrlGatewayAddress;
	CIPAddressCtrl		m_ctrlSubnetMask;
	unsigned int		m_nStreamPort;
	unsigned int		m_nHTTPPort;
	int					m_nScanCount;

	int					m_nVersion;
	int					m_nIsDHCP; // 0 Static, 1 - DHCP
	int					m_nCurrentIsDHCP;
	CString				m_strSubnetMask;
	CString				m_strID;
	CString				m_strPassword;
	COXMaskedEdit		m_editMACAddr;

	CNetScanVision *	m_pScanner;

	CListItem*			m_pListItem;

	int					m_iCheckCount;


protected:
//	BOOL				_ValidateMacAddress(CString strMacAddress);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	//int			CallHttpRequest(CString ipaddr, int port, CString query, CString &response);
	//BOOL		ConnectionCheck(CString ipaddr, int port);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);


public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnMoveWnd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButReset();
	afx_msg void OnBnClickedNetDhcp();
	afx_msg void OnBnClickedNetStatic2();
	afx_msg void OnEnChangeStartMacaddr();
	afx_msg void OnLvnItemchangedSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangePassword();
	afx_msg void OnEnChangeUserid();
	afx_msg void OnEnChangeStreamPortEdit();
	afx_msg void OnEnChangeHttpPortEdit();
	afx_msg LRESULT OnIPChangeMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScanMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedCheckSameip();

	CButton m_ctlButtonApply;
	CButton m_ctlButtonCancel;
	CButton m_ctlCheckSameIP;
};
