/************************************************************************
 * $Revision: 33 $
 * $Date: 2024-10-28 17:50:24 +0100 (Mon, 28 Oct 2024) $
 ************************************************************************/
/************************************************************************
 * File: NotifyTrayIcon.h - Implementation of the CNotifyIconData class and the CNotifyTrayIcon template.
 * Copyright: 2002 Rob Caldecott, 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Rob Caldecott (http://www.codeproject.com/script/Membership/View.aspx?mid=3688)
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#pragma once

#include <atlmisc.h>
#include <shellapi.h>

#ifndef WM_TRAYICON
#define WM_TRAYICON (WM_APP + 100)
#endif

// Wrapper class for the Win32 NOTIFYICONDATA structure
class CNotifyIconData : public NOTIFYICONDATA
{
public:	
	CNotifyIconData()
	{
        ::ZeroMemory(this, sizeof(NOTIFYICONDATA));
		cbSize = sizeof(NOTIFYICONDATA);
	}
};

// Template used to support adding an icon to the taskbar.
// This class will maintain a taskbar icon and associated context menu.
template <class T>
class CNotifyTrayIcon
{
private:
	CNotifyIconData m_nid;
	bool m_bTrayIconInstalled;
	UINT m_uDefaultTrayMID;
    UINT m_uVersion;
public:	
	CNotifyTrayIcon()
        : m_bTrayIconInstalled(false)
        , m_uDefaultTrayMID(0)
        , m_uVersion(NOTIFYICON_VERSION)
	{
	}
	
	~CNotifyTrayIcon()
	{
		// Remove the icon
		RemoveNotifyIcon();
	}

	// Install a taskbar icon
	// 	lpszToolTip 	- The tooltip to display
	//	hIcon 		- The icon to display
	// 	nID		- The resource ID of the context menu
	/// returns true on success
    bool AddNotifyIcon(HICON hIcon, UINT nID, LPCTSTR lpszToolTip = NULL, UINT uVersion = NOTIFYICON_VERSION)
	{
        m_uVersion = uVersion;
		T* pT = static_cast<T*>(this);
		// Fill in the data		
		m_nid.hWnd = pT->m_hWnd;
		m_nid.uID = nID;
		m_nid.hIcon = hIcon;
        m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
		m_nid.uCallbackMessage = WM_TRAYICON;
        if (NULL == lpszToolTip)
        {
            CString s;
            pT->GetWindowText(s);
            ::lstrcpyn(m_nid.szTip, s, sizeof(m_nid.szTip));
        }
        else {
            ::lstrcpyn(m_nid.szTip, lpszToolTip, sizeof(m_nid.szTip));
        }
		// Install
		m_bTrayIconInstalled = ::Shell_NotifyIcon(NIM_ADD, &m_nid) ? true : false;

        if (m_bTrayIconInstalled && 0 < m_uVersion && ::IsWindowsVistaOrGreater())
        {
            m_nid.uFlags = NIF_SHOWTIP;
            m_nid.uVersion = m_uVersion;
            ::Shell_NotifyIcon(NIM_SETVERSION, &m_nid);
            m_nid.uVersion = 0;
        }
		// Done
		return m_bTrayIconInstalled;
	}

	// Remove taskbar icon
	// returns true on success
	bool RemoveNotifyIcon()
	{
		if (!m_bTrayIconInstalled)
			return false;
		// Remove
		m_nid.uFlags = 0;
		return ::Shell_NotifyIcon(NIM_DELETE, &m_nid) ? true : false;
	}

	// Set the icon tooltip text
	// returns true on success
	bool SetNotifyTooltipText(LPCTSTR pszTooltipText = NULL)
	{
        if (!m_bTrayIconInstalled)
            return false;
        // Fill the structure
		m_nid.uFlags = NIF_TIP;
        if (NULL == pszTooltipText)
        {
            T* pT = static_cast<T*>(this);
            CString s;
            pT->GetWindowText(s);
            ::lstrcpyn(m_nid.szTip, s, sizeof(m_nid.szTip));
        }
        else {
            ::lstrcpyn(m_nid.szTip, pszTooltipText, sizeof(m_nid.szTip));
        }
		// Set
		return ::Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
	}

    bool SetNotifyIcon(HICON hIcon)
    {
        if (!m_bTrayIconInstalled)
            return false;
        m_nid.uFlags = NIF_ICON;
        m_nid.hIcon = hIcon;
        return ::Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
    }

    bool DisplayNotifyInfo(LPCTSTR pszInfoText, LPCTSTR pszInfoTitle = NULL, DWORD dwInfoFlags = NIIF_USER | 0x00000080, UINT uTimeout = 20000)
    {
        if (!pszInfoText || !m_bTrayIconInstalled)
            return false;
        m_nid.uFlags = NIF_INFO;
        m_nid.dwInfoFlags = dwInfoFlags;
        m_nid.uTimeout = uTimeout;
        ::lstrcpyn(m_nid.szInfo, pszInfoText, sizeof(m_nid.szInfo));

        if (NULL == pszInfoTitle)
        {
            T* pT = static_cast<T*>(this);
            CString s;
            pT->GetWindowText(s);
            ::lstrcpyn(m_nid.szInfoTitle, s, sizeof(m_nid.szInfoTitle));
        }
        else {
            ::lstrcpyn(m_nid.szInfoTitle, pszInfoTitle, sizeof(m_nid.szInfoTitle));
        }
        BOOL fRes = ::Shell_NotifyIcon(NIM_MODIFY, &m_nid);
        m_nid.uTimeout = 0;
        return fRes ? true : false;
    }

    bool HideNotifyInfo()
    {
        if (!m_bTrayIconInstalled)
            return false;
        m_nid.uFlags = NIF_INFO;
        m_nid.szInfo[0] = _T('\0');
        return ::Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
    }

	// Set the default menu item ID
	inline void SetDefaultItem(UINT nID) { m_uDefaultTrayMID = nID; }

	BEGIN_MSG_MAP(CTrayIcon)
		MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
	END_MSG_MAP()

	LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Is this the ID we want?
		if (wParam != m_nid.uID)
			return 0;
		T* pT = static_cast<T*>(this);
		// Was the right-button clicked?
		if (LOWORD(lParam) == WM_RBUTTONUP)
		{
			// Load the menu
			CMenu oMenu;
			if (!oMenu.LoadMenu(m_nid.uID))
				return 0;
			// Get the sub-menu
			CMenuHandle oPopup(oMenu.GetSubMenu(0));
			// Prepare
			pT->PrepareNotifyMenu(oPopup);
			// Get the menu position
			CPoint pos;
			::GetCursorPos(&pos);
			// Make app the foreground
			::SetForegroundWindow(pT->m_hWnd);
			// Set the default menu item
			if (m_uDefaultTrayMID == 0)
				oPopup.SetMenuDefaultItem(0, TRUE);
			else
				oPopup.SetMenuDefaultItem(m_uDefaultTrayMID);
			// Track
			oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pT->m_hWnd);
			// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
			pT->PostMessage(WM_NULL);
			// Done
		}
		else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
		{
			// Make app the foreground
			::SetForegroundWindow(pT->m_hWnd);
			// Load the menu
			CMenu oMenu;
			if (!oMenu.LoadMenu(m_nid.uID))
				return 0;
			// Get the sub-menu
			CMenuHandle oPopup(oMenu.GetSubMenu(0));			
			// Get the item
			if (m_uDefaultTrayMID)
			{
				// Send
				pT->SendMessage(WM_COMMAND, m_uDefaultTrayMID, 0);
			}
			else
			{
				UINT nItem = oPopup.GetMenuItemID(0);
				// Send
				pT->SendMessage(WM_COMMAND, nItem, 0);
			}
			// Done
		}
		return 0;
	}

	// Allow the menu items to be enabled/checked/etc.
	virtual void PrepareNotifyMenu(HMENU /*hMenu*/)
	{
		// Stub
	}
};
