#include "stdafx.h"
#include "FileViews.h"
#include "FileViewFactory.h"
#include "resources/resource.h"

LRESULT FilesView::OnCreate(LPCREATESTRUCT pcs)
{
    if (pcs == nullptr) {
        return -1;
    }

    if (TabViewImpl::OnCreate(pcs) == -1) {
        return -1;
    }

    LOGFONT lf{};
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = 17;
    lf.lfWeight = FW_SEMIBOLD;
    Checked::tcsncpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Tahoma"), _TRUNCATE);

    m_tabFont.CreateFontIndirect(&lf);
    m_tabViewCtrl.m_tabCtrl.SetFont(m_tabFont);

    m_tabViewCtrl.SetTopMargin(1);
    m_tabViewCtrl.SetViewBorder(2);

    return 0;
}

LRESULT FilesView::OnTabSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    ATLASSERT(idCtrl == TabViewCtrl::m_nTabID);
    ATLASSERT(pnmh->hwndFrom == m_tabViewCtrl.m_tabCtrl);
    ATLASSERT(pnmh->code == TCN_SELCHANGE);

    auto index = m_tabViewCtrl.GetActiveTab();
    if (index == -1) {
        return 0;
    }

    SetActivePage(index);
    OnPageActivated(index);
    UpdateLayout();

    bHandled = TRUE;

    return 0;
}

BOOL FilesView::ActivateFile(const CString& path, LPVOID data)
{
    auto it = m_data.find(data);
    if (it != m_data.end()) {
        SetActivePage(it->second);
        OnPageActivated(it->second);
        UpdateLayout();
        return TRUE;
    }

    auto fileView = FileViewFactory::CreateFileView(path, *this, rcDefault);
    if (fileView == nullptr) {
        CString message;
        message.Format(L"Unable to create a file view for \"%s\".\nThis file type may be unsupported.",
                       path.GetString());
        AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
        return FALSE;
    }

    if (!fileView->LoadFile(path)) {
        CString message;
        message.Format(L"Unable to load file \"%s\".", path.GetString());
        AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
        return FALSE;
    }

    auto nPages = GetPageCount();
    m_data[data] = nPages;

    CString title = PathFindFileName(path);

    m_views.emplace_back(fileView);

    AddPage(*fileView, title, 0, data);

    ATLASSERT(m_tabViewCtrl.m_tabCtrl.IsWindow());

    TOOLINFO ti{};
    ti.cbSize = sizeof(TOOLINFO);
    ti.hwnd = m_tabViewCtrl.m_tabCtrl;
    ti.uId = nPages;
    ti.hinst = _Module.GetResourceInstance();
    ti.lpszText = const_cast<LPWSTR>(path.GetString());

    m_tabViewCtrl.m_tabCtrl.GetToolTips().UpdateTipText(&ti);
    SetActivePage(nPages);

    return TRUE;
}

BOOL FilesView::CloseFileByData(LPVOID data)
{
    auto it = m_data.find(data);
    if (it == m_data.end()) {
        return FALSE;
    }

    CloseFile(it->second);

    return TRUE;
}

IFileView::Ptr FilesView::ActiveFile() const
{
    int activePage = GetActivePage();

    if (activePage < 0) {
        return nullptr;
    }

    return m_views[activePage];
}

int FilesView::ActivePage() const
{
    return GetActivePage();
}

const std::vector<IFileView::Ptr>& FilesView::Files() const
{
    return m_views;
}

PVOID FilesView::GetData(int index) const
{
    if (index < 0 || index >= GetPageCount()) {
        return nullptr;
    }

    auto hItem = GetPageData(index);

    return hItem;
}

PVOID FilesView::CloseFile(int index)
{
    if (index < 0 || index >= GetPageCount()) {
        return nullptr;
    }

    const auto& vit = m_views.begin() + index;
    auto& fileView = *vit;

    if (fileView->IsDirty()) {
        CString message;
        message.Format(L"Save changes to \"%s\"?", fileView->GetPath());
        auto result = AtlMessageBox(*this, static_cast<LPCWSTR>(message), IDR_MAINFRAME,
                                    MB_YESNOCANCEL | MB_ICONQUESTION | MB_ICONWARNING);
        if (result == IDCANCEL) {
            return nullptr;
        }

        if (result == IDYES) {
            if (!fileView->SaveFile()) {
                message.Format(L"Unable to save file \"%s\".", fileView->GetPath());
                AtlMessageBox(*this, static_cast<LPCWSTR>(message), nullptr, MB_ICONERROR);
                return nullptr;
            }
        }
    }

    auto hItem = GetData(index);

    auto it = m_data.find(hItem);
    if (it != m_data.end()) {
        m_data.erase(it);

        // adjust indexes
        for (auto& idx : m_data | std::views::values) {
            if (idx > index) {
                --idx;
            }
        }
    }

    fileView->Destroy();

    m_views.erase(vit);

    RemovePage(index);

    // Update tooltips
    TOOLINFO ti{};
    ti.cbSize = sizeof(TOOLINFO);
    ti.hwnd = m_tabViewCtrl.m_tabCtrl;
    ti.uId = index;
    ti.hinst = _Module.GetResourceInstance();

    auto toolTips = m_tabViewCtrl.m_tabCtrl.GetToolTips();

    auto toolCount = toolTips.GetToolCount();
    for (auto i = 0; i < toolCount; ++i) {
        ti.uId = i;
        toolTips.DelTool(&ti);
    }

    for (auto i = 0; i < GetPageCount(); ++i) {
        ti.uId = i;
        ti.lpszText = const_cast<LPWSTR>(m_views[i]->GetPath());
        toolTips.AddTool(&ti);
    }

    UpdateLayout();

    return hItem;
}

PVOID FilesView::CloseOtherFiles(int index)
{
    auto hItem = GetData(index);

    for (auto i = GetPageCount() - 1; i >= 0; --i) {
        if (i == index) {
            continue;
        }
        CloseFile(i);
    }

    return hItem;
}

void FilesView::CloseAllFiles()
{
    for (auto i = GetPageCount() - 1; i >= 0; --i) {
        CloseFile(i);
    }
}

PVOID FilesView::CloseActiveFile()
{
    return CloseFile(GetActivePage());
}

FileEncoding FilesView::FileEncoding(int index) const
{
    if (index < 0 || index >= GetPageCount()) {
        return UNKNOWN;
    }

    ATLASSERT(index < static_cast<int>(m_views.size()));

    return m_views[index]->GetEncoding();
}

BOOL FilesView::IsDirty(int index) const
{
    if (index < 0 || index >= GetPageCount()) {
        return FALSE;
    }

    return m_views[index]->IsDirty();
}

BOOL FilesView::IsDirty() const
{
    for (const auto& view : m_views) {
        if (view->IsDirty()) {
            return TRUE;
        }
    }

    return FALSE;
}
