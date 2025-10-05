#include "pch.h"
#include "Timer.h"

#include <chrono>
#include <sstream>
#include <iomanip>

Timer::Timer() : start_(Clock::now())
{
}

std::chrono::nanoseconds Timer::elapsed() const
{
    using std::chrono::duration_cast;
    auto now = Clock::now();
    return duration_cast<std::chrono::nanoseconds>(now - start_);
}

std::string Timer::str() const
{
    using std::chrono::duration_cast;

    auto diff = elapsed();

    auto hours = duration_cast<std::chrono::hours>(diff);
    auto minutes = duration_cast<std::chrono::minutes>(diff) % 60;
    auto seconds = duration_cast<std::chrono::seconds>(diff) % 60;
    auto millis = duration_cast<std::chrono::milliseconds>(diff) % 1000;
    auto micros = duration_cast<std::chrono::microseconds>(diff) % 1000;
    auto nanos = diff % 1000;

    std::ostringstream oss;
    oss.fill('0'); // Ensure proper zero-padding

    if (hours.count()) {
        oss << hours.count() << ':'
            << std::setw(2) << minutes.count() << ':'
            << std::setw(2) << seconds.count() << " hours";
    } else if (minutes.count()) {
        oss << minutes.count() << ':'
            << std::setw(2) << seconds.count() << " minutes";
    } else if (millis.count()) {
        oss << seconds.count() << '.'
            << std::setw(3) << millis.count() << " seconds";
    } else {
        oss << micros.count() << '.'
            << std::setw(3) << nanos.count() << " microseconds";
    }

    return oss.str();
}

void Timer::restart()
{
    start_ = Clock::now();
}
