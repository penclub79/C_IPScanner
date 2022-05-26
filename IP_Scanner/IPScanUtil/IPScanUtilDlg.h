
// IPScanUtilDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "NetDef_IPUTIL.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"
#include "ConnectCheckThread.h"
#include <vector>
//#include "MyListCtrl.h"
// 2013-01-30 hkeins : Check List ctrl code separate, jwjang add
#include "CheckListCtrl.h"
#include "VHNetworkAdaptorInfo.h"

// CIPScanUtilDlg 대화 상자
class CIPScanUtilDlg : public CDialog
{
// 생성입니다.
public:
	CIPScanUtilDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_IPSCANUTIL_DIALOG };

	enum {
		SUBITEM_IPADDRESS			= 0 ,
		SUBITEM_MACADDRESS			= 1 ,
		SUBITEM_IS_DHCP				= 2 ,
		SUBITEM_PORTSTREAM			= 3 ,
		SUBITEM_PORTHTTP			= 4 ,
		SUBITEM_PORTUPGRADE			= 5 ,
		SUBITEM_SYSTEMNAME			= 6 ,
		SUBITEM_MODELTYPE			= 7 ,
		SUBITEM_FIRMWAREVERSION		= 8 ,
		SUBITEM_MCU_MODELTYPE		= 9 ,
		SUBITEM_MCU_VERSION			= 10,
		SUBITEM_SUPPORTRESOLUTION	= 11,
		SUBITEM_VIDEOFORMAT			= 12,
		SUBITEM_ALARMINCOUNT		= 13,
		SUBITEM_ALARMOUTCOUNT		= 14,
		SUBITEM_AUDIOINCOUNT		= 15,
		SUBITEM_AUDIOOUTCOUNT		= 16, 
	};

	int  CompareScanInfo(int nItemColumn, tagSCAN_STRUCT* pInfo1, tagSCAN_STRUCT* pInfo2);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


	//CListCtrl m_cSvrList;
	CCheckListCtrl m_cSvrList;
	CButton m_btnScan;
	CButton m_btnChangeIP;
	CButton m_btnClearList;
	CButton m_btnUpgrade;
	CComboBox m_cmbProtocol;
	CComboBox m_cmbNetAdaptor;

	/////////////////// Scanning Class 
	CNetScanVision* m_pScannerVision;
	CNetScanMarkIn* m_pScannerMarkIn;
	//////////////////////////////////

	BOOL	m_bScanning;
	int     m_nListItemCount;
	int		m_nScanAniCount;
	int		m_nCurSvrListSel;
	BOOL    m_bInit;
	CRITICAL_SECTION m_mt;
	CMenu   m_cPopupMenu;
	BOOL	m_bSortAscending;
	int     m_nSortOrient;
	int     m_iSelectVersion;
	ULONG   m_ulAcceptAddress;
	//WCHAR	m_szTempValue[256 * 8];
	CFont   m_DISP_FONT;

	void  _ReadBindAddress(); // read accept address from UI
	void  _LoadNetworkAdaptorInformation();
	void  _ClearAdaptorInfos();
	//WCHAR*	_mbcs2widechar(char* pString);

	// TEST : Connection check test
	//std::vector<CConnectCheckThread*> m_CheckThreadList;
	//void _ClearThreadList();

	VHNetworkAdaptorInfo m_netInfo;
// 구현입니다.
protected:
	HICON			m_hIcon;

	void AddProtocolToCombo();
	void SetStatusMsg(WCHAR* szMsg);
	void SetStatusMsg(CString& strMsg);
	void SetCountMsg(int nCount);
	void ClearScanList();
	int _GetScanErrorMessage(UINT32 uErrorCode, CString& strMsg);
	BOOL CallLayoutManager();
	int _GetInsertPosition(SCAN_INFO* pInfo);
	void _Lock();
	void _Unlock();


// 생성된 메시지 맵 함수
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
	afx_msg void OnBnClickedTestBtn();
	afx_msg void OnBnClickedTestBtn2();
	afx_msg void OnDestroy();
	afx_msg void OnNMRClickSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSetupResolution();
	afx_msg void OnSetupOSD();
	afx_msg void OnNMClickSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedUpgradeBtn();
	afx_msg void OnBnClickedChangeipBtn2();
	afx_msg void OnNMDblclkSvrList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkSvrList3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedFactoryBtn();
	afx_msg void OnCbnSelchangeProtocalCombo();
	LRESULT OnConnectCheck(WPARAM wParam, LPARAM lParam);
	//WM_SORT_REQUEST
	afx_msg LRESULT OnSortRequest(WPARAM wParam, LPARAM lParam);
	
	afx_msg void OnCbnSelchangeAdaptorCmb();
};
