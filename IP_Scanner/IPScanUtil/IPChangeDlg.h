#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "OXMaskedEdit.h"

// CIPChangeDlg 대화 상자입니다.

class CIPChangeDlg : public CDialog
{
	DECLARE_DYNAMIC(CIPChangeDlg)

public:
	CIPChangeDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CIPChangeDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CAM_IPCHANGE };
private:
	CRect			m_rcWnd;

public:
	CString			m_strMACAddress;
	CString         m_strOldMACAddress;
	CString			m_strIPAddress;
	CString			m_strGatewayAddress;
	CIPAddressCtrl	m_ctrlIPAddress;
	CIPAddressCtrl	m_ctrlGatewayAddress;
	CIPAddressCtrl	m_ctrlSubnetMask;
	int				m_nStreamPort;
	int				m_nUpgdPort;
	int				m_nHTTPPort;
	int				m_nScanCount;

	int				m_nVersion;
	int             m_nIsDHCP; // 0 Static, 1 - DHCP
	int				m_nCurrentIsDHCP;
	CString         m_strSubnetMask;
	CString			m_strID;
	CString			m_strPassword;
	COXMaskedEdit	m_editMACAddr;

	void			SetDHCPMode(BOOL bMode);
protected:
	BOOL			_ValidateMacAddress();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	
public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnMoveWnd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButReset();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedNetDhcp();
	afx_msg void OnBnClickedNetStatic2();
};
