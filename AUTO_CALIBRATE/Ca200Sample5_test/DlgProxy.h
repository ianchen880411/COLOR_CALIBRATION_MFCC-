

#if !defined(AFX_DLGPROXY_H__7FB41356_024C_422C_9652_3C16821D32D8__INCLUDED_)
#define AFX_DLGPROXY_H__7FB41356_024C_422C_9652_3C16821D32D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCa200SampleDlg;

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleDlgAutoProxy Command Target

class CCa200SampleDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CCa200SampleDlgAutoProxy)

	CCa200SampleDlgAutoProxy();           
	
// Attribute 
public:
	CCa200SampleDlg* m_pDialog;

// Operation
public:

// Override
	//{{AFX_VIRTUAL(CCa200SampleDlgAutoProxy)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCa200SampleDlgAutoProxy();

	//{{AFX_MSG(CCa200SampleDlgAutoProxy)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CCa200SampleDlgAutoProxy)

	//{{AFX_DISPATCH(CCa200SampleDlgAutoProxy)
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGPROXY_H__7FB41356_024C_422C_9652_3C16821D32D8__INCLUDED_)

//Copyright (c) 2002-2013 KONICA MINOLTA, INC. 
