#include "HistoryDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CHistoryDlg, CDialogEx)

CHistoryDlg::CHistoryDlg(sqlite3* db, CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_HISTORY_DIALOG, pParent), m_db(db)
{
}

CHistoryDlg::~CHistoryDlg()
{
}

void CHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_HISTORY, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CHistoryDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CHistoryDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Initialize List Control
    // Add LVS_EX_FULLROWSELECT and LVS_EX_GRIDLINES for table-like appearance
    m_listCtrl.SetExtendedStyle(m_listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    // Remove button style from header to make it non-clickable
    CHeaderCtrl* pHeader = m_listCtrl.GetHeaderCtrl();
    if (pHeader)
    {
        pHeader->ModifyStyle(HDS_BUTTONS, 0);
    }
    
    // Insert Columns
    CRect rect;
    m_listCtrl.GetClientRect(&rect);
    int totalWidth = rect.Width();
    int scrollWidth = GetSystemMetrics(SM_CXVSCROLL);
    
    // Calculate column widths
    // Reserve space for scrollbar to prevent horizontal scrolling when items are added
    int totalAvailableWidth = rect.Width() - scrollWidth;
    
    // Safety margin to absolutely prevent horizontal scrollbar due to borders/padding
    totalAvailableWidth -= 4;

    // Set columns to 1:1 ratio
    int col1Width = totalAvailableWidth / 2;
    int col2Width = totalAvailableWidth - col1Width; // Give remaining width to second column
    
    // Ensure minimum widths
    if (col1Width < 50) col1Width = 50;

    m_listCtrl.InsertColumn(0, L"Date", LVCFMT_LEFT, col1Width);
    m_listCtrl.InsertColumn(1, L"Duration (min)", LVCFMT_LEFT, col2Width);

    LoadHistoryData();

    return TRUE;
}

void CHistoryDlg::LoadHistoryData()
{
    if (!m_db) return;

    // Query last 50 records
    const char* sql = "SELECT start_time, duration_minutes FROM sessions ORDER BY id DESC LIMIT 50;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        MessageBox(L"Failed to query history.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    int index = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char* timeText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int minutes = sqlite3_column_int(stmt, 1);

        wchar_t wTime[64] = {0};
        if (timeText)
        {
            MultiByteToWideChar(CP_ACP, 0, timeText, -1, wTime, static_cast<int>(sizeof(wTime) / sizeof(wchar_t)));
        }

        CString strMinutes;
        strMinutes.Format(L"%d", minutes);

        // Insert Item (Row)
        m_listCtrl.InsertItem(index, wTime);
        // Set SubItem (Column 1)
        m_listCtrl.SetItemText(index, 1, strMinutes);
        
        index++;
    }

    sqlite3_finalize(stmt);
}
