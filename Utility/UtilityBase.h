#pragma once

#include <array>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <fstream>
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

#ifndef ASSERT
#ifdef _DEBUG
#include <crtdbg.h>
#define ASSERT(expr) _ASSERT(expr)
#else
#define ASSERT(expr)
#endif // _DEBUG
#endif // ASSERT 

using UInt8Ptr = std::unique_ptr<uint8_t[]>;
using ByteBuffer = std::pair<UInt8Ptr, size_t>;
