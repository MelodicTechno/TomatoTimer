#include "TomatoTimerApp.h"
#include "TomatoTimerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTomatoTimerApp theApp;

BOOL CTomatoTimerApp::InitInstance()
{
    CWinApp::InitInstance();

    AfxEnableControlContainer();

    CTomatoTimerDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    return FALSE;
}
