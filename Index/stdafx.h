#pragma once

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <xmemory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

enum class SeekMode : DWORD {
    Begin = FILE_BEGIN,
    Current = FILE_CURRENT,
    End = FILE_END
};
