#pragma once

#include "framework.h"

#define WM_ADDCHAR (WM_APP + 1)

#define MSG_WM_ADDCHAR(func) \
    if (uMsg == WM_ADDCHAR) { \
        this->SetMsgHandled(TRUE); \
        func(LOWORD(wParam)); \
        lResult = 0; \
        if(this->IsMsgHandled()) \
            return TRUE; \
    }
