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
    // Load bitmap
    CBitmap bmp;
    if (!bmp.LoadBitmap(id)) {
        return nullptr;
    }

    BITMAP bm{};
    bmp.GetBitmap(&bm);

    CDC memDC;
    memDC.CreateCompatibleDC(nullptr);

    HBITMAP hOldBmp = memDC.SelectBitmap(bmp);

    // Create mask
    CDC maskDC;
    maskDC.CreateCompatibleDC();
    CBitmap maskBmp;
    maskBmp.CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, nullptr);

    HBITMAP hOldMask = maskDC.SelectBitmap(maskBmp);

    COLORREF oldBk = memDC.SetBkColor(transparent);
    maskDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, memDC, 0, 0, SRCCOPY);
    memDC.SetBkColor(oldBk);

    ICONINFO ii{};
    ii.fIcon = TRUE;
    ii.hbmColor = bmp;
    ii.hbmMask = maskBmp;

    CIcon icon;
    icon.CreateIconIndirect(&ii);

    auto hIcon = static_cast<HICON>(CopyImage(icon, IMAGE_ICON, cx, cy, LR_COPYFROMRESOURCE));

    memDC.SelectBitmap(hOldBmp);
    maskDC.SelectBitmap(hOldMask);

    return hIcon;
}

CString Util::MakeUUID(BOOL bHandle)
{
    GUID uuid;
    if (FAILED(CoCreateGuid(&uuid))) {
        return "";
    }

    wchar_t buffer[40];
    StringFromGUID2(uuid, buffer, _countof(buffer));

    // Strip curly braces
    auto* pbuf = buffer;
    size_t len = wcslen(buffer);
    if (buffer[0] == L'{') {
        buffer[len - 1] = L'\0';
        pbuf++;
    }

    // Lowercase
    std::ranges::transform(pbuf, pbuf + wcslen(pbuf), pbuf, ::towlower);

    CString s(pbuf);
    if (bHandle) { // Larian handle format
        s.Replace(L"-", L"g");
        s.Insert(0, L"h");
    }

    return s;
}
