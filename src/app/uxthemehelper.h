#pragma once

enum class PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

using FnShouldAppsUseDarkMode = bool (WINAPI*)(); // ordinal 132
using FnAllowDarkModeForWindow = bool (WINAPI*)(HWND hWnd, bool allow); // ordinal 133
using FnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903

class CUxTheme
{
public:
    CUxTheme()
        : m_hUxtheme(NULL), m_pfnAllowDarkModeForWindow(NULL), m_pfnShouldAppsUseDarkMode(NULL), m_pfnSetPreferredAppMode(NULL)
    {
        m_hUxtheme = ::LoadLibraryEx(_T("uxtheme.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
        ATLASSERT(m_hUxtheme);
        if (m_hUxtheme)
        {
            m_pfnShouldAppsUseDarkMode = (FnShouldAppsUseDarkMode)::GetProcAddress(m_hUxtheme, MAKEINTRESOURCEA(132));
            ATLASSERT(m_pfnShouldAppsUseDarkMode);
            m_pfnAllowDarkModeForWindow = (FnAllowDarkModeForWindow)::GetProcAddress(m_hUxtheme, MAKEINTRESOURCEA(133));
            ATLASSERT(m_pfnAllowDarkModeForWindow);
            m_pfnSetPreferredAppMode = (FnSetPreferredAppMode)::GetProcAddress(m_hUxtheme, MAKEINTRESOURCEA(135));
            ATLASSERT(m_pfnSetPreferredAppMode);
        }
    }

    ~CUxTheme()
    {
        if (m_hUxtheme)
            ::FreeLibrary(m_hUxtheme);
    }

    bool ShouldAppsUseDarkMode() const
    {
        if (m_pfnShouldAppsUseDarkMode)
            return m_pfnShouldAppsUseDarkMode();
        return false;
    }

    bool AllowDarkModeForWindow(HWND hWnd, bool allow) const
    {
        if (m_pfnAllowDarkModeForWindow)
            return m_pfnAllowDarkModeForWindow(hWnd, allow);
        return false;
    }

    PreferredAppMode SetPreferredAppMode(PreferredAppMode appMode) const
    {
        if (m_pfnSetPreferredAppMode)
            return m_pfnSetPreferredAppMode(appMode);
        return PreferredAppMode::Default;
    }

private:

    HMODULE m_hUxtheme;
    FnAllowDarkModeForWindow m_pfnAllowDarkModeForWindow;
    FnShouldAppsUseDarkMode m_pfnShouldAppsUseDarkMode;
    FnSetPreferredAppMode m_pfnSetPreferredAppMode;
};

