#include "stdafx.h"
#include "Util.h"

CString Util::GetModulePath()
{
    TCHAR path[MAX_PATH];
    GetModuleFileName(_Module.GetModuleInstance(), path, MAX_PATH);

    return path;
}

CString Util::GetModuleDirectory()
{
    auto path = GetModulePath();
    auto pos = path.ReverseFind('\\');
    if (pos == -1) {
        return CString();
    }
    return path.Left(pos);
}

CString Util::GetModuleFileNameW()
{
    auto path = GetModulePath();
    auto pos = path.ReverseFind('\\');
    if (pos == -1) {
        return CString();
    }
    return path.Mid(pos + 1);
}

CString Util::GetModuleFileNameWithoutExtension()
{
    auto path = GetModuleFileName();
    auto pos = path.ReverseFind('.');
    if (pos == -1) {
        return path;
    }
    return path.Left(pos);
}

CString Util::GetCurrentTimeString()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    CString strTime;
    strTime.Format(L"%02d:%02d:%02d.%03d",
                   st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    return strTime;
}

void Util::CopyToClipboard(HWND hWnd, const CString& str)
{
    if (!OpenClipboard(hWnd)) {
        return;
    }

    EmptyClipboard();

    auto hGlob = GlobalAlloc(GMEM_MOVEABLE, (str.GetLength() + 1) * sizeof(TCHAR));

    if (hGlob) {
        wcscpy_s(static_cast<wchar_t*>(GlobalLock(hGlob)), str.GetLength() + 1, str);
        GlobalUnlock(hGlob);
        SetClipboardData(CF_UNICODETEXT, hGlob);
    }

    CloseClipboard();
}

void Util::SetMenuItemIcon(HMENU hMenu, UINT itemID, UINT iconResID, int cx, int cy)
{
    auto hIcon = static_cast<HICON>(LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(iconResID),
        IMAGE_ICON,
        cx, cy,
        LR_CREATEDIBSECTION | LR_DEFAULTCOLOR));
    if (!hIcon) {
        return;
    }

    // Create a 32-bit ARGB DIB section
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    auto hdcScreen = GetDC(nullptr);
    auto hbm = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    auto hdcMem = CreateCompatibleDC(hdcScreen);
    auto old = SelectObject(hdcMem, hbm);

    DrawIconEx(hdcMem, 0, 0, hIcon, cx, cy, 0, nullptr, DI_NORMAL);

    SelectObject(hdcMem, old);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    DestroyIcon(hIcon);

    MENUITEMINFO mii{};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = hbm;
    SetMenuItemInfo(hMenu, itemID, FALSE, &mii);
}
