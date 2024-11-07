// shellmgr.cpp

#include "stdafx.h"
#include <atlctrls.h>
#include <atlctrlx.h>

#include "ShellMgr.h"

#ifndef SHELLMGR_CTXCMD_OFFSET
#define SHELLMGR_CTXCMD_OFFSET 0x2000
#endif // !SHELLMGR_CTXCMD_OFFSET

#ifndef SHELLMGR_CTXCMD_LAST
#define SHELLMGR_CTXCMD_LAST 0x7FFF
#endif // !SHELLMGR_CTXCMD_LAST

#ifdef _DEBUG
LONG CShellItemIDList::s_cInstances = 0;
#endif // _DEBUG


int CShellMgr::GetIconIndex(LPITEMIDLIST lpi, UINT uFlags)
{
	SHFILEINFO sfi = { 0 };
	DWORD_PTR dwRet = ::SHGetFileInfo((LPCTSTR)lpi, 0, &sfi, sizeof(SHFILEINFO), uFlags);
	return (dwRet != 0) ? sfi.iIcon : -1;
}

void CShellMgr::GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTVITEM lptvitem)
{
	int nRet = lptvitem->iImage = GetIconIndex(lpifq, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ATLASSERT(nRet >= 0);
	nRet = lptvitem->iSelectedImage = GetIconIndex(lpifq, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON);
	ATLASSERT(nRet >= 0);
}

LPITEMIDLIST CShellMgr::ConcatIDLs(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	UINT cb1 = 0;
	if (pidl1 != NULL)   // May be NULL
		cb1 = GetSizeOf(pidl1) - sizeof(pidl1->mkid.cb);

	UINT cb2 = GetSizeOf(pidl2);

	LPITEMIDLIST pidlNew = (LPITEMIDLIST)::CoTaskMemAlloc(cb1 + cb2);
	if (pidlNew != NULL)
	{
		if (pidl1 != NULL)
			::memcpy(pidlNew, pidl1, cb1);

		::memcpy(((LPSTR)pidlNew) + cb1, pidl2, cb2);
	}

	return pidlNew;
}

BOOL CShellMgr::GetLocalizedFileName(LPCWSTR lpstrPath, LPWSTR lpstrResult, int cMaxLen)
{
	BOOL result = FALSE;
	int idRes = 0;
	CString strResource;
	if (SUCCEEDED(::SHGetLocalizedName(lpstrPath, strResource.GetBufferSetLength(MAX_PATH), MAX_PATH, &idRes)))
	{
		strResource.ReleaseBuffer();
		auto hMod = ::LoadLibraryEx(strResource, NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
		if (hMod)
		{
			result = 0 < ::LoadStringW(hMod, idRes, lpstrResult, cMaxLen);
			::FreeLibrary(hMod);
		}
	}
	else
	{
		result = 0 < ::lstrcpynW(lpstrResult, ::PathFindFileNameW(lpstrPath), cMaxLen);
	}
	return result;
}

BOOL CShellMgr::GetDisplayNameOf(LPCITEMIDLIST plAbsoluteIDL, LPWSTR lpstrResult, SIGDN sigdnName, int cMaxLen)
{
	BOOL result = TRUE;
	CComPtr<IShellItem> pItem;
	auto hr = ::SHCreateItemFromIDList(plAbsoluteIDL, IID_IShellItem, (void**)&pItem);
	result = SUCCEEDED(hr) && pItem;
	if (result)
	{
		CComHeapPtr<WCHAR> pstr;
		result = SUCCEEDED(pItem->GetDisplayName(sigdnName, &pstr));
		if (result)
		{
			ATLTRACE(_T(__FUNCTION__) _T("(%p)=%s\n"), plAbsoluteIDL, pstr.m_pData);
			result = 0 < ::lstrcpynW(lpstrResult, pstr, cMaxLen);
		}
	}
	return result;
}

BOOL CShellMgr::GetNameOf(LPSHELLFOLDER lpParentFolder, LPCITEMIDLIST lpcChildIDL, LPTSTR lpFriendlyName, DWORD dwFlags, int cMaxLen)
{
	BOOL bSuccess = TRUE;
	auto localVars = false;
	LPITEMIDLIST lpChildIDL = const_cast<LPITEMIDLIST>(lpcChildIDL);
	if (NULL == lpParentFolder)
	{
		CComPtr<IShellItem> pItem;
		if (SUCCEEDED(::SHCreateItemFromIDList(lpChildIDL, IID_IShellItem, (void**)&pItem))
			&& SUCCEEDED(CComQIPtr<IParentAndItem>(pItem)->GetParentAndItem(NULL, &lpParentFolder, &lpChildIDL)))
		{
			localVars = NULL != lpParentFolder || NULL != lpChildIDL;
		}
	}

	STRRET str = { STRRET_CSTR };
	if (lpParentFolder && lpcChildIDL && SUCCEEDED(lpParentFolder->GetDisplayNameOf(lpChildIDL, dwFlags, &str)))
	{
		if (FAILED(::StrRetToBuf(&str, lpChildIDL, lpFriendlyName, cMaxLen)))
			bSuccess = FALSE;
	}
	else
	{
		bSuccess = FALSE;
	}
	if (localVars)
	{
		if (lpParentFolder)
			lpParentFolder->Release();
		if (lpChildIDL)
			::CoTaskMemFree(lpChildIDL);
	}

	return bSuccess;
}

LPITEMIDLIST CShellMgr::NextIDL(LPCITEMIDLIST pidl)
{
	LPSTR lpMem = (LPSTR)pidl;
	lpMem += pidl->mkid.cb;
	return (LPITEMIDLIST)lpMem;
}

UINT CShellMgr::GetSizeOf(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl != NULL)
	{
		cbTotal += sizeof(pidl->mkid.cb);   // Null terminator
		while (pidl->mkid.cb != NULL)
		{
			cbTotal += pidl->mkid.cb;
			pidl = NextIDL(pidl);
		}
	}

	return cbTotal;
}

LPITEMIDLIST CShellMgr::ParseShellItemName(LPCWSTR lpstrName)
{
	LPITEMIDLIST result = NULL;
	auto hr = ::SHParseDisplayName(lpstrName, NULL, &result, 0, NULL);
	ATLTRACE(_T(__FUNCTION__) _T("(%s) hr=%p, lpidl=%p\n"), lpstrName, hr, result);
	return result;
}

LPITEMIDLIST CShellMgr::GetAbsoluteIDL(LPSHELLFOLDER lpParentFolder, LPCITEMIDLIST lpChildIDL)
{
	WCHAR szBuff[MAX_PATH] = { 0 };

	if (!GetNameOf(lpParentFolder, lpChildIDL, szBuff, SHGDN_FORPARSING))
		return NULL;

	CComPtr<IShellFolder> spDeskTop;
	HRESULT hr = ::SHGetDesktopFolder(&spDeskTop);
	if (FAILED(hr))
		return NULL;

	LPITEMIDLIST result = NULL;
	ULONG ulAttribs = 0;
	USES_CONVERSION;
	hr = spDeskTop->ParseDisplayName(NULL, NULL, T2W(szBuff), NULL, &result, &ulAttribs);

	if (FAILED(hr))
		return NULL;

	return result;
}

INT CShellMgr::DoContextMenu(HWND hWnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpChildIDL, POINT point, UINT popupFlags, UINT menuFlags)
{
	CComPtr<IContextMenu> spContextMenu;
	HRESULT hr = lpsfParent->GetUIObjectOf(hWnd, 1, (const struct _ITEMIDLIST**)&lpChildIDL, IID_IContextMenu, 0, (LPVOID*)&spContextMenu);
	if(FAILED(hr))
		return 0;

	CMenuHandle mnu;
	mnu.CreatePopupMenu();
	if(!mnu.IsMenu())
		return 0;

	// Get the context menu for the item.
	hr = spContextMenu->QueryContextMenu(mnu, 0, SHELLMGR_CTXCMD_OFFSET, SHELLMGR_CTXCMD_LAST, menuFlags);
	if(FAILED(hr))
		return 0;

	INT idCmd = ::TrackPopupMenu(mnu, popupFlags | TPM_RETURNCMD, point.x, point.y, 0, hWnd, NULL);

	if (idCmd != 0)
	{
		// Execute the command that was selected.
		CMINVOKECOMMANDINFO cmi = { 0 };
		cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
		cmi.fMask = CMIC_MASK_NOASYNC;
		cmi.hwnd = hWnd;
		cmi.lpVerb = MAKEINTRESOURCEA(idCmd - SHELLMGR_CTXCMD_OFFSET);
		cmi.lpParameters = NULL;
		cmi.lpDirectory = NULL;
		cmi.nShow = SW_SHOWNORMAL;
		cmi.dwHotKey = 0;
		cmi.hIcon = NULL;

		hr = spContextMenu->InvokeCommand(&cmi);
		ATLTRACE(_T(__FUNCTION__) _T(" hWnd=%p idCmd=%d, hr=%p\n"), hWnd, idCmd, hr);
	}

	return idCmd;
}
