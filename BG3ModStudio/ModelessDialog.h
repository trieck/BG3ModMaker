#pragma once

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE ModelessDialog : public CDialogImpl<T, TBase>
{
public:
    ModelessDialog() = default;
    void RunModal(HWND hWndParent = GetActiveWindow());
    void Destroy();

protected:
    void PumpMessages();
    BOOL m_bPseudoModal = FALSE;
};

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
void ModelessDialog<T, TBase>::PumpMessages()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
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
