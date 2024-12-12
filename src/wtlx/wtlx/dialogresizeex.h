/************************************************************************
 * $Revision: 4 $
 * $Date: 2016-11-25 01:28:53 +0100 (Fri, 25 Nov 2016) $
 ************************************************************************/
/************************************************************************
 * File: dialogresizeex.h
 * Copyright: 2004 Rob Caldecott, 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Rob Caldecott (http://www.codeproject.com/script/Membership/View.aspx?mid=3688)
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#pragma once

#define DECLARE_DLGRESIZEEX_REGKEY(key) \
    virtual LPCTSTR GetDlgResizeExKey() \
    { \
        if (m_strKeyName.IsEmpty()) m_strKeyName = (key); \
        return m_strKeyName; \
    }

#if (_ATL_VER >= 0x0700)
#define DLGRESIZEEX_REGQUERY_DWORD(regk, name, val) regk.QueryDWORDValue(FormatDlgRegValueName(name), val)
#define DLGRESIZEEX_REGSET_DWORD(regk, name, val) regk.SetDWORDValue(FormatDlgRegValueName(name), val)
#else
#define DLGRESIZEEX_REGQUERY_DWORD(regk, name, val) regk.QueryValue(val, FormatDlgRegValueName(name))
#define DLGRESIZEEX_REGSET_DWORD(regk, name, val) regk.SetValue(val, FormatDlgRegValueName(name))
#endif

// Extension to WTL CDialogResize allowing persistent dialog size
template <class T>
class CDialogResizeEx : public CDialogResize<T>
{
private:
    WINDOWPLACEMENT m_wp;

protected:
    HKEY m_hKeyParent;
    CString m_strKeyName;

public:

	CDialogResizeEx(void)
        : m_wp({ 0 })
		, m_hKeyParent(NULL)
	{
	};

    virtual LPCTSTR GetDlgResizeExKey()
    {
        return m_strKeyName;
    }

	void DlgResize_InitEx(bool bAddGripper = true, bool bUseMinTrackSize = true, DWORD dwForceStyle = WS_CLIPCHILDREN)
	{
		DlgResize_Init(bAddGripper, bUseMinTrackSize, dwForceStyle);
	}
	
	// Load the dialog size from the registry.  Base the registry
	// value on the dialog ID.
    void LoadSize(LPCTSTR lpszKeyName = NULL, HKEY hKeyParent = NULL)
	{
        m_hKeyParent = NULL == hKeyParent ? HKEY_CURRENT_USER : hKeyParent;
        if (NULL != lpszKeyName) m_strKeyName = lpszKeyName;
        ATLASSERT(m_hKeyParent);
        ATLASSERT(m_strKeyName.GetLength());

		ATL::CRegKey reg;
        LSTATUS lstat = reg.Open(m_hKeyParent, m_strKeyName, KEY_READ);
        if (ERROR_SUCCESS == lstat)
		{
            ::ZeroMemory(&m_wp, sizeof(WINDOWPLACEMENT));

 			DWORD dw;
            lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("left"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.rcNormalPosition.left = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("top"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.rcNormalPosition.top = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("right"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.rcNormalPosition.right = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("bottom"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.rcNormalPosition.bottom = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("max_x"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.ptMaxPosition.x = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("max_y"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.ptMaxPosition.y = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("min_x"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.ptMinPosition.x = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("min_y"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.ptMinPosition.y = dw;
            if (ERROR_SUCCESS == lstat) lstat = DLGRESIZEEX_REGQUERY_DWORD(reg, _T("show"), dw);
            if (ERROR_SUCCESS == lstat)
                m_wp.showCmd = dw;

            if (SW_SHOWMINIMIZED == m_wp.showCmd || SW_HIDE == m_wp.showCmd) m_wp.showCmd = SW_SHOWDEFAULT;
            if (ERROR_SUCCESS == lstat) m_wp.length = sizeof(WINDOWPLACEMENT);
		}
	}

	// Save the dialog size to the registry.
	void SaveSize() const
	{
        if (0 < m_wp.length && m_hKeyParent && !m_strKeyName.IsEmpty())
        {
            ATL::CRegKey reg;
            if (reg.Create(m_hKeyParent, m_strKeyName) == ERROR_SUCCESS)
            {
                DLGRESIZEEX_REGSET_DWORD(reg, _T("left"), m_wp.rcNormalPosition.left);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("top"), m_wp.rcNormalPosition.top);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("right"), m_wp.rcNormalPosition.right);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("bottom"), m_wp.rcNormalPosition.bottom);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("max_x"), m_wp.ptMaxPosition.x);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("max_y"), m_wp.ptMaxPosition.y);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("min_x"), m_wp.ptMinPosition.x);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("min_y"), m_wp.ptMinPosition.y);
                DLGRESIZEEX_REGSET_DWORD(reg, _T("show"), m_wp.showCmd);
            }
        }
	}

    CString FormatDlgRegValueName(LPCTSTR lpstrSuff) const
	{
		CString result;
        result.Format(_T("dialog_%d_%s"), T::IDD, lpstrSuff);
        return result;
	}

	BEGIN_MSG_MAP(CDialogResizeEx)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CDialogResize<T>)
	END_MSG_MAP()

    LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        GetDlgResizeExKey();
        if (!m_strKeyName.IsEmpty()) LoadSize();

        T* pT = static_cast<T*>(this);
        if (wParam && !pT->IsWindowVisible())
        {
            // Size the dialog and update the control layout
            if (0 < m_wp.length)
            {
                pT->SetWindowPlacement(&m_wp);

                CRect rectClient;
                pT->GetClientRect(&rectClient);
                DlgResize_UpdateLayout(rectClient.Width(), rectClient.Height());
            }
            else {
                pT->CenterWindow();
            }
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);

		// Save the window size
        ZeroMemory(&m_wp, sizeof(WINDOWPLACEMENT));
        m_wp.length = sizeof(WINDOWPLACEMENT);
        if (pT->GetWindowPlacement(&m_wp)) {
            SaveSize();
        }

		bHandled = FALSE;
		return 0;
	}
};
