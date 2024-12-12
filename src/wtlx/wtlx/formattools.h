/************************************************************************
 * $Revision: 4 $
 * $Date: 2016-11-25 01:28:53 +0100 (Fri, 25 Nov 2016) $
 ************************************************************************/
/************************************************************************
 * File: formattools.h
 * Copyright: 2016 diVISION Soft, some rights reserved
 * License: GPLv3, s. LICENSE.txt
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
#pragma once
#include <OleAuto.h>

#define DTFORMAT_LOCALTIME  0x10000000
#define DTFORMAT_NODATE     0x20000000
#define DTFORMAT_NOTIME     0x20000000

enum MemoryUnits
{
    MU_BYTES
    , MU_KB
    , MU_MB
    , MU_GB
    , MU_TB
};
STDAPI FormatMemoryUnits(_In_ const DECIMAL *pdecIn, _In_ LCID lcid, _Out_ BSTR *pbstrOut);
STDAPI FormatError(_In_ HRESULT hr, _In_ LCID lcid, _Out_ BSTR *pbstrOut);
STDAPI FormatSysDateTime(_In_ const LPSYSTEMTIME pST, _In_ LCID lcid, _In_ DWORD dwDateFlags, _In_ DWORD dwTimeFlags, _Out_ BSTR *pbstrOut);
STDAPI FormatOLEDateTime(_In_ DATE dt, _In_ LCID lcid, _In_ DWORD dwDateFlags, _In_ DWORD dwTimeFlags, _Out_ BSTR *pbstrOut);