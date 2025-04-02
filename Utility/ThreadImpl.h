#pragma once

template <class T>
class ThreadImpl
{
public:
    ThreadImpl() = default;

    ~ThreadImpl()
    {
        Join();
    }

    BOOL Start()
    {
        if (m_handle) {
            return FALSE;
        }

        m_handle = reinterpret_cast<HANDLE>(
            _beginthreadex(
                nullptr,
                0,
                &ThreadImpl::ThreadProc,
                static_cast<T*>(this),
                0,
                &m_threadId
            )
        );

        return m_handle != nullptr;
    }

    void Join()
    {
        if (m_handle && !m_joined) {
            WaitForSingleObject(m_handle, INFINITE);
            CloseHandle(m_handle);
            m_handle = nullptr;
            m_joined = TRUE;
        }
    }

    uint32_t GetThreadId() const
    {
        return m_threadId;
    }

    static uint32_t __stdcall ThreadProc(void* arg)
    {
        T* pThis = static_cast<T*>(arg);
        return pThis->Run();
    }

private:
    HANDLE m_handle = nullptr;
    uint32_t m_threadId = 0;
    BOOL m_joined = FALSE;
};
