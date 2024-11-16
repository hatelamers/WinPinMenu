// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrm.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	return FALSE;
}

int CMainFrame::GetMessageString(CShellBrowseMenu::MessageID msgID, CString& msg) const
{
	msg = m_strMenuMessages[msgID - 1];
	return msg.GetLength();
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	std::ignore = m_strMenuMessages[CShellBrowseMenu::MessageID::SBM_EMPTY_FOLDER - 1].LoadString(IDS_EMPTY_FOLDER);
	std::ignore = m_strMenuMessages[CShellBrowseMenu::MessageID::SBM_INVALID_SOURCE - 1].LoadString(IDS_INVALID_SOURCE);

	m_mnuMain.Attach(GetMenu());
	m_shellMenu.Bind(this);

	ParseCommandLine();
	SetupActionSource();

	m_icoRunning.LoadIcon(IDI_APP_RUNNING);
	SetIcon(m_icoRunning);
	SetIcon(m_icoRunning, FALSE);
	PostMessage(GetDisplayPopupMessage());
	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	PostQuitMessage(0);
	return 1;
}


LRESULT CMainFrame::OnMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if ((HMENU)lParam == m_mnuMain.GetSubMenu(0) && MAINFRM_OWNED_MENUS > wParam)
	{
		ATLTRACE(_T(__FUNCTION__) _T(" ind=%d hMenu=%p\n"), wParam, lParam);
		UINT id = 0;
		switch (wParam)
		{
		case MNUPOS_APP_EXIT:
			id = ID_APP_EXIT;
			break;
		case MNUPOS_APP_ABOUT:
			id = ID_APP_ABOUT;
			break;
		}
		if (id)
			SendMessage(WM_COMMAND, MAKEWPARAM(id, 0), 0);
		bHandled = TRUE;
	}
	return 1L;
}

LRESULT CMainFrame::OnMenuSelect(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;

	auto flags = HIWORD(wParam);
	ATLTRACE(_T(__FUNCTION__) _T(" flags=%x item=%d hmenu=%p\n"), flags, LOWORD(wParam), lParam);
	if (flags == 0xFFFF && lParam == NULL)   // Menu closing
	{
		//PostMessage(WM_CLOSE);
	}
	else if (MF_HILITE == (MF_HILITE & flags))
	{
		m_mnuItemSelected = -1;
		auto item = LOWORD(wParam);
		if (MF_POPUP != (MF_POPUP & flags) && (HMENU)lParam == m_shellMenu.GetTopMenu()
			&& (ID_APP_ABOUT == item || ID_APP_EXIT == item))
		{
			m_mnuItemSelected = item;
			bHandled = TRUE;
		}
	}
	return 1;
}


LRESULT CMainFrame::OnDisplayPopup(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	TriggerActionPopup();
	bHandled = TRUE;
	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	if (IDRETRY == dlg.DoModal())
	{
		TriggerActionPopup();
	}
	else
	{
		PostMessage(WM_CLOSE);
	}
	return 0;
}

bool CMainFrame::ParseCommandLine()
{
	int argc = 0;
	auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
	auto expectToken = 0;
	for (auto i = 1; argv && i < argc; i++)
	{
		CString p(argv[i]);
		if (_T("/t") == p && 0 == m_strSourceTitle.GetLength())
		{
			expectToken = 1;
		}
		else if (1 == expectToken)
		{
			if (0 == m_strSourceTitle.GetLength())
			{
				m_strSourceTitle = p;
			}
			expectToken = 0;
		}
		else
		{
			m_strActionSource = p;
		}
	}
	if (argv)
		::LocalFree(argv);

	if (0 < m_strActionSource.GetLength())
	{
		CString strRes;
		if (0 < ::ExpandEnvironmentStrings(m_strActionSource, strRes.GetBufferSetLength(MAX_PATH), MAX_PATH))
		{
			m_strActionSource = strRes;
		}
		strRes.ReleaseBuffer();
	}
	return 0 < m_strActionSource.GetLength();
}

HRESULT CMainFrame::SetupActionSource()
{
	if (0 == m_strActionSource.GetLength())
		return E_FAIL;

	auto result = m_shellMenu.SetRoot(CShellMgr::ParseShellItemName(m_strActionSource));

	if (0 == m_strSourceTitle.GetLength())
	{
		auto& idl = m_shellMenu.GetRootIDL();
		if (!idl.IsNull())
		{
			CShellMgr::GetDisplayNameOf(idl, m_strSourceTitle.GetBufferSetLength(MAX_PATH));
			m_strSourceTitle.ReleaseBuffer();
		}
	}
	if (m_strSourceTitle.GetLength())
	{
		CString strTitle;
		if (strTitle.LoadString(IDR_MAINFRAME))
		{
			strTitle.Format(_T("%s - %s"), (LPCTSTR)m_strSourceTitle, (LPCTSTR)strTitle);
			SetWindowText(strTitle);
		}
	}
	return result;
}

INT CMainFrame::TriggerActionPopup()
{
	auto pt = CalculatePopupPosision();

	auto popupMenu = m_shellMenu.GetTopMenu();
	auto sel = popupMenu.TrackPopupMenu(TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	ATLTRACE(_T(__FUNCTION__) _T(" sel=%d shellMenu.hasSelection=%d mnuItemSelected=%d\n"), sel, m_shellMenu.HasSelection(), m_mnuItemSelected);
	if (sel && m_shellMenu.HasSelection())
	{
		if (m_shellMenu.InvokeWithSelection())
		{
			PostMessage(WM_CLOSE);
		}
	}
	else if (-1 != m_mnuItemSelected && sel == m_mnuItemSelected)
	{
		SendMessage(WM_COMMAND, MAKEWPARAM(m_mnuItemSelected, 0));
	}
	else
	{
		PostMessage(WM_CLOSE);
	}
	return sel;
}

POINT CMainFrame::CalculatePopupPosision()
{
	ATLTRACE(__FUNCTION__ _T(" START\n"));
	POINT result{ -1,-1 };
	POINT cursorPos{ 0,0 };
	if (::GetCursorPos(&cursorPos))
	{
		result.x = cursorPos.x;
		result.y = cursorPos.y;
	}

	CTaskbarAutomation tba;
	auto hrButtons = tba.ForEveryButton([this,&result,&cursorPos](const CComPtr<IUIAutomationElement>& button, int /*tbIndex*/, int /*btnIndex*/)
		{
			RECT rc{ 0,0,0,0 };
			if (SUCCEEDED(button->get_CachedBoundingRectangle(&rc)))
			{
				auto hasFocus = FALSE;
				button->get_CurrentHasKeyboardFocus(&hasFocus);
				if (!hasFocus)
				{
					hasFocus = (cursorPos.x >= rc.left && cursorPos.x <= rc.right && cursorPos.y >= rc.top && cursorPos.y <= rc.bottom);
				}
				ATLTRACE(__FUNCTION__ _T(" taskbar button at x=%d y=%d, hasFocus=%d\n"), rc.left, rc.top, hasFocus);
				if (hasFocus)
				{
					result.x = 10 > rc.left ? rc.right : rc.left;
					result.y = 10 > rc.top ? rc.bottom + 2 : rc.top - 2;
					return false;
				}
			}
			return true;
		});

	if (S_FALSE != hrButtons)
	{
		if (-1 == result.x)
		{
			result.x = 100;
			result.y = -1;
		}

		auto hTaskBar = ::FindWindow(_T("Shell_TrayWnd"), NULL);
		if (hTaskBar)
		{
			RECT rc{ 0,0,0,0 };
			if (::GetWindowRect(hTaskBar, &rc) &&
				(0 > result.y
					|| (cursorPos.x >= rc.left && cursorPos.x <= rc.right && cursorPos.y >= rc.top && cursorPos.y <= rc.bottom)))
			{
				result.y = 10 > rc.top ? rc.bottom + 2 : rc.top - 2;
			}
		}
	}
	ATLTRACE(__FUNCTION__ _T(" END\n"));
	return result;
}

UINT CMainFrame::GetDisplayPopupMessage()
{
	static UINT displayPopupMessage = 0U;
	if (0 == displayPopupMessage)
	{
		CStaticDataInitCriticalSectionLock lock;
		if (FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CMainFrame::GetDisplayPopupMessage.\n"));
			ATLASSERT(FALSE);
			return 0;
		}

		if (0 == displayPopupMessage)
			displayPopupMessage = ::RegisterWindowMessage(_T("WinPinMenu_InternalDiplayPopupMsg"));

		lock.Unlock();
	}
	ATLASSERT(displayPopupMessage != 0);
	return displayPopupMessage;
}
