#pragma once

#include "targetver.h"
#include "UtilityBase.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlpath.h>
#include <atlscrl.h>
#include <atlsplit.h>
#include <atlstr.h>
#include <atltrace.h>
#include <atltypes.h>
#include <atlwin.h>
#include <comdef.h>

#ifndef MSG_WM_PAINT2
#define MSG_WM_PAINT2(func) \
    if (uMsg == WM_PAINT) { \
        this->SetMsgHandled(TRUE); \
        func(CPaintDC(*this)); \
        lResult = 0; \
        if(this->IsMsgHandled()) \
            return TRUE; \
    }
#endif  // MSG_WM_PAINT2

#ifndef MSG_WM_DIRECT2DPAINT
#define MSG_WM_DIRECT2DPAINT(func) \
    if (uMsg == WM_PAINT) { \
        this->SetMsgHandled(TRUE); \
        func(/* use Direct2D */); \
        lResult = 0; \
        if(this->IsMsgHandled()) \
            return TRUE; \
    }

#endif  // MSG_WM_DIRECT2DPAINT

#ifndef COMMAND_ID_HANDLER3
#define COMMAND_ID_HANDLER3(id, func) \
    if(uMsg == WM_COMMAND && (id) == LOWORD(wParam)) { \
        func(); \
    }
#endif  // COMMAND_ID_HANDLER3

