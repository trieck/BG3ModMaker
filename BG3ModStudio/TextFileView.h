#pragma once

#include "DocStyler.h"
#include "IFileView.h"
#include "ScintillaCtrl.h"
#include "StreamBase.h"
#include "UTF8Stream.h"

class TextFileView : public CWindowImpl<TextFileView>, public IFileView
{
public:
    using Base = CWindowImpl;
    using Ptr = std::shared_ptr<TextFileView>;

    BEGIN_MSG_MAP(TextFileView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER_EX(SCN_MODIFIED, OnModified)
        ALT_MSG_MAP(1)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(L"BG3_TextFileView", nullptr)

    TextFileView() : m_edit(this, 1)
    {
    }

    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnModified(LPNMHDR pnmh);
    void OnSize(UINT nType, CSize size);

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL LoadBuffer(const CString& path, const ByteBuffer& buffer) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    const CString& GetPath() const override;
    VOID SetPath(const CString& path) override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

    void SetReadOnly(BOOL);

private:
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
    CContainedWindowT<ScintillaCtrl> m_edit;
    DocStyler::Ptr m_styler;
    BOOL m_bDirty = FALSE;
    BOOL m_bReadOnly = FALSE;
};
