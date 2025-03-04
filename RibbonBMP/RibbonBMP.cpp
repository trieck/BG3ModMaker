#include <windows.h>
#include <gdiplus.h>
#include <cstdio>
#include <iostream>

#include "Exception.h"
#include "FileStream.h"
#include "GDIPlus.h"

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

static void makeRibbonBMP(wchar_t* inputImage, wchar_t* outputImage);

static GDIPlus gdiplus;

int wmain(int argc, wchar_t* argv[])
{
    if (argc != 3) {
        wprintf(L"usage: %s <input-image> <output.bmp>\n", argv[0]);
        return 1;
    }
    
    if (!gdiplus.Init()) {
        std::wcerr << L"Failed to initialize GDI+." << std::endl;
        return 1;
    }

    try {
        makeRibbonBMP(argv[1], argv[2]);
    } catch (Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void makeRibbonBMP(wchar_t* inputImage, wchar_t* outputImage)
{
    std::unique_ptr<Bitmap> bmp;
    
    // If the input is an ICO file, extract the best 32x32 icon
    if (wcsstr(inputImage, L".ico") != nullptr) {
        HICON hIcon;
        auto extracted = PrivateExtractIconsW(inputImage, 0, 32, 32, &hIcon, nullptr, 1, LR_LOADTRANSPARENT);
        if (extracted == 0 || hIcon == nullptr) {
            throw Exception("Failed to extract icon.");
        }

        ICONINFO iconInfo;
        GetIconInfo(hIcon, &iconInfo);
        bmp = std::make_unique<Bitmap>(iconInfo.hbmColor, nullptr);

        // Cleanup
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        DestroyIcon(hIcon);
    } else {
        bmp = std::make_unique<Bitmap>(inputImage);
    }

    if (!bmp) {
        throw Exception("Failed to load BMP.");
    }

    // Ensure it is 32x32
    Bitmap resized(32, 32, PixelFormat32bppARGB);
    Graphics g(&resized);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(bmp.get(), 0, 0, 32, 32);

    // Fix transparency: Set all pure black pixels (0,0,0) to transparent
    BitmapData bmpData;
    Rect rect(0, 0, static_cast<int>(resized.GetWidth()), static_cast<int>(resized.GetHeight()));
    resized.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bmpData);

    auto* pixels = static_cast<uint32_t*>(bmpData.Scan0);
    auto pixelCount = resized.GetWidth() * resized.GetHeight();
    for (auto i = 0u; i < pixelCount; i++) {
        if ((pixels[i] & 0x00FFFFFF) == 0x00000000) { // If RGB is (0,0,0), set alpha to 0
            pixels[i] = 0x00000000;
        }
    }

    resized.UnlockBits(&bmpData);

    CLSID bmpClsid;
    gdiplus.GetEncoderClsid(L"image/bmp", &bmpClsid);
    if (resized.Save(outputImage, &bmpClsid, nullptr) != Ok) {
        throw Exception("Failed to save BMP.");
    }
}
