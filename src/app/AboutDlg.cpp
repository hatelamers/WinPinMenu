// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "productmeta.h"
#include "Draw.h"

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
    m_clrLink = m_lnkLicense.m_clrLink;
    m_clrVisited = m_lnkLicense.m_clrVisited;

    UpdateColors();
    return TRUE;
}

LRESULT CAboutDlg::OnThemeChange(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
    bHandled = FALSE;
    UpdateColors();
    return 0L;
}


LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

void CAboutDlg::UpdateColors()
{
    auto clrLink = m_clrLink;
    auto clrVisited = m_clrVisited;
    if (IsInDarkMode())
    {
        clrLink = HLS_TRANSFORM(clrLink, 35, 0);
        clrVisited = HLS_TRANSFORM(clrVisited, 30, 0);
    }
    ApplyLinkColor(m_lnkLicense, clrLink, clrVisited);
    ApplyLinkColor(m_lnkRelNotes, clrLink, clrVisited);
}

void CAboutDlg::ApplyLinkColor(CHyperLink& link, COLORREF clrLink, COLORREF clrVisited) const
{
    link.m_clrLink = clrLink;
    link.m_clrVisited = clrVisited;
}
