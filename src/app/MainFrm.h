// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShellBrowseMenu.h"

#define MAINFRM_OWNED_MENUS 2
#define MNUPOS_APP_ABOUT 1
#define MNUPOS_APP_EXIT 0

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>,
	public CUxModeWindow<CMainFrame>,
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler, public CShellBrowseMenu::ShellMenuController
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)


	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	virtual BOOL OnIdle() override;

	virtual HWND GetHWnd() const override
	{
		return m_hWnd;
	}

	virtual HMENU GetTopHMenu() const override
	{
		return m_mnuMain.IsMenu() ? m_mnuMain.GetSubMenu(0) : NULL;
	}

	virtual int GetShellMenuAnchor() const override
	{
		return MNUPOS_APP_ABOUT + 1;
	}

	virtual int GetMessageString(CShellBrowseMenu::MessageID msgID, CString& msg) const override;

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		CHAIN_MSG_MAP(CUxModeWindow<CMainFrame>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MENUCOMMAND, OnMenuCommand)
		MESSAGE_HANDLER(GetDisplayPopupMessage(), OnDisplayPopup)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP_MEMBER(m_shellMenu)
		MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	static UINT GetDisplayPopupMessage();

private:

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnMenuCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMenuSelect(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDisplayPopup(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	bool ParseCommandLine();
	HRESULT SetupActionSource();
	INT TriggerActionPopup();
	POINT CalculatePopupPosision();

private:
	CMenuHandle m_mnuMain;
	CShellBrowseMenu m_shellMenu;
	CString m_strActionSource;
	CString m_strSourceTitle;
	INT m_mnuItemSelected{-1};
	CIcon m_icoRunning;
	CString m_strMenuMessages[2]{ {}, {} };
};
