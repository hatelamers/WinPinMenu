/************************************************************************
 * $Revision: 33 $
 * $Date: 2024-10-28 17:50:24 +0100 (Mon, 28 Oct 2024) $
 ************************************************************************/
/************************************************************************
 * File: formattools.cpp
 * Copyright: 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#include "pch.h"
#include "wtlx/formattools.h"
#include <comdef.h>
#include <comutil.h>
#include <lmerr.h>


static const _bstr_t MU_ABBREVIATIONS[] = {
    _bstr_t(L"B"),
    _bstr_t(L"KiB"),
    _bstr_t(L"MiB"),
    _bstr_t(L"GiB"),
    _bstr_t(L"TiB")
};

STDAPI FormatMemoryUnits(_In_ const DECIMAL *pdecIn, _In_ LCID lcid, _Out_ BSTR *pbstrOut)
{
    DECIMAL din = *pdecIn;
    DECIMAL dres = *pdecIn;
    BSTR lpUnit = MU_ABBREVIATIONS[MU_BYTES];
    DECIMAL ddiv;
    HRESULT hr = ::VarDecFromInt(1024, &ddiv);
    for (int i = MU_KB; SUCCEEDED(hr) && i < sizeof(MU_ABBREVIATIONS) / sizeof(_bstr_t); i++)
    {
        hr = ::VarDecCmp(&dres, &ddiv);
        if (VARCMP_LT < hr)
        {
            lpUnit = MU_ABBREVIATIONS[i];
            hr = ::VarDecDiv(&din, &ddiv, &dres);
            din = dres;
        }
        else {
            break;
        }
    }
    if (FAILED(hr)) return hr;

    hr = ::VarDecRound(&din, 2, &dres);
    if (FAILED(hr)) return hr;

    _bstr_t bstr;
    hr = ::VarBstrFromDec(&dres, lcid, 0, bstr.GetAddress());
    if (SUCCEEDED(hr))
    {
        bstr += L" ";
        bstr += lpUnit;
        *pbstrOut = bstr.copy();
    }
    return hr;
}

STDAPI FormatError(_In_ HRESULT hr, _In_ LCID lcid, _Out_ BSTR *pbstrOut)
{
    HRESULT result = S_OK;
    HINSTANCE hInst = NULL;
    if (FACILITY_MSMQ == HRESULT_FACILITY(hr)) {
        hInst = ::LoadLibrary(_T("MQUTIL.DLL"));
    }
    else if (FACILITY_WINDOWSUPDATE == HRESULT_FACILITY(hr)) {
        //TODO: is WUAPI.DLL right for error messages?
        hInst = ::LoadLibrary(_T("WUAPI.DLL"));
    }
    else if (NERR_BASE <= HRESULT_CODE(hr) && MAX_NERR >= HRESULT_CODE(hr)) {
        hInst = ::LoadLibrary(_T("NETMSG.DLL"));
    }

    HLOCAL pBuffer = NULL;
    DWORD dwRes = ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER
        | (hInst ? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM)
        | FORMAT_MESSAGE_IGNORE_INSERTS
        , hInst, (DWORD)hr, lcid, (LPTSTR) &pBuffer, 1024, NULL
        );

    _bstr_t bstr;
    if (0 == dwRes)
    {
        result = HRESULT_FROM_WIN32(::GetLastError());

        dwRes = ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS
            , NULL, (DWORD)E_FAIL, lcid, (LPTSTR)&pBuffer, 1024, NULL
            );

        _bstr_t bstrDesc;
        if (dwRes && pBuffer)
        {
            bstrDesc = (LPTSTR)pBuffer;
            ::LocalFree(pBuffer);
        }
        else {
            bstrDesc = _T("Unknown error");
        }

        pBuffer = ::LocalAlloc(0, 64 * sizeof(TCHAR));
        if (pBuffer)
        {
            ZeroMemory(pBuffer, 64 * sizeof(TCHAR));
            ::_stprintf_s((LPTSTR)pBuffer, 64, _T("%s 0x%0lX"), bstrDesc.GetBSTR(), hr);
        }
    }
    if (pBuffer) bstr = (LPTSTR)pBuffer;
    *pbstrOut = bstr.copy();

    if (pBuffer) ::LocalFree(pBuffer);
    if (hInst) ::FreeLibrary(hInst);
    return result;
}

STDAPI FormatSysDateTime(_In_ const LPSYSTEMTIME pST, _In_ LCID lcid, _In_ DWORD dwDateFlags, _In_ DWORD dwTimeFlags, _Out_ BSTR *pbstrOut)
{
    HRESULT hr = S_OK;
    if (NULL == pbstrOut || NULL == pST) return E_POINTER;
    if (DTFORMAT_NODATE == dwDateFlags && DTFORMAT_NOTIME == dwTimeFlags) return E_INVALIDARG;

    _bstr_t bstr;
    int iRes = 0;
    LPSYSTEMTIME lpST = pST;
    if (DTFORMAT_LOCALTIME & dwTimeFlags)
    {
        dwTimeFlags &= ~DTFORMAT_LOCALTIME;

        TIME_ZONE_INFORMATION tzi{ 0 };
        if (TIME_ZONE_ID_INVALID == ::GetTimeZoneInformation(&tzi)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        }
        else
        {
            lpST = new SYSTEMTIME{ 0 };
            if (!::SystemTimeToTzSpecificLocalTime(&tzi, pST, lpST)) {
                hr = ATL::AtlHresultFromLastError();
            }
        }

    }
    if (SUCCEEDED(hr))
    {
        TCHAR szBuff[128];
        if (DTFORMAT_NODATE != dwDateFlags)
        {
            iRes = ::GetDateFormat(lcid, dwDateFlags, lpST, NULL, szBuff, 128);
            if (0 == iRes) {
                hr = HRESULT_FROM_WIN32(::GetLastError());
            }
        }

        if (SUCCEEDED(hr))
        {
            bstr = szBuff;
            if (DTFORMAT_NOTIME != dwTimeFlags)
            {
                szBuff[0] = _T('\0');
                iRes = ::GetTimeFormat(lcid, dwTimeFlags, lpST, NULL, szBuff, 128);
                if (0 == iRes) {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                }
                else
                {
                    bstr += _T(" ");
                    bstr += szBuff;
                }
            }
        }
    }
    if (pST != lpST) delete lpST;
    if (SUCCEEDED(hr)) *pbstrOut = bstr.copy();

    return hr;
}

STDAPI FormatOLEDateTime(_In_ DATE dt, _In_ LCID lcid, _In_ DWORD dwDateFlags, _In_ DWORD dwTimeFlags, _Out_ BSTR *pbstrOut)
{
    HRESULT hr = S_OK;
    SYSTEMTIME st = { 0 };
    if (!::VariantTimeToSystemTime(dt, &st))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        if (SUCCEEDED(hr)) hr = E_FAIL;
    }
    if (SUCCEEDED(hr)) hr = ::FormatSysDateTime(&st, lcid, dwDateFlags, dwTimeFlags, pbstrOut);
    return hr;
}