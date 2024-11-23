// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#include "targetver.h"
#ifndef OEMRESOURCE
#define OEMRESOURCE 1
#endif // !OEMRESOURCE


#include <tuple>

#include <dwmapi.h>

#include <atlbase.h>
#if (_ATL_VER >= 0x0700)
#include <atlstr.h>
#include <atltypes.h>
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NO_UNION_CLASSES
#endif

#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlddx.h> 
#include <atldlgs.h>

#include <commoncontrols.h>

#include "taskbarautomation.h"
#include "uxthemehelper.h"
