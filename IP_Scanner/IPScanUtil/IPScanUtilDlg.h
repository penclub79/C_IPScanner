
// IPScanUtilDlg.h : ��� ����
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "NetDef_IPUTIL.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"

#include "ConnectCheckThread.h"
#include <vector>
//#include "MyListCtrl.h"
// 2013-01-30 hkeins : Check List ctrl code separate, jwjang add
#include "CheckListCtrl.h"
#include "VHNetworkAdaptorInfo.h"

// CIPScanUtilDlg ��ȭ ����
class CIPScanUtilDlg : public CDialog
{
// �����Դϴ�.
public:
	CIPScanUtilDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_IPSCANUTIL_DIALOG };

	enum {
		SUBITEM_IPADDRESS			= 0 ,
		SUBITEM_MACADDRESS			= 1 ,
		SUBITEM_IS_DHCP				= 2 ,
		//SUBITEM_PORTSTREAM		= 3 ,
		SUBITEM_PORTHTTP			= 3 ,
		SUBITEM_PORTUPGRADE			= 4 ,
		SUBITEM_PORTBASE			= 5 ,
		//SUBITEM_SYSTEMNAME			= 5 ,
		SUBITEM_MODELTYPE			= 6 ,
		SUBITEM_FIRMWAREVERSION		= 7 ,
		SUBITEM_SW_VERSION			= 8 ,
		SUBITEM_VIDEOCOUNT			= 9 ,
		//SUBITEM_MCU_MODELTYPE		= 10,
		//SUBITEM_MCU_VERSION		= 11,
		//SUBITEM_SUPPORTRESOLUTION	= 12,
		//SUBITEM_VIDEOFORMAT		= 13,
		//SUBITEM_ALARMINCOUNT		= 14,
		//SUBITEM_ALARMOUTCOUNT		= 15,
		//SUBITEM_AUDIOINCOUNT		= 16,
		//SUBITEM_AUDIOOUTCOUNT		= 17, 
	};

	int  CompareScanInfo(int nItemColumn, tagSCAN_STRUCT* pInfo1, tagSCAN_STRUCT* pInfo2);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


	//CListCtrl m_cSvrList;
	CCheckListCtrl m_cSvrList;
	CButton m_btnScan;
	CButton m_btnChangeIP;
	CButton m_btnClearList;
	CButton m_btnUpgrade;
	//CComboBox m_cmbProtocol;
	CComboBox m_cmbNetAdaptor;
	CIPScanUtilDlg* m_dlg;
	/////////////////// Scanning Class 
	CNetScanVision* m_pScannerVision;
	CNetScanMarkIn* m_pScannerMarkIn;
	//////////////////////////////////

	BOOL	m_bScanning;
	int     m_nListItemCount;
	int		m_nScanAniCount;
	int		m_nCurSvrListSel;
	BOOL    m_bInit;

	/*
	������ ������ ����ȭ
	CRITICAL_SECTION ��ü�� �ʱ�ȭ�Ѵ�. ��ȯ���� ����.
	������忡�� ����(Ŀ�� ������Ʈ�� �ƴ�)
	�Ӱ迵���� ������ ���� CRITICAL_SECTION ������Ʈ ����(key)�� ��� ���
	*/
	CRITICAL_SECTION m_mt;

	CMenu   m_cPopupMenu;
	BOOL	m_bSortAscending;
	int     m_nSortOrient;
	int     m_iSelectVersion;
	ULONG   m_ulAcceptAddress;
	
	// Window GDI(�׷��� ����̽� �������̽�) �۲��� ĸ��ȭ�ϰ� �۲� ������ ���� ��� �Լ��� ����
	CFont   m_DISP_FONT;

	void  _ReadBindAddress(); // read accept address from UI
	void  _LoadNetworkAdaptorInformation();
	void  _ClearAdaptorInfos();
	//WCHAR*	_mbcs2widechar(char* pString);

	// TEST : Connection check test
	//std::vector<CConnectCheckThread*> m_CheckThreadList;
	//void _ClearThreadList();

	VHNetworkAdaptorInfo m_netInfo;
// �����Դϴ�.
protected:
	HICON			m_hIcon;

	//void AddProtocolToCombo();
	void SetStatusMsg(WCHAR* szMsg);
	void SetStatusMsg(CString& strMsg);
	void SetCountMsg(int nCount);
	void ClearScanList();
	int _GetScanErrorMessage(UINT32 uErrorCode, CString& strMsg);
	BOOL CallLayoutManager();
	int _GetInsertPosition(SCAN_INFO* pInfo);
	void _Lock();
	void _Unlock();


// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedScanBtn();
	afx_msg void OnBnClickedChangeipBtn();
	afx_msg void OnBnClickedClose();
	afx_msg void OnClose();
	afx_msg void OnBnClickedClearBtn();
	afx_msg LRESULT OnScanMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScanCloseDlgMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLvnItemchangedSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	/*afx_msg void OnBnClickedTestBtn();
	afx_msg void OnBnClickedTestBtn2();*/
	afx_msg void OnDestroy();
	afx_msg void OnNMRClickSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSetupResolution();
	afx_msg void OnSetupOSD();
	afx_msg void OnNMClickSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnBnClickedUpgradeBtn();
	afx_msg void OnBnClickedChangeipBtn2();
	afx_msg void OnNMDblclkSvrList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkSvrList3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedFactoryBtn();
	//afx_msg void OnCbnSelchangeProtocalCombo();
	LRESULT OnConnectCheck(WPARAM wParam, LPARAM lParam);
	//WM_SORT_REQUEST
	afx_msg LRESULT OnSortRequest(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCbnSelchangeAdaptorCmb();
	//afx_msg void OnCbnSelchangeProtocalCombo();
};
