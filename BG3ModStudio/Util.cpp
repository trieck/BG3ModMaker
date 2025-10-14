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
    // Load the icon (32-bit, color, scaled)
    CIcon icon;
    icon.LoadIcon(iconResID, cx, cy, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
    if (icon.IsNull()) {
        ATLTRACE("Failed to load icon resource %u\n", iconResID);
        return;
    }

    CClientDC dcScreen(nullptr);

    // Create 32-bpp top-down DIB section
    CBitmap bmp;
    void* pBits = nullptr;
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    if (!bmp.CreateDIBSection(dcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0)) {
        ATLTRACE("Failed to create DIB section\n");
        return;
    }

    // Draw icon into memory DC backed by that DIB
    CDC dcMem;
    dcMem.CreateCompatibleDC(dcScreen);
    HBITMAP hOldBmp = dcMem.SelectBitmap(bmp);
    dcMem.DrawIconEx(0, 0, icon, cx, cy);
    dcMem.SelectBitmap(hOldBmp);

    // Assign bitmap to the menu item (menu owns it now)
    MENUITEMINFO mii = {};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = bmp.Detach();

    SetMenuItemInfo(hMenu, itemID, FALSE, &mii);
}

CIconHandle Util::LoadBitmapAsIcon(_U_STRINGorID id, int cx, int cy, COLORREF transparent)
{
    // Create imagelist with 32-bpp, mask+alpha support
    CImageListManaged il;
    if (!il.Create(cx, cy, ILC_COLOR32 | ILC_MASK, 1, 1)) {
        ATLTRACE("Failed to create image list\n");
        return nullptr;
    }

    // Add the bitmap (black background becomes transparent)
    CBitmap bmp;
    if (!bmp.LoadBitmap(id)) {
        ATLTRACE("Failed to load bitmap\n");
        return nullptr;
    }

    il.Add(bmp, transparent);

    // Extract the HICON
    CIcon icon(il.ExtractIcon(0));
    if (icon.IsNull()) {
        ATLTRACE("Failed to extract icon from image list\n");
        return nullptr;
    }

    // CopyImage forces the handle into a system-registered icon
    auto hSysIcon = static_cast<HICON>(CopyImage(icon, IMAGE_ICON, cx, cy, LR_COPYFROMRESOURCE));
    if (!hSysIcon) {
        ATLTRACE("Failed to copy icon\n");
        return nullptr;
    }

    return hSysIcon;
}
