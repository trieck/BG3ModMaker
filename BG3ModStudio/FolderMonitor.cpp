#include "stdafx.h"
#include "FolderMonitor.h"

extern CComCriticalSection g_csFile;

FolderMonitor::FolderMonitor(HWND hWndTarget, const CString& directory) :
    m_hWndTarget(hWndTarget),
    m_directory(directory),
    m_hThread(nullptr),
    m_hStopEvent(nullptr)
{
    m_hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

FolderMonitor::~FolderMonitor()
{
    if (m_hStopEvent != nullptr) {
        CloseHandle(m_hStopEvent);
    }
}

FolderMonitor::Ptr FolderMonitor::Create(HWND hWndTarget, const CString& directory)
{
    return Ptr(new FolderMonitor(hWndTarget, directory));
}

void FolderMonitor::Start()
{
    m_hThread = CreateThread(nullptr, 0, MonitorProc, this, 0, nullptr);
}

void FolderMonitor::Stop(DWORD dwTimeout)
{
    if (m_hThread != nullptr) {
        SetEvent(m_hStopEvent);
        WaitForSingleObject(m_hThread, dwTimeout);
        CloseHandle(m_hThread);
        m_hThread = nullptr;
    }
}

DWORD FolderMonitor::MonitorProc(LPVOID lpParam)
{
    auto pThis = static_cast<FolderMonitor*>(lpParam);
    pThis->Monitor();
    return 0;
}

void FolderMonitor::Monitor()
{
    auto hDir = CreateFile(m_directory, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

    if (hDir == INVALID_HANDLE_VALUE) {
        ATLTRACE("Failed to open directory for monitoring.\n");
        return;
    }

    OVERLAPPED ov{};
    ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    DWORD buffer[4096]{}, cb;
    HANDLE events[] { ov.hEvent, m_hStopEvent };

    while (TRUE) {
        ResetEvent(ov.hEvent);

        if (!ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), TRUE,
                                   FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                   FILE_NOTIFY_CHANGE_CREATION, &cb, &ov, nullptr)) {
            continue;
        }

        // Wait for either a directory change or a stop event
        auto result = WaitForMultipleObjects(2, events, FALSE, INFINITE);
        if (result == WAIT_OBJECT_0 + 1) {  // stop event was signaled
            ATLTRACE("Folder monitor shutdown requested.\n");
            break;
        }

        if (result != WAIT_OBJECT_0) {  // if not a directory change event then keep waiting
            continue;
        }

        CComCritSecLock lock(g_csFile); // wait to acquire the lock to ensure UI is not being updated

        DWORD bytesTransferred = 0;
        if (!GetOverlappedResult(hDir, &ov, &bytesTransferred, FALSE)) {
            continue; // failed to get the number of bytes transferred
        }

        if (bytesTransferred == 0) {
            continue; // nothing to process 
        }

        auto* pBase = reinterpret_cast<BYTE*>(buffer);
        auto* pEnd = pBase + bytesTransferred;

        while (pBase < pEnd) {
            auto* pFni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(pBase);

            CStringW fileName(pFni->FileName, static_cast<int>(pFni->FileNameLength / sizeof(WCHAR)));
            CStringW path;
            path.Format(L"%s\\%s", m_directory.GetString(), fileName.GetString());

            auto bstrFilename = SysAllocStringLen(path.GetString(), path.GetLength());

            if (bstrFilename == nullptr) {
                ATLTRACE("Failed to allocate BSTR for file change path: %S\n", path.GetString());
            } else {
                PostMessage(m_hWndTarget, WM_FILE_CHANGED, pFni->Action, reinterpret_cast<LPARAM>(bstrFilename));
            }

            if (pFni->NextEntryOffset == 0) {
                break;
            }

            pBase += pFni->NextEntryOffset;
        }
    }

    CloseHandle(ov.hEvent);
    CloseHandle(hDir);
}
