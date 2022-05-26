#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "NetScanVision.h"
#include "UpgradeThreadTcp.h"
#include "UpgradeThreadHttp.h"
#include "ProgressListCtrl.h"

typedef struct tagUPGRADETHREAD {

	CUpgradeThreadTcp*	pUpgradeThreadTcp;
	CUpgradeThreadHttp*	pUpgradeThreadHttp;
	int					iUpgradeType;
	
	tagUPGRADETHREAD()
	{
		pUpgradeThreadTcp	= NULL;
		pUpgradeThreadHttp	= NULL;
		iUpgradeType		= 0;
	}

	~tagUPGRADETHREAD() 
	{
		if( pUpgradeThreadTcp )
			delete pUpgradeThreadTcp;
		
		if( pUpgradeThreadHttp )
			delete pUpgradeThreadHttp;

		pUpgradeThreadTcp	= NULL;
		pUpgradeThreadHttp	= NULL;
	}

} UPGRADETHREAD, *PUPGRADETHREAD;

// CUpgradeDlg 대화 상자입니다.

class CUpgradeDlg : public CDialog
{
public:
	CUpgradeDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CUpgradeDlg();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_UPGRADE };

	enum {
		SUBITEM_IPADDRESS			= 0 ,
		SUBITEM_MODEL				= 1 ,
		SUBITEM_FIRMWAREVERSION_FROM= 2 ,
		SUBITEM_FIRMWAREVERSION_TO	= 3 ,
		SUBITEM_ID					= 4 ,
		SUBITEM_PASSWORD			= 5 ,
		SUBITEM_PROGRESS			= 6 ,
		SUBITEM_STATUS				= 7 };


	enum TYPE_PROTOCOL {
		TYPE_NONE	= 0x00,
		TYPE_TCP	,
		TYPE_HTTP	,
	};
	enum {
		FILE_OPEN_ERROR = -1,
		UNKNOWN_FILE = 0,
		HTTP_UPGRADE_FILE = 1,
		TCP_UPGRADE_FILE = 2
	};


private:
	CRect			m_rcWnd;
	CString			m_strFilePath;
	CString			m_strFileName;
	CString			m_strID;
	CString			m_strPassword;
	CString			m_strModel;
	int				m_iSelectVersion;
	BOOL			m_bInit;

	void AddData();
	void AddModelToCombo();
	void AddProtocolToCombo();

public:
	CProgressListCtrl m_cUpgradeList;
	CButton m_btnOpenFile;
	CButton m_btnUpgrade;
	CButton m_btnClose;
	int m_nModelCnt;

	void SetScaninfo( int iCount, SCAN_INFO*	pScanInfo);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	BOOL			_ValidateMacAddress();
	void			CallLayoutManager();
	int				_CheckUpgradeFileType(WCHAR* szUpgradeFileName);

	int				m_nUpgradeCnt;
	int				m_nRetryUpgradeID;
	SCAN_INFO*		m_pScanInfo; // SCAN_INFO[m_nUpgradeCnt] 배열

	CMap < int, int, UPGRADETHREAD*, UPGRADETHREAD* > m_aUpgradeThread;

	CComboBox		m_cmbModel;
	int				m_iRebootCheck;

public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnMoveWnd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOpenFile();
	afx_msg void OnRetryUpgrade();
	afx_msg void OnBnClickedUpgrade();
	afx_msg void OnBnClickedClose();
	afx_msg void OnSelchangeCmbModel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	afx_msg LRESULT OnUpgradeMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRetryUpgradeMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnChangeUpgradeUserid();
	afx_msg void OnEnChangeUpgradePassword();
	afx_msg void OnNMRClickUpgradeList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedUpgradeList(NMHDR *pNMHDR, LRESULT *pResult);
};

