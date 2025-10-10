#include "stdafx.h"
#include "ShellNotifyRegistrar.h"

ShellNotifyRegistration::ShellNotifyRegistration(ULONG id) noexcept : m_id(id)
{
}

ShellNotifyRegistration::ShellNotifyRegistration(ShellNotifyRegistration&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
{
}

ShellNotifyRegistration& ShellNotifyRegistration::operator=(ShellNotifyRegistration&& other) noexcept
{
    if (this != &other) {
        reset();
        m_id = std::exchange(other.m_id, 0);
    }

    return *this;
}

ShellNotifyRegistration::~ShellNotifyRegistration()
{
    reset();
}

void ShellNotifyRegistration::reset(ULONG id) noexcept
{
    if (m_id != 0) {
        SHChangeNotifyDeregister(m_id);
        m_id = 0;
    }
    m_id = id;
}

ULONG ShellNotifyRegistration::get() const noexcept
{
    return m_id;
}

ShellNotifyRegistration::operator bool() const noexcept
{
    return m_id != 0;
}
