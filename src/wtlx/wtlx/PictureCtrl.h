/************************************************************************
 * $Revision: 41 $
 * $Date: 2024-12-05 17:37:38 +0100 (Thu, 05 Dec 2024) $
 ************************************************************************/
/************************************************************************
 * File: PictureCtrl.h - implementation of the CPictureCtrl class
 * Copyright: 2011 Ed Gadziemski, 2016 diVISION Soft, some rights reserved
 * License: CPOL, s. https://en.wikipedia.org/wiki/Code_Project_Open_License
 * @author Ed Gadziemski (http://www.codeproject.com/Members/Ed-Gadziemski)
 * @author Dmitri Zoubkov (dimamizou@users.sf.net)
 ************************************************************************/
//////////////////////////////////////////////////////
//
// Classes in this file:
//
// CPictureCtrl - GDI+ picture control implementation
// CISSHelperWTL - implementation of ISquentialStream
//                 for reading OLEDB binary columns
//
//////////////////////////////////////////////////////
#if !defined(__PICTURECTRL_H__)
#define __PICTURECTRL_H__

#pragma once

#include <atlimage.h>

// The dictionary for image format GUIDs
typedef CSimpleMap<LPCTSTR, GUID> CFormatMap;

// Provisions for image scaling
#include <float.h>
enum ImageScale { Normal, AutoFit, Stretch };

// Comment out this define if you don't want or need database
// functionality. Those sections of code will be left out
//#define __DATABASE_SUPPORT__

// OLEDB client header and ISequentialStream implementation
#ifdef __DATABASE_SUPPORT__
#include <atldbcli.h>
class CISSHelperWTL;
#endif

#include "Draw.h"

////////////////////////////////////////////////////////////////
// CPictureCtrl attaches to an owner-drawn picture control and //
// reads, displays, and saves disk or database images         //
////////////////////////////////////////////////////////////////
template <class TBase>
class CPictureCtrlT : public TBase
{
public:
	CImage m_img;
	CFormatMap m_formatMap;
    CImageListManaged m_iml;
    INT m_iIcon;

#ifdef __DATABASE_SUPPORT__
	CISSHelperWTL m_stream;
#endif

// Constructors, etc.
    CPictureCtrlT(HWND hWnd = NULL)
        : TBase(hWnd)
        , m_iIcon(0)
	{
		// Initialize the format map with the encoder GUIDs
		m_formatMap.Add(L"BMP", Gdiplus::ImageFormatBMP);
        m_formatMap.Add(L"GIF", Gdiplus::ImageFormatGIF);
        m_formatMap.Add(L"JPG", Gdiplus::ImageFormatJPEG);
        m_formatMap.Add(L"PNG", Gdiplus::ImageFormatPNG);
        m_formatMap.Add(L"TIF", Gdiplus::ImageFormatTIFF);
        m_img.SetHasAlphaChannel(true);

        m_iml.Create(::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), ::ImageList_GetBitsPerPixelFlag() | ILC_MASK, 0, 1);
	}

	CPictureCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
        return TBase::Create(_T("WTLPICTURE"), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

	~CPictureCtrlT()
	{
        Release();
	}

    ///////////////////////
    // Utility Functions //
    ///////////////////////
    inline BOOL IsNull()
    {
        return m_img.IsNull();
    }

    void RefreshWindow()
    {
        if (::IsWindow(m_hWnd))
        {
            RECT rc;
            GetClientRect(&rc);
            InvalidateRect(&rc, true);
        }
    }

    void Release(BOOL fDestroyWnd = FALSE)
    {
        if (!IsNull()) {
            m_img.Destroy();
        }
        if (fDestroyWnd) DestroyWindow();
    }

    GUID GetFormatGUID(int index)
    {
        return m_formatMap.GetValueAt(index);
    }

// Drawing functions
	BOOL Render(ImageScale imageScale = AutoFit)
	{
        BOOL res = FALSE;
        if (m_img.IsNull())
        {
            if (0 < m_iml.GetImageCount())
            {
                CDC dc = GetDC();
                dc.SetBkColor(CLR_NONE);
                res = m_iml.Draw(dc, m_iIcon, 0, 0, ILD_TRANSPARENT);
            }
        }
        else
        {
            CRect rcDest;
            CalculateScale(rcDest, imageScale);

            // Draw the image
            CDC dc = GetDC();
            dc.SetBkColor(CLR_NONE);
            res = m_img.Draw(dc, rcDest);
        }
		return res;
	}

	void CalculateScale(RECT& rcDest, ImageScale imageScale)
	{
		auto width = m_img.GetWidth();
		auto height = m_img.GetHeight();
		DOUBLE scaleWidth = 1.0, scaleHeight = 1.0;
		RECT rcBmp = { 0, 0, width, height };
		RECT rcClnt = { 0 };

		GetClientRect(&rcClnt);

		if (imageScale == AutoFit)
		{
			if (rcBmp.right > rcClnt.right)
			{
                scaleWidth = ((DOUBLE)rcClnt.right / (DOUBLE)rcBmp.right);
			}
			if (rcBmp.bottom > rcClnt.bottom)
			{
                scaleHeight = ((DOUBLE)rcClnt.bottom / (DOUBLE)rcBmp.bottom);
			}
			if (scaleWidth > scaleHeight) scaleWidth = scaleHeight;
			else scaleHeight = scaleWidth;
		
            rcDest.right = (LONG)(DOUBLE)(width * scaleWidth);
            rcDest.bottom = (LONG)(DOUBLE)(height * scaleHeight);
		}
		else if (imageScale == Stretch)
		{
            rcDest.right = rcClnt.right;
            rcDest.bottom = rcClnt.bottom;
		}
		else if (imageScale == Normal)
		{
            rcDest.right = rcBmp.right;
            rcDest.bottom = rcBmp.bottom;
		}
	}

    int AddIcon(HICON hIcon)
    {
        auto result = m_iml.AddIcon(hIcon);
		return m_iml.ReplaceIcon(result, hIcon);
    }

    INT SetIconIndex(INT iIndex)
    {
        ATLASSERT(-1 < iIndex && iIndex < m_iml.GetImageCount());
        INT result = m_iIcon;
        m_iIcon = iIndex;
        RefreshWindow();
        return result;
    }

    HRESULT LoadResource(ATL::_U_STRINGorID nID, ATL::_U_STRINGorID nType = RT_BITMAP)
    {
        m_img.Destroy();

        HINSTANCE hInst = ModuleHelper::GetResourceInstance();
        HRSRC hResource = NULL;
        LPVOID pResourceData = NULL;
        if (RT_ICON == nType.m_lpstr) {
            nType = RT_GROUP_ICON;
            hResource = ::FindResource(hInst, nID.m_lpstr, nType.m_lpstr);
            if (!hResource)
                return ATL::AtlHresultFromLastError();
            pResourceData = ::LockResource(::LoadResource(hInst, hResource));
            if (!pResourceData)
                return ATL::AtlHresultFromLastError();

            CRect rc;
            GetClientRect(&rc);
            UINT iID = ::LookupIconIdFromDirectoryEx((PBYTE)pResourceData, TRUE, rc.right - rc.left, rc.bottom - rc.top, LR_DEFAULTCOLOR);
            if (!iID)
                return ATL::AtlHresultFromLastError();
            nID = iID;
            nType = RT_ICON;
        }
        hResource = ::FindResource(hInst, nID.m_lpstr, nType.m_lpstr);
        if (!hResource)
            return ATL::AtlHresultFromLastError();

        DWORD imageSize = ::SizeofResource(hInst, hResource);
        if (!imageSize)
            return ATL::AtlHresultFromLastError();

        pResourceData = ::LockResource(::LoadResource(hInst, hResource));
        if (!pResourceData)
            return ATL::AtlHresultFromLastError();

        HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
        if (!hBuffer)
            return ATL::AtlHresultFromLastError();

        HRESULT hr = E_FAIL;
        LPVOID pBuffer = ::GlobalLock(hBuffer);
        if (pBuffer)
        {
            CopyMemory(pBuffer, pResourceData, imageSize);

            CComPtr<IStream> pStream;
            hr = ::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream);
            if (SUCCEEDED(hr)) {
                hr = m_img.Load(pStream);
            }
            ::GlobalUnlock(hBuffer);
        }
        ::GlobalFree(hBuffer);

        hr = SUCCEEDED(hr) && m_img.IsNull() ? E_FAIL : hr;
        if (SUCCEEDED(hr)) {
            RefreshWindow();
        }
        return hr;
    }

    HRESULT LoadURL(LPCTSTR lpszURL)
    {
        m_img.Destroy();
        CComPtr<IStream> pStream;
        // TODO make LoadURL non-blocking
        HRESULT hr = ::URLOpenBlockingStream(NULL, lpszURL, &pStream, 0, NULL);
        if (SUCCEEDED(hr)) {
            hr = m_img.Load(pStream);
        }
        if (SUCCEEDED(hr)) {
            RefreshWindow();
        }
        return hr;
    }

    HRESULT LoadFile(LPCTSTR lpszFileName)
    {
        m_img.Destroy();
        HRESULT hr = m_img.Load(lpszFileName);
        if (SUCCEEDED(hr)) {
            RefreshWindow();
        }
        return hr;
    }

    HRESULT SaveToFile(LPCTSTR lpszFileName)
    {
        if (m_img.IsNull()) return E_NOT_VALID_STATE;
        return m_img.Save(lpszFileName);
    }

#ifdef __DATABASE_SUPPORT__
///////////////////////////////
// Database Stream Functions //
///////////////////////////////
	BOOL ReadISS(ISequentialStream* &pISS, ULONG& ulLength, ULONG& ulStatus)
	{
		BOOL bSuccess = FALSE;

		if (ulStatus == DBSTATUS_S_OK)
		{
			// Read the supplied ISequential stream
			LPVOID buffer = ::CoTaskMemAlloc(ulLength);
			if (pISS->Read(buffer, ulLength, NULL) == S_OK)
			{
                m_img.Destroy();

				// Create an IStream and load the image
                CComPtr<IStream> pStream;
                HRESULT hr = ::CreateStreamOnHGlobal(buffer, TRUE, &pStream);
                if (SUCCEEDED(hr)) {
                    hr = m_img.Load(pStream);
                }
                bSuccess = SUCCEEDED(hr);
				RefreshWindow();
			}
			if (pISS != NULL)
			{
				pISS->Release();
				pISS = NULL;
			}
            ::CoTaskMemFree(buffer);
		}
		return bSuccess;
	}

	BOOL WriteISS(ISequentialStream* &pISS, ULONG& ulLength, ULONG& ulStatus, int format = 0)
	{
		BOOL bSuccess = FALSE;
		LARGE_INTEGER liOffset = { 0, 0 };
		ULARGE_INTEGER lBytesWritten = { 0, 0 };

		// Create the IStream and place the picture bytes in it
		CComPtr<IStream> pStream = NULL;
        ::CreateStreamOnHGlobal(NULL, TRUE, &pStream);
        HRESULT hr = m_img.Save(pStream, GetFormatGUID(format));
        if (SUCCEEDED(hr))
		{
			// Get the count of bytes written to the stream
			STATSTG stat;
            pStream->Stat(&stat, STATFLAG_NONAME);
			lBytesWritten = stat.cbSize;

			// Reset read position to 0
            if (pStream->Seek(liOffset, STREAM_SEEK_SET, NULL) == S_OK)
			{
				m_stream.Clear();

				// Copy the image data to the stream helper
				LPVOID buffer = ::CoTaskMemAlloc((size_t)lBytesWritten.QuadPart);
                if (pStream->Read(buffer, (ULONG)lBytesWritten.QuadPart, NULL) == S_OK)
					bSuccess = SUCCEEDED(m_stream.Write(buffer, (ULONG)lBytesWritten.QuadPart, &ulLength));

				// Assign the newly writtem stream to the passed-in stream
				if (bSuccess)
				{
					if (pISS != NULL) pISS->Release();
					pISS = &m_stream;
					ulStatus = DBSTATUS_S_OK;
				}

				::CoTaskMemFree(buffer);
			}
		}
		return bSuccess;
	}
#endif // database support
};

typedef CPictureCtrlT<ATL::CWindow> CPictureCtrl;


#ifdef __DATABASE_SUPPORT__
/////////////////////////////////////////////////////
// WTL port of the AOTBLOB sample CISSHelper class //
/////////////////////////////////////////////////////
class CISSHelperWTL : public ISequentialStream  
{
private:
	ULONG		m_cRef;			// Reference count.
	ULONG       m_iReadPos;     // Current index position for reading from the buffer.
	ULONG       m_iWritePos;    // Current index position for writing to the buffer.

public:
	void*       m_pBuffer;		// Buffer
	ULONG       m_ulLength;     // Total buffer size.
	ULONG       m_ulStatus;     // Column status.

	CISSHelperWTL()
	{
		m_cRef		= 0;
		m_pBuffer	= NULL;
		m_ulLength	= 0;
		m_ulStatus  = DBSTATUS_S_OK;
		m_iReadPos	= 0;
		m_iWritePos	= 0;
	}

	~CISSHelperWTL() { Clear(); }

	void Clear() 
	{
		CoTaskMemFree( m_pBuffer );
		m_cRef		= 0;
		m_pBuffer	= NULL;
		m_ulLength	= 0;
		m_ulStatus  = DBSTATUS_S_OK;
		m_iReadPos	= 0;
		m_iWritePos	= 0;
	}

	STDMETHODIMP_(ULONG) AddRef(void) { return ++m_cRef; }

	STDMETHODIMP_(ULONG) Release(void) { return --m_cRef; }

	STDMETHODIMP_(HRESULT) QueryInterface( REFIID riid, void** ppv )
	{
		*ppv = NULL;
		if ( riid == IID_IUnknown )			 *ppv = this;
		if ( riid == IID_ISequentialStream ) *ppv = this;
		if ( *ppv )
		{
			( (IUnknown*) *ppv )->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(HRESULT) Read( void *pv,	ULONG cb, ULONG* pcbRead )
	{
		// Check parameters.
		if ( pcbRead ) *pcbRead = 0;
		if ( !pv ) return STG_E_INVALIDPOINTER;
		if ( 0 == cb ) return S_OK; 

		// Calculate bytes left and bytes to read.
		ULONG cBytesLeft = m_ulLength - m_iReadPos;
		ULONG cBytesRead = cb > cBytesLeft ? cBytesLeft : cb;

		// If no more bytes to retrieve return S_FALSE.
		if ( 0 == cBytesLeft ) return S_FALSE;

		// Copy to users buffer the number of bytes requested or remaining
		memcpy( pv, (void*)((BYTE*)m_pBuffer + m_iReadPos), cBytesRead );
		m_iReadPos += cBytesRead;

		// Return bytes read to caller.
		if ( pcbRead ) *pcbRead = cBytesRead;
		if ( cb != cBytesRead ) return S_FALSE; 

		return S_OK;
	}
        
	STDMETHODIMP_(HRESULT) Write( const void *pv, ULONG cb, ULONG* pcbWritten )
	{
		// Check parameters.
		if ( !pv ) return STG_E_INVALIDPOINTER;
		if ( pcbWritten ) *pcbWritten = 0;
		if ( 0 == cb ) return S_OK;

		// Enlarge the current buffer.
		m_ulLength += cb;

		// Grow internal buffer to new size.
		m_pBuffer = CoTaskMemRealloc( m_pBuffer, m_ulLength );

		// Check for out of memory situation.
		if ( NULL == m_pBuffer ) 
		{
			Clear();
			return E_OUTOFMEMORY;
		}

		// Copy callers memory to internal buffer and update write position.
		memcpy( (void*)((BYTE*)m_pBuffer + m_iWritePos), pv, cb );
		m_iWritePos += cb;

		// Return bytes written to caller.
		if ( pcbWritten ) *pcbWritten = cb;

		return S_OK;
	}
};
#endif

#endif // !defined(__PICTURECTRL_H__)