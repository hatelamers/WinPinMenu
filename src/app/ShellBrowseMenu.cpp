#include "stdafx.h"

#include "resource.h"

#include "ShellMgr.h"
#include "ShellBrowseMenu.h"

CShellBrowseMenu::CShellBrowseMenu(const ShellMenuController* controller)
	: m_controller(controller), m_isRendered(false), m_isCtxMenuShowing(false)
{
	LoadMenuImages();
}

HRESULT CShellBrowseMenu::Rebuild()
{
	ATLASSERT(m_controller);
	auto hrBind = E_INVALIDARG;
	if (!m_controller)
		return hrBind;

	CComPtr<IShellFolder> pFolder;
	if (!m_rootIDL.IsNull())
	{
		hrBind = ::SHBindToObject(NULL, m_rootIDL, NULL, IID_IShellFolder, (void**) &pFolder);
		ATLTRACE(_T(__FUNCTION__) _T(" hr=%p pFolder=%p\n"), hrBind, pFolder.p);
	}
	auto hrMnu = BuildFolderMenu(pFolder, m_mnuTop);
	return FAILED(hrBind) && SUCCEEDED(hrMnu) ? hrBind : hrMnu;
}

BOOL CShellBrowseMenu::InvokeWithSelection(LPCTSTR strVerb) const
{
	if (HasSelection())
	{
		SHELLEXECUTEINFO sei =
		{
			sizeof(SHELLEXECUTEINFO),
			SEE_MASK_INVOKEIDLIST,               // fMask
			NULL, //m_hwndOwner,   // hwnd of parent
			strVerb,                          // lpVerb
			NULL,                                // lpFile
			_T(""),
			_T(""),                              // lpDirectory
			SW_SHOWNORMAL,                       // nShow
			NULL, //_Module.GetModuleInstance(),         // hInstApp
			(LPVOID)NULL,                        // lpIDList...will set below
			NULL,                                // lpClass
			0,                                   // hkeyClass
			0,                                   // dwHotKey
			NULL                                 // hIcon
		};
		sei.lpIDList = CShellMgr::GetAbsoluteIDL(m_selection.parentFolder, m_selection.relativeIDL);
		return ::ShellExecuteEx(&sei);
	}
	return FALSE;
}

void CShellBrowseMenu::UxModeUpdateColorSettings()
{
	CUxModeMenuBase::UxModeUpdateColorSettings();
	if (!uxTheme.IsInDarkMode())
	{
		m_menuColors.crHighlightBg = UXCOLOR_LIGHTER(m_menuColors.crHighlightBg, 0.3);
	}
}

LRESULT CShellBrowseMenu::OnInitMenuPopup(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if (m_isCtxMenuShowing)
		return 1L;

	auto hMenu = (HMENU)wParam;
	ATLTRACE(_T(__FUNCTION__) _T("(%d, %p) openMenus=%d\n"), lParam, hMenu, m_openMenus.GetSize());
	if (m_mnuTop.m_hMenu == hMenu)
	{
		CComPtr<IShellFolder> pFolder;
		if (!m_rootIDL.IsNull())
		{
			auto hr = ::SHBindToObject(NULL, m_rootIDL, NULL, IID_IShellFolder, (void**)&pFolder);
			ATLTRACE(_T(__FUNCTION__) _T(" hr=%p pFolder=%p\n"), hr, pFolder.p);
		}
		BuildFolderMenu(pFolder, hMenu);

		//auto hMenuWnd = ::FindWindow(_T("#32768"), NULL);
		//::SetWindowTheme(hMenuWnd, L"DarkMode", L"Menu");
	}
	else
	{
		auto pos = LOWORD(lParam);
		CMenuHandle menuParent;
		if (0 < m_openMenus.GetSize())
		{
			if (hMenu == m_openMenus.GetCurrent())
			{
				if (1 < m_openMenus.GetSize())
					menuParent = m_openMenus[m_openMenus.GetSize() - 2];
			}
			else
			{
				menuParent = m_openMenus.GetCurrent();
			}
		}
		else
		{
			menuParent = m_mnuTop.m_hMenu;
		}

		if (menuParent.IsMenu())
		{
			CMenuItemInfo mii;
			mii.fMask = MIIM_DATA;
			if (menuParent.GetMenuItemInfo(pos, TRUE, &mii) && mii.dwItemData)
			{
				auto pData = (LPSHELLITEMDATA)mii.dwItemData;
				//CComPtr<IShellFolder> pFolder;
				//auto hr = ::SHBindToObject(pData->parentFolder, pData->relativeIDL, NULL, IID_IShellFolder, (void**)&pFolder);
				//ATLTRACE(_T(__FUNCTION__) _T(" hr=%p pFolder=%p\n"), hr, pFolder.p);
				BuildFolderMenu(pData->thisFolder, hMenu);
				m_openMenus.Push(hMenu);
			}
		}
	}
	bHandled = TRUE;
	return 1L;
}

LRESULT CShellBrowseMenu::OnUninitMenuPopup(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
	bHandled = FALSE;
	if (m_isCtxMenuShowing)
		return 1L;

	ATLTRACE(_T(__FUNCTION__) _T("(%p) hTop=%p\n"), wParam, m_mnuTop.m_hMenu);
	CleanUpMenuData((HMENU)wParam);
	auto cStack = m_openMenus.GetSize();
	if (cStack && m_openMenus[cStack - 1] == (HMENU) wParam)
		m_openMenus.Pop();
	bHandled = (HMENU) wParam != m_mnuTop;
	return 1;
}

LRESULT CShellBrowseMenu::OnMenuSelect(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if (m_isCtxMenuShowing)
		return 1L;

	auto flags = HIWORD(wParam);
	// Check if a menu is closing, do a cleanup
	if (flags == 0xFFFF && lParam == NULL)   // Menu closing
	{
		ATLTRACE(_T(__FUNCTION__) _T(" closing menu, openMenus=%d\n"), m_openMenus.GetSize());
		if (0 == m_openMenus.GetSize())
		{
			CleanUpMenuData(m_mnuTop);
		}
		else
		{
			HMENU hMenu = NULL;
			while (NULL != (hMenu = m_openMenus.Pop()))
			{
				CleanUpMenuData(hMenu);
			}
		}
	}
	else if (MF_HILITE == (MF_HILITE & flags))
	{
		m_folderSelection.Reset();
		m_selection.Reset();

		auto item = LOWORD(wParam);
		auto hMenu = (HMENU)lParam;
		CMenuItemInfo mii;
		mii.fMask = MIIM_DATA;
		auto isPopup = MF_POPUP == (MF_POPUP & flags);
		ATLTRACE(_T(__FUNCTION__) _T(" hMenu=%p item=%d isPopup=%d\n"), hMenu, item, isPopup);
		if (::GetMenuItemInfo(hMenu, item, isPopup, &mii) && mii.dwItemData)
		{
			auto pData = (LPSHELLITEMDATA)mii.dwItemData;
			if (pData->parentFolder && !pData->relativeIDL.IsNull())
			{
				ShellItemSelection& selObject = isPopup ? m_folderSelection : m_selection;

				selObject.hMenu = hMenu;
				selObject.byPosition = isPopup;
				selObject.menuItem = item;

				selObject.parentFolder = pData->parentFolder.p;
				selObject.parentFolder.p->AddRef();
				selObject.relativeIDL.CopyFrom(pData->relativeIDL);

				if (isPopup)
				{
					::GetMenuItemRect(m_controller->GetHWnd(), hMenu, item, &m_folderSelection.itemRect);
				}
				bHandled = TRUE;
			}
		}
	}
	return 1;
}

LRESULT CShellBrowseMenu::OnMenuRButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	CMenuItemInfo mii;
	mii.fMask = MIIM_DATA;
	if (::GetMenuItemInfo((HMENU)lParam, (int)wParam, TRUE, &mii) && mii.dwItemData)
	{
		ATLTRACE(_T(__FUNCTION__) _T(" index=%d, pData=%p hMenu=%p\n"), wParam, mii.dwItemData, (HMENU)lParam);
		CPoint pt;
		if (!::GetCursorPos(&pt))
		{
			pt.x = 100;
			pt.y = 100;
		}
		LPSHELLITEMDATA pData = (LPSHELLITEMDATA) mii.dwItemData;
		m_isCtxMenuShowing = true;
		auto menuFlags = CMF_VERBSONLY | CMF_NODEFAULT;
		if (HIWORD(::GetKeyState(VK_SHIFT)))
			menuFlags |= CMF_EXTENDEDVERBS;
		CShellMgr::DoContextMenu(m_controller->GetHWnd(), pData->parentFolder, pData->relativeIDL, pt, TPM_LEFTALIGN | TPM_RECURSE, menuFlags);
		m_isCtxMenuShowing = false;
		bHandled = TRUE;
	}
	return 0;
}

LRESULT CShellBrowseMenu::OnRButtonUp(UINT, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if (m_folderSelection.parentFolder && !m_folderSelection.relativeIDL.IsNull())
	{
		ATLTRACE(_T(__FUNCTION__) _T("\n"));
		CPoint pt(lParam);
		if (pt.x >= m_folderSelection.itemRect.left && pt.x <= m_folderSelection.itemRect.right
			&& pt.y >= m_folderSelection.itemRect.top && pt.y <= m_folderSelection.itemRect.bottom)
		{
			::PostMessage(m_controller->GetHWnd(), WM_MENURBUTTONUP, m_folderSelection.menuItem, (LPARAM)m_folderSelection.hMenu);
			//CShellMgr::DoContextMenu(m_hwndOwner, m_folderSelection.parentFolder, m_folderSelection.relativeIDL, pt, TPM_LEFTALIGN | TPM_RECURSE, CMF_VERBSONLY);
			bHandled = TRUE;
		}
	}
	return 0;
}

LRESULT CShellBrowseMenu::OnMenuCommand(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	auto hMenu = (HMENU) lParam;
	if (hMenu != m_mnuTop.m_hMenu || (int)wParam > m_controller->GetShellMenuAnchor())
	{
		ATLTRACE(_T(__FUNCTION__) _T(" hMenu=%p id=%d sel.hMenu=%p sel.byPos=%d sel.item=%d\n")
			, hMenu, wParam, m_selection.hMenu, m_selection.byPosition, m_selection.menuItem);

		if (hMenu == m_selection.hMenu)
		{
			CMenuHandle menuParent = hMenu;
			ATLASSERT(menuParent.IsMenu());
			if (m_selection.menuItem == wParam)
			{
				InvokeWithSelection();
			}
		}

		bHandled = TRUE;
	}
	return 1L;
}

LRESULT CShellBrowseMenu::OnDrawItem(UINT, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	LPDRAWITEMSTRUCT lpDis = (LPDRAWITEMSTRUCT)lParam;
	if (NULL == lpDis || ODT_MENU != lpDis->CtlType || NULL == lpDis->itemData)
	{
		return FALSE;
	}

	bHandled = TRUE;
	return CustomDrawPopupMenuItem(lpDis);
}

LRESULT CShellBrowseMenu::OnMeasureItem(UINT, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	LPMEASUREITEMSTRUCT lpMis = (LPMEASUREITEMSTRUCT)lParam;
	if (NULL == lpMis || ODT_MENU != lpMis->CtlType || NULL == lpMis->itemData)
	{
		return FALSE;
	}

	bHandled = TRUE;
	auto pData = reinterpret_cast<LPSHELLMENUITEMDATA>(lpMis->itemData);
	return MeasurePopupMenuItem(lpMis, pData ? (LPCTSTR)pData->caption : NULL);
}

BOOL CShellBrowseMenu::CustomDrawPopupMenuItem(LPDRAWITEMSTRUCT lpDis)
{
	CustomDrawPopupMenuBackground(lpDis);
	auto itemState = CustomDrawPopupMenuItemBackground(lpDis);

	CRect rc(lpDis->rcItem);
	if (m_menuMetrics.sizeIcon.cx)
	{
		rc.left += m_menuMetrics.sizeIcon.cx + (m_menuMetrics.paddingIcon.cx * 2);
	}

	auto pData = (LPSHELLMENUITEMDATA)lpDis->itemData;
	CDCHandle dc = lpDis->hDC;

	if (NULL == pData || 0 == pData->caption.GetLength())
	{
		m_menuTheme.DrawThemeBackground(dc, MENU_POPUPSEPARATOR, 0, rc, NULL);
	}
	else
	{
		auto isDisabled = ODS_GRAYED == (lpDis->itemState & ODS_GRAYED);

		if (-1 < pData->iconIndex && m_pImageList)
		{
			IMAGELISTDRAWPARAMS ildp;
			::ZeroMemory(&ildp, sizeof(IMAGELISTDRAWPARAMS));
			ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
			ildp.himl = (HIMAGELIST)m_pImageList.p;
			ildp.i = pData->iconIndex;
			ildp.hdcDst = lpDis->hDC;
			ildp.x = lpDis->rcItem.left + m_menuMetrics.paddingIcon.cx;
			ildp.y = lpDis->rcItem.top + m_menuMetrics.paddingIcon.cy;
			ildp.rgbBk = CLR_NONE;
			ildp.rgbFg = CLR_DEFAULT;
			ildp.fStyle = ILD_TRANSPARENT;

			m_pImageList->Draw(&ildp);
		}

		dc.SetBkMode(TRANSPARENT);

		rc.left += m_menuMetrics.paddingText.cx;
		DWORD dwFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
		if (ODS_NOACCEL == (ODS_NOACCEL & lpDis->itemState)) {
			dwFlags |= DT_HIDEPREFIX;
		}
		m_menuTheme.DrawThemeText(dc, MENU_POPUPITEM, itemState, pData->caption, -1, dwFlags, 0, rc);

		if (::IsMenu((HMENU)(UINT_PTR)lpDis->itemID))
		{
			CustomDrawMenuArrow(dc, &lpDis->rcItem, isDisabled);
		}

		dc.ExcludeClipRect(&lpDis->rcItem);
	}
	return TRUE;
}

HRESULT CShellBrowseMenu::LoadMenuImages()
{
	auto result = S_FALSE;
	if (!m_pImageList)
	{
		result = ::SHGetImageList(SHIL_SMALL, IID_IImageList, (void**)&m_pImageList);
		if (SUCCEEDED(result) && m_pImageList)
		{
			m_pImageList->GetIconSize((int*)&m_menuMetrics.sizeIcon.cx, (int*)&m_menuMetrics.sizeIcon.cy);
			m_pImageList->GetIconSize((int*)&m_menuMetrics.sizeIcon.cx, (int*)&m_menuMetrics.sizeIcon.cy);
		}
	}
	if (S_FALSE == result)
		m_menuMetrics.sizeIcon.cx = m_menuMetrics.sizeIcon.cy = 16;
	return result;
}

BOOL CShellBrowseMenu::SetupMenuInfo(CMenuHandle& menu)
{
	if (menu.IsMenu())
	{
		MENUINFO mi;
		::ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_STYLE;
		mi.dwStyle = MNS_CHECKORBMP;
		UxModeUpdateMenuInfo(mi);
		return menu.SetMenuInfo(&mi);
	}
	return FALSE;
}

HRESULT CShellBrowseMenu::BuildFolderMenu(LPSHELLFOLDER pFolder, HMENU hMenu)
{
	ATLTRACE(_T(__FUNCTION__) _T(" pFolder=%p hMenu=%p\n"), pFolder, hMenu);
	CWaitCursor wait;

	auto hr = S_OK;
	ATLASSERT(hMenu);

	CMenuHandle menuPopup(hMenu);
	auto isTopMenu = hMenu == m_mnuTop.m_hMenu;

	if (isTopMenu)
		m_isRendered = true;

	SetupMenuInfo(menuPopup);
	CleanUpMenuData(hMenu);
	for (auto i = menuPopup.GetMenuItemCount() - 1; i > (isTopMenu ? m_controller->GetShellMenuAnchor() : -1); i--)
	{
		menuPopup.DeleteMenu((UINT)i, MF_BYPOSITION);
	}

	WORD id = ID_ACTION_FIRST + (WORD)(m_openMenus.GetSize() * 0x1000);
	ATLASSERT((int)id == ID_ACTION_FIRST + (m_openMenus.GetSize() * 0x1000));
	UINT pos = isTopMenu ? m_controller->GetShellMenuAnchor() + 1 : 0;
	auto idMsg = MessageID::SBM_NONE;

	if (pFolder)
	{
		TCHAR szBuff[MAX_PATH];
		szBuff[0] = _T('\0');

		CShellItemIDList idlParent;
		CComQIPtr<IPersistFolder2> persFolder(pFolder);
		if (!persFolder || FAILED(persFolder->GetCurFolder(&idlParent)))
		{
			ATLTRACE2(_T(__FUNCTION__) _T(" failed to retrieve folder IDL\n"));
		}

		CShellItemIDList idlChild;
		ULONG ulFetched = 0;

		auto enumObjectsWorker = [&](SHCONTF itemType)
			{
				CComPtr<IEnumIDList> spIDList;
				hr = pFolder->EnumObjects(m_controller->GetHWnd(), itemType, &spIDList);
				if (SUCCEEDED(hr) && spIDList)
				{
					while (S_OK == (hr = spIDList->Next(1, &idlChild, &ulFetched)))
					{
						// we cannot assign menu IDs (WORD) for subelements which would produce overflow
						auto isFolder = SHCONTF_FOLDERS == (SHCONTF_FOLDERS & itemType) && USHRT_MAX > id + 0x1000;
						if (CShellMgr::GetNameOf(pFolder, idlChild, szBuff))
						{
							// use within global enumeration
							//if (isFolder)
							//{
							//	ULONG ulAttrs = SFGAO_FOLDER;
							//	if (SUCCEEDED(pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&idlChild, &ulAttrs)) && !(ulAttrs & SFGAO_FOLDER))
							//		isFolder = false;
							//}
							CComPtr<IShellFolder> pSelf = NULL;
							if (isFolder)
							{
								if (FAILED(::SHBindToObject(pFolder, idlChild, NULL, IID_IShellFolder, (void**)&pSelf)) || !pSelf)
								{
									isFolder = false;
								}
							}
							auto succ = isFolder
								? menuPopup.AppendMenu(MF_STRING | MF_POPUP, ::CreatePopupMenu(), szBuff)
								: menuPopup.AppendMenu(MF_STRING, id, szBuff);
							if (succ)
							{
								auto pData = new ShellMenuItemData;

								if (!idlParent.IsNull())
								{
									CShellItemIDList absIDL(CShellMgr::ConcatIDLs(idlParent, idlChild));
									pData->iconIndex = CShellMgr::GetIconIndex(absIDL, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
								}
								pData->caption = szBuff;
								pData->parentFolder.p = pFolder;
								pFolder->AddRef();
								pData->relativeIDL.CopyFrom(idlChild);
								if (pSelf)
								{
									pData->thisFolder.p = pSelf;
									pSelf.p->AddRef();
								}

								//CMenuItemInfo mii;
								//mii.fMask = MIIM_DATA | MIIM_BITMAP;
								//mii.hbmpItem = HBMMENU_CALLBACK;
								//mii.dwItemData = (LONG_PTR)pData;

								CMenuItemInfo mii;
								mii.cch = ::lstrlen(szBuff);
								mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
								mii.dwTypeData = szBuff;

								menuPopup.GetMenuItemInfo(pos, TRUE, &mii);

								mii.fType |= MFT_OWNERDRAW;
								mii.dwItemData = (LONG_PTR)pData;

								succ = menuPopup.SetMenuItemInfo(pos, TRUE, &mii);
								ATLTRACE(_T(__FUNCTION__) _T(" itemType=%d, id=%d, pos=%d '%s': %d\n"), itemType, id, pos, szBuff, succ);

								id++;
								pos++;
							}
						}
					}
				}
			};
		enumObjectsWorker(SHCONTF_FOLDERS);
		enumObjectsWorker(SHCONTF_NONFOLDERS);

		if (pos == (isTopMenu ? m_controller->GetShellMenuAnchor() + 1 : 0U))
		{
			idMsg = MessageID::SBM_EMPTY_FOLDER;
		}
	}
	else
	{
		idMsg = MessageID::SBM_INVALID_SOURCE;
	}

	if (idMsg)
	{
		ATLTRACE(_T(__FUNCTION__) _T(" hMenu=%p idMsg=%d\n"), hMenu, idMsg);
		CString strMsg;
		if (!m_controller->GetMessageString(idMsg, strMsg))
		{
			strMsg = _T("<error>");
		}
		menuPopup.AppendMenu(MF_STRING, id, strMsg);
		menuPopup.EnableMenuItem(id, MF_BYCOMMAND | MF_DISABLED);
	}
	else if (isTopMenu)
	{
		for (auto i = 0; i <= m_controller->GetShellMenuAnchor(); i++)
		{
			auto pData = new ShellMenuItemData;

			CMenuItemInfo mii;
			mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;

			menuPopup.GetMenuItemInfo(i, TRUE, &mii);
			if (MFT_SEPARATOR != (mii.fType & MFT_SEPARATOR))
			{
				::GetMenuString(menuPopup, i, pData->caption.GetBufferSetLength(MAX_PATH), MAX_PATH, MF_BYPOSITION);
				pData->caption.ReleaseBuffer();
			}

			mii.fType |= MFT_OWNERDRAW;
			mii.dwItemData = (LONG_PTR)pData;
			menuPopup.SetMenuItemInfo(i, TRUE, &mii);
		}
	}
	
	return hr;
}

void CShellBrowseMenu::CleanUpMenuData(HMENU hMenu)
{
	ATLTRACE(_T(__FUNCTION__) _T("(%p)\n"), hMenu);
	CMenuHandle menuPopup = hMenu;
	ATLASSERT(menuPopup.IsMenu());

	BOOL bRet = FALSE;
	for (int i = 0; i < menuPopup.GetMenuItemCount(); i++)
	{
		CMenuItemInfo mii;
		mii.fMask = MIIM_DATA;
		bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
		ATLASSERT(bRet);

		LPSHELLITEMDATA pData = (LPSHELLITEMDATA)mii.dwItemData;
		if (pData)
		{
			mii.dwItemData = NULL;
			bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
			ATLASSERT(bRet);

			delete pData;
		}
	}

}
