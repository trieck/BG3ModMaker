#pragma once

namespace Util {

CString GetModulePath();
CString GetModuleDirectory();
CString GetModuleFileName();
CString GetModuleFileNameWithoutExtension();
CString GetCurrentTimeString();
void CopyToClipboard(HWND hWnd, const CString& str);
void SetMenuItemIcon(HMENU hMenu, UINT itemID, UINT iconResID, int cx = 16, int cy = 16);
CIconHandle LoadBitmapAsIcon(_U_STRINGorID id, int cx = 16, int cy = 16, COLORREF transparent = 0);
CString MakeUUID(BOOL bHandle = FALSE);
CFontHandle CreateFixedWidthFont(int nPointSize);
CFontHandle CreateDialogFont(int nPointSize, BOOL bBold = FALSE);

} // namespace Util
