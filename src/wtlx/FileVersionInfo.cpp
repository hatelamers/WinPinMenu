/************************************************************************
 * $Revision: 4 $
 * $Date: 2016-11-25 01:28:53 +0100 (Fri, 25 Nov 2016) $
 ************************************************************************/
/************************************************************************
 * File: FileVersionInfo.cpp
 * Copyright: 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#include "pch.h"
#include <strsafe.h>
#include "wtlx/FileVersionInfo.h"

CFileVersionInfo::CFileVersionInfo()
    : m_lpData(NULL)
    , m_pTranslations(NULL)
    , m_uTranslCount(0)
    , m_pFFI(NULL)
    , m_iTranslThread(-1)
{
}


CFileVersionInfo::~CFileVersionInfo()
{
    Close();
}

void CFileVersionInfo::Close()
{
    if (m_lpData) delete [] m_lpData;
    m_lpData = NULL;
    m_pTranslations = NULL;
    m_uTranslCount = 0;
    m_pFFI = NULL;
    m_iTranslThread = -1;
}

BOOL CFileVersionInfo::Open(HMODULE hModule)
{
    TCHAR szPath[MAX_PATH];
    DWORD dwRes = ::GetModuleFileName(hModule, szPath, MAX_PATH);
    if (0 == dwRes) return FALSE;
    return Open(szPath);
}


BOOL CFileVersionInfo::Open(LPCTSTR lpszFileName)
{
    Close();
    DWORD dwHandle(0);
    DWORD dwSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (0 == dwSize) return FALSE;

    m_lpData = new BYTE[dwSize];
    if (!m_lpData) return FALSE;

    if (!::GetFileVersionInfo(lpszFileName, dwHandle, dwSize, m_lpData) || ::GetLastError())
        throw FALSE;

    LPVOID lpBuff(NULL);
    UINT uLen(0);
    if (::VerQueryValue(m_lpData, _T("\\"), &lpBuff, &uLen) && 0 < uLen)
    {
        ATLASSERT(sizeof(VS_FIXEDFILEINFO) == uLen);
        m_pFFI = (VS_FIXEDFILEINFO*) lpBuff;
    }

    if (::VerQueryValue(m_lpData, _T("\\VarFileInfo\\Translation"), &lpBuff, &uLen) && 0 < uLen)
    {
        m_uTranslCount = uLen / sizeof(TranslationID);
        m_pTranslations = (TranslationID*) lpBuff;
    }
    return TRUE;
}

DWORD CFileVersionInfo::GetFileFlagsMask()
{
    return m_pFFI->dwFileFlagsMask;
}

DWORD CFileVersionInfo::GetFileFlags()
{
    return m_pFFI->dwFileFlags;
}

DWORD CFileVersionInfo::GetFileOs()
{
    return m_pFFI->dwFileOS;
}

DWORD CFileVersionInfo::GetFileType()
{
    return m_pFFI->dwFileType;
}

DWORD CFileVersionInfo::GetFileSubtype()
{
    return m_pFFI->dwFileSubtype;
}

BOOL CFileVersionInfo::GetFileDate(LPFILETIME lpResult)
{
    if (NULL == lpResult || NULL == m_pFFI) return FALSE;
    lpResult->dwLowDateTime = m_pFFI->dwFileDateLS;
    lpResult->dwHighDateTime = m_pFFI->dwFileDateMS;
    return TRUE;
}

WORD CFileVersionInfo::GetFileVersionDigit(VerDigit vn)
{
    if (NULL == m_pFFI) return 0;
    switch (vn)
    {
    case VerDigit::VN_MAJOR:
        return (WORD)((0xFFFF0000 & m_pFFI->dwFileVersionMS) >> 16);
    case VerDigit::VN_MINOR:
        return (WORD)(0x0000FFFF & m_pFFI->dwFileVersionMS);
    case VerDigit::VN_PATCH:
        return (WORD)((0xFFFF0000 & m_pFFI->dwFileVersionLS) >> 16);
    case VerDigit::VN_BUILD:
        return (WORD)(0x0000FFFF & m_pFFI->dwFileVersionLS);
    default:
        return 0;
    }
}

WORD CFileVersionInfo::GetProductVersionDigit(VerDigit vn)
{
    if (NULL == m_pFFI) return 0;
    switch (vn)
    {
    case VerDigit::VN_MAJOR:
        return (WORD)((0xFFFF0000 & m_pFFI->dwProductVersionMS) >> 16);
    case VerDigit::VN_MINOR:
        return (WORD)(0x0000FFFF & m_pFFI->dwProductVersionMS);
    case VerDigit::VN_PATCH:
        return (WORD)((0xFFFF0000 & m_pFFI->dwProductVersionLS) >> 16);
    case VerDigit::VN_BUILD:
        return (WORD)(0x0000FFFF & m_pFFI->dwProductVersionLS);
    default:
        return 0;
    }
}

ULONGLONG CFileVersionInfo::GetFileVersion(WORD* pwMajor, WORD* pwMinor, WORD* pwPatch, WORD* pwBuild)
{
    if (NULL == m_pFFI) return 0ULL;
    ATLASSERT(NULL != pwMajor);
    *pwMajor = GetFileVersionDigit(VerDigit::VN_MAJOR);
    if (NULL != pwMinor) *pwMinor = GetFileVersionDigit(VerDigit::VN_MINOR);
    if (NULL != pwPatch) *pwPatch = GetFileVersionDigit(VerDigit::VN_PATCH);
    if (NULL != pwBuild) *pwBuild = GetFileVersionDigit(VerDigit::VN_BUILD);
    return (((ULONGLONG) m_pFFI->dwFileVersionLS) | ((ULONGLONG) m_pFFI->dwFileVersionMS << 32));
}

ULONGLONG CFileVersionInfo::GetProductVersion(WORD* pwMajor, WORD* pwMinor, WORD* pwPatch, WORD* pwBuild)
{
    if (NULL == m_pFFI) return 0ULL;
    ATLASSERT(NULL != pwMajor);
    *pwMajor = GetProductVersionDigit(VerDigit::VN_MAJOR);
    if (NULL != pwMinor) *pwMinor = GetProductVersionDigit(VerDigit::VN_MINOR);
    if (NULL != pwPatch) *pwPatch = GetProductVersionDigit(VerDigit::VN_PATCH);
    if (NULL != pwBuild) *pwBuild = GetProductVersionDigit(VerDigit::VN_BUILD);
    return (((ULONGLONG) m_pFFI->dwProductVersionLS) | ((ULONGLONG) m_pFFI->dwProductVersionMS << 32));
}


UINT CFileVersionInfo::GetTranslationCount()
{
    return m_uTranslCount;
}


DWORD CFileVersionInfo::GetTranslationID(UINT uIndex, WORD* pwLangID, WORD* pwCodePage)
{
    if (uIndex >= m_uTranslCount || NULL == m_pTranslations) return 0;
    if (NULL != pwLangID) *pwLangID = m_pTranslations[uIndex].wLanguage;
    if (NULL != pwCodePage) *pwCodePage = m_pTranslations[uIndex].wCodePage;
    return (DWORD) MAKELONG(m_pTranslations[uIndex].wLanguage, m_pTranslations[uIndex].wCodePage);
}


INT CFileVersionInfo::LookupTranslation(LANGID wLangID)
{
    if (0 == m_uTranslCount || NULL == m_pTranslations) return -1;
    if (MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) == wLangID) wLangID = ::GetThreadUILanguage();
    BOOL fThreadDefault = (wLangID == ::GetThreadUILanguage());
    if (fThreadDefault && -1 < m_iTranslThread) return m_iTranslThread;

    USHORT uPLang = PRIMARYLANGID(wLangID);
    INT iRes = -1;
    WORD wLastPrim = USHRT_MAX;
    WORD wLastNeut = USHRT_MAX;
    for (UINT i = 0; i < m_uTranslCount; i++)
    {
        if (m_pTranslations[i].wLanguage == wLangID)
            return (INT) i;

        USHORT uPrim = PRIMARYLANGID(m_pTranslations[i].wLanguage);
        if (uPLang == uPrim)
        {
            if (wLastPrim > m_pTranslations[i].wLanguage)
            {
                wLastPrim = m_pTranslations[i].wLanguage;
                iRes = i;
            }
        }
        else if (wLastPrim < USHRT_MAX && (LANG_SYSTEM_DEFAULT >= uPrim || LANG_INVARIANT == uPrim))
        {
            if (wLastNeut > m_pTranslations[i].wLanguage)
            {
                wLastNeut = m_pTranslations[i].wLanguage;
                iRes = i;
            }
        }
    }
    if (0 > iRes && LANG_ENGLISH != uPLang) iRes = LookupTranslation(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    if (fThreadDefault) m_iTranslThread = iRes;
    return iRes;
}

BOOL CFileVersionInfo::GetStringFileInfo(LPCTSTR lpszInfoField, LPTSTR& lpValue, PUINT pcValue, LANGID wLangID)
{
    if (NULL == m_lpData || NULL == m_pTranslations) return FALSE;
    INT iTransl = LookupTranslation(wLangID);
    if (0 > iTransl) return FALSE;
    
    TCHAR szBlockName[60];
    HRESULT hr = ::StringCchPrintf(szBlockName, 60, _T("\\StringFileInfo\\%04x%04x\\%s")
        , m_pTranslations[iTransl].wLanguage, m_pTranslations[iTransl].wCodePage, lpszInfoField);
    if (FAILED(hr))
    {
        ::SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }
    
    LPVOID lpBuffer(NULL);
    UINT uLen(0);
    BOOL result = ::VerQueryValue(m_lpData, szBlockName, &lpBuffer, &uLen);
    if (result && (0 == *pcValue || uLen <= *pcValue))
    {
        if (0 == *pcValue) {
            lpValue = (LPTSTR)lpBuffer;
        }
        else
        {
            if (!::lstrcpyn(lpValue, (LPCTSTR)lpBuffer, uLen))
                result = FALSE;
        }
    }
    else {
        result = FALSE;
    }
    *pcValue = uLen;
    return result;
}

BOOL CFileVersionInfo::SetInfoDlgItemText(HWND hwndDlg, int iDlgItem, LPCTSTR lpszInfoField, LANGID wLangID)
{
    if (!::IsWindow(hwndDlg)) return FALSE;
    LPTSTR lpValue(NULL);
    UINT uLen(0);
    BOOL result = GetStringFileInfo(lpszInfoField, lpValue, &uLen, wLangID);
    if (result) {
        result = ::SetDlgItemText(hwndDlg, iDlgItem, lpValue);
    }
    return result;
}
