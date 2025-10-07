#pragma once

#include <chrono>

class Timer
{
public:
    Timer();
    ~Timer() = default;

    std::chrono::nanoseconds elapsed() const;

    std::string str() const;
    void restart();

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
    TimePoint start_;
};

inline std::ostream& operator<<(std::ostream& s, const Timer& timer)
{
    return s << timer.str();
}
