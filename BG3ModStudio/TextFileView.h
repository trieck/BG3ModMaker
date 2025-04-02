#pragma once

#include "IFileView.h"
#include "RTFFormatterRegistry.h"
#include "StreamBase.h"
#include "SyntaxHighlighter.h"
#include "UTF8Stream.h"

class TextFileView : public CWindowImpl<TextFileView>, public IFileView
{
public:
    using Base = CWindowImpl;
    using Ptr = std::shared_ptr<TextFileView>;

    BEGIN_MSG_MAP(TextFileView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_GET_TEXT_RANGE(OnGetTextRange)
        MSG_WM_HIGHLIGHT_READY(OnHighlightReady)
        MSG_WM_SIZE(OnSize)
        COMMAND_CODE_HANDLER(EN_CHANGE, OnEditChange)
        ALT_MSG_MAP(1)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"BG3_TextFileView", nullptr)

    TextFileView() : m_richEdit(this, 1)
    {
    }

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnEditChange(UINT uNotifyCode, int nID, CWindow wndCtl, BOOL& bHandled);
    void OnHighlightReady(LPHILIGHT_RANGE range);
    void OnGetTextRange(LPTEXT_RANGE range);
    void OnSize(UINT nType, CSize size);

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
    void SetDefaultFormat();
    BOOL Write(LPCWSTR text) const;
    BOOL Write(LPCWSTR text, size_t length) const;
    BOOL Write(LPCSTR text) const;
    BOOL Write(LPCSTR text, size_t length) const;
    static BOOL SkipBOM(LPSTR& str, size_t size);
    BOOL WriteBOM(StreamBase& stream) const;
    BOOL Flush();

    CString m_path;
    FileEncoding m_encoding{UNKNOWN};
    CComObjectStack<UTF8Stream> m_stream;
    CContainedWindowT<CRichEditCtrl> m_richEdit;
    RTFStreamFormatter::Ptr m_formatter;
    SyntaxHighlighter m_highlighter;
};
