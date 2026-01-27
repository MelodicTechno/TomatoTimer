#include "TomatoTimerDlg.h"
#include "HistoryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTomatoTimerDlg, CDialogEx)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_START, &CTomatoTimerDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDC_BUTTON_STOP, &CTomatoTimerDlg::OnBnClickedStop)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CTomatoTimerDlg::OnBnClickedReset)
    ON_BN_CLICKED(IDC_BUTTON_HISTORY, &CTomatoTimerDlg::OnBnClickedHistory)
    ON_WM_DESTROY()
    ON_MESSAGE(WM_TRAY_ICON, &CTomatoTimerDlg::OnTrayIcon)
END_MESSAGE_MAP()

CTomatoTimerDlg::CTomatoTimerDlg(CWnd* pParent)
    : CDialogEx(IDD_TOMATOTIMER_DIALOG, pParent)
    , m_timerId(0)
    , m_remainingSeconds(0)
    , m_workMinutes(25)
    , m_shortBreakMinutes(5)
    , m_longBreakMinutes(15)
    , m_roundsPerSet(4)
    , m_currentRound(1)
    , m_phase(TimerPhase::Work)
    , m_running(false)
    , m_statusPrefix()
    , m_hIcon(nullptr)
    , m_db(nullptr)
{
    memset(&m_nid, 0, sizeof(m_nid));
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTomatoTimerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CTomatoTimerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (m_hIcon)
    {
        SetIcon(m_hIcon, TRUE);
        SetIcon(m_hIcon, FALSE);
    }

    SetDlgItemInt(IDC_EDIT_WORK, m_workMinutes);
    SetDlgItemInt(IDC_EDIT_SHORTBREAK, m_shortBreakMinutes);
    SetDlgItemInt(IDC_EDIT_LONGBREAK, m_longBreakMinutes);
    SetDlgItemInt(IDC_EDIT_ROUNDS, m_roundsPerSet);
    m_statusPrefix = L"Ready";
    CWnd* pStatic = GetDlgItem(IDC_STATIC_STATUS);
    if (pStatic) pStatic->SetWindowText(m_statusPrefix);

    EnsureDatabase();
    InitTrayIcon();

    return TRUE;
}

void CTomatoTimerDlg::OnDestroy()
{
    if (m_timerId != 0)
    {
        KillTimer(m_timerId);
        m_timerId = 0;
    }
    CloseDatabase();
    RemoveTrayIcon();
    CDialogEx::OnDestroy();
}

void CTomatoTimerDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != m_timerId || !m_running)
    {
        CDialogEx::OnTimer(nIDEvent);
        return;
    }

    if (m_remainingSeconds > 0)
    {
        m_remainingSeconds -= 1;
        UpdateCountdownLabel();
        return;
    }

    ShowPhaseNotification();

    if (m_phase == TimerPhase::Work)
    {
        RecordFocusSession(m_workMinutes);
        if (m_currentRound < m_roundsPerSet)
        {
            m_phase = TimerPhase::ShortBreak;
            StartShortBreakPhase();
        }
        else
        {
            m_phase = TimerPhase::LongBreak;
            StartLongBreakPhase();
            m_currentRound = 0;
        }
    }
    else if (m_phase == TimerPhase::ShortBreak)
    {
        m_phase = TimerPhase::Work;
        m_currentRound++;
        StartWorkPhase();
    }
    else if (m_phase == TimerPhase::LongBreak)
    {
        m_phase = TimerPhase::Work;
        m_currentRound = 1;
        StartWorkPhase();
    }
}

void CTomatoTimerDlg::OnBnClickedStart()
{
    if (m_running)
        return;

    // Resume if paused
    if (m_remainingSeconds > 0)
    {
        m_timerId = SetTimer(1, 1000, nullptr);
        m_running = true;

        // Restore status text
        if (m_phase == TimerPhase::Work)
            m_statusPrefix.Format(L"Work round %d", m_currentRound);
        else if (m_phase == TimerPhase::ShortBreak)
            m_statusPrefix.Format(L"Short break round %d", m_currentRound);
        else
            m_statusPrefix = L"Long break";
            
        UpdateCountdownLabel();
        return;
    }

    LoadSettingsFromControls();

    m_currentRound = 1;
    m_phase = TimerPhase::Work;
    StartWorkPhase();
    m_running = true;
}

void CTomatoTimerDlg::OnBnClickedStop()
{
    if (!m_running)
        return;

    if (m_timerId != 0)
    {
        KillTimer(m_timerId);
        m_timerId = 0;
    }

    m_running = false;
    m_statusPrefix = L"Stopped";
    CWnd* pStatic = GetDlgItem(IDC_STATIC_STATUS);
    if (pStatic) pStatic->SetWindowText(m_statusPrefix);
}

void CTomatoTimerDlg::OnBnClickedReset()
{
    if (m_timerId != 0)
    {
        KillTimer(m_timerId);
        m_timerId = 0;
    }

    m_running = false;
    m_currentRound = 1;
    m_phase = TimerPhase::Work;
    m_remainingSeconds = 0; // Or reset to work minutes, but 0 indicates "Ready" state mostly
    
    // Refresh settings in case user changed them
    LoadSettingsFromControls();
    
    // Set UI to initial state
    m_statusPrefix = L"Ready";
    CWnd* pStatic = GetDlgItem(IDC_STATIC_STATUS);
    if (pStatic) pStatic->SetWindowText(m_statusPrefix);
}

void CTomatoTimerDlg::OnBnClickedHistory()
{
    if (!m_db)
        OpenDatabase();
    if (!m_db)
    {
        MessageBox(L"Database is not available.", L"History", MB_OK | MB_ICONWARNING);
        return;
    }

    CHistoryDlg dlg(m_db, this);
    dlg.DoModal();
}

void CTomatoTimerDlg::LoadSettingsFromControls()
{
    BOOL ok = FALSE;
    int work = GetDlgItemInt(IDC_EDIT_WORK, &ok, FALSE);
    if (ok && work > 0)
        m_workMinutes = work;

    int shortBreak = GetDlgItemInt(IDC_EDIT_SHORTBREAK, &ok, FALSE);
    if (ok && shortBreak > 0)
        m_shortBreakMinutes = shortBreak;

    int longBreak = GetDlgItemInt(IDC_EDIT_LONGBREAK, &ok, FALSE);
    if (ok && longBreak > 0)
        m_longBreakMinutes = longBreak;

    int rounds = GetDlgItemInt(IDC_EDIT_ROUNDS, &ok, FALSE);
    if (ok && rounds > 0)
        m_roundsPerSet = rounds;
}

void CTomatoTimerDlg::StartWorkPhase()
{
    m_remainingSeconds = m_workMinutes * 60;
    if (m_timerId != 0)
        KillTimer(m_timerId);
    m_timerId = SetTimer(1, 1000, nullptr);
    m_statusPrefix.Format(L"Work round %d", m_currentRound);
    UpdateCountdownLabel();
}

void CTomatoTimerDlg::StartShortBreakPhase()
{
    m_remainingSeconds = m_shortBreakMinutes * 60;
    if (m_timerId != 0)
        KillTimer(m_timerId);
    m_timerId = SetTimer(1, 1000, nullptr);
    m_statusPrefix.Format(L"Short break round %d", m_currentRound);
    UpdateCountdownLabel();
}

void CTomatoTimerDlg::StartLongBreakPhase()
{
    m_remainingSeconds = m_longBreakMinutes * 60;
    if (m_timerId != 0)
        KillTimer(m_timerId);
    m_timerId = SetTimer(1, 1000, nullptr);
    m_statusPrefix = L"Long break";
    UpdateCountdownLabel();
}

void CTomatoTimerDlg::UpdateCountdownLabel()
{
    int minutes = m_remainingSeconds / 60;
    int seconds = m_remainingSeconds % 60;
    CString timeText;
    timeText.Format(L"%02d:%02d", minutes, seconds);
    
    CString finalText;
    finalText.Format(L"%s  %s", m_statusPrefix.GetString(), timeText.GetString());
    
    CWnd* pStatic = GetDlgItem(IDC_STATIC_STATUS);
    if (pStatic) pStatic->SetWindowText(finalText);
}

void CTomatoTimerDlg::ShowPhaseNotification()
{
    m_nid.uFlags = NIF_INFO;
    m_nid.dwInfoFlags = NIIF_INFO;
    wcscpy_s(m_nid.szInfoTitle, L"Tomato Timer");

    if (m_phase == TimerPhase::Work)
    {
        MessageBeep(MB_ICONASTERISK);
        wcscpy_s(m_nid.szInfo, L"Work finished, take a break");
    }
    else if (m_phase == TimerPhase::ShortBreak)
    {
        MessageBeep(MB_ICONASTERISK);
        wcscpy_s(m_nid.szInfo, L"Short break finished, back to work");
    }
    else if (m_phase == TimerPhase::LongBreak)
    {
        MessageBeep(MB_ICONASTERISK);
        wcscpy_s(m_nid.szInfo, L"Long break finished, cycle will restart");
    }

    Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

void CTomatoTimerDlg::InitTrayIcon()
{
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hWnd;
    m_nid.uID = 1001;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAY_ICON;
    
    // Load default application icon
    m_nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    // If no icon found, load standard warning icon as fallback
    if (!m_nid.hIcon)
        m_nid.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
        
    wcscpy_s(m_nid.szTip, L"Tomato Timer");

    Shell_NotifyIcon(NIM_ADD, &m_nid);
}

void CTomatoTimerDlg::RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

LRESULT CTomatoTimerDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
    // Handle tray icon events if needed (e.g. click to restore window)
    return 0;
}

void CTomatoTimerDlg::EnsureDatabase()
{
    OpenDatabase();
    if (!m_db)
        return;

    const char* sql = "CREATE TABLE IF NOT EXISTS sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "start_time TEXT,"
        "duration_minutes INTEGER"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        if (errMsg)
            sqlite3_free(errMsg);
    }
}

void CTomatoTimerDlg::OpenDatabase()
{
    if (m_db)
        return;

    int rc = sqlite3_open("tomato_timer.db", &m_db);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void CTomatoTimerDlg::CloseDatabase()
{
    if (!m_db)
        return;

    sqlite3_close(m_db);
    m_db = nullptr;
}

void CTomatoTimerDlg::RecordFocusSession(int minutes)
{
    if (!m_db)
        return;

    time_t now = time(nullptr);
    tm timeInfo;
    localtime_s(&timeInfo, &now);
    char dateBuf[32];
    // Use date only (YYYY-MM-DD) for daily aggregation
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &timeInfo);

    // Check if we already have a record for today
    const char* querySql = "SELECT id, duration_minutes FROM sessions WHERE start_time = ? LIMIT 1;";
    sqlite3_stmt* queryStmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, querySql, -1, &queryStmt, nullptr);
    
    bool recordExists = false;
    int existingId = -1;
    
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(queryStmt, 1, dateBuf, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(queryStmt) == SQLITE_ROW)
        {
            recordExists = true;
            existingId = sqlite3_column_int(queryStmt, 0);
        }
        sqlite3_finalize(queryStmt);
    }

    if (recordExists)
    {
        // Update existing record
        const char* updateSql = "UPDATE sessions SET duration_minutes = duration_minutes + ? WHERE id = ?;";
        sqlite3_stmt* updateStmt = nullptr;
        if (sqlite3_prepare_v2(m_db, updateSql, -1, &updateStmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_int(updateStmt, 1, minutes);
            sqlite3_bind_int(updateStmt, 2, existingId);
            sqlite3_step(updateStmt);
            sqlite3_finalize(updateStmt);
        }
    }
    else
    {
        // Insert new record for today
        const char* insertSql = "INSERT INTO sessions (start_time, duration_minutes) VALUES (?, ?);";
        sqlite3_stmt* insertStmt = nullptr;
        if (sqlite3_prepare_v2(m_db, insertSql, -1, &insertStmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(insertStmt, 1, dateBuf, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(insertStmt, 2, minutes);
            sqlite3_step(insertStmt);
            sqlite3_finalize(insertStmt);
        }
    }

    // Delete records older than 7 days
    // Calculate date 7 days ago
    time_t cutoff = now - (7 * 24 * 60 * 60);
    tm cutoffInfo;
    localtime_s(&cutoffInfo, &cutoff);
    char cutoffBuf[32];
    strftime(cutoffBuf, sizeof(cutoffBuf), "%Y-%m-%d", &cutoffInfo);

    // Using string comparison for dates in YYYY-MM-DD format works correctly
    const char* delSql = "DELETE FROM sessions WHERE start_time < ?";
    sqlite3_stmt* delStmt = nullptr;
    if (sqlite3_prepare_v2(m_db, delSql, -1, &delStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(delStmt, 1, cutoffBuf, -1, SQLITE_TRANSIENT);
        sqlite3_step(delStmt);
        sqlite3_finalize(delStmt);
    }
}
