#pragma once

class ShellNotifyRegistration
{
public:
    ShellNotifyRegistration() = default;

    explicit ShellNotifyRegistration(ULONG id) noexcept;
    ShellNotifyRegistration(const ShellNotifyRegistration&) = delete;
    ShellNotifyRegistration& operator=(const ShellNotifyRegistration&) = delete;
    ShellNotifyRegistration(ShellNotifyRegistration&& other) noexcept;
    ShellNotifyRegistration& operator=(ShellNotifyRegistration&& other) noexcept;
    ~ShellNotifyRegistration();

    void reset(ULONG id = 0) noexcept;
    ULONG get() const noexcept;
    explicit operator bool() const noexcept;

private:
    ULONG m_id = 0;
};
