// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "FileVersionInfo.h"

class CAboutDlg;
using CUxAboutDialog = CUxModeWindow<CAboutDlg>;

class CAboutDlg
	: public CDialogImpl<CAboutDlg>
	, public CUxAboutDialog
	, public CWinDataExchange<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChange)
		CHAIN_MSG_MAP(CUxAboutDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDRETRY, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CAboutDlg)
		DDX_CONTROL(IDC_LNK_LICENSE, m_lnkLicense)
		DDX_CONTROL(IDC_LNK_RELNOTES, m_lnkRelNotes)
	END_DDX_MAP()

private:
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnThemeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void UpdateColors();
	void ApplyLinkColor(CHyperLink& link, COLORREF clrLink, COLORREF clrVisited) const;

private:
	COLORREF m_clrLink{ 0 };
	COLORREF m_clrVisited{ 0 };
	CHyperLink m_lnkLicense;
	CHyperLink m_lnkRelNotes;
	CFileVersionInfo m_fvi;
};
