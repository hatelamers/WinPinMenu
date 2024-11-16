#pragma once

#pragma comment(lib, "Dwmapi.lib")

enum class PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

enum WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};


using FnShouldAppsUseDarkMode = bool (WINAPI*)(); // ordinal 132
using FnAllowDarkModeForWindow = bool (WINAPI*)(HWND hWnd, bool allow); // ordinal 133
using FnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903
using FnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);

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
        auto hModule = ::GetModuleHandle(_T("user32.dll"));
        if (hModule)
        {
            m_pfnSetWindowCompositionAttribute = reinterpret_cast<FnSetWindowCompositionAttribute>(::GetProcAddress(hModule, "SetWindowCompositionAttribute"));
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

    PreferredAppMode SetPreferredAppMode(PreferredAppMode appMode) const
    {
        if (m_pfnSetPreferredAppMode)
            return m_pfnSetPreferredAppMode(appMode);
        return PreferredAppMode::Default;
    }

    bool AllowDarkModeForWindow(HWND hWnd, bool allow) const
    {
        DWMNCRENDERINGPOLICY ncrp = allow ? DWMNCRP_ENABLED : DWMNCRP_DISABLED;
        ::DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));

        auto result = false;
        if (m_pfnAllowDarkModeForWindow)
            result = m_pfnAllowDarkModeForWindow(hWnd, allow);

        return result;
    }

    BOOL SetWindowCompositionAttribute(HWND hWnd, WINDOWCOMPOSITIONATTRIB attr, BOOL value) const
    {
        if (m_pfnSetWindowCompositionAttribute)
        {
            WINDOWCOMPOSITIONATTRIBDATA data = { attr, &value, sizeof(value) };
            return m_pfnSetWindowCompositionAttribute(hWnd, &data);
        }
        return FALSE;
    }

    bool SwitchWindowDarkMode(HWND hWnd, bool setDark, bool immersive = false) const
    {
        auto result = false;
        if (immersive)
        {
            BOOL value = setDark ? TRUE : FALSE;
            result = SUCCEEDED(::DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value)));
        }
        else
        {
            result = SetWindowCompositionAttribute(hWnd, WINDOWCOMPOSITIONATTRIB::WCA_USEDARKMODECOLORS, setDark ? TRUE : FALSE);
        }
        return result;
    }

private:

    HMODULE m_hUxtheme;
    FnAllowDarkModeForWindow m_pfnAllowDarkModeForWindow;
    FnShouldAppsUseDarkMode m_pfnShouldAppsUseDarkMode;
    FnSetPreferredAppMode m_pfnSetPreferredAppMode;
    FnSetWindowCompositionAttribute m_pfnSetWindowCompositionAttribute;
};

