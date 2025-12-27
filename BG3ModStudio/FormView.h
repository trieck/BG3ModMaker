#pragma once

class IFormView
{
public:
    virtual ~IFormView() = default;
    virtual BOOL Destroy() = 0;
    virtual HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) = 0;
    virtual operator HWND() const = 0;
    using Ptr = std::unique_ptr<IFormView>;
};

template <typename T>
class DialogFormImpl : public CDialogImpl<T>, public IFormView
{
public:
    using Base = CDialogImpl<T>;

    // IFormView
    HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override
    {
        return Base::Create(hWndParent, dwInitParam);
    }

    BOOL Destroy() override
    {
        return Base::DestroyWindow();
    }

    operator HWND() const override
    {
        return this->m_hWnd;
    }
};
