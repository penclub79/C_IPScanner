#pragma once
#include <afxinet.h>
#include "afxwin.h"
#include "afxcmn.h"

#include "resource.h"
#include "progresslistctrl.h"
#include "NetScanVision.h"
#include "OSDChangeThread.h"


// COSDChangeDlg 대화 상자입니다.

class COSDChangeDlg : public CDialog
{
	DECLARE_DYNAMIC(COSDChangeDlg)

public:
	COSDChangeDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~COSDChangeDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CAM_OSDCHANGE };
	enum {	SUBITEM_MODEL					= 0 ,
			SUBITEM_IPADDRESS				= 1 ,
			SUBITEM_SETTEXT					= 2 ,
			SUBITEM_USER_ID					= 3 ,
			SUBITEM_USER_PW					= 4 ,
			SUBITEM_STATUS					= 5 };

	BOOL	SetScaninfo( int iCount, SCAN_INFO* pScanInfo, int iAdapterID);
	int		GetSelectScanInfoCount();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()


private:
	void				CheckArp(int iAdapterID);
	void				EnableControlMain( BOOL bEnable );

	int					m_iAdapterID;
	int					m_nScanInfoCnt;
	SCAN_INFO*			m_pScanInfo;


	CInternetSession*	m_pSession;
	CHttpConnection*	m_pHttpConnect;
	CHttpFile*			m_pHttpFile;

	CMap < int, int, COSDChangeThread*, COSDChangeThread* > m_aSettingThread;

public:
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedCheckSetosdIpadd();
	virtual BOOL OnInitDialog();



	CProgressListCtrl	m_ctlListDevice;
	CButton				m_ctlCheckSetOSDIPADD;
	CStatic				m_ctlStaticOverlayText;
	CEdit				m_ctlEditOverlayText;
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnEnChangeEditOverlaytext();
	afx_msg void OnEnChangeUserid();
	afx_msg void OnEnChangePassword();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnMessageStatus(WPARAM wParam, LPARAM lParam);
	CButton m_ctlButtonApply;
	CButton m_ctlButtonCancel;
	CComboBox m_ctlComboPosition;
	CButton m_ctlCheckUsed;
	afx_msg void OnBnClickedCheckUsed();
	CStatic m_ctlStaticTextPosition;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CString m_strOverlayText;
};
