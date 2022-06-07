// IPChangeDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "IPChangeDlg.h"
#include "NetCommon.h"
#include "NetScanVision.h"
#include "NetScanMarkIn.h"

IMPLEMENT_DYNAMIC(CIPChangeDlg, CDialog)

CIPChangeDlg::CIPChangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIPChangeDlg::IDD, pParent)
	, m_strMACAddress(L"")
	, m_strIPAddress(L"")
	, m_strGatewayAddress(L"")
	, m_nStreamPort(0)
	, m_nHTTPPort(0)
	, m_nScanCount(0)
	, m_nVersion(1)
	, m_nIsDHCP(0)
	, m_nCurrentIsDHCP(0)
	, m_strSubnetMask(L"255.255.255.0")
	, m_strID(_T(""))
	, m_strPassword(_T(""))
{
}

CIPChangeDlg::~CIPChangeDlg()
{
}

void CIPChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDR, m_ctrlIPAddress);
	DDX_Control(pDX, IDC_GWADDR, m_ctrlGatewayAddress);
	DDX_Text(pDX, IDC_STREAM_PORT_EDIT, m_nStreamPort);
	DDX_Text(pDX, IDC_HTTP_PORT_EDIT, m_nHTTPPort);
	DDX_Control(pDX, IDC_SUBNETMASK, m_ctrlSubnetMask);
	DDX_Text(pDX, IDC_USERID, m_strID);
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Control(pDX, IDC_MACADDR, m_editMACAddr);
}


BEGIN_MESSAGE_MAP(CIPChangeDlg, CDialog)
	ON_BN_CLICKED(IDC_BUT_RESET, &CIPChangeDlg::OnBnClickedButReset)
	ON_BN_CLICKED(IDOK, &CIPChangeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CIPChangeDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_NET_DHCP, &CIPChangeDlg::OnBnClickedNetDhcp)
	ON_BN_CLICKED(IDC_NET_STATIC2, &CIPChangeDlg::OnBnClickedNetStatic2)
END_MESSAGE_MAP()


// CIPChangeDlg 메시지 처리기입니다.

BOOL CIPChangeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_editMACAddr.SetPromptSymbol(_T('0'));
	m_editMACAddr.SetMask(_T("hh:hh:hh:hh:hh:hh"));

	GetClientRect(&m_rcWnd);
	CString str; str.LoadString(IDS_IP_CHANGE);
/////
	//ASSERT(m_strMACAddress != L""); // Dialog를 DoModal로 실행하기 전에 띄워줘야한다
	m_nCurrentIsDHCP = m_nIsDHCP;
	if(m_strSubnetMask == L"")
		m_strSubnetMask = L"255.255.255.0";

	if(m_nVersion == VERSION_2)
	{
		GetDlgItem(IDC_NET_DHCP)->EnableWindow(TRUE);
		GetDlgItem(IDC_SUBNETMASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_USERID)->EnableWindow(TRUE);
		GetDlgItem(IDC_PASSWORD)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_NET_DHCP)->EnableWindow(FALSE);
		GetDlgItem(IDC_SUBNETMASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_USERID)->EnableWindow(FALSE);
		GetDlgItem(IDC_PASSWORD)->EnableWindow(FALSE);
	}

/////
	OnBnClickedButReset();
	m_strOldMACAddress = m_strMACAddress;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CIPChangeDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}

HBRUSH CIPChangeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

LRESULT CIPChangeDlg::OnMoveWnd(WPARAM wParam, LPARAM lParam)
{
	CRect rc; GetWindowRect(&rc);
	MoveWindow(rc.left - (LONG)wParam, rc.top - (LONG)lParam, rc.Width(),rc.Height());
	return 0L;
}

void CIPChangeDlg::OnBnClickedButReset()
{
	SetDlgItemText(IDC_MACADDR,	m_strMACAddress);
	SetDlgItemText(IDC_IPADDR,	m_strIPAddress);
	SetDlgItemText(IDC_GWADDR,	m_strGatewayAddress);
	SetDlgItemInt(IDC_STREAM_PORT_EDIT,	m_nStreamPort);
	SetDlgItemInt(IDC_HTTP_PORT_EDIT,	m_nHTTPPort);
	SetDlgItemText(IDC_SUBNETMASK, m_strSubnetMask);
	SetDlgItemText(IDC_USERID,   L"");
	SetDlgItemText(IDC_PASSWORD, L"");
	SetDHCPMode(m_nIsDHCP); // DHCP - 0, Static - 1
}

void CIPChangeDlg::OnBnClickedOk()
{
	GetDlgItemText(IDC_MACADDR,	m_strMACAddress);
	m_strMACAddress.MakeLower();
	if(!_ValidateMacAddress()) // MAC address validation Check
	{
		MessageBox(L"Invalid MAC address");
		GetDlgItem(IDC_MACADDR)->SetFocus();
		return;
	}
	GetDlgItemText(IDC_IPADDR,	m_strIPAddress);
	GetDlgItemText(IDC_GWADDR,	m_strGatewayAddress);
	GetDlgItemText(IDC_SUBNETMASK, m_strSubnetMask);
	m_nStreamPort = GetDlgItemInt(IDC_STREAM_PORT_EDIT);
	m_nHTTPPort   = GetDlgItemInt(IDC_HTTP_PORT_EDIT);
	GetDlgItemText(IDC_USERID,  m_strID);
	GetDlgItemText(IDC_PASSWORD,m_strPassword);
	if(m_nVersion == VERSION_2 && m_strID.IsEmpty())
	{
		CString str;
		str.LoadString(IDS_ENTER_LOGIN_INFO);
		AfxMessageBox(str, MB_ICONWARNING);
		GetDlgItem(IDC_USERID)->SetFocus();
		return;
	}
	m_nIsDHCP     = m_nCurrentIsDHCP;
	// 데이터 유효성 검사?
	// 변경 요청 후 다이얼로그 닫기
	if(m_nVersion == VERSION_1)
	{
		CNetScanVision::RequestIPChange(
			(LPTSTR)(LPCTSTR)m_strMACAddress, (LPTSTR)(LPCTSTR)m_strIPAddress, (LPTSTR)(LPCTSTR)m_strGatewayAddress,
			m_nStreamPort, m_nHTTPPort);
	}
	if(m_nVersion == VERSION_2)
	{
	CNetScanVision::RequestIPChange2(
		(LPTSTR)(LPCTSTR)m_strMACAddress, (LPTSTR)(LPCTSTR)m_strIPAddress, (LPTSTR)(LPCTSTR)m_strGatewayAddress,
		m_nStreamPort, m_nHTTPPort, m_nIsDHCP, (LPTSTR)(LPCTSTR)m_strSubnetMask, m_strID.GetBuffer(1024), m_strPassword.GetBuffer(1024), 0);
	m_strID.ReleaseBuffer();
	m_strPassword.ReleaseBuffer();
	}
	else
	{
		ASSERT(0); // not support
	}
	// wait a seconds progress dialog here
	// FIX ME : show dialog:!

	OnOK();
}

void CIPChangeDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CIPChangeDlg::OnBnClickedNetDhcp()
{
	m_nCurrentIsDHCP = 1;
	SetDHCPMode(TRUE);
}

void CIPChangeDlg::OnBnClickedNetStatic2()
{
	m_nCurrentIsDHCP = 0;
	SetDHCPMode(FALSE);
}


void CIPChangeDlg::SetDHCPMode(BOOL bMode)
{
	CButton *pButton[2];
	pButton[0] = (CButton*)GetDlgItem(IDC_NET_DHCP);
	pButton[1] = (CButton*)GetDlgItem(IDC_NET_STATIC2);

	if(pButton[0] && pButton[1])
	{
		pButton[0]->SetCheck(bMode);
		pButton[1]->SetCheck(!bMode);
	}
	else
	{
		ASSERT(0);
	}

	GetDlgItem(IDC_STATIC_IPADDR)->EnableWindow(!bMode);
	GetDlgItem(IDC_IPADDR)->EnableWindow(!bMode);
	GetDlgItem(IDC_STATIC_GATEWAY)->EnableWindow(!bMode);
	GetDlgItem(IDC_GWADDR)->EnableWindow(!bMode);
	if(m_nVersion == VERSION_1)
	{
		GetDlgItem(IDC_STATIC_MASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_SUBNETMASK)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_STATIC_MASK)->EnableWindow(!bMode);
		GetDlgItem(IDC_SUBNETMASK)->EnableWindow(!bMode);
	}
}

BOOL CIPChangeDlg::_ValidateMacAddress()
{
	int i;

	// 빈 경우
	if(m_strMACAddress.Compare(L"") == 0)
		return FALSE;

	// 자리수가 넘어간 경우
	if(m_strMACAddress.GetLength() != 17)
		return FALSE;

	// 허용되지 않는 문자가 들어간 경우
	WCHAR ch;

	for(i = 0; i < m_strMACAddress.GetLength(); i++)
	{	
		ch = m_strMACAddress.GetAt(i);
		if((ch == L':') ||
		(ch >= L'0' && ch <= L'9') ||
		(ch >= L'a' && ch <= L'f') ||
		(ch >= L'A' && ch <= L'F'))
		{
			// OK bypass
		}
		else
		{
			return FALSE;
		}

	}
	// 요런 포멧이 아닌 경우
	// 00:00:00:00:00:00
	int nPos = 0;
	int nColumnCount = 0;
	while(nPos >= 0)
	{
		nPos = m_strMACAddress.Find(L':', nPos + 1);
		if(nPos >= 0) nColumnCount++;
	}
	if(nColumnCount != 5) // : 개수가 5개가 아닌 경우
		return FALSE;

	return TRUE;
}