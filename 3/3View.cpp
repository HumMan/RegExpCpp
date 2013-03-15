// 3View.cpp : implementation of the CMy3View class
//

#include "stdafx.h"
#include "3.h"

#include "3Doc.h"
#include "3View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMy3View

IMPLEMENT_DYNCREATE(CMy3View, CFormView)

BEGIN_MESSAGE_MAP(CMy3View, CFormView)
	ON_EN_CHANGE(IDC_EDIT2, &CMy3View::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_CHECK1, &CMy3View::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CMy3View::OnBnClickedButton1)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CMy3View construction/destruction

CMy3View::CMy3View()
	: CFormView(CMy3View::IDD)
	, m_expr(_T("(a|b[^\\n ])abc(a|[\\s0-9])"))
	, m_text(_T("aabca"))
	, m_match(FALSE)
	, m_time_match(_T(""))
	, m_time_match2(_T(""))
	, m_use_dfa(FALSE)
	, m_dfa_last_char(_T(""))
{
	// TODO: add construction code here

}

CMy3View::~CMy3View()
{
}

void CMy3View::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_expr);
	DDX_Text(pDX, IDC_EDIT1, m_text);
	DDX_Check(pDX, IDC_CHECK1, m_match);
	DDX_Text(pDX, IDC_STATIC5, m_time_match);
	DDX_Text(pDX, IDC_STATIC6, m_time_match2);
	DDX_Control(pDX, IDC_BUTTON1, m_match_ctrl);
	DDX_Check(pDX, IDC_CHECK2, m_use_dfa);
	DDX_Text(pDX, IDC_STATIC7, m_dfa_last_char);
}

BOOL CMy3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CMy3View::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

}


// CMy3View diagnostics

#ifdef _DEBUG
void CMy3View::AssertValid() const
{
	CFormView::AssertValid();
}

void CMy3View::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CMy3Doc* CMy3View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMy3Doc)));
	return (CMy3Doc*)m_pDocument;
}
#endif //_DEBUG


// CMy3View message handlers

void CMy3View::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CMy3View::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
}

#include "RegExp.h"

void CMy3View::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	int t1,t2;
	UpdateData(true);
	if(!m_use_dfa)
	{
		t1=GetTickCount();
		for(int i=0;i<500;i++)
		{
			TNFA nfa;
			nfa.SetExpression(m_expr.GetBuffer());
		}
		t2=GetTickCount();
		m_time_match.Format(_T("%d"),t2-t1);
		t1=GetTickCount();
		{
			TNFA nfa;
			nfa.SetExpression(m_expr.GetBuffer());
			for(int i=0;i<100000;i++)
				m_match=nfa.Match(m_text.GetBuffer());
		}
		t2=GetTickCount();
		m_time_match2.Format(_T("%d"),t2-t1);
	}else
	{
		t1=GetTickCount();
		for(int i=0;i<500;i++)
		{
			TDFA nfa;
			nfa.SetExpression(m_expr.GetBuffer());
		}
		t2=GetTickCount();
		m_time_match.Format(_T("%d"),t2-t1);
		t1=GetTickCount();
		{
			TDFA nfa;
			int last_char;
			nfa.SetExpression(m_expr.GetBuffer());
			for(int i=0;i<100000;i++)
				m_match=nfa.Match(m_text.GetBuffer(),last_char);
			m_dfa_last_char.Format("last_char = %d",last_char+1);
		}
		t2=GetTickCount();
		m_time_match2.Format(_T("%d"),t2-t1);
	}
	UpdateData(false);
}

void CMy3View::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CButton* ct=(CButton*)GetDlgItem(IDC_BUTTON1);
	CRect cr;
	GetClientRect(cr);
	//if (ct) ct->MoveWindow(cr.right-100,cr.bottom-200,100,10);
}
