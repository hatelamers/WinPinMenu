#pragma once

#include "ShellMgr.h"

#ifndef ID_ACTION_FIRST
#define ID_ACTION_FIRST  10
#endif

class CShellBrowseMenu
{
public:
	enum MessageID
	{
		SBM_NONE, SBM_EMPTY_FOLDER, SBM_INVALID_SOURCE
	};

	struct ShellMenuController
	{
		virtual HWND GetHWnd() const = 0;
		virtual HMENU GetTopHMenu() const = 0;
		virtual int GetShellMenuAnchor() const = 0;
		virtual int GetMessageString(MessageID msgID, CString& msg) const = 0;
	};

	typedef struct ShellMenuItemData
		: public ShellItemData
	{
		int iconIndex{ -1 };
		CString caption;
		virtual ~ShellMenuItemData() = default;
	} SHELLMENUITEMDATA, * LPSHELLMENUITEMDATA;

	struct ShellItemSelection
		: public ShellItemData
	{
		HMENU hMenu{NULL};
		UINT menuItem{(UINT) -1};
		bool byPosition{ false };
		virtual ~ShellItemSelection() = default;
		void Reset()
		{
			hMenu = NULL;
			menuItem = (UINT)-1;
			byPosition = false;
			parentFolder.Release();
			relativeIDL.Attach(NULL);
		}
	};

	struct FolderSelection
		: public ShellItemSelection
	{
		CRect itemRect;
		virtual ~FolderSelection() = default;
	};

	struct MenuMetrics
	{
		COLORREF crText{RGB(0,0,0)};
		COLORREF crTextDisabled{ RGB(128,128,128) };
		COLORREF crBg{ RGB(255,255,255) };
		COLORREF crHihghlight{ RGB(200,200,200) };
		COLORREF crHighlightBg{ RGB(200,200,200) };
		COLORREF crBorder{ RGB(200,200,200) };
		CSize sizeIcon{ 0, 0 };
		CSize sizeMnuArrow{ 16, 16 };
		CSize paddingText{ 5, 5 };
		CSize paddingIcon{ 2, 2 };
		int itemHeight{-1};
		NONCLIENTMETRICS metricsNC{ 0 };
		CBrush brushText;
		CBrush brushTextDisabled;
		CBrush brushBg;
		CFont fontMnu;
	};

	CShellBrowseMenu(const ShellMenuController* controller = NULL);

	virtual ~CShellBrowseMenu() = default;

	BEGIN_MSG_MAP(CShellBrowseMenu)
		MESSAGE_HANDLER(WM_MENURBUTTONUP, OnMenuRButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_MENUCOMMAND, OnMenuCommand)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_UNINITMENUPOPUP, OnUninitMenuPopup)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChange)
	END_MSG_MAP()

	const CShellItemIDList& GetRootIDL() const
	{
		return m_rootIDL;
	}

	bool HasSelection() const
	{
		return m_selection.parentFolder && !m_selection.relativeIDL.IsNull();
	}

	CMenuHandle& GetTopMenu()
	{
		return m_mnuTop;
	}

	void Bind(const ShellMenuController* controller)
	{
		ATLASSERT(controller);
		m_controller = controller;
		m_mnuTop.Attach(m_controller ? m_controller->GetTopHMenu() : NULL);
		SetupMenuInfo(m_mnuTop);
		if (!m_rootIDL.IsNull())
		{
			Rebuild();
		}
	}

	HRESULT SetRoot(LPITEMIDLIST lpRootIDL)
	{
		auto rebuild = m_isRendered && lpRootIDL != m_rootIDL;
		m_rootIDL = lpRootIDL;
		return rebuild ? Rebuild() : (m_rootIDL.IsNull() ? S_FALSE : S_OK);
	}

	HRESULT Rebuild();
	BOOL InvokeWithSelection(LPCTSTR strVerb = _T("Open")) const;


private:
	LRESULT OnThemeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnInitMenuPopup(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnUninitMenuPopup(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMenuRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BOOL CustomDrawMenuItem(LPDRAWITEMSTRUCT lpDis);
	BOOL CustomDrawMenuArrow(HDC hdcItem, LPRECT rcItem, bool isDisabled);
	BOOL MeasureMenuItem(LPMEASUREITEMSTRUCT lpMis);

	HRESULT LoadIconImages();
	void UpdateMetrics(bool colorsOnly = false);
	BOOL SetupMenuInfo(CMenuHandle& menu);
	HRESULT BuildFolderMenu(LPSHELLFOLDER pFolder, HMENU hMenu);
	void CleanUpMenuData(HMENU hMenu);

	static inline bool IsInDarkMode()
	{
		return uxTheme.ShouldAppsUseDarkMode() && !uxTheme.IsHighContrast();
	}

private:
	const ShellMenuController* m_controller;
	CMenuHandle m_mnuTop;
	bool m_isRendered;
	bool m_isCtxMenuShowing;
	MenuMetrics m_metrics;
	CShellItemIDList m_rootIDL;
	CSimpleStack<HMENU> m_openMenus;
	CComPtr<IImageList> m_pImageList;
	ShellItemSelection m_selection;
	FolderSelection m_folderSelection;
};

