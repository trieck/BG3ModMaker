#include "pch.h"
#include "GR2Stream.h"

GR2Stream::GR2Stream(uint8_t* buf) : m_buf(buf)
{
}

GR2Stream::GR2Stream(const GR2Stream& rhs)
{
    *this = rhs;
}

GR2Stream& GR2Stream::operator=(const GR2Stream& rhs)
{
    if (this != &rhs) {
        m_buf = rhs.m_buf;
    }

    return *this;
}

uint8_t*& GR2Stream::data()
{
    return m_buf;
}

GR2RefStream::GR2RefStream(uint8_t** buf) : m_buf(buf)
{
}

GR2RefStream::GR2RefStream(const GR2RefStream& rhs)
{
    *this = rhs;
}

GR2RefStream& GR2RefStream::operator=(const GR2RefStream& rhs)
{
    if (this != &rhs) {
        m_buf = rhs.m_buf;
    }

    return *this;
}

bool GR2RefStream::isNull() const
{
    return m_buf == nullptr || *m_buf == nullptr;
}

uint8_t** GR2RefStream::data() const
{
    return m_buf;
}
