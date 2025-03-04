#pragma once

#include <windows.h>
#include <gdiplus.h>

class GDIPlus
{
public:
    GDIPlus();
    ~GDIPlus();

    BOOL Init();
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

private:
    ULONG_PTR  m_token;
};

