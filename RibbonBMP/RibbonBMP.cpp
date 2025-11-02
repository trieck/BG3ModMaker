#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <iostream>

#include "Exception.h"
#include "FileStream.h"
#include "GDIPlus.h"

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

static void makeRibbonBMP(wchar_t* inputImage, wchar_t* outputImage, int size, uint32_t threshold);

static GDIPlus gdiplus;

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 3) {
        wprintf(L"usage: %s <input-image> <output.bmp> [size = 32] [threshold = 30]\n", argv[0]);
        return 1;
    }

    if (!gdiplus.Init()) {
        std::wcerr << L"Failed to initialize GDI+." << std::endl;
        return 1;
    }

    int size = 32; // Default size
    int threshold = 30; // Default threshold

    if (argc > 3) {
        size = _wtoi(argv[3]);
        if (size <= 0) {
            std::wcerr << L"invalid size specified." << std::endl;
            return 1;
        }
    }

    if (argc > 4) {
        threshold = _wtoi(argv[4]);
        if (threshold < 0 || threshold > 255) {
            std::wcerr << L"invalid threshold specified." << std::endl;
            return 1;
        }
    }

    try {
        makeRibbonBMP(argv[1], argv[2], size, threshold);
    } catch (Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void makeRibbonBMP(wchar_t* inputImage, wchar_t* outputImage, int size, uint32_t threshold)
{
    std::unique_ptr<Bitmap> bmp;

    // If the input is an ICO file, extract the best size x size icon
    if (wcsstr(inputImage, L".ico") != nullptr) {
        HICON hIcon = nullptr;
        auto extracted = PrivateExtractIconsW(inputImage, 0, size, size, &hIcon, nullptr, 1, LR_LOADTRANSPARENT);
        if (extracted == 0 || hIcon == nullptr) {
            throw Exception("Failed to extract icon.");
        }

        ICONINFO iconInfo{};
        if (!GetIconInfo(hIcon, &iconInfo)) {
            DestroyIcon(hIcon);
            throw Exception("Failed to get icon info.");
        }

        // Create bitmap from icon info
        bmp = std::make_unique<Bitmap>(iconInfo.hbmColor, nullptr);

        // Cleanup icon resources (always executed whether exception occurs or not)
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        DestroyIcon(hIcon);
    } else {
        bmp = std::make_unique<Bitmap>(inputImage);
    }

    if (!bmp) {
        throw Exception("Failed to load BMP.");
    }

    Bitmap resized(size, size, PixelFormat32bppARGB);
    Graphics gr(&resized);
    gr.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    gr.DrawImage(bmp.get(), 0, 0, size, size);

    // Fix transparency: Set all pure black pixels (0,0,0) to transparent
    BitmapData bmpData;
    Rect rect(0, 0, static_cast<int>(resized.GetWidth()), static_cast<int>(resized.GetHeight()));
    resized.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bmpData);

    auto* pixels = static_cast<uint32_t*>(bmpData.Scan0);
    auto pixelCount = resized.GetWidth() * resized.GetHeight();
    for (auto i = 0u; i < pixelCount; ++i) {
        auto r = (pixels[i] >> 16) & 0xFF;
        auto g = (pixels[i] >> 8) & 0xFF;
        auto b = pixels[i] & 0xFF;

        // Anything that's near-black, kill it
        if (r <= threshold && g <= threshold && b <= threshold) {
            pixels[i] = 0x00000000;
        } else {
            auto alpha = (pixels[i] >> 24) & 0xFF;
            if (alpha < 255 && alpha > 0) {
                // Un-premultiply to remove black bleed
                r = (r * 255 + (alpha / 2)) / alpha;
                g = (g * 255 + (alpha / 2)) / alpha;
                b = (b * 255 + (alpha / 2)) / alpha;
                r = std::min<uint32_t>(r, 255);
                g = std::min<uint32_t>(g, 255);
                b = std::min<uint32_t>(b, 255);
                pixels[i] = (alpha << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }

    resized.UnlockBits(&bmpData);

    CLSID bmpClsid;
    gdiplus.GetEncoderClsid(L"image/bmp", &bmpClsid);
    if (resized.Save(outputImage, &bmpClsid, nullptr) != Ok) {
        throw Exception("Failed to save BMP.");
    }
}
