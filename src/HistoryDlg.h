#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "sqlite3.h"

class CHistoryDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CHistoryDlg)

public:
    CHistoryDlg(sqlite3* db, CWnd* pParent = nullptr);   // standard constructor
    virtual ~CHistoryDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_HISTORY_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    sqlite3* m_db;
    CListCtrl m_listCtrl;
    
    void LoadHistoryData();
};
