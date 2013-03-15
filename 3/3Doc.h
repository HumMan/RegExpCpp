// 3Doc.h : interface of the CMy3Doc class
//


#pragma once


class CMy3Doc : public CDocument
{
protected: // create from serialization only
	CMy3Doc();
	DECLARE_DYNCREATE(CMy3Doc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CMy3Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


