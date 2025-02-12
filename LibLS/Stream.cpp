#include "pch.h"

#include "Exception.h"
#include "Stream.h"
#include "Utility.h"

Stream::Stream()
{
    m_stream = std::make_unique<std::istringstream>();
}

Stream::Stream(const char* buf, size_t size)
{
    m_stream = createMemoryStream(buf, size);
}

Stream::Stream(const ByteBuffer& buf)
{
    m_stream = createMemoryStream(buf);
}

Stream::Stream(const std::string& str)
{
    m_stream = createMemoryStream(str);
}

Stream::~Stream()
= default;

void Stream::read(char* buf, size_t size) const
{
    m_stream->read(buf, static_cast<std::streamsize>(size));
    if (m_stream->fail()) {
        throw Exception(std::format("Unable to read bytes of size {} from stream.", size));
    }
}

void Stream::seek(int64_t offset, SeekMode mode) const
{
    m_stream->seekg(offset, static_cast<std::ios::seekdir>(mode));
    if (m_stream->fail()) {
        throw Exception(GetLastError());
    }
}

size_t Stream::tell() const
{
    return m_stream->tellg();
}

size_t Stream::size() const
{
    auto pos = m_stream->tellg();
    m_stream->seekg(0, std::ios::end);
    auto size = m_stream->tellg();
    m_stream->seekg(pos);
    return size;
}

Stream::Ptr Stream::makeStream(const char* buf, size_t size)
{
    return std::make_unique<Stream>(buf, size);
}

Stream::Ptr Stream::makeStream(const ByteBuffer& buf)
{
    return std::make_unique<Stream>(buf);
}

Stream::Ptr Stream::makeStream(const std::string& str)
{
    return std::make_unique<Stream>(str);
}

Stream::Ptr Stream::read(uint32_t bytes) const
{
    auto buf = std::make_unique<char[]>(bytes);

    read(buf.get(), bytes);

    return std::make_unique<Stream>(buf.get(), bytes);
}

std::string Stream::str() const
{
    return dynamic_cast<std::istringstream*>(m_stream.get())->str();
}

std::istream& Stream::stream() const
{
    return *m_stream;
}
