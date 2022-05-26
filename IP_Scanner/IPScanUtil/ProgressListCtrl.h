#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CAdvHeaderCtrl window

class CAdvHeaderCtrl : public CHeaderCtrl
{
public:
	CAdvHeaderCtrl();

public:
	BOOL Init(CHeaderCtrl *pHeader);
	virtual ~CAdvHeaderCtrl();

protected:
	void OnEndTrack(NMHDR * pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CProgressListCtrl window

class CProgressListCtrl : public CListCtrl
{
public:
	CProgressListCtrl();
	virtual ~CProgressListCtrl();

public:
	void CreateProgress	(int iIndex, int iSubIndex);
	void SetProgress	(int iIndex, int iSubIndex, int prog);
	void ResizeProg();
	void DeleteProgress	(int iIndex, int iSubIndex);

	void CreateEdit		(int iIndex, int iSubIndex, BOOL bPassword );
	void SetEditText	(int iIndex, int iSubIndex, CString strText );
	void SetEditEnable	(int iIndex, int iSubIndex, BOOL bEnable );
	void SetLimitText	(int iIndex, int iSubIndex, int iMax);

	CString GetEditText	(int iIndex, int iSubIndex );
	void ResizeEdit();
	void DeleteEdit		(int iIndex, int iSubIndex);
	void SetFocusEdit   (int iIndex, int iSubIndex ); // 2013-07-19 hkeins : Add set focus on edit function

	void CreateColumns();
	void InitHdr();

protected:

	void settingScrollShow();
	void settingScroll();

	class CProgressEntry {
	public:
		CProgressEntry(int iIndex, int SubIndex);
		virtual ~CProgressEntry();

		int m_Index;
		int m_SubIndex;
		CProgressCtrl m_Prog;
	};

	class CEditEntry {
	public:
		CEditEntry(int iIndex, int SubIndex);
		virtual ~CEditEntry();

		int m_Index;
		int m_SubIndex;
		CEdit m_Edit;
	};

	CMap < int, int, CProgressEntry*, CProgressEntry*& > m_ProgEntries;
	CMap < int, int, CEditEntry*, CEditEntry*& > m_EditEntries;

	CAdvHeaderCtrl m_AdvHdr;
	CFont* m_pEditFont;

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHdnTrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LPARAM OnResizeItems( WPARAM wParam, LPARAM lParam );

	//afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMThemeChanged(NMHDR *pNMHDR, LRESULT *pResult);
};
