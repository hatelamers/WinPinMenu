#pragma once

#include "targetver.h"
#ifndef OEMRESOURCE
#define OEMRESOURCE 1
#endif // !OEMRESOURCE

#include <atlbase.h>
#if (_ATL_VER >= 0x0700)
#include <atlstr.h>
#include <atltypes.h>
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NO_UNION_CLASSES
#endif

#include <atlapp.h>
#include <atlwin.h>
