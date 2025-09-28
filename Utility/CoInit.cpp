#include "pch.h"

#include "CoInit.h"
#include "Exception.h"

COMInitializer::COMInitializer(DWORD dwCoInit, LPVOID pvReserved)
{
    auto hr = CoInitializeEx(pvReserved, dwCoInit);
    if (FAILED(hr)) {
        throw Exception(std::format("Failed to initialize COM library: 0x{:08X}", hr));
    }
}

COMInitializer::~COMInitializer()
{
    CoUninitialize();
}
