/************************************************************************
 * $Revision: 4 $
 * $Date: 2016-11-25 01:28:53 +0100 (Fri, 25 Nov 2016) $
 ************************************************************************/
/************************************************************************
 * File: FileVersionInfo.h
 * Copyright: 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#pragma once

#pragma comment(lib,"Version.lib")

#define SFI_COMPANYNAME         _T("CompanyName")
#define SFI_FILEDESCRIPTION     _T("FileDescription")
#define SFI_FILEVERSION         _T("FileVersion")
#define SFI_INTERNALNAME        _T("InternalName")
#define SFI_LEGALCOPYRIGHT      _T("LegalCopyright")
#define SFI_ORIGINALFILENAME    _T("OriginalFileName")
#define SFI_PRODUCTNAME         _T("ProductName")
#define SFI_PRODUCTVERSION      _T("ProductVersion")
#define SFI_COMMENTS            _T("Comments")
#define SFI_LEGALTRADEMARKS     _T("LegalTrademarks")
#define SFI_PRIVATEBUILD        _T("PrivateBuild")
#define SFI_SPECIALBUILD        _T("SpecialBuild")

class CFileVersionInfo
{
public:
    enum VerDigit
    {
        VN_MAJOR
        , VN_MINOR
        , VN_PATCH
        , VN_BUILD
    };
    struct TranslationID
    {
        WORD wLanguage;
        WORD wCodePage;
    };
private:
    LPBYTE m_lpData;
    VS_FIXEDFILEINFO* m_pFFI;
    TranslationID* m_pTranslations;
    UINT m_uTranslCount;
    INT m_iTranslThread;

public:

    CFileVersionInfo();
    virtual ~CFileVersionInfo();

    BOOL Open(HMODULE hModule = NULL);
    BOOL Open(LPCTSTR lpszFileName);
    void Close();
    DWORD GetFileFlagsMask();
    DWORD GetFileFlags();
    DWORD GetFileOs();
    DWORD GetFileType();
    DWORD GetFileSubtype();
    BOOL GetFileDate(LPFILETIME lpResult);
    WORD GetFileVersionDigit(VerDigit vn);
    WORD GetProductVersionDigit(VerDigit vn);
    ULONGLONG GetFileVersion(WORD* pwMajor, WORD* pwMinor = NULL, WORD* pwPatch = NULL, WORD* pwBuild = NULL);
    ULONGLONG GetProductVersion(WORD* pwMajor, WORD* pwMinor = NULL, WORD* pwPatch = NULL, WORD* pwBuild = NULL);
    UINT GetTranslationCount();
    DWORD GetTranslationID(UINT uIndex, WORD* pwLangID = NULL, WORD* pwCodePage = NULL);
    INT LookupTranslation(LANGID wLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    BOOL GetStringFileInfo(LPCTSTR lpszInfoField, LPTSTR& lpValue, PUINT pcValue, LANGID wLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    BOOL SetInfoDlgItemText(HWND hwndDlg, int iDlgItem, LPCTSTR lpszInfoField, LANGID wLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
};

