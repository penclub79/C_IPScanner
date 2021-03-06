#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "OXMaskedEdit.h"
#include "NetScanVision.h"
#include "ProgressListCtrl.h"

// CFactoryDefaultDlg 대화 상자입니다.

class CFactoryDefaultDlg : public CDialog
{
	DECLARE_DYNAMIC(CFactoryDefaultDlg)

public:
	CFactoryDefaultDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CFactoryDefaultDlg();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_FACTORYDEFAULT };
	enum {
		SUBITEM_IP               = 0,
		SUBITEM_MAC              = 1,
		SUBITEM_ID               = 2,
		SUBITEM_PASS             = 3,
		SUBITEM_MODEL            = 4,
		SUBITEM_FIRMWARE_VERSION = 5,
		SUBITEM_STREAM_PORT      = 6,
		SUBITEM_HTTP_PORT        = 7,
		SUBITEM_UPGRADE_PORT     = 8,
		SUBITEM_SERVER_NAME      = 9,
		SUBITEM_RESOLUTION       = 10,
		SUBITEM_VIDEO_FORMAT     = 11,
		SUBITEM_ALARMIN_CNT      = 12,
		SUBITEM_ALARMOUT_CNT     = 13,
		SUBITEM_AUDIOIN_CNT      = 14,
		SUBITEM_AUDIOOUT_CNT     = 15
	};
private:
	CRect			m_rcWnd;

	void			AddData();

public:
	CProgressListCtrl  m_cSvrList;
	CString			   m_strID;
	CString			   m_strPassword;

	int				   m_nSelectedCnt;
	SCAN_INFO*		   m_pScanInfo;		// SCAN_INFO[m_nSelectedCnt] 배열
protected:
	int CallHttpRequest(CString ipaddr, int port, CString query, CString &response);
	BOOL ConnectionCheck(CString ipaddr, int port);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnMoveWnd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLvnItemchangedSvrList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeUserid();
	afx_msg void OnEnChangePassword();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	CButton m_ctlButtonApply;
	CButton m_ctlButtonCancel;
};
