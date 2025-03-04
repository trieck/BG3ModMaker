#include <windows.h>
#include <gdiplus.h>
#include <memory>

#include "GDIPlus.h"

using namespace Gdiplus;

GDIPlus::GDIPlus() : m_token(0)
{
}

GDIPlus::~GDIPlus()
{
    if (m_token) {
        GdiplusShutdown(m_token);
    }
}

BOOL GDIPlus::Init()
{
    GdiplusStartupInput input;
    return GdiplusStartup(&m_token, &input, nullptr) == Ok;
}

int GDIPlus::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT numEncoders = 0, size = 0;

    // Get the number of image encoders and their total size
    GetImageEncodersSize(&numEncoders, &size);
    if (size == 0) {
        return -1; // No encoders available
    }

    // Allocate memory for encoders
    std::unique_ptr<ImageCodecInfo[]> pImageCodecInfo(new ImageCodecInfo[size]);
    if (!pImageCodecInfo) {
        return -1;
    }

    // Get the encoders
    GetImageEncoders(numEncoders, size, pImageCodecInfo.get());

    // Find the encoder for the requested format
    for (auto i = 0u; i < numEncoders; ++i) {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            return i; // Success
        }
    }

    return -1; // Not found
}
