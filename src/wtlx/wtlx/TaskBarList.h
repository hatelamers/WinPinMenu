/************************************************************************
 * $Revision: 33 $
 * $Date: 2024-10-28 17:50:24 +0100 (Mon, 28 Oct 2024) $
 ************************************************************************/
/************************************************************************
 * File: TaskBarList.h
 * Copyright: 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#pragma once

#include <Shobjidl.h>

#if (WINVER <= 0x0600)

#define MSGFLT_RESET                            (0)
#define MSGFLT_ALLOW                            (1)
#define MSGFLT_DISALLOW                         (2)

typedef struct tagCHANGEFILTERSTRUCT {
    DWORD cbSize;
    DWORD ExtStatus;
} CHANGEFILTERSTRUCT, *PCHANGEFILTERSTRUCT;

typedef BOOL(WINAPI *PFN_ChangeWindowMessageFilterEx)(
    _In_        HWND                hWnd,
    _In_        UINT                message,
    _In_        DWORD               action,
    _Inout_opt_ PCHANGEFILTERSTRUCT pChangeFilterStruct
    );

static BOOL WINAPI AllowWindowMessageFilter(_In_ HWND hWnd, _In_ UINT message)
{
    BOOL fRes = FALSE;
    HMODULE hLib = ::LoadLibrary(_T("user32.dll"));
    if (hLib)
    {
        PFN_ChangeWindowMessageFilterEx pfn = (PFN_ChangeWindowMessageFilterEx) ::GetProcAddress(hLib, "ChangeWindowMessageFilterEx");
        if (pfn) {
            fRes = pfn(hWnd, message, MSGFLT_ALLOW, NULL);
        }
        ::FreeLibrary(hLib);
    }
    if (!fRes) {
        fRes = ::ChangeWindowMessageFilter(message, MSGFLT_ADD);
    }
    return fRes;
}
#else
static BOOL WINAPI AllowWindowMessageFilter(_In_ HWND hWnd, _In_ UINT message)
{
    return ::ChangeWindowMessageFilterEx(hWnd, message, MSGFLT_ALLOW, NULL);
}
#endif

class CTaskBarList
{
private:
    BOOL m_fIsInitialized;
    HWND m_hwnd;
    CComPtr<ITaskbarList3> m_pTaskbarList;
    ULONGLONG m_ullPrgCompleted;
    ULONGLONG m_ullPrgTotal;
    CString m_strTip;
    TBPFLAG m_tbpFlags;

public:
    const UINT WM_TaskbarButtonCreated;
    
    CTaskBarList()
        : WM_TaskbarButtonCreated(::RegisterWindowMessage(_T("TaskbarButtonCreated")))
        , m_fIsInitialized(FALSE)
        , m_ullPrgCompleted(0LL)
        , m_ullPrgTotal(0LL)
        , m_tbpFlags(TBPFLAG::TBPF_NOPROGRESS)
    {
    }

    virtual ~CTaskBarList()
    {
    }

    BEGIN_MSG_MAP(CTaskBarList)
        MESSAGE_HANDLER(WM_TaskbarButtonCreated, OnTaskbarButtonCreated)
    END_MSG_MAP()

    BOOL Initialize(HWND hwnd)
    {
        if (!::IsWindows7OrGreater())
        {
            ATLTRACE(_T("Unsupported Windows version\n"));
            return FALSE;
        }

        m_fIsInitialized = 0 != WM_TaskbarButtonCreated;
        ATLASSERT(m_fIsInitialized);
        if (m_fIsInitialized)
        {
            m_hwnd = hwnd;
            m_fIsInitialized = ::IsWindow(m_hwnd);
            ATLASSERT(m_fIsInitialized);
        }
        if (m_fIsInitialized) {
            m_fIsInitialized = ::AllowWindowMessageFilter(m_hwnd, WM_TaskbarButtonCreated);
        }
        return m_fIsInitialized;
    }

    HRESULT SetProgressState(TBPFLAG tbpFlags)
    {
        if (m_pTaskbarList) {
            return m_pTaskbarList->SetProgressState(m_hwnd, tbpFlags);
        }
        else
        {
            m_tbpFlags = tbpFlags;
        }
        return E_NOT_VALID_STATE;
    }

    HRESULT SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal = 100LL)
    {
        if (m_pTaskbarList) {
            return m_pTaskbarList->SetProgressValue(m_hwnd, ullCompleted, ullTotal);
        }
        else
        {
            m_ullPrgCompleted = ullCompleted;
            m_ullPrgTotal = ullTotal;
        }
        return E_NOT_VALID_STATE;
    }

    HRESULT SetThumbnailTooltip(LPCWSTR pszTip = NULL)
    {
        if (m_pTaskbarList) {
            return m_pTaskbarList->SetThumbnailTooltip(m_hwnd, pszTip);
        }
        else
        {
            if (NULL == pszTip)
                m_strTip.Empty();
            else
                m_strTip = pszTip;
        }
        return E_NOT_VALID_STATE;
    }

public:
    LRESULT OnTaskbarButtonCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        ATLTRACE(_T("OnTaskbarButtonCreated() m_fIsInitialized=%d\n"), m_fIsInitialized);
        if (m_fIsInitialized)
        {
            HRESULT hr = m_pTaskbarList.CoCreateInstance(CLSID_TaskbarList);
            if (SUCCEEDED(hr) && m_pTaskbarList)
            {
                if (TBPFLAG::TBPF_NOPROGRESS != m_tbpFlags)
                {
                    SetProgressState(m_tbpFlags);
                    if (TBPFLAG::TBPF_NORMAL == m_tbpFlags) SetProgressValue(m_ullPrgCompleted, m_ullPrgTotal);
                }
                if (m_strTip.GetLength()) SetThumbnailTooltip(m_strTip);
            }
            else if (FAILED(hr)) {
                ATLTRACE(_T("Failed to create TaskbarList: 0x%0lX\n"), hr);
            }
        }
        return TRUE;
    }
};

