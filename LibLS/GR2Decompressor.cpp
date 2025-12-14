#include "pch.h"
#include "Bitknit2Decompressor.h"
#include "GR2Decompressor.h"

namespace { // anonymous

using GrannyDecompressData = bool (*)(uint32_t format,
                                      bool bytesReversed,
                                      uint32_t compressedSize,
                                      void* compressedBytes,
                                      uint32_t stop0,
                                      uint32_t stop1,
                                      uint32_t stop2,
                                      void* decompressedBytes);

std::unordered_map<std::string, FARPROC> functionTable = {
    {"GrannyDecompressData", nullptr},
    {"GrannyBeginFileDecompression", nullptr},
    {"GrannyDecompressIncremental", nullptr},
    {"GrannyEndFileDecompression", nullptr},
};
} // anonymous namespace

GR2Decompressor::~GR2Decompressor()
{
    Unload();
}

BOOL GR2Decompressor::Load()
{
    if (IsLoaded()) {
        return TRUE; // Already loaded
    }

    if (!m_hModule) {
        m_hModule = LoadLibrary("granny2.dll");
        if (!m_hModule) {
            return FALSE; // Failed to load the DLL
        }
    }

    for (auto& [funcName, funcPtr] : functionTable) {
        funcPtr = GetProcAddress(m_hModule, funcName.c_str());
        if (!funcPtr) {
            ATLTRACE("GR2Decompressor::Load: Failed to get address for function %s.\n", funcName.c_str());
            Unload();
            return FALSE; // Failed to get function address
        }
    }

    return TRUE; // Successfully loaded
}

void GR2Decompressor::Unload()
{
    if (m_hModule) {
        FreeLibrary(m_hModule);
        m_hModule = nullptr;
    }
}

BOOL GR2Decompressor::IsLoaded() const
{
    return m_hModule != nullptr;
}

BOOL GR2Decompressor::Decompress(uint32_t type, uint32_t compressedSize, void* compressedData,
                                 void* decompressedData, uint32_t stop0, uint32_t stop1, uint32_t stop2)
{
    if (type == COMPRESSION_BITKNIT2) {
        return DecompressBitknit2(compressedSize,
                                  compressedData,
                                  stop2,
                                  decompressedData);
    }

    if (!IsLoaded() && !Load()) {
        return FALSE; // Failed to load the decompressor
    }

    const auto& decompressProc = functionTable["GrannyDecompressData"];
    if (!decompressProc) {
        return FALSE; // Function not loaded
    }

    auto decompressFunc = reinterpret_cast<GrannyDecompressData>(decompressProc);

    return decompressFunc(type,
                          false,
                          compressedSize,
                          compressedData,
                          stop0,
                          stop1,
                          stop2,
                          decompressedData);
}


BOOL GR2Decompressor::DecompressBitknit2(uint32_t compressedSize, void* compressedData, uint32_t decompressedSize,
                                         void* decompressedData)
{
    Bitknit2Decompressor decompressor;
    return decompressor.Decompress(compressedSize, compressedData, decompressedSize, decompressedData);
}
