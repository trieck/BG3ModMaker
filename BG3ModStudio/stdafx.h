#pragma once

#include "targetver.h"

#ifdef _ATL_MIN_CRT
#undef _ATL_MIN_CRT
#endif

#define _WTL_FORWARD_DECLARE_CSTRING

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <atlbase.h>
#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlpath.h>
#include <atlribbon.h>
#include <atlscrl.h>
#include <atlsplit.h>
#include <atlstr.h>
#include <atltrace.h>
#include <atltypes.h>
#include <atlwin.h>
#include <comdef.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ranges>
#include <bit>
#include <memory>

#ifdef _UNICODE
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

#define MAKE_TREEITEM(n, t) \
    CTreeItem(((LPNMTREEVIEW)(n))->itemNew.hItem, t)
#define MAKE_OLDTREEITEM(n, t) \
    CTreeItem(((LPNMTREEVIEW)(n))->itemOld.hItem, t)

#define MSG_WM_PAINT2(func) \
    if (uMsg == WM_PAINT) { \
        this->SetMsgHandled(TRUE); \
        func(CPaintDC(*this)); \
        lResult = 0; \
        if(this->IsMsgHandled()) \
            return TRUE; \
    }

#define COMMAND_ID_HANDLER2(id, func) \
    if(uMsg == WM_COMMAND && (id) == LOWORD(wParam)) { \
        lResult = func(); \
    }

extern CAppModule _Module;

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
