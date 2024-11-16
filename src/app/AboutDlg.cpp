// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "productmeta.h"

#include "aboutdlg.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    uxTheme.AllowDarkModeForWindow(m_hWnd, true);
    if (uxTheme.ShouldAppsUseDarkMode())
    {
        uxTheme.SwitchWindowDarkMode(m_hWnd, true);
    }
    SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
    //EnableThemeDialogTexture(ETDT_ENABLETAB);

    DoDataExchange(FALSE);
    //m_lnkLicense.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON, HLINK_COMMANDBUTTON);
    if (m_fvi.Open())
    {
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_PRODUCTNAME, SFI_PRODUCTNAME);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_FILEDESCRIPTION, SFI_FILEDESCRIPTION);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_LEGALCOPYRIGHT, SFI_LEGALCOPYRIGHT);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_PRODUCTVERSION, SFI_PRODUCTVERSION);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_FILEVERSION, SFI_FILEVERSION);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_COMPANYNAME, SFI_COMPANYNAME);

        //LPTSTR lpValue(NULL);
        //UINT uLen(0);
        //if (m_fvi.GetStringFileInfo(SFI_LEGALTRADEMARKS, lpValue, &uLen))
        //    m_lnkLicense.SetToolTipText(lpValue);
    }
    m_lnkLicense.SetHyperLink(PRODUCT_LICENSE_URL);

#ifdef PRODUCT_RELNOTES_URL
    m_lnkRelNotes.SetHyperLink(PRODUCT_RELNOTES_URL);
#else
    m_lnkRelNotes.ShowWindow(SW_HIDE);
#endif
    return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
