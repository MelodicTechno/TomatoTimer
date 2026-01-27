#pragma once

#include <afxwin.h>
#include <afxdialogex.h>
#include <shellapi.h>
#include <string>
#include <ctime>
#include "sqlite3.h"
#include "resource.h"

#define WM_TRAY_ICON (WM_USER + 100)

enum class TimerPhase
{
    Work,
    ShortBreak,
    LongBreak
};

class CTomatoTimerDlg : public CDialogEx
{
public:
    CTomatoTimerDlg(CWnd* pParent = nullptr);

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedStop();
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedHistory();
    afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    UINT_PTR m_timerId;
    int m_remainingSeconds;
    int m_workMinutes;
    int m_shortBreakMinutes;
    int m_longBreakMinutes;
    int m_roundsPerSet;
    int m_currentRound;
    TimerPhase m_phase;
    bool m_running;
    CString m_statusPrefix;
    HICON m_hIcon;
    sqlite3* m_db;
    NOTIFYICONDATA m_nid;

    void LoadSettingsFromControls();
    void StartWorkPhase();
    void StartShortBreakPhase();
    void StartLongBreakPhase();
    void UpdateCountdownLabel();
    void ShowPhaseNotification();
    void RecordFocusSession(int minutes);
    void EnsureDatabase();
    void OpenDatabase();
    void CloseDatabase();
    void InitTrayIcon();
    void RemoveTrayIcon();
};
