#pragma once

namespace Util {

CString GetModulePath();
CString GetModuleDirectory();
CString GetModuleFileName();
CString GetModuleFileNameWithoutExtension();
CString GetCurrentTimeString();
void CopyToClipboard(HWND hWnd, const CString& str);
void SetMenuItemIcon(HMENU hMenu, UINT itemID, UINT iconResID, int cx = 16, int cy = 16);

} // namespace Util
