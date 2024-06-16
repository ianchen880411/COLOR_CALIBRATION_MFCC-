

#include "stdafx.h"
#include "Ca200Sample.h"
#include "DlgProxy.h"
#include "Ca200SampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleDlgAutoProxy

IMPLEMENT_DYNCREATE(CCa200SampleDlgAutoProxy, CCmdTarget)

CCa200SampleDlgAutoProxy::CCa200SampleDlgAutoProxy()
{
	EnableAutomation();
	
	AfxOleLockApp();

	ASSERT (AfxGetApp()->m_pMainWnd != NULL);
	ASSERT_VALID (AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(CCa200SampleDlg, AfxGetApp()->m_pMainWnd);
	m_pDialog = (CCa200SampleDlg*) AfxGetApp()->m_pMainWnd;
	m_pDialog->m_pAutoProxy = this;
}

CCa200SampleDlgAutoProxy::~CCa200SampleDlgAutoProxy()
{
	if (m_pDialog != NULL)
		m_pDialog->m_pAutoProxy = NULL;
	AfxOleUnlockApp();
}

void CCa200SampleDlgAutoProxy::OnFinalRelease()
{

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CCa200SampleDlgAutoProxy, CCmdTarget)
	//{{AFX_MSG_MAP(CCa200SampleDlgAutoProxy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CCa200SampleDlgAutoProxy, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CCa200SampleDlgAutoProxy)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


// {01E08351-86D6-424F-BEFC-2E6938925AB2}
static const IID IID_ICa200Sample =
{ 0x1e08351, 0x86d6, 0x424f, { 0xbe, 0xfc, 0x2e, 0x69, 0x38, 0x92, 0x5a, 0xb2 } };

BEGIN_INTERFACE_MAP(CCa200SampleDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CCa200SampleDlgAutoProxy, IID_ICa200Sample, Dispatch)
END_INTERFACE_MAP()

// {AE212B5F-FD44-4D07-9DF0-49E73D660FF8}
IMPLEMENT_OLECREATE2(CCa200SampleDlgAutoProxy, "Ca200Sample.Application", 0xae212b5f, 0xfd44, 0x4d07, 0x9d, 0xf0, 0x49, 0xe7, 0x3d, 0x66, 0xf, 0xf8)

//Copyright (c) 2002-2013 KONICA MINOLTA, INC. 
