#include "stdafx.h"
#include "FileViews.h"
#include "FileViewFactory.h"
#include "resources/resource.h"

void FilesView::UpdateLayout()
{
    /*RECT rc;
    GetClientRect(&rc);

    MoveWindow(&rc);*/
}

BOOL FilesView::ActivateFile(const CString& path, void* data)
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

    SetActivePage(nPages);

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

    // TODO: Update tooltips

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
