#pragma once

#include <atlapp.h>
#include <atlwin.h>
#include <atltheme.h>

#include <dwmapi.h>

#include <functional>
#include <type_traits>

#include <winrt/Windows.UI.ViewManagement.h>

#pragma comment(lib, "Dwmapi.lib")

#if !defined(UXMODE_SUPPORT_SCROLLBAR) && (defined(UXMODE_SUPPORT_LISTVIEW) || defined(UXMODE_SUPPORT_TREEVIEW))
#define UXMODE_SUPPORT_SCROLLBAR
#endif

#ifdef UXMODE_SUPPORT_SCROLLBAR
#include "wtlx/IatHook.h"
#endif // UXMODE_SUPPORT_SCROLLBAR


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


using FnOpenNcThemeData = HTHEME(WINAPI*)(HWND hWnd, LPCWSTR pszClassList); // ordinal 49
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
            UXTHEME_INIT_ORD_FUNC(OpenNcThemeData, 49, m_hUxtheme);
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

    HTHEME OpenNcThemeData(HWND hWnd, LPCWSTR pszClassList) const
    {
        if (m_pfnOpenNcThemeData)
            return m_pfnOpenNcThemeData(hWnd, pszClassList);
        return NULL;
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
            auto value = setDark ? DWM_BB_ENABLE : FALSE;
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

    bool IsInDarkMode()
    {
        return ShouldAppsUseDarkMode() && !IsHighContrast();
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


#ifdef UXMODE_SUPPORT_SCROLLBAR
    inline void FixDarkScrollBar();
#endif // UXMODE_SUPPORT_SCROLLBAR

private:

    HMODULE m_hUxtheme{NULL};
    UXTHEME_DECLARE_FUNC(OpenNcThemeData);
    UXTHEME_DECLARE_FUNC(RefreshImmersiveColorPolicyState);
    UXTHEME_DECLARE_FUNC(GetIsImmersiveColorUsingHighContrast);
    UXTHEME_DECLARE_FUNC(AllowDarkModeForWindow);
    UXTHEME_DECLARE_FUNC(ShouldAppsUseDarkMode);
    UXTHEME_DECLARE_FUNC(SetPreferredAppMode);
    UXTHEME_DECLARE_FUNC(FlushMenuThemes);
    UXTHEME_DECLARE_FUNC(SetWindowCompositionAttribute);
    winrt::Windows::UI::ViewManagement::UISettings WinRTSettings;
    bool m_hasComCtlHook{ false };
};

extern CUxTheme uxTheme;

#ifdef UXMODE_SUPPORT_SCROLLBAR
void CUxTheme::FixDarkScrollBar()
{
    if (!m_hasComCtlHook)
    {
        HMODULE hComctl = ::LoadLibraryEx(_T("comctl32.dll"), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (hComctl)
        {
            auto addr = ::FindDelayLoadThunkInModule(hComctl, "uxtheme.dll", 49); // OpenNcThemeData
            if (addr)
            {
                DWORD oldProtect;
                if (::VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect))
                {
                    auto MyOpenThemeData = [](HWND hWnd, LPCWSTR classList) -> HTHEME {
                        if (wcscmp(classList, L"ScrollBar") == 0)
                        {
                            hWnd = nullptr;
                            classList = L"Explorer::ScrollBar";
                        }
                        return uxTheme.OpenNcThemeData(hWnd, classList);
                        };

                    addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<FnOpenNcThemeData>(MyOpenThemeData));
                    ::VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
                }
            }
        }
        m_hasComCtlHook = true;
    }
}
#endif // UXMODE_SUPPORT_SCROLLBAR

#define UXCOLOR_DARKER(color, factor) \
RGB(max(0x0, GetRValue(color) - (0xff * factor)), max(0x0, GetGValue(color) - (0xff * factor)), max(0x0, GetBValue(color) - (0xff * factor)))

#define UXCOLOR_LIGHTER(color, factor) \
RGB( \
min(0xff, GetRValue(color) + (0xff * factor)), \
min(0xff, GetGValue(color) + (0xff * factor)), \
min(0xff, GetBValue(color) + (0xff * factor)) \
)

struct UxModeMenuColors
{
    COLORREF crText{ RGB(0,0,0) };
    COLORREF crTextDisabled{ RGB(128,128,128) };
    COLORREF crBg{ RGB(255,255,255) };
    COLORREF crHihghlight{ RGB(200,200,200) };
    COLORREF crHighlightBg{ RGB(200,200,200) };
    COLORREF crBorder{ RGB(200,200,200) };
    CBrush brushText;
    CBrush brushTextDisabled;
    CBrush brushBg;

    void Update(bool isDark)
    {
        crBg = isDark ? UXCOLOR_LIGHTER(uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background), 0.2) : ::GetSysColor(COLOR_3DFACE);
        crText = isDark ? uxTheme.GetSysColorValue(CUxTheme::UIColorType::Foreground) : ::GetSysColor(COLOR_MENUTEXT);
        crTextDisabled = isDark ? UXCOLOR_DARKER(crText, 0.5) : ::GetSysColor(COLOR_GRAYTEXT);
        crBorder = isDark ? UXCOLOR_DARKER(crText, 0.6) : ::GetSysColor(COLOR_SCROLLBAR);
        crHihghlight = isDark ? uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background) : ::GetSysColor(COLOR_HIGHLIGHT);
        crHighlightBg = isDark ? UXCOLOR_LIGHTER(crBg, 0.18) : uxTheme.GetSysColorValue(CUxTheme::UIColorType::AccentLight3);
        if (isDark)
        {
            if (!brushBg.IsNull())
                brushBg.DeleteObject();
            brushBg.CreateSolidBrush(crBg);
        }
        if (!brushText.IsNull())
            brushText.DeleteObject();
        brushText.CreateSolidBrush(crText);
        if (!brushTextDisabled.IsNull())
            brushTextDisabled.DeleteObject();
        brushTextDisabled.CreateSolidBrush(crTextDisabled);

    }
};
struct UxModeMenuMetrics
{
    CSize sizeIcon{ 0, 0 };
    CSize sizeMnuArrow{ 16, 16 };
    CSize paddingText{ 5, 5 };
    CSize paddingIcon{ 2, 2 };
    int itemHeight{ -1 };
    NONCLIENTMETRICS metricsNC{ 0 };
};

template<class T>
class CUxModeBase
{
protected:

    virtual void UxModeSetup()
    {
        if (!m_menuTheme.IsThemeNull())
            m_menuTheme.CloseThemeData();
        auto hwnd = GetOwnerHWND();
        if (::IsWindow(hwnd))
            m_menuTheme.OpenThemeData(hwnd, VSCLASS_MENU);
        UxModeUpdateColorSettings();
        m_lastModeWasDark = uxTheme.IsInDarkMode();
    }

    virtual void UxModeUpdateColorSettings()
    {
    }

    virtual HWND GetOwnerHWND()
    {
        return NULL;
    }

    virtual T* GetThis()
    {
        return dynamic_cast<T*>(this);
    }

    LRESULT UxModeOnThemeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        if (m_lastModeWasDark != uxTheme.IsInDarkMode())
        {
            bHandled = TRUE;
            UxModeSetup();
            auto hwnd = GetOwnerHWND();
            if (::IsWindow(hwnd))
                ::UpdateWindow(hwnd);
        }
        return 0L;
    }

    LRESULT UxModeOnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = FALSE;
        if (!m_menuTheme.IsThemeNull())
            m_menuTheme.CloseThemeData();

        return 1;
    }

protected:
    bool m_lastModeWasDark{ false };
    CTheme m_menuTheme;
};

template<class T>
class CUxModeMenuHelper
    : public virtual CUxModeBase<T>
{
public:

    BEGIN_MSG_MAP(CUxModeMenuHelper)
        MESSAGE_HANDLER(WM_THEMECHANGED, UxModeOnThemeChange)
        MESSAGE_HANDLER(WM_DESTROY, UxModeOnDestroy)
    END_MSG_MAP()

protected:
    virtual void UxModeSetup() override
    {
        CBitmap bmpArrow;
        BITMAP bm;
        if (bmpArrow.LoadOEMBitmap(OBM_MNARROW) && bmpArrow.GetBitmap(bm))
        {
            m_menuMetrics.sizeMnuArrow.cx = bm.bmWidth;
            m_menuMetrics.sizeMnuArrow.cy = bm.bmHeight;
        }

        m_menuMetrics.metricsNC.cbSize = sizeof(NONCLIENTMETRICS);
        if (::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, m_menuMetrics.metricsNC.cbSize, &m_menuMetrics.metricsNC, 0))
        {
            m_menuMetrics.itemHeight = max(m_menuMetrics.metricsNC.iMenuHeight, ::abs(m_menuMetrics.metricsNC.lfMenuFont.lfHeight) + (m_menuMetrics.paddingText.cy * 2));
            if (m_menuMetrics.sizeIcon.cy && m_menuMetrics.itemHeight < (m_menuMetrics.paddingIcon.cy * 2) + m_menuMetrics.sizeIcon.cy)
            {
                m_menuMetrics.itemHeight = (m_menuMetrics.paddingIcon.cy * 2) + m_menuMetrics.sizeIcon.cy;
            }
            if (m_menuMetrics.paddingIcon.cy > m_menuMetrics.itemHeight - m_menuMetrics.sizeIcon.cy - m_menuMetrics.paddingIcon.cy)
            {
                m_menuMetrics.paddingIcon.cy = (m_menuMetrics.itemHeight - m_menuMetrics.sizeIcon.cy) / 2;
            }
        }
        CUxModeBase<T>::UxModeSetup();
        if (!m_menuTheme.IsThemeNull())
        {
            HBITMAP hBmp = NULL;
            auto hr = m_menuTheme.GetThemeBitmap(MENU_POPUPBACKGROUND, 0, TMT_DIBDATA, GBF_DIRECT, hBmp);
            if (SUCCEEDED(hr))
            {
                if (!m_menuColors.brushBg.IsNull())
                    m_menuColors.brushBg.DeleteObject();
                m_menuColors.brushBg.CreatePatternBrush(hBmp);
            }
        }
    }

    virtual void UxModeUpdateColorSettings() override
    {
        m_menuColors.Update(uxTheme.IsInDarkMode());
    }

    void UxModeUpdateMenuInfo(MENUINFO& mi)
    {
        if (uxTheme.IsInDarkMode())
        {
            mi.fMask |= MIM_BACKGROUND;
            mi.hbrBack = m_menuColors.brushBg;
        }
    }

    BOOL CustomDrawMenuArrow(HDC hdcItem, LPRECT rcItem, bool isDisabled)
    {
        CRect rcArrow(rcItem);
        rcArrow.left = rcItem->right - m_menuMetrics.sizeMnuArrow.cx;

        if (rcArrow.Height() > m_menuMetrics.sizeMnuArrow.cy)
        {
            rcArrow.top += (rcArrow.Height() - m_menuMetrics.sizeMnuArrow.cy) / 2;
            rcArrow.bottom = rcArrow.top + m_menuMetrics.sizeMnuArrow.cy;
        }

        auto result = FALSE;
        CDCHandle dc(hdcItem);

#ifdef UXMODE_FULL_CUSTOMDRAWN_MENUARROW
        CDC arrowDC;
        arrowDC.CreateCompatibleDC(hdcItem);

        CBitmap arrowBitmap;
        arrowBitmap.CreateCompatibleBitmap(dc, rcArrow.Width(), rcArrow.Height());
        auto oldArrowBmp = arrowDC.SelectBitmap(arrowBitmap);

        CDC fillDC;
        fillDC.CreateCompatibleDC(hdcItem);

        CBitmap fillBitmap;
        fillBitmap.CreateCompatibleBitmap(dc, rcArrow.Width(), rcArrow.Height());
        auto oldFillBmp = fillDC.SelectBitmap(fillBitmap);

        CRect rcTemp(0, 0, rcArrow.Width(), rcArrow.Height());
        result = arrowDC.DrawFrameControl(&rcTemp, DFC_MENU, DFCS_MENUARROW);

        fillDC.FillRect(rcTemp, isDisabled ? m_menuColors.brushTextDisabled : m_menuColors.brushText);

        dc.BitBlt(rcArrow.left, rcArrow.top, rcArrow.Width(), rcArrow.Height(), fillDC, 0, 0, SRCINVERT);
        dc.BitBlt(rcArrow.left, rcArrow.top, rcArrow.Width(), rcArrow.Height(), arrowDC, 0, 0, SRCAND);
        dc.BitBlt(rcArrow.left, rcArrow.top, rcArrow.Width(), rcArrow.Height(), fillDC, 0, 0, SRCINVERT);

        arrowDC.SelectBitmap(oldArrowBmp);
        fillDC.SelectBitmap(oldFillBmp);
#else
        result = SUCCEEDED(m_menuTheme.DrawThemeBackground(dc, MENU_POPUPSUBMENU, isDisabled ? MSM_DISABLED : MSM_NORMAL, rcArrow, NULL));

#endif //UXMODE_FULL_CUSTOMDRAW_MENUARROW
        return result;
    }

protected:
    UxModeMenuMetrics m_menuMetrics;
    UxModeMenuColors m_menuColors;
};

#define WM_UAHDRAWMENU         0x0091	// lParam is UAHMENU
#define WM_UAHDRAWMENUITEM     0x0092	// lParam is UAHDRAWMENUITEM
#define WM_UAHMEASUREMENUITEM  0x0094	// lParam is UAHMEASUREMENUITEM

// hmenu is the main window menu; hdc is the context to draw in
typedef struct tagUAHMENU
{
    HMENU hmenu;
    HDC hdc;
    DWORD dwFlags; // no idea what these mean, in my testing it's either 0x00000a00 or sometimes 0x00000a10
} UAHMENU;

// describes the sizes of the menu bar or menu item
typedef union tagUAHMENUITEMMETRICS
{
    // cx appears to be 14 / 0xE less than rcItem's width!
    // cy 0x14 seems stable, i wonder if it is 4 less than rcItem's height which is always 24 atm
    struct {
        DWORD cx;
        DWORD cy;
    } rgsizeBar[2];
    struct {
        DWORD cx;
        DWORD cy;
    } rgsizePopup[4];
} UAHMENUITEMMETRICS;

// not really used in our case but part of the other structures
typedef struct tagUAHMENUPOPUPMETRICS
{
    DWORD rgcx[4];
    DWORD fUpdateMaxWidths : 2; // from kernel symbols, padded to full dword
} UAHMENUPOPUPMETRICS;

// menu items are always referred to by iPosition here
typedef struct tagUAHMENUITEM
{
    int iPosition; // 0-based position of menu item in menubar
    UAHMENUITEMMETRICS umim;
    UAHMENUPOPUPMETRICS umpm;
} UAHMENUITEM;

// the DRAWITEMSTRUCT contains the states of the menu items, as well as
// the position index of the item in the menu, which is duplicated in
// the UAHMENUITEM's iPosition as well
typedef struct UAHDRAWMENUITEM
{
    DRAWITEMSTRUCT dis; // itemID looks uninitialized
    UAHMENU um;
    UAHMENUITEM umi;
} UAHDRAWMENUITEM;

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
template<typename T>
struct is_prop_sheet
{
private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template<typename U> static auto test(int) -> decltype(std::declval<U>().GetActivePage() != nullptr, yes());

    template<typename> static no test(...);

public:

    static constexpr bool value = std::is_same<decltype(test<T>(0)), yes>::value;
};
enum class UxModeWindowType
{
    GENERIC, DIALOG, PROP_SHEET
};

#define UXPROP_LISTVIEWPROC _T("UxModeListViewProc")

template<class T>
class CUxModeWindow
    : public virtual CUxModeBase<T>
{
public:

    BEGIN_MSG_MAP(CUxModeWindow)
        switch (UxWindowType)
        {
        case UxModeWindowType::DIALOG:
            MESSAGE_HANDLER(WM_INITDIALOG, UxModeOnInitDialog)
            break;
        default:
            MESSAGE_HANDLER(WM_CREATE, UxModeOnCreate)
            break;
        }
        MESSAGE_HANDLER(WM_CTLCOLORDLG, UxModeOnCtlColorDlg)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, UxModeOnCtlColorDlg)
        MESSAGE_HANDLER(WM_SETTINGCHANGE, UxModeOnSettingChange)
        MESSAGE_HANDLER(WM_THEMECHANGED, UxModeOnThemeChange)
        MESSAGE_HANDLER(WM_NCPAINT, UxModeOnNcPaint)
        MESSAGE_HANDLER(WM_NCACTIVATE, UxModeOnNcPaint)
        MESSAGE_HANDLER(WM_UAHDRAWMENU, UxModeOnUahDrawMenu)
        MESSAGE_HANDLER(WM_UAHDRAWMENUITEM, UxModeOnUahDrawMenuItem)
        MESSAGE_HANDLER(WM_DESTROY, UxModeOnDestroy)
#ifdef UXMODE_SUPPORT_LISTVIEW
#ifdef UXMODE_CUSTOMDRAW_LISTVIEW_GROUPS
#pragma warning( push )
#pragma warning( disable : 26454 )
        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, UxModeOnCustomDraw)
#pragma warning( pop )
#endif
#endif // UXMODE_SUPPORT_LISTVIEW
    END_MSG_MAP()

#ifdef UXMODE_SUPPORT_LISTVIEW

    static LRESULT CALLBACK UxModeSubclassedListViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        auto OldProc = ::GetProp(hWnd, UXPROP_LISTVIEWPROC);
        LRESULT result = 0L;
        switch (uMsg)
        {
        case WM_NOTIFY:
        {
            auto lpHdr = reinterpret_cast<LPNMHDR>(lParam);
            if (lpHdr->code == NM_CUSTOMDRAW && ListView_GetHeader(hWnd) == lpHdr->hwndFrom)
            {
                LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)lParam;
                switch (lpcd->dwDrawStage)
                {
                case CDDS_PREPAINT:
                    if (uxTheme.IsInDarkMode())
                        return CDRF_NOTIFYITEMDRAW;
                    break;
                case CDDS_ITEMPREPAINT:
                {
                    auto cr = uxTheme.GetSysColorValue(CUxTheme::UIColorType::Foreground);
                    if (CDIS_GRAYED == (CDIS_GRAYED & lpcd->uItemState) || CDIS_DISABLED == (CDIS_DISABLED & lpcd->uItemState))
                    {
                        cr = uxTheme.IsInDarkMode() ? UXCOLOR_DARKER(cr, 0.3) : ::GetSysColor(COLOR_GRAYTEXT);
                    }
                    SetTextColor(lpcd->hdc, cr);
                    return CDRF_NEWFONT;
                }
                break;
                }
            }
            break;
        }
        case WM_DESTROY:
            ::RemoveProp(hWnd, UXPROP_LISTVIEWPROC);
            if (OldProc)
                ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OldProc));
            break;
        }
        if (OldProc)
        {
            result = ::CallWindowProc(reinterpret_cast<WNDPROC>(OldProc), hWnd, uMsg, wParam, lParam);
        }
        return result;
    }
#endif // UXMODE_SUPPORT_LISTVIEW

public:
    static UxModeWindowType UxWindowType;

protected:

    virtual HWND GetOwnerHWND() override
    {
        return GetThis()->m_hWnd;
    }

    virtual T* GetThis() override
    {
        return static_cast<T*>(this);
    }

    virtual void UxModeSetup() override
    {
        ATLTRACE(_T(__FUNCTION__) _T(" WindowType=%d IsAppThemed=%d IsThemeActive=%d bgColor=%x\n"), UxWindowType, ::IsAppThemed(), ::IsThemeActive()
            , uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));

        auto isDark = uxTheme.IsInDarkMode();
        if (isDark && m_lastModeWasDark != isDark)
        {
            if (!m_brushBg.IsNull())
                m_brushBg.DeleteObject();
            m_brushBg.CreateSolidBrush(uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));
        }

        m_lastModeWasDark = isDark;

        auto pSelf = GetThis();
        if (!m_initialized)
        {
            uxTheme.AllowDarkModeForWindow(pSelf->m_hWnd, true);
            MENUBARINFO mbi = { sizeof(mbi) };
            m_hasMenuBar = ::GetMenuBarInfo(pSelf->m_hWnd, OBJID_MENU, 0, &mbi);
#ifdef UXMODE_SUPPORT_SCROLLBAR
            uxTheme.FixDarkScrollBar();
#endif //UXMODE_SUPPORT_SCROLLBAR
        }
        uxTheme.SwitchWindowDarkMode(pSelf->m_hWnd, isDark);

        CUxModeBase<T>::UxModeSetup();

        ::EnumChildWindows(pSelf->m_hWnd, [](HWND hwnd, LPARAM lParam) -> BOOL
            {
                auto pParent = reinterpret_cast<CUxModeWindow*>(lParam);
                CString str;
                ::GetClassName(hwnd, str.GetBufferSetLength(MAX_PATH), MAX_PATH);
                str.ReleaseBuffer();
                if (_T("Button") == str || _T("Edit") == str)
                {
                    pParent->SetUxModeForThemedControl(hwnd, L"Explorer");
                }
                else if (_T("ComboBox") == str)
                {
                    pParent->SetUxModeForThemedControl(hwnd, L"CFD");
                }
#ifdef UXMODE_SUPPORT_LISTVIEW
                else if (_T("SysListView32") == str)
                {
                    pParent->SetUxModeForListView(hwnd);
                }
#endif // UXMODE_SUPPORT_LISTVIEW
#ifdef UXMODE_SUPPORT_PROGRESSBAR
                else if (_T("msctls_progress32") == str)
                {
                    pParent->SetUxModeForProgressBar(hwnd);
                }
#endif // UXMODE_SUPPORT_PROGRESSBAR
#ifdef UXMODE_SUPPORT_TABCONTROL
                else if (_T("SysTabControl32") == str)
                {
                    pParent->SetUxModeForThemedControl(hwnd, L"DarkMode_Explorer");
                }
#endif // UXMODE_SUPPORT_TABCONTROL
                else
                {
                    ::SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
                }

                return TRUE;
            }, reinterpret_cast<LPARAM>(this));

        if (!m_initialized)
            pSelf->SendMessage(WM_THEMECHANGED);

        m_initialized = true;
    }

    bool SetUxModeForThemedControl(HWND hWnd, LPCWSTR strTheme, LPCWSTR strSublist = NULL, bool force = false) const
    {
        auto r = uxTheme.AllowDarkModeForWindow(hWnd, uxTheme.IsInDarkMode());
        auto hr = !force && m_initialized ? S_FALSE : ::SetWindowTheme(hWnd, strTheme, strSublist);
        if (S_FALSE == hr)
        {
            ::SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
        }
        return r && SUCCEEDED(hr);
    }

#ifdef UXMODE_SUPPORT_PROGRESSBAR
    bool SetUxModeForProgressBar(HWND hWnd)
    {
        HRESULT hr = S_OK;
        if (uxTheme.IsInDarkMode())
        {
            hr = ::SetWindowTheme(hWnd, L"", L"");
            ::SendMessage(hWnd, PBM_SETBKCOLOR, 0, UXCOLOR_LIGHTER(uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background), 0.18));
        }
        else {
            hr = ::SetWindowTheme(hWnd, VSCLASS_PROGRESS, NULL);
        }
        return SUCCEEDED(hr);
    }
#endif // UXMODE_SUPPORT_PROGRESSBAR

#ifdef UXMODE_SUPPORT_LISTVIEW
    bool SetUxModeForListView(HWND hWnd)
    {
        ListView_SetBkColor(hWnd, uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));
        ListView_SetTextBkColor(hWnd, uxTheme.GetSysColorValue(CUxTheme::UIColorType::Background));
        ListView_SetTextColor(hWnd, uxTheme.GetSysColorValue(CUxTheme::UIColorType::Foreground));
        auto isDark = uxTheme.IsInDarkMode();
        if (isDark)
            UxModeSubclassListView(hWnd);

        //auto result = SetUxModeForThemedControl(hWnd, isDark ? L"DarkMode_Explorer" : L"Explorer", NULL, true);
        auto result = SetUxModeForThemedControl(hWnd, isDark ? L"DarkMode_ItemsView" : L"ItemsView", NULL, true);
        SetUxModeForThemedControl(ListView_GetHeader(hWnd), isDark ? L"DarkMode_ItemsView" : L"ItemsView", L"Header", true);
        return result;
    }

    LONG_PTR UxModeSubclassListView(HWND hWnd) const
    {
        auto result = reinterpret_cast<LONG_PTR>(::GetProp(hWnd, UXPROP_LISTVIEWPROC));
        if (!result)
        {
            result = ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CUxModeWindow::UxModeSubclassedListViewProc));
            if (result)
            {
                ::SetProp(hWnd, UXPROP_LISTVIEWPROC, reinterpret_cast<HANDLE>(result));
            }
        }
        return result;
    }
#endif // UXMODE_SUPPORT_LISTVIEW

    bool UxModeDrawGroupBox(HWND hWnd, HDC hDC)
    {
        CWindow wnd(hWnd);
        CDCHandle dc(hDC);

        CRect rc;
        wnd.GetClientRect(rc);

        CRect rcFrame(rc);
        rcFrame.top += 10;
        auto crBorder = uxTheme.GetSysColorValue(CUxTheme::UIColorType::Foreground);
        crBorder = UXCOLOR_DARKER(crBorder, 0.6);
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

    int UxModeDrawMenuBar(UAHMENU* pUDM)
    {
        auto pSelf = GetThis();
        MENUBARINFO mbi = { sizeof(mbi) };
        if (::GetMenuBarInfo(pSelf->m_hWnd, OBJID_MENU, 0, &mbi))
        {
            CRect rcWindow;
            pSelf->GetWindowRect(rcWindow);

            CRect rc = mbi.rcBar;
            rc.OffsetRect(-rcWindow.left, -rcWindow.top);

            m_menuTheme.DrawThemeBackground(pUDM->hdc, MENU_POPUPBACKGROUND, 0, rc, NULL);
        }
        return 0;
    }

    BOOL UxModeDrawMenuBarSeparator()
    {
        auto pSelf = GetThis();
        CRect rcClient;
        pSelf->GetClientRect(rcClient);
        pSelf->MapWindowPoints(nullptr, rcClient);

        CRect rcWindow;
        pSelf->GetWindowRect(rcWindow);

        rcClient.OffsetRect(-rcWindow.left, -rcWindow.top);

        CRect rcLine = rcClient;
        rcLine.bottom = rcLine.top;
        rcLine.top--;

        CWindowDC dc(pSelf->m_hWnd);
        m_menuTheme.DrawThemeBackground(dc, MENU_POPUPBACKGROUND, 0, rcLine, NULL);
        return FALSE;
    }

    BOOL UxModeDrawMenuBarItem(UAHDRAWMENUITEM* pUDMI)
    {
        CDCHandle dc(pUDMI->um.hdc);

        auto isDisabled = ODS_DISABLED == (ODS_DISABLED & pUDMI->dis.itemState) || ODS_GRAYED == (ODS_GRAYED & pUDMI->dis.itemState);
        auto isHot = ODS_HOTLIGHT == (ODS_HOTLIGHT & pUDMI->dis.itemState) || ODS_FOCUS == (ODS_FOCUS & pUDMI->dis.itemState);
        auto itemState = MPI_NORMAL;
        //if (isHot)
        //    itemState = isDisabled ? MBI_DISABLEDHOT : MBI_HOT;
        if (isHot || ODS_SELECTED == (ODS_SELECTED & pUDMI->dis.itemState))
            itemState = isDisabled ? MPI_DISABLEDHOT : MPI_HOT;
        m_menuTheme.DrawThemeBackground(dc, MENU_POPUPITEM, itemState, &pUDMI->dis.rcItem, NULL);

        CString caption;
        CMenuItemInfo mii;
        mii.fMask = MIIM_STRING;
        mii.dwTypeData = caption.GetBufferSetLength(MAX_PATH);
        mii.cch = MAX_PATH;

        ::GetMenuItemInfo(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii);
        caption.ReleaseBuffer();

        DWORD dwFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
        if (ODS_NOACCEL == (ODS_NOACCEL & pUDMI->dis.itemState)) {
            dwFlags |= DT_HIDEPREFIX;
        }

        m_menuTheme.DrawThemeText(dc, MENU_POPUPITEM, itemState, caption, -1, dwFlags, 0, &pUDMI->dis.rcItem);
        return FALSE;
    }

#ifdef UXMODE_SUPPORT_LISTVIEW
#ifdef UXMODE_CUSTOMDRAW_LISTVIEW_GROUPS
    LRESULT UxModeCustomDrawListView(LPNMLVCUSTOMDRAW pNMLVCD)
    {
        LRESULT result = CDRF_DODEFAULT;
        switch (pNMLVCD->nmcd.dwDrawStage)
        {
        case CDDS_PREPAINT:
            //result = CDRF_NOTIFYITEMDRAW;
            if (LVCDI_GROUP == pNMLVCD->dwItemType)
            {
                LVGROUP group{sizeof(LVGROUP)};
                group.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE;
                group.stateMask = LVGS_FOCUSED | LVGS_SELECTED;
                ListView_GetGroupInfo(pNMLVCD->nmcd.hdr.hwndFrom, pNMLVCD->nmcd.dwItemSpec, &group);
                ATLTRACE(_T(__FUNCTION__) _T(" CDDS_PREPAINT hwnd=%p, groupId=%d, itemState=%d, groupState=%d, pszHeader=%s\n")
                    , pNMLVCD->nmcd.hdr.hwndFrom, pNMLVCD->nmcd.dwItemSpec, pNMLVCD->nmcd.uItemState, group.state, group.pszHeader);

                CRect rcHeader(pNMLVCD->rcText);

                auto isSelected = LVGS_FOCUSED ==(LVGS_FOCUSED & group.state) || LVGS_SELECTED == (LVGS_SELECTED & group.state); //CDIS_HOT == (CDIS_HOT & pNMLVCD->nmcd.uItemState) || CDIS_FOCUS == (CDIS_FOCUS & pNMLVCD->nmcd.uItemState) || CDIS_SELECTED == (CDIS_SELECTED & pNMLVCD->nmcd.uItemState);
                auto isDisabled = CDIS_DISABLED == (CDIS_DISABLED & pNMLVCD->nmcd.uItemState) || CDIS_GRAYED == (CDIS_GRAYED & pNMLVCD->nmcd.uItemState);

                pNMLVCD->clrText = uxTheme.GetSysColorValue(CUxTheme::UIColorType::AccentLight2);
                pNMLVCD->clrTextBk = ListView_GetBkColor(pNMLVCD->nmcd.hdr.hwndFrom);

                CDCHandle dc(pNMLVCD->nmcd.hdc);

                CPen pen;
                pen.CreatePen(PS_SOLID, 1, isSelected && !isDisabled ? m_menuColors.crHihghlight : pNMLVCD->clrTextBk);
                CBrush brush;
                brush.CreateSolidBrush(isSelected && !isDisabled ? m_menuColors.crHighlightBg : pNMLVCD->clrTextBk);
                auto oldPen = dc.SelectPen(pen);
                auto oldBrush = dc.SelectBrush(brush);
                
                dc.Rectangle(rcHeader);

                if (oldPen)
                    dc.SelectPen(oldPen);
                if (oldBrush)
                    dc.SelectBrush(oldBrush);


                CRect rcLabel;
                ListView_GetGroupRect(pNMLVCD->nmcd.hdr.hwndFrom, pNMLVCD->nmcd.dwItemSpec, LVGGR_LABEL, rcLabel);

                CRect rcLine;
                rcLine.left = rcHeader.left + rcLabel.Width() + (rcLabel.left - rcHeader.left) + 5;
                rcLine.right = rcHeader.right - (rcLabel.left - rcHeader.left);
                rcLine.top = rcHeader.top + (rcHeader.Height() / 2);
                rcLine.bottom = rcLine.top + 1;
                
                brush.DeleteObject();
                brush.CreateSolidBrush(isSelected ? UXCOLOR_DARKER(m_menuColors.crBorder, 0.5) : m_menuColors.crBorder);
                if (!brush.IsNull())
                {
                    dc.FillRect(rcLine, brush);
                }

                dc.SetTextColor(pNMLVCD->clrText);
                dc.SetBkColor(pNMLVCD->clrTextBk);
                dc.SetBkMode(TRANSPARENT);
                dc.DrawText(group.pszHeader, group.cchHeader, rcLabel, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

                DWORD dwFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
                DTTOPTS opts = { sizeof(opts), DTT_TEXTCOLOR, pNMLVCD->clrText };
                if (isDisabled)
                {
                    opts.crText = m_menuColors.crTextDisabled;
                }
                m_menuTheme.DrawThemeTextEx(dc, MENU_BARITEM, MBI_NORMAL, group.pszHeader, -1, dwFlags, rcLabel, &opts);
                //dc.ExcludeClipRect(rcHeader);
                result = CDRF_SKIPDEFAULT;
            }
            break;
        }
        return result;
    }
#endif
#endif // UXMODE_SUPPORT_LISTVIEW

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
        if (uxTheme.IsInDarkMode() && !m_brushBg.IsNull())
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

    LRESULT UxModeOnUahDrawMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        //ATLTRACE(_T(__FUNCTION__) _T("\n"));
        if (m_hasMenuBar && uxTheme.IsInDarkMode())
        {
            bHandled = TRUE;
            UxModeDrawMenuBar(reinterpret_cast<UAHMENU*>(lParam));
            return TRUE;
        }
        return 0L;
    }

    LRESULT UxModeOnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = TRUE;
        auto pSelf = GetThis();
        auto result = ::DefWindowProc(pSelf->m_hWnd, uMsg, wParam, lParam);
        if (m_hasMenuBar && uxTheme.IsInDarkMode())
        {
            MENUBARINFO mbi = { sizeof(mbi) };
            if (::GetMenuBarInfo(pSelf->m_hWnd, OBJID_MENU, 0, &mbi))
            {
                UxModeDrawMenuBarSeparator();
            }

        }
        return result;
    }

    LRESULT UxModeOnUahDrawMenuItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = FALSE;
        if (m_hasMenuBar && uxTheme.IsInDarkMode())
        {
            bHandled = TRUE;
            UxModeDrawMenuBarItem(reinterpret_cast<UAHDRAWMENUITEM*>(lParam));
            return TRUE;
        }
        return FALSE;
    }

#ifdef UXMODE_CUSTOMDRAW_LISTVIEW_GROUPS
    LRESULT UxModeOnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
    {
        bHandled = FALSE;
        if (uxTheme.IsInDarkMode())
        {
            CString str;
            ::GetClassName(pnmh->hwndFrom, str.GetBufferSetLength(MAX_PATH), MAX_PATH);
            str.ReleaseBuffer();
            ATLTRACE(_T(__FUNCTION__) _T(" class=%s\n"), (LPCTSTR)str);
            if (_T("SysListView32") == str)
            {
                auto result = UxModeCustomDrawListView(reinterpret_cast<LPNMLVCUSTOMDRAW>(pnmh));
                if (CDRF_DODEFAULT != result)
                    bHandled = TRUE;
                return result;
            }
        }
        return CDRF_DODEFAULT;
    }
#endif

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

protected:
    bool m_initialized{ false };
    bool m_hasMenuBar{ false };
    CBrush m_brushBg;
};

template<class T>
UxModeWindowType CUxModeWindow<T>::UxWindowType = (is_dialog<T>::value ? UxModeWindowType::DIALOG : (is_prop_sheet<T>::value ? UxModeWindowType::PROP_SHEET : UxModeWindowType::GENERIC));
