// 3View.h : interface of the CMy3View class
//


#pragma once
#include "afxwin.h"


class CMy3View : public CFormView
{
protected: // create from serialization only
	CMy3View();
	DECLARE_DYNCREATE(CMy3View)

public:
	enum{ IDD = IDD_MY3_FORM };

// Attributes
public:
	CMy3Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CMy3View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit2();
	CString m_expr;
	CString m_text;
	afx_msg void OnBnClickedCheck1();
	BOOL m_match;
	afx_msg void OnBnClickedButton1();
	CString m_time_match;
	CString m_time_match2;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton m_match_ctrl;
	BOOL m_use_dfa;
	CString m_dfa_last_char;
};

#ifndef _DEBUG  // debug version in 3View.cpp
inline CMy3Doc* CMy3View::GetDocument() const
   { return reinterpret_cast<CMy3Doc*>(m_pDocument); }
#endif

