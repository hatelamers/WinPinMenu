#pragma once

struct UxModeMenuItemData
{
    int iconIndex{ -1 };
    bool isSubmenu{ false };
    CString caption;
};

template<class T, typename TData = UxModeMenuItemData>
class CUxModePopupMenu
    : public CUxModeMenuBase<T>
{
public:

    BEGIN_MSG_MAP(CUxModePopupMenu)
        CHAIN_MSG_MAP(CUxModeMenuBase<T>)
        MESSAGE_HANDLER(WM_INITMENUPOPUP, UxModeOnInitMenuPopup)
        MESSAGE_HANDLER(WM_MENUSELECT, UxModeOnMenuSelect)
        MESSAGE_HANDLER(WM_UNINITMENUPOPUP, UxModeOnUninitMenuPopup)
        MESSAGE_HANDLER(WM_DRAWITEM, UxModeMenuOnDrawItem)
        MESSAGE_HANDLER(WM_MEASUREITEM, UxModeMenuOnMeasureItem)
    END_MSG_MAP()

protected:
    BOOL HandleMenuItemSelected(HMENU /*hMenu*/, WORD /*item*/, WORD /*flags*/)
    {
        return FALSE;
    }

    int UpdateMenuItemData(TData* /*pData*/, UINT /*itemID*/, UINT /*itemPos*/, UINT /*itemType*/)
    {
        return -1;
    }

    HRESULT LoadMenuImages()
    {
        if (!m_menuItemImages.IsNull())
        {
            m_menuItemImages.GetIconSize(m_menuMetrics.sizeIcon);
            return S_OK;
        }
        m_menuMetrics.sizeIcon.cx = m_menuMetrics.sizeIcon.cy = 16;
        return S_FALSE;
    }

    virtual void UxModeSetup(bool init = true) override
    {
        ATLTRACE(_T(__FUNCTION__) _T("\n"));
        CUxModeMenuBase<T>::UxModeSetup(init);
        if (init)
        {
            GetThis()->LoadMenuImages();
        }
    }

    BOOL CustomDrawPopupMenuItem(LPDRAWITEMSTRUCT lpDis)
    {
        CustomDrawPopupMenuBackground(lpDis);
        auto itemState = CustomDrawPopupMenuItemBackground(lpDis);

        CRect rc(lpDis->rcItem);
        if (m_menuMetrics.sizeIcon.cx)
        {
            rc.left += m_menuMetrics.sizeIcon.cx + (m_menuMetrics.paddingIcon.cx * 2);
        }

        auto pData = reinterpret_cast<TData*>(lpDis->itemData);
        //ATLTRACE(_T(__FUNCTION__) _T(" id=%d, caption=%s, itemState=%d, themeItemState=%d\n")
        //    , lpDis->itemID, pData ? (LPCTSTR)pData->caption : _T("null"), lpDis->itemState, itemState);
        CDCHandle dc = lpDis->hDC;
        if (NULL == pData || 0 == pData->caption.GetLength())
        {
            m_menuTheme.DrawThemeBackground(dc, MENU_POPUPSEPARATOR, 0, rc, NULL);
        }
        else
        {
            auto isDisabled = ODS_GRAYED == (lpDis->itemState & ODS_GRAYED);
            if (S_FALSE == CustomDrawPopupMenuCheck(lpDis) && !m_menuItemImages.IsNull() && -1 < pData->iconIndex)
            {
                CRect rcIcon(lpDis->rcItem);
                rcIcon.left += m_menuMetrics.paddingIcon.cx;
                rcIcon.top += m_menuMetrics.paddingIcon.cy;
                rcIcon.right = rcIcon.left + m_menuMetrics.sizeIcon.cx;
                rcIcon.bottom = rcIcon.top + m_menuMetrics.sizeIcon.cy + m_menuMetrics.paddingIcon.cy;

                if (isDisabled)
                {
                    CIcon ico = m_menuItemImages.ExtractIcon(pData->iconIndex);
                    dc.DrawState(rcIcon.TopLeft(), rcIcon.Size(), ico, DST_ICON | DSS_DISABLED, m_menuColors.brushBg);
                }
                else
                {
                    m_menuTheme.DrawThemeIcon(dc, MENU_POPUPITEM, itemState, rcIcon, m_menuItemImages, pData->iconIndex);
                }
            }

            int nTab = pData->caption.Find('\t');
      
            dc.SetBkMode(TRANSPARENT);

            rc.left += m_menuMetrics.paddingText.cx;
            rc.right -= m_menuMetrics.sizeMnuArrow.cx + m_menuMetrics.paddingText.cx;

            DWORD dwFlags = DT_SINGLELINE | DT_VCENTER | DT_LEFT;
            if (ODS_NOACCEL == (ODS_NOACCEL & lpDis->itemState)) {
                dwFlags |= DT_HIDEPREFIX;
            }
            m_menuTheme.DrawThemeText(dc, MENU_POPUPITEM, itemState, pData->caption, nTab, dwFlags, 0, rc);
            if (-1 < nTab)
            {
                dwFlags &= ~DT_LEFT;
                dwFlags |= DT_RIGHT;
                m_menuTheme.DrawThemeText(dc, MENU_POPUPITEM, itemState, pData->caption.GetBuffer() + nTab + 1, -1, dwFlags, 0, rc);
                pData->caption.ReleaseBuffer();
            }

            if (pData->isSubmenu)
            {
                CustomDrawMenuArrow(dc, &lpDis->rcItem, isDisabled);
            }

            dc.ExcludeClipRect(&lpDis->rcItem);
        }
        return TRUE;
    }

    BOOL InitMenuPopup(HMENU hMenu, WORD /*submenuPos*/)
    {
        CMenuHandle menuPopup = hMenu;
        ATLASSERT(menuPopup.IsMenu());

        MENUINFO mi;
        ::ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (m_menuMetrics.sizeIcon.cx)
        {
            mi.fMask = MIM_STYLE;
            mi.dwStyle = MNS_CHECKORBMP;
        }
        UxModeUpdateMenuInfo(mi);
        if (mi.fMask)
            menuPopup.SetMenuInfo(&mi);

        TCHAR szCaption[MAX_PATH];
        szCaption[0] = _T('\0');
        for (UINT i = 0; i < (UINT)menuPopup.GetMenuItemCount(); i++)
        {
            CMenuItemInfo mii;
            mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
            mii.dwTypeData = szCaption;
            mii.cch = MAX_PATH;
            if (menuPopup.GetMenuItemInfo(i, TRUE, &mii) && MFT_OWNERDRAW != (MFT_OWNERDRAW & mii.fType))
            {
                mii.fMask = MIIM_TYPE;
                if (MFT_SEPARATOR != mii.fType)
                {
                    auto pData = new TData;
                    mii.dwItemData = reinterpret_cast<ULONG_PTR>(pData);
                    pData->caption = szCaption;
                    pData->isSubmenu = ::IsMenu(mii.hSubMenu);
                    GetThis()->UpdateMenuItemData(pData, mii.wID, i, mii.fType);
                    mii.fMask |= MIIM_DATA;
                }
                mii.fType |= MFT_OWNERDRAW;

                if (!menuPopup.SetMenuItemInfo(i, TRUE, &mii))
                {
                    ATLTRACE(_T("SetMenuItemInfo() failed with error: 0x%0lX, fType=0x%0lX, fState=0x%0lX\n")
                        , ATL::AtlHresultFromLastError(), mii.fType, mii.fState);
                }
            }
        }
        m_openMenus.Push(hMenu);
        return TRUE;
    }

    BOOL UnInitMenuPopup(HMENU hMenu)
    {
        CleanUpMenuData(hMenu);
        auto cMenus = m_openMenus.GetSize();
        if (cMenus && m_openMenus[cMenus - 1] == hMenu)
            m_openMenus.Pop();
        return TRUE;
    }

    void ClearAllMenus()
    {
        ATLTRACE(_T(__FUNCTION__) _T("\n"));
        HMENU hMenu = NULL;
        while (NULL != (hMenu = m_openMenus.Pop()))
        {
            CleanUpMenuData(hMenu);
        }
    }

    void CleanUpMenuData(HMENU hMenu)
    {
        ATLTRACE(_T(__FUNCTION__) _T("(%p)\n"), hMenu);
        CMenuHandle menuPopup = hMenu;
        ATLASSERT(menuPopup.IsMenu());

        BOOL bRet = FALSE;
        for (int i = 0; i < menuPopup.GetMenuItemCount(); i++)
        {
            CMenuItemInfo mii;
            mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
            bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
            ATLASSERT(bRet);

            auto pData = reinterpret_cast<TData*>(mii.dwItemData);
            if (pData)
            {
                //mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
                mii.fType &= ~MFT_OWNERDRAW;
                mii.dwItemData = NULL;
                mii.cch = pData->caption.GetLength();
                mii.dwTypeData = pData->caption.GetBuffer();

                bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
                ATLASSERT(bRet);

                pData->caption.ReleaseBuffer();
                delete pData;
            }
        }
    }

private:

    LRESULT UxModeOnInitMenuPopup(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ATLTRACE(_T(__FUNCTION__) _T("\n"));
        bHandled = FALSE;
        // System menu, do nothing
        if ((BOOL)HIWORD(lParam))
        {
            return 1;
        }
        InitMenuPopup(reinterpret_cast<HMENU>(wParam), LOWORD(lParam));
        return 1L;
    }

    LRESULT UxModeOnUninitMenuPopup(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        ATLTRACE(_T(__FUNCTION__) _T("\n"));
        UnInitMenuPopup(reinterpret_cast<HMENU>(wParam));
        return 1L;
    }

    LRESULT UxModeOnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        //ATLTRaCE(_T(__FUNCTION__) _T("\n"));
        bHandled = FALSE;
        auto flags = HIWORD(wParam);
        if (flags == 0xFFFF && lParam == NULL)   // Menu closing
        {
            ClearAllMenus();
        }
        else if (MF_HILITE == (MF_HILITE & flags))
        {
            bHandled = HandleMenuItemSelected(reinterpret_cast<HMENU>(lParam), LOWORD(wParam), flags);
        }
        return 1L;
    }

    LRESULT UxModeMenuOnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        //ATLTRACE(_T(__FUNCTION__) _T("\n"));
        bHandled = FALSE;
        auto lpDis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
        if (NULL == lpDis || ODT_MENU != lpDis->CtlType)
        {
            return FALSE;
        }

        bHandled = TRUE;
        return CustomDrawPopupMenuItem(lpDis);
    }

    LRESULT UxModeMenuOnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        //ATLTRACE(_T(__FUNCTION__) _T("\n"));
        bHandled = FALSE;
        auto lpMis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
        if (NULL == lpMis || ODT_MENU != lpMis->CtlType)
        {
            return FALSE;
        }

        bHandled = TRUE;
        auto pData = reinterpret_cast<TData*>(lpMis->itemData);
        return MeasurePopupMenuItem(lpMis, pData ? (LPCTSTR)pData->caption : NULL);
    }

protected:
    CSimpleStack<HMENU> m_openMenus;
    CImageList m_menuItemImages;
};