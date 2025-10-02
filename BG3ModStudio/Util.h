#pragma once

namespace Util {

CString GetModulePath();
CString GetModuleDirectory();
CString GetModuleFileName();
CString GetModuleFileNameWithoutExtension();
CString GetCurrentTimeString();
void CopyToClipboard(HWND hWnd, const CString& str);

} // namespace Util
