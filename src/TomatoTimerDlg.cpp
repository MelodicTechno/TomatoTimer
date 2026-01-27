#include "TomatoTimerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CTomatoTimerDlg, CDialogEx)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_START, &CTomatoTimerDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDC_BUTTON_STOP, &CTomatoTimerDlg::OnBnClickedStop)
    ON_WM_DESTROY()
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
    , m_db(nullptr)
{
}

void CTomatoTimerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CTomatoTimerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetDlgItemInt(IDC_EDIT_WORK, m_workMinutes);
    SetDlgItemInt(IDC_EDIT_SHORTBREAK, m_shortBreakMinutes);
    SetDlgItemInt(IDC_EDIT_LONGBREAK, m_longBreakMinutes);
    SetDlgItemInt(IDC_EDIT_ROUNDS, m_roundsPerSet);
    SetDlgItemText(IDC_STATIC_STATUS, L"Ready");

    EnsureDatabase();

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
    SetDlgItemText(IDC_STATIC_STATUS, L"Stopped");
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
    CString text;
    text.Format(L"Work round %d", m_currentRound);
    SetDlgItemText(IDC_STATIC_STATUS, text);
    UpdateCountdownLabel();
}

void CTomatoTimerDlg::StartShortBreakPhase()
{
    m_remainingSeconds = m_shortBreakMinutes * 60;
    if (m_timerId != 0)
        KillTimer(m_timerId);
    m_timerId = SetTimer(1, 1000, nullptr);
    CString text;
    text.Format(L"Short break round %d", m_currentRound);
    SetDlgItemText(IDC_STATIC_STATUS, text);
    UpdateCountdownLabel();
}

void CTomatoTimerDlg::StartLongBreakPhase()
{
    m_remainingSeconds = m_longBreakMinutes * 60;
    if (m_timerId != 0)
        KillTimer(m_timerId);
    m_timerId = SetTimer(1, 1000, nullptr);
    CString text(L"Long break");
    SetDlgItemText(IDC_STATIC_STATUS, text);
    UpdateCountdownLabel();
}

void CTomatoTimerDlg::UpdateCountdownLabel()
{
    int minutes = m_remainingSeconds / 60;
    int seconds = m_remainingSeconds % 60;
    CString timeText;
    timeText.Format(L"%02d:%02d", minutes, seconds);
    CString statusText;
    GetDlgItemText(IDC_STATIC_STATUS, statusText);
    CString finalText;
    finalText.Format(L"%s  %s", statusText.GetString(), timeText.GetString());
    SetDlgItemText(IDC_STATIC_STATUS, finalText);
}

void CTomatoTimerDlg::ShowPhaseNotification()
{
    if (m_phase == TimerPhase::Work)
    {
        MessageBeep(MB_ICONASTERISK);
        MessageBox(L"Work finished, take a break", L"Tomato Timer", MB_OK | MB_ICONINFORMATION);
    }
    else if (m_phase == TimerPhase::ShortBreak)
    {
        MessageBeep(MB_ICONASTERISK);
        MessageBox(L"Short break finished, back to work", L"Tomato Timer", MB_OK | MB_ICONINFORMATION);
    }
    else if (m_phase == TimerPhase::LongBreak)
    {
        MessageBeep(MB_ICONASTERISK);
        MessageBox(L"Long break finished, cycle will restart", L"Tomato Timer", MB_OK | MB_ICONINFORMATION);
    }
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
    char timeBuf[32];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeInfo);

    const char* sql = "INSERT INTO sessions (start_time, duration_minutes) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, timeBuf, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, minutes);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
