#include "pch.h"
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

using GrannyFileDecompressor = struct GrannyFileDecompressor_s;

using GrannyBeginFileDecompression = GrannyFileDecompressor* (*)(uint32_t format,
                                                                 bool bytesReversed,
                                                                 uint32_t decompressedSize,
                                                                 void* decompressedBytes,
                                                                 uint32_t workMemSize,
                                                                 void* workMemBuffer);

using GrannyDecompressIncremental = bool (*)(GrannyFileDecompressor* decompressor,
                                             uint32_t compressedBytesSize, void* compressedBytes);

using GrannyEndFileDecompression = bool (*)(GrannyFileDecompressor* decompressor);

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
    if (!IsLoaded() && !Load()) {
        return FALSE; // Failed to load the decompressor
    }

    if (type == COMPRESSION_BITKNIT2) {
        return DecompressBitknit2(compressedSize,
                                  compressedData,
                                  stop2,
                                  decompressedData);
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
    if (!IsLoaded() && !Load()) {
        return FALSE; // Failed to load the decompressor
    }

    const auto& beginProc = functionTable["GrannyBeginFileDecompression"];
    if (!beginProc) {
        return FALSE; // Function not loaded
    }

    const auto& incrementalProc = functionTable["GrannyDecompressIncremental"];
    if (!incrementalProc) {
        return FALSE; // Function not loaded
    }

    const auto& endProc = functionTable["GrannyEndFileDecompression"];
    if (!endProc) {
        return FALSE; // Function not loaded
    }

    auto beginFunc = reinterpret_cast<GrannyBeginFileDecompression>(beginProc);
    auto incrementalFunc = reinterpret_cast<GrannyDecompressIncremental>(incrementalProc);
    auto endFunc = reinterpret_cast<GrannyEndFileDecompression>(endProc);

    constexpr auto WORK_MEM_SIZE = 16384u;
    auto workBuffer = std::make_unique<uint8_t[]>(WORK_MEM_SIZE);

    auto* decompressor = beginFunc(COMPRESSION_BITKNIT2,
                                   false,
                                   decompressedSize,
                                   decompressedData,
                                   WORK_MEM_SIZE,
                                   workBuffer.get());
    if (!decompressor) {
        return FALSE;
    }

    for (auto i = 0u; i < compressedSize;) {
        auto chunkSize = std::min(4096u, compressedSize - i);
        auto* chunkData = static_cast<uint8_t*>(compressedData) + i;
        auto result = incrementalFunc(decompressor, chunkSize, chunkData);
        if (!result) {
            endFunc(decompressor);
            return FALSE;
        }
        i += chunkSize;
    }

    if (!endFunc(decompressor)) {
        return FALSE;
    }

    return TRUE;
}
