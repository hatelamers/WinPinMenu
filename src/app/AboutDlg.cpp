// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "resource.h"
#include "productmeta.h"

#include "aboutdlg.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ATLTRACE(_T(__FUNCTION__) _T("\n"));

    DoDataExchange(FALSE);
    if (m_fvi.Open())
    {
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_PRODUCTNAME, SFI_PRODUCTNAME);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_FILEDESCRIPTION, SFI_FILEDESCRIPTION);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_LEGALCOPYRIGHT, SFI_LEGALCOPYRIGHT);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_PRODUCTVERSION, SFI_PRODUCTVERSION);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_FILEVERSION, SFI_FILEVERSION);
        m_fvi.SetInfoDlgItemText(m_hWnd, IDC_TXT_COMPANYNAME, SFI_COMPANYNAME);

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
