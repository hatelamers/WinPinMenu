#pragma once

#ifndef __ATLAPP_H__
#error UxModeHyperLink.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
#error UxModeHyperLink.h requires atlctrls.h to be included first
#endif

class CUxModeHyperLink
    : public CHyperLinkImpl<CUxModeHyperLink>
{
public:
    DECLARE_WND_CLASS(_T("WTLX_HyperLink"))

    BEGIN_MSG_MAP(CUxModeHyperLink)
        MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChange)
        CHAIN_MSG_MAP(CHyperLinkImpl<CUxModeHyperLink>)
    END_MSG_MAP()

    LRESULT OnThemeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        UpdateColors();
        return 0L;
    }

    void Init()
    {
        CHyperLinkImpl<CUxModeHyperLink>::Init();
        m_clrDefLink = m_clrLink;
        m_clrDefVisited = m_clrVisited;
        UpdateColors(false);
    }
protected:
    void UpdateColors(bool repaint = true)
    {
        if (uxTheme.IsInDarkMode())
        {
            m_clrLink = UXCOLOR_LIGHTER(m_clrDefLink, 0.5);
            m_clrVisited = UXCOLOR_LIGHTER(m_clrDefVisited, 0.2);
        }
        else
        {
            m_clrLink = m_clrDefLink;
            m_clrVisited = m_clrDefVisited;
        }
        if (repaint)
            UpdateWindow();
    }

protected:
    COLORREF m_clrDefLink{ 0 };
    COLORREF m_clrDefVisited{ 0 };
};