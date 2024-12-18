#pragma once

#include "ShellMgr.h"

#ifndef ID_ACTION_FIRST
#define ID_ACTION_FIRST  10
#endif

class CShellBrowseMenu
	: public CUxModeMenuBase<CShellBrowseMenu>
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

	CShellBrowseMenu(const ShellMenuController* controller = NULL);

	virtual ~CShellBrowseMenu() = default;

	BEGIN_MSG_MAP(CShellBrowseMenu)
		CHAIN_MSG_MAP(CUxModeMenuBase<CShellBrowseMenu>)
		MESSAGE_HANDLER(WM_MENURBUTTONUP, OnMenuRButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_MENUCOMMAND, OnMenuCommand)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
		MESSAGE_HANDLER(WM_UNINITMENUPOPUP, OnUninitMenuPopup)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
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
		auto init = NULL == m_controller;
		m_controller = controller;
		m_mnuTop.Attach(m_controller ? m_controller->GetTopHMenu() : NULL);
		UxModeSetup(init);
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

protected:
	virtual void UxModeUpdateColorSettings() override;

	virtual HWND GetOwnerHWND() override
	{
		return m_controller ? m_controller->GetHWnd() : NULL;
	}
	HRESULT LoadMenuImages();

private:
	LRESULT OnInitMenuPopup(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnUninitMenuPopup(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMenuRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BOOL CustomDrawPopupMenuItem(LPDRAWITEMSTRUCT lpDis);

	BOOL SetupMenuInfo(CMenuHandle& menu);
	HRESULT BuildFolderMenu(LPSHELLFOLDER pFolder, HMENU hMenu);
	void CleanUpMenuData(HMENU hMenu);

private:
	const ShellMenuController* m_controller;
	CMenuHandle m_mnuTop;
	bool m_isRendered;
	bool m_isCtxMenuShowing;
	CShellItemIDList m_rootIDL;
	CSimpleStack<HMENU> m_openMenus;
	CComPtr<IImageList> m_pImageList;
	ShellItemSelection m_selection;
	FolderSelection m_folderSelection;
};

