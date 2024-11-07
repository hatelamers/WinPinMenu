// shellmgr.h

#ifndef __SHELLMGR_H__
#define __SHELLMGR_H__

#pragma once

#include <shlobj.h>
#include <atlctrls.h>

// "shell:::" links: https://www.elevenforum.com/t/list-of-windows-11-clsid-key-guid-shortcuts.1075/
// "shell:" commands: https://pureinfotech.com/windows-11-shell-commands-list

class CShellItemIDList
{
public:
	LPITEMIDLIST m_pidl;

	CShellItemIDList(LPITEMIDLIST pidl = NULL) : m_pidl(pidl)
	{
		//ATLTRACE(_T(__FUNCTION__) _T("(%p)\n"), pidl);
#ifdef _DEBUG
		::InterlockedIncrement(&s_cInstances);
#endif // _DEBUG

	}

	CShellItemIDList(LPCITEMIDLIST pidl)
	{
#ifdef _DEBUG
		::InterlockedIncrement(&s_cInstances);
#endif // _DEBUG

		CopyFrom(pidl);
	}

	~CShellItemIDList()
	{
		//ATLTRACE(_T(__FUNCTION__) _T("(%p)\n"), m_pidl);
#ifdef _DEBUG
		::InterlockedDecrement(&s_cInstances);
#endif // _DEBUG
		::CoTaskMemFree(m_pidl);
	}

	void Attach(LPITEMIDLIST pidl)
	{
		//ATLTRACE(_T(__FUNCTION__) _T("(%p) m_pidl=%p\n"), pidl, m_pidl);
		if (m_pidl)
			::CoTaskMemFree(m_pidl);
		m_pidl = pidl;
	}

	LPITEMIDLIST Detach()
	{
		LPITEMIDLIST pidl = m_pidl;
		m_pidl = NULL;
		return pidl;
	}

	bool IsNull() const
	{
		return (m_pidl == NULL);
	}

	CShellItemIDList& operator =(LPITEMIDLIST pidl)
	{
		Attach(pidl);
		return *this;
	}

	CShellItemIDList& operator =(LPCITEMIDLIST pidl)
	{
		CopyFrom(pidl);
		return *this;
	}

	LPITEMIDLIST* operator &()
	{
		return &m_pidl;
	}

	operator LPITEMIDLIST()
	{
		return m_pidl;
	}

	operator LPCITEMIDLIST() const
	{
		return m_pidl;
	}

	operator LPCTSTR()
	{
		return (LPCTSTR)m_pidl;
	}

	operator LPTSTR()
	{
		return (LPTSTR)m_pidl;
	}

	void CreateEmpty(UINT cbSize)
	{
		if (m_pidl)
			::CoTaskMemFree(m_pidl);
		m_pidl = (LPITEMIDLIST)::CoTaskMemAlloc(cbSize);
		ATLASSERT(m_pidl != NULL);
		if(m_pidl != NULL)
			memset(m_pidl, 0, cbSize);
	}

	bool CopyFrom(LPCITEMIDLIST pidl)
	{
		//ATLTRACE(_T(__FUNCTION__) _T("(%p)\n"), pidl);
		ATLASSERT(pidl);
		if (m_pidl)
			::CoTaskMemFree(m_pidl);
		if (NULL == pidl)
		{
			m_pidl = NULL;
		}
		else
		{
			m_pidl = (LPITEMIDLIST)::CoTaskMemAlloc(pidl->mkid.cb + sizeof(pidl->mkid.cb));
			if (m_pidl)
				::CopyMemory((PVOID)m_pidl, (CONST VOID*)pidl, pidl->mkid.cb + sizeof(pidl->mkid.cb));
		}
		return NULL != m_pidl;
	}
#ifdef _DEBUG
	static LONG s_cInstances;
#endif // _DEBUG

};


typedef struct ShellItemData
{
	CComPtr<IShellFolder> parentFolder;
	CComPtr<IShellFolder> thisFolder;
	CShellItemIDList relativeIDL;
	virtual ~ShellItemData() = default;
} SHELLITEMDATA, *LPSHELLITEMDATA;


class CShellMgr
{
public:
	static int GetIconIndex(LPITEMIDLIST lpi, UINT uFlags);

	static void GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTVITEM lptvitem);

	static LPITEMIDLIST ConcatIDLs(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

	static BOOL GetLocalizedFileName(LPCWSTR lpstrPath, LPWSTR lpstrResult, int cMaxLen = MAX_PATH);
	static BOOL GetDisplayNameOf(LPCITEMIDLIST plAbsoluteIDL, LPWSTR lpstrResult, SIGDN sigdnName = SIGDN::SIGDN_NORMALDISPLAY, int cMaxLen = MAX_PATH);
	static BOOL GetNameOf(LPSHELLFOLDER lpParentFolder, LPCITEMIDLIST lpcChildIDL, LPTSTR lpFriendlyName, DWORD dwFlags = SHGDN_NORMAL, int cMaxLen = MAX_PATH);
	static LPITEMIDLIST NextIDL(LPCITEMIDLIST pidl);
	static UINT GetSizeOf(LPCITEMIDLIST pidl);

	static LPITEMIDLIST ParseShellItemName(LPCWSTR lpstrName);
	static LPITEMIDLIST GetAbsoluteIDL(LPSHELLFOLDER lpParentFolder, LPCITEMIDLIST lpChildIDL);

	static INT DoContextMenu(HWND hwnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpChildIDL, POINT point, UINT popupFlags = TPM_LEFTALIGN | TPM_RIGHTBUTTON, UINT menuFlags = CMF_NORMAL);
};

#endif //__SHELLMGR_H__
