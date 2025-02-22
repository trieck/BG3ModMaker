#pragma once

std::string comma(int64_t i);
IOStreamPtr createMemoryStream(const uint8Ptr& data, size_t size);
IOStreamPtr createMemoryStream(const std::string& data);
IOStreamPtr createMemoryStream(const ByteBuffer& data);
IOStreamPtr createMemoryStream(const char* data, size_t size);