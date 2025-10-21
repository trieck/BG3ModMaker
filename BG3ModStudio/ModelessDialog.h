#pragma once

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE ModelessDialog : public CDialogImpl<T, TBase>
{
public:
    ModelessDialog() = default;
    void Run(HWND hWndParent = GetActiveWindow(), LPARAM lParam = 0);
    void RunModal(HWND hWndParent = GetActiveWindow());
    void Destroy();

    static HWND FindWindow();
    static void RunOnce(HWND hWndParent = GetActiveWindow(), LPARAM lParam = 0);

private:
    static LPCWSTR GetDialogID();

    void PumpMessages();
    BOOL m_bPseudoModal = FALSE;
};

template <class T, class TBase>
void ModelessDialog<T, TBase>::Run(HWND hWndParent, LPARAM lParam)
{
    auto hWnd = this->Create(hWndParent, lParam);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create dialog.\n"));
        return;
    }

    SetProp(hWnd, GetDialogID(), hWnd);

    this->ShowWindow(SW_SHOWNORMAL);
    this->UpdateWindow();

    while (this->IsWindow()) {
        this->PumpMessages();
        WaitMessage();
    }

    this->Destroy();
}

template <class T, class TBase>
void ModelessDialog<T, TBase>::RunModal(HWND hWndParent)
{
    m_bPseudoModal = TRUE;

    auto hWnd = this->Create(hWndParent);
    if (hWnd == nullptr) {
        ATLTRACE(_T("Unable to create dialog.\n"));
        return;
    }

    SetWindowPos(hWndParent, HWND_TOP, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    EnableWindow(hWndParent, FALSE);

    this->ShowWindow(SW_SHOWNORMAL);
    this->UpdateWindow();

    while (this->IsWindow()) {
        this->PumpMessages();
        WaitMessage();
    }

    this->Destroy();
}

template <class T, class TBase>
void ModelessDialog<T, TBase>::Destroy()
{
    if (!this->IsWindow()) {
        return;
    }

    auto hwndParent = GetParent(*this);
    auto hOwner = hwndParent ? hwndParent : GetActiveWindow();

    this->DestroyWindow();

    if (m_bPseudoModal) {
        EnableWindow(hOwner, TRUE);
        SetActiveWindow(hOwner);
    }
}

template <class T, class TBase>
HWND ModelessDialog<T, TBase>::FindWindow()
{
    struct Finder
    {
        static BOOL CALLBACK EnumProc(HWND hWnd, LPARAM lParam)
        {
            auto* data = reinterpret_cast<std::pair<LPCWSTR, HWND*>*>(lParam);
            if (GetProp(hWnd, data->first)) {
                *data->second = hWnd;
                return FALSE; // stop
            }
            return TRUE;
        }
    };

    HWND hWnd = nullptr;
    std::pair data = {GetDialogID(), &hWnd};

    EnumThreadWindows(GetCurrentThreadId(), Finder::EnumProc, reinterpret_cast<LPARAM>(&data));

    return hWnd;
}

template <class T, class TBase>
void ModelessDialog<T, TBase>::RunOnce(HWND hWndParent, LPARAM lParam)
{
    auto hWnd = FindWindow();
    if (hWnd) {
        ::ShowWindow(hWnd, SW_RESTORE);
        ::SetActiveWindow(hWnd);
    } else {
        T dlg;
        dlg.Run(hWndParent, lParam);
    }
}

template <class T, class TBase>
LPCWSTR ModelessDialog<T, TBase>::GetDialogID()
{
    return MAKEINTRESOURCEW(T::IDD);
}

template <class T, class TBase>
void ModelessDialog<T, TBase>::PumpMessages()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage(static_cast<int>(msg.wParam));
            return;
        }

        if (!this->IsWindow() || !this->IsDialogMessage(&msg)) {
            TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    auto* pLoop = _Module.GetMessageLoop();
    if (pLoop) {
        pLoop->OnIdle(0);
    }
}
