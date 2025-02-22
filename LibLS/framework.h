#pragma once

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>
#include <string_view>
#include <string>
#include <vector>
#include <xmemory>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

using uint8Ptr = std::unique_ptr<uint8_t[]>;
using ByteBuffer = std::pair<uint8Ptr, size_t>;
using IOStreamPtr = std::unique_ptr<std::iostream>;
