#pragma once

class ScopeGuardSimple {
public:
    ScopeGuardSimple(std::function<void()>&& start, std::function<void()>&& end)
        : m_start(std::move(start)), m_end(std::move(end)) {
        m_start();
    }

    ~ScopeGuardSimple() {
        m_end();
    }

private:
    std::function<void()> m_start, m_end;
};

template <typename Obj, typename Start, typename End, typename... Args>
class ScopeGuard
{
public:
    ScopeGuard(Obj& obj, Start&& start, End&& end) :
        m_obj(obj), m_start(std::move(start)), m_end(std::move(end))
    {
        std::invoke(m_start, m_obj);
    }

    ScopeGuard(Obj& obj, Start&& start, End&& end, std::tuple<Args...> startArgs, std::tuple<Args...> endArgs) :
        m_obj(obj), m_start(std::move(start)), m_end(std::move(end)), m_startArgs(std::move(startArgs)),
            m_endArgs(std::move(endArgs))
    {
        std::apply([this]<typename... T0>(T0&&... args) {
            std::invoke(m_start, m_obj.get(), std::forward<T0>(args)...);
            }, m_startArgs);
    }

    ~ScopeGuard()
    {
        std::apply([this]<typename... T0>(T0&&... args) {
            std::invoke(m_end, m_obj.get(), std::forward<T0>(args)...);
            }, m_endArgs);
    }

private:
    std::reference_wrapper<Obj> m_obj;
    Start m_start;
    End m_end;
    std::tuple<Args...> m_startArgs, m_endArgs;
};
