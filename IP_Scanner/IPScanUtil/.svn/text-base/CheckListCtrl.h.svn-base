//
// Copyright (C) Since 2013 VISIONHITECH. All rights reserved.
// 
// Description: ip camera connection check class
// 2013-01-30 hkeins : Check List ctrl code separate, jwjang add
// first column check box and column click separation
//
//
#ifndef _CHECK_LIST_CTRL_V1_
#define _CHECK_LIST_CTRL_V1_
#pragma once
/////////////////////////////////////////////////////////////////////////////
// CCheckHeadCtrl window

class CCheckHeadCtrl : public CHeaderCtrl
{
	// Construction
public:
	CCheckHeadCtrl();

	// Attributes
public:

	// Operations
public:

	BOOL ClearState();
	void SetChecked(BOOL nChecked);
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckHeadCtrl)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CCheckHeadCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckHeadCtrl)
	afx_msg void OnItemClicked(NMHDR* pNMHDR, LRESULT* pResult);
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	BOOL m_bAscending;
	BOOL m_bChecked;
	int	 m_nLastOrient; // 마지막 선택 컬럼

	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
};


class CCheckListCtrl : public CListCtrl
{
	// Construction
public:
	CCheckListCtrl();

	// Attributes
public:

	// Operations
private:
	BOOL	m_blInited;
	CImageList	m_checkImgList;


public:
	BOOL Init();
	BOOL ClearState(); // 초기 상태로

protected:
	CCheckHeadCtrl	m_checkHeadCtrl;
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckListCtrl)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CCheckListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckListCtrl)
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);		
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
#endif
