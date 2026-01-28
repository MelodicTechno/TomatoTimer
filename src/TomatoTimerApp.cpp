#include "TomatoTimerApp.h"
#include "TomatoTimerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTomatoTimerApp theApp;

BOOL CTomatoTimerApp::InitInstance()
{
    // Check for existing instance
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"TomatoTimer_Instance_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CWnd* pWnd = CWnd::FindWindow(NULL, L"Tomato Timer");
        if (pWnd)
        {
            // Restore window if minimized or hidden
            pWnd->ShowWindow(SW_RESTORE);
            pWnd->SetForegroundWindow();
        }
        return FALSE;
    }

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    CTomatoTimerDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    return FALSE;
}
