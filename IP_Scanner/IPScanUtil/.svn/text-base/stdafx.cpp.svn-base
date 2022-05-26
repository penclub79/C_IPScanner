
// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// IPScanUtil.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"

// return parsed IP number
// parse IP "a.b.c.d" format
inline BOOL _ParseIP(WCHAR* szIP, int* nP1, int* nP2, int* nP3, int* nP4)
{
	if(szIP == NULL || wcscmp(szIP, L"") == 0)
		return FALSE;

	// IPv4 parsing test
	WCHAR szTemp[30] = {0, };
	WCHAR *szPtrPrev = NULL, *szPtr = NULL;
	
	szPtrPrev = szPtr;
	szPtr = wcschr(szIP, L'.');
	if(szPtr == NULL)
		return FALSE;

	memset(szTemp, 0, sizeof(szTemp));
		
	wcsncpy_s(szTemp, szIP, szPtr - szIP);
	//TRACE(L"F1: %s\n", szTemp);
	if(nP1) *nP1 = _wtoi(szTemp);
	szPtrPrev = szPtr;
	szPtr = wcschr(szPtr + 1, L'.');
	if(szPtr == NULL)
		return FALSE;

	memset(szTemp, 0, sizeof(szTemp));
	if(szPtr - szPtrPrev > 0)
		wcsncpy_s(szTemp, szPtrPrev + 1, szPtr - szPtrPrev - 1);

	//TRACE(L"F2: %s\n", szTemp);
	if(nP2) *nP2 = _wtoi(szTemp);
	szPtrPrev = szPtr;
	szPtr = wcschr(szPtr + 1, L'.');
	if(szPtr == NULL)
		return FALSE;

	memset(szTemp, 0, sizeof(szTemp));
	if(szPtr - szPtrPrev > 0)
		wcsncpy_s(szTemp, szPtrPrev + 1, szPtr - szPtrPrev - 1);
	
	//TRACE(L"F3: %s\n", szTemp);
	if(nP3) *nP3 = _wtoi(szTemp);
	// etc F4
	memset(szTemp, 0, sizeof(szTemp));
	wcscpy_s(szTemp, 30, szPtr + 1);
	//TRACE(L"F4: %s\n", szTemp);
	if(nP4) *nP4 = _wtoi(szTemp);

	return TRUE;
}

// Compare to IP string equal = 0, more = 1 less = -1
int  _CompareIP(WCHAR* szIP1, WCHAR* szIP2)
{
	//{{ TEST 2 : parse IP and compare each digit
	int nP1[4] = { 0, };
	int nD1[4] = { 0, };
	int i = 0;
	if(!_ParseIP(szIP1, &nP1[0], &nP1[1], &nP1[2], &nP1[3]) || !_ParseIP(szIP2, &nD1[0], &nD1[1], &nD1[2], &nD1[3]))
	{
		ASSERT(0);
		return 0;
	}
	for(i = 0; i < 4; i++)
	{
		if(nP1[i] > nD1[i])
			return 1;
		else if(nP1[i] < nD1[i])
			return -1;
		// 현재 비교하는 자리가 같으면 다음 자리 비교
	}
	//}} TEST2
	return 0;
}
