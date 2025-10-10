#include "stdafx.h"
#include "PIDL.h"

PIDL::PIDL(LPITEMIDLIST pidl) noexcept : m_pidl(pidl)
{
}

PIDL::PIDL(PIDL&& other) noexcept
    : m_pidl(std::exchange(other.m_pidl, nullptr))
{
}

PIDL& PIDL::operator=(PIDL&& other) noexcept
{
    if (this->get() != other.get()) {
        reset();
        m_pidl = std::exchange(other.m_pidl, nullptr);
    }

    return *this;
}

PIDL::~PIDL()
{
    reset();
}

void PIDL::reset(LPITEMIDLIST pidl) noexcept
{
    if (m_pidl != nullptr) {
        ILFree(m_pidl);
        m_pidl = nullptr;
    }

    m_pidl = pidl;
}

LPITEMIDLIST PIDL::get() const noexcept
{
    return m_pidl;
}

LPITEMIDLIST PIDL::release() noexcept
{
    return std::exchange(m_pidl, nullptr);
}

PIDL::operator bool() const noexcept
{
    return m_pidl != nullptr;
}

LPITEMIDLIST* PIDL::put() noexcept
{
    reset();
    return &m_pidl;
}
