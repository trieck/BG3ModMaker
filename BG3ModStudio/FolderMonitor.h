#pragma once

#define WM_FILE_CHANGED (WM_APP + 1)

class FolderMonitor
{
public:
    using Ptr = std::unique_ptr<FolderMonitor>;
private:
    FolderMonitor(HWND hWndTarget, const CString& directory);
public:
    ~FolderMonitor();

    static Ptr Create(HWND hWndTarget, const CString& directory);

    void Start();
    void Stop(DWORD dwTimeout = INFINITE);

private:
    static DWORD WINAPI MonitorProc(LPVOID lpParam);
    void Monitor();

    HWND m_hWndTarget;
    CString m_directory;
    HANDLE m_hThread;
    HANDLE m_hStopEvent;
};
