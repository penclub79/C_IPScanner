
// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.


#include <afxdisp.h>        // MFC 자동화 클래스입니다.



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원


#include <afxsock.h>            // MFC 소켓 확장
#include "afxcmn.h"
#include "afxwin.h"


// includes
#include "netdef.h"
#include "netdef2.h"
#include "NetDef_IPUTIL.h"

#ifndef _DEFAULT_ID_PASSWORD_
//#define _DEFAULT_ID_PASSWORD_
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if((x) != NULL) { delete (x); (x) = NULL; }
#endif

#ifndef SAFE_DELETEA
#define SAFE_DELETEA(x) if((x) != NULL) { delete[] (x); (x) = NULL; }
#endif

const UINT WM_SCAN_MSG				= WM_USER + 1000;
const UINT WM_SCAN_CLOSE_DLG_MSG	= WM_USER + 1001;
const UINT WM_CONNECT_CHECK			= WM_USER + 1002;
const UINT WM_SORT_REQUEST			= WM_USER + 1003;

const UINT WM_UPGRADE_MSG			= WM_USER + 205;
const UINT WM_RETRY_UPGRADE_MSG		= WM_USER + 206;

const UINT TM_SCANNING_ANI	= 1000; // scanning animate timer
//const UINT TM_SCANNING		= 1001; // scan request timer


// 2013-01-30 hkeins : sort function impl.
extern int  _CompareIP(WCHAR* szIP1, WCHAR* szIP2);
inline BOOL _ParseIP(WCHAR* szIP, int* nP1, int* nP2, int* nP3, int* nP4);


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


