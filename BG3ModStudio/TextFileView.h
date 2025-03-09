#pragma once

#include "IFileView.h"
#include "RTFFormatterRegistry.h"
#include "StreamBase.h"
#include "UTF8Stream.h"
#include "XmlLineFormatter.h"

class TextFileView : public CWindowImpl<TextFileView>, public IFileView
{
public:
    using Base = CWindowImpl;
    using Ptr = std::shared_ptr<TextFileView>;

    BEGIN_MSG_MAP(TextFileView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
        COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChange)
        ALT_MSG_MAP(1)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"BG3_TextFileView", nullptr)

    TextFileView() : m_richEdit(this, 1)
    {
    }

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnEditChange(UINT uNotifyCode, int nID, CWindow wndCtl, BOOL& bHandled);
    void OnSize(UINT nType, CSize size);
    void OnTimer(UINT_PTR nIDEvent);

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    const CString& GetPath() const override;
    VOID SetPath(const CString& path) override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

private:
    BOOL Write(LPCWSTR text) const;
    BOOL Write(LPCWSTR text, size_t length) const;
    BOOL Write(LPCSTR text) const;
    BOOL Write(LPCSTR text, size_t length) const;
    static BOOL SkipBOM(LPSTR& str, size_t size);
    BOOL WriteBOM(StreamBase& stream) const;
    BOOL Flush();
    void ApplySyntaxHighlight();

    CString m_path;
    FileEncoding m_encoding{UNKNOWN};
    CComObjectStack<UTF8Stream> m_stream;
    CContainedWindowT<CRichEditCtrl> m_richEdit;
    RTFStreamFormatter::Ptr m_formatter;
    XmlLineFormatter m_lineFormatter;   // This is temporary
};
