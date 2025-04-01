#pragma once

#include "framework.h"

#define WM_UPDATELAYOUT (WM_APP + 1)

#define MSG_WM_UPDATELAYOUT(func) \
    if (uMsg == WM_UPDATELAYOUT) { \
        this->SetMsgHandled(TRUE); \
        func(); \
        lResult = 0; \
        if(this->IsMsgHandled()) \
            return TRUE; \
    }
