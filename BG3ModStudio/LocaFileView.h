#pragma once
#include "IFileView.h"
#include "Localization.h"

class LocaFileView : public CWindowImpl<LocaFileView>, public IFileView
{
public:
    using Base = CWindowImpl;

    LocaFileView();
    ~LocaFileView() override = default;

    BEGIN_MSG_MAP(LocaFileView)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
    END_MSG_MAP()

    // IFileView
    BOOL Create(HWND parent, _U_RECT rect = nullptr, DWORD dwStyle = 0, DWORD dwStyleEx = 0) override;
    BOOL LoadFile(const CString& path) override;
    BOOL LoadBuffer(const CString& path, const ByteBuffer& buffer) override;
    BOOL SaveFile() override;
    BOOL SaveFileAs(const CString& path) override;
    BOOL Destroy() override;
    BOOL IsDirty() const override;
    BOOL IsEditable() const override;
    BOOL IsText() const override;
    const CString& GetPath() const override;
    VOID SetPath(const CString& path) override;
    FileEncoding GetEncoding() const override;
    operator HWND() const override;

private:
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    LRESULT OnGetDispInfo(NMHDR* pNMHDR);
    void OnSize(UINT nType, CSize size);

    void Populate();

    CListViewCtrl m_list;
    LocaResource m_resource;
    CString m_path;
};
