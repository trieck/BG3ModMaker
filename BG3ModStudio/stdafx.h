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

#include <array>
#include <bit>
#include <deque>
#include <functional>
#include <memory>
#include <ranges>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <tsl/ordered_map.h>

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

#define MSG_WM_NCCALCSIZE2(func) \
    if (uMsg == WM_NCCALCSIZE) \
    { \
        this->SetMsgHandled(TRUE); \
        lResult = func((BOOL)wParam, (LPNCCALCSIZE_PARAMS)lParam); \
        if(this->IsMsgHandled()) \
            return TRUE; \
    }

#define COMMAND_ID_HANDLER2(id, func) \
    if (uMsg == WM_COMMAND && (id) == LOWORD(wParam)) { \
        lResult = func(); \
    }

#define COMMAND_ID_HANDLER3(id, func) \
    if (uMsg == WM_COMMAND && (id) == LOWORD(wParam)) { \
        func(); \
    }

#define COMMAND_HANDLER3(id, code, func) \
    if (uMsg == WM_COMMAND && (id) == LOWORD(wParam) && (code) == HIWORD(wParam)) { \
        func(); \
    }

#define MESSAGE_HANDLER2(msg, func) \
    if(uMsg == (msg)) \
    { \
        this->SetMsgHandled(TRUE); \
        func(wParam, lParam); \
        return TRUE; \
    }

#define MESSAGE_HANDLER3(msg, func) \
    if(uMsg == (msg)) \
    { \
        this->SetMsgHandled(TRUE); \
        func(); \
        return TRUE; \
    }

#define MESSAGE_HANDLER4(msg, func) \
    if(uMsg == (msg)) \
    { \
        this->SetMsgHandled(TRUE); \
        lResult = func(); \
        return TRUE; \
    }

struct RenameInfo
{
    CString oldPath;
    CString newPath;
};

constexpr DWORD CD_RENAME_EVENT = 0x52454E41; // 'RENA'

extern CAppModule _Module;

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
