#pragma once

#include <functional>
#include <type_traits>

#include <winrt/Windows.UI.ViewManagement.h>

#pragma comment(lib, "Dwmapi.lib")

enum class IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};

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


using FnRefreshImmersiveColorPolicyState = void (WINAPI*)(); // ordinal 104
using FnGetIsImmersiveColorUsingHighContrast = bool (WINAPI*)(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
using FnShouldAppsUseDarkMode = bool (WINAPI*)(); // ordinal 132
using FnAllowDarkModeForWindow = bool (WINAPI*)(HWND hWnd, bool allow); // ordinal 133
using FnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903
using FnFlushMenuThemes = void (WINAPI*)(); // ordinal 136
using FnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);

#define UXTHEME_DECLARE_FUNC(name) \
Fn##name m_pfn##name{NULL}

#define UXTHEME_INIT_FUNC(name, hMod) \
m_pfn##name = reinterpret_cast<Fn##name>(::GetProcAddress(hMod, #name)); \
ATLASSERT(m_pfn##name)

#define UXTHEME_INIT_ORD_FUNC(name, ord, hMod) \
m_pfn##name = reinterpret_cast<Fn##name>(::GetProcAddress(hMod, MAKEINTRESOURCEA(ord))); \
ATLASSERT(m_pfn##name)

class CUxTheme
{
public:
    using UIColorType = winrt::Windows::UI::ViewManagement::UIColorType;
    using UIElementType = winrt::Windows::UI::ViewManagement::UIElementType;

    CUxTheme()
    {
        m_hUxtheme = ::LoadLibraryEx(_T("uxtheme.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
        ATLASSERT(m_hUxtheme);
        if (m_hUxtheme)
        {
            UXTHEME_INIT_ORD_FUNC(RefreshImmersiveColorPolicyState, 104, m_hUxtheme);
            UXTHEME_INIT_ORD_FUNC(GetIsImmersiveColorUsingHighContrast, 106, m_hUxtheme);
            UXTHEME_INIT_ORD_FUNC(ShouldAppsUseDarkMode, 132, m_hUxtheme);
            UXTHEME_INIT_ORD_FUNC(AllowDarkModeForWindow, 133, m_hUxtheme);
            UXTHEME_INIT_ORD_FUNC(SetPreferredAppMode, 135, m_hUxtheme);
            UXTHEME_INIT_ORD_FUNC(FlushMenuThemes, 136, m_hUxtheme);
        }
        auto hModule = ::GetModuleHandle(_T("user32.dll"));
        if (hModule)
        {
            UXTHEME_INIT_FUNC(SetWindowCompositionAttribute, hModule);
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

    bool IsHighContrast() const
    {
        HIGHCONTRASTW highContrast = { sizeof(highContrast) };
        if (::SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
            return highContrast.dwFlags & HCF_HIGHCONTRASTON;
        return false;
    }

    PreferredAppMode SetPreferredAppMode(PreferredAppMode appMode) const
    {
        if (m_pfnSetPreferredAppMode)
            return m_pfnSetPreferredAppMode(appMode);
        return PreferredAppMode::Default;
    }

    void FlushMenuThemes() const
    {
        if (m_pfnFlushMenuThemes)
            m_pfnFlushMenuThemes();
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

    bool IsColorSchemeChangeMessage(LPARAM lParam) const
    {
        auto result = false;
        
        if (lParam && 0 == ::lstrcmpi(reinterpret_cast<LPCTCH>(lParam), _T("ImmersiveColorSet")))
        {
            if (m_pfnRefreshImmersiveColorPolicyState)
            {
                m_pfnRefreshImmersiveColorPolicyState();
            }
            result = true;
        }
        if (m_pfnGetIsImmersiveColorUsingHighContrast)
        {
            m_pfnGetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE::IHCM_REFRESH);
        }
        return result;
    }

    bool IsColorSchemeChangeMessage(UINT message, LPARAM lParam) const
    {
        if (message == WM_SETTINGCHANGE)
            return IsColorSchemeChangeMessage(lParam);
        return false;
    }

    COLORREF UIElementSysColor(UIElementType type) const
    {
        auto col = WinRTSettings.UIElementColor(type);
        return RGB(col.R, col.G, col.B);
    }

    COLORREF GetSysColorValue(UIColorType type) const
    {
        auto col = WinRTSettings.GetColorValue(type);
        return RGB(col.R, col.G, col.B);
    }

private:

    HMODULE m_hUxtheme{NULL};
    UXTHEME_DECLARE_FUNC(RefreshImmersiveColorPolicyState);
    UXTHEME_DECLARE_FUNC(GetIsImmersiveColorUsingHighContrast);
    UXTHEME_DECLARE_FUNC(AllowDarkModeForWindow);
    UXTHEME_DECLARE_FUNC(ShouldAppsUseDarkMode);
    UXTHEME_DECLARE_FUNC(SetPreferredAppMode);
    UXTHEME_DECLARE_FUNC(FlushMenuThemes);
    UXTHEME_DECLARE_FUNC(SetWindowCompositionAttribute);
    winrt::Windows::UI::ViewManagement::UISettings WinRTSettings;
};

extern CUxTheme uxTheme;

#define UXCOLOR_DARKER(color, factor) \
max(RGB(0x50, 0x50, 0x50), RGB(((color & 0xff0000) >> 16) * factor, ((color & 0x00ff00) >> 8) * factor, (color & 0x0000ff) * factor))

#define UXCOLOR_LIGHTER(color, factor) \
min(RGB(0xfe, 0xfe, 0xfe), RGB(((color & 0xff0000) >> 16) * factor, ((color & 0x00ff00) >> 8) * factor, (color & 0x0000ff) * factor))

template<typename T>
struct is_dialog
{
private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template<typename U> static auto test(int) -> decltype(std::declval<U>().GetDialogProc() != nullptr, yes());

    template<typename> static no test(...);

public:

    static constexpr bool value = std::is_same<decltype(test<T>(0)), yes>::value;
};

template<class T>
class CUxModeWindow
{
public:
    BEGIN_MSG_MAP(CUxModeWindow)
        if (IsDialog)
        {
            MESSAGE_HANDLER(WM_INITDIALOG, UxModeOnInitDialog)
        }
        else
        {
            MESSAGE_HANDLER(WM_CREATE, UxModeOnCreate)
        }
        MESSAGE_HANDLER(WM_CTLCOLORDLG, UxModeOnCtlColorDlg)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, UxModeOnCtlColorDlg)
        MESSAGE_HANDLER(WM_SETTINGCHANGE, UxModeOnSettingChange)
        MESSAGE_HANDLER(WM_THEMECHANGED, UxModeOnThemeChange)
    END_MSG_MAP()

    static bool IsInDarkMode()
    {
        return uxTheme.ShouldAppsUseDarkMode() && !uxTheme.IsHighContrast();
    }

public:
    static bool IsDialog;

protected:

    bool SetUxModeForButton(HWND hWnd) const
    {
        auto r = uxTheme.AllowDarkModeForWindow(hWnd, IsInDarkMode());
        auto hr = m_initialized ? S_OK : ::SetWindowTheme(hWnd, L"Explorer", NULL);
        if (m_initialized)
        {
            ::SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
        }
        return r && SUCCEEDED(hr);
    }

    bool UxModeDrawGroupBox(HWND hWnd, HDC hDC)
    {
        CWindow wnd(hWnd);
        CDCHandle dc(hDC);

        CRect rc;
        wnd.GetClientRect(rc);

        CRect rcFrame(rc);
        rcFrame.top += 10;
        auto crBorder = uxTheme.GetSysColorValue(CUxTheme::UIColorType::Foreground);
        crBorder = UXCOLOR_DARKER(crBorder, 0.4);
        CPen pen;
        pen.CreatePen(PS_SOLID, 1, crBorder);
        auto oldPen = dc.SelectPen(pen);
        auto oldBrush = dc.SelectBrush(reinterpret_cast<HBRUSH>(::GetStockObject(HOLLOW_BRUSH)));
        auto result = TRUE == dc.Rectangle(rcFrame);
        dc.SelectPen(oldPen);
        dc.SelectBrush(oldBrush);

        CString str;
        wnd.GetWindowText(str);
        if (str.GetLength())
        {
            CRect rcText(rc);
            rcText.left += 10;
            rcText.bottom = 20;
            auto oldFont = dc.SelectFont(reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT)));
            result = 0 != dc.DrawText(str, str.GetLength(), rcText, DT_SINGLELINE | DT_VCENTER | DT_LEFT) && result;
            dc.SelectFont(oldFont);
        }

        if (result)
        {
            dc.ExcludeClipRect(rc);
        }
        return result;
    }

private:

    LRESULT UxModeOnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        UxModeSetup();
        return 0L;
    }

    LRESULT UxModeOnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        UxModeSetup();
        return TRUE;
    }

    LRESULT UxModeOnCtlColorDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        //ATLTRACE(_T(__FUNCTION__) _T("\n"));
        if (IsInDarkMode() && !m_brushBg.IsNull())
        {
            bHandled = TRUE;

            CWindow wnd(reinterpret_cast<HWND>(lParam));

            CDCHandle dc(reinterpret_cast<HDC>(wParam));
            dc.SetBkColor(uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));
            dc.SetTextColor(uxTheme.GetSysColorValue(CUxTheme::UIColorType::Foreground));

            if (BS_GROUPBOX == (BS_GROUPBOX & wnd.GetWindowLongPtr(GWL_STYLE)))
            {
                UxModeDrawGroupBox(wnd, dc);
            }
            return reinterpret_cast<LONG_PTR>(WS_EX_TRANSPARENT == (WS_EX_TRANSPARENT & wnd.GetWindowLongPtr(GWL_EXSTYLE))
                ? ::GetStockObject(HOLLOW_BRUSH)
                : m_brushBg.m_hBrush);
        }
        return FALSE;
    }

    LRESULT UxModeOnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        if (uxTheme.IsColorSchemeChangeMessage(lParam))
        {
            bHandled = TRUE;
            GetThis()->SendMessage(WM_THEMECHANGED);
        }
        return 0L;
    }

    LRESULT UxModeOnThemeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        if (m_lastModeWasDark != IsInDarkMode())
        {
            bHandled = TRUE;
            UxModeSetup();
            GetThis()->UpdateWindow();
        }
        return 0L;
    }

    T* GetThis()
    {
        return static_cast<T*>(this);
    }

    void UxModeSetup()
    {
        ATLTRACE(_T(__FUNCTION__) _T(" IsDialog=%d IsAppThemed=%d IsThemeActive=%d bgColor=%x\n"), IsDialog, ::IsAppThemed(), ::IsThemeActive()
            , uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));

        auto isDark = IsInDarkMode();
        if (isDark && m_lastModeWasDark != isDark)
        {
            if (!m_brushBg.IsNull())
                m_brushBg.DeleteObject();
            m_brushBg.CreateSolidBrush(uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));
        }

        m_lastModeWasDark = isDark;

        auto pSelf = GetThis();
        if (!m_initialized)
            uxTheme.AllowDarkModeForWindow(pSelf->m_hWnd, true);
        uxTheme.SwitchWindowDarkMode(pSelf->m_hWnd, isDark);

        ::EnumChildWindows(pSelf->m_hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL
            {
                auto pParent = reinterpret_cast<CUxModeWindow*>(lParam);
                CString str;
                ::GetClassName(hwnd, str.GetBufferSetLength(MAX_PATH), MAX_PATH);
                str.ReleaseBuffer();
                if (_T("Button") == str)
                {
                    pParent->SetUxModeForButton(hwnd);
                }
                return TRUE;
            }, reinterpret_cast<LPARAM>(this));

        if (!m_initialized)
            pSelf->SendMessage(WM_THEMECHANGED);

        m_initialized = true;
    }

protected:
    bool m_initialized{ false };
    bool m_lastModeWasDark{false};
    CBrush m_brushBg;

};

template<class T>
bool CUxModeWindow<T>::IsDialog = is_dialog<T>::value;