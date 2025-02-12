#pragma once

std::string comma(int64_t i);
IStreamPtr createMemoryStream(const uint8Ptr& data, size_t size);
IStreamPtr createMemoryStream(const std::string& data);
IStreamPtr createMemoryStream(const ByteBuffer& data);
IStreamPtr createMemoryStream(const char* data, size_t size);