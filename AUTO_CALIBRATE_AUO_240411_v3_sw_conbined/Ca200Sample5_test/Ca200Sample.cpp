


#include "stdafx.h"
#include "Ca200Sample.h"
#include "Ca200SampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleApp

BEGIN_MESSAGE_MAP(CCa200SampleApp, CWinApp)
	//{{AFX_MSG_MAP(CCa200SampleApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleApp Construction of class 

CCa200SampleApp::CCa200SampleApp()
{

}

/////////////////////////////////////////////////////////////////////////////
// Only CCa200SampleApp Object

CCa200SampleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCa200SampleApp Initialization of class

BOOL CCa200SampleApp::InitInstance()
{
	
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();



#ifdef _AFXDLL
				
#else
	Enable3dControlsStatic();	
#endif

	
	if (RunEmbedded() || RunAutomated())
	{
		
		
		COleTemplateServer::RegisterAll();
	}
	else
	{
		COleObjectFactory::UpdateRegistryAll();
	}

	CCa200SampleDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		
	}
	else if (nResponse == IDCANCEL)
	{
		
	}

	
	return FALSE;
}

//Copyright (c) 2002-2013 KONICA MINOLTA, INC. 
