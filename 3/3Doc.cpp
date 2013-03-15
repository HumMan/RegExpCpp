// 3Doc.cpp : implementation of the CMy3Doc class
//

#include "stdafx.h"
#include "3.h"

#include "3Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMy3Doc

IMPLEMENT_DYNCREATE(CMy3Doc, CDocument)

BEGIN_MESSAGE_MAP(CMy3Doc, CDocument)
END_MESSAGE_MAP()


// CMy3Doc construction/destruction

CMy3Doc::CMy3Doc()
{
	// TODO: add one-time construction code here

}

CMy3Doc::~CMy3Doc()
{
}

BOOL CMy3Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CMy3Doc serialization

void CMy3Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CMy3Doc diagnostics

#ifdef _DEBUG
void CMy3Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMy3Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMy3Doc commands
