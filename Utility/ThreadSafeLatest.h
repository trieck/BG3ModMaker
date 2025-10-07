#pragma once

#include <mutex>
#include <optional>

template <typename T>
class ThreadSafeLatest
{
    std::optional<T> m_value;
    std::mutex m_mutex;

public:
    void set(const T& v)
    {
        std::lock_guard lock(m_mutex);
        m_value = std::move(v);
    }

    std::optional<T> get()
    {
        std::lock_guard lock(m_mutex);
        auto v = std::move(m_value);
        m_value.reset();
        return v; // newest only, clears after read
    }
};
