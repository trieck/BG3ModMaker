#pragma once

class PIDL
{
public:
    PIDL() = default;

    explicit PIDL(PIDLIST_ABSOLUTE pidl) noexcept;

    PIDL(const PIDL&) = delete;
    PIDL& operator=(const PIDL&) = delete;
    PIDL(PIDL&& other) noexcept;
    PIDL& operator=(PIDL&& other) noexcept;
    ~PIDL();

    void reset(PIDLIST_ABSOLUTE pidl = nullptr) noexcept;

    PIDLIST_ABSOLUTE get() const noexcept;
    PIDLIST_ABSOLUTE release() noexcept;
    explicit operator bool() const noexcept;

    PIDLIST_ABSOLUTE* put() noexcept;

private:
    PIDLIST_ABSOLUTE m_pidl = nullptr;
};
