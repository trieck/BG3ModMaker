#pragma once

template <class T, class TBase = CWindow>
class ATL_NO_VTABLE ModelessDialog : public CDialogImpl<T, TBase>
{
public:
    ModelessDialog() = default;
    void RunModal();
    void Destroy();

protected:
    void PumpMessages();
};

template <class T, class TBase>
void ModelessDialog<T, TBase>::RunModal()
{
    while (this->IsWindow()) {
        this->PumpMessages();
        WaitMessage();
    }
}

template <class T, class TBase>
void ModelessDialog<T, TBase>::Destroy()
{
    if (this->IsWindow()) {
        this->DestroyWindow();
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
