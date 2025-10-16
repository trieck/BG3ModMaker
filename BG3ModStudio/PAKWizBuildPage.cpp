#include "stdafx.h"
#include "Compress.h"
#include "Localization.h"
#include "Package.h"
#include "PAKIgnore.h"
#include "PAKWizBuildPage.h"
#include "PAKWriter.h"
#include "Resource.h"
#include "ResourceUtils.h"
#include "StringHelper.h"

#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

static void for_each_file(const fs::path& root, const std::function<void(const fs::path&)>& fn);

PAKWizBuildPage::PAKWizBuildPage(PAKWizard* pWiz, _U_STRINGorID title) : BasePage(title), m_pWiz(pWiz)
{
    SetHeaderTitle(_T("BG3 PAK Builder"));
    SetHeaderSubTitle(_T("Building the PAK file..."));
}

int PAKWizBuildPage::OnSetActive()
{
    SetWizardButtons(0);

    return 0;
}

int PAKWizBuildPage::OnWizardBack()
{
    return 0;
}

int PAKWizBuildPage::OnWizardNext()
{
    return 0;
}

BOOL PAKWizBuildPage::OnQueryCancel()
{
    return FALSE;
}

LRESULT PAKWizBuildPage::OnPAKRange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_progress.SetRange32(0, static_cast<int>(lParam));
    m_progress.SetPos(0);
    m_progress.SetState(PBST_NORMAL);

    return 0;
}

LRESULT PAKWizBuildPage::OnPAKProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_progress.SetPos(static_cast<int>(wParam));
    return 0;
}

LRESULT PAKWizBuildPage::OnPAKComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == 0) {
        AtlMessageBox(*this, L"Package created successfully.", nullptr, MB_ICONINFORMATION);
        SetWizardButtons(PSWIZB_FINISH);
    } else {
        auto lower = 0, upper = 0;
        m_progress.GetRange(lower, upper);
        m_progress.SetPos(upper);
        m_progress.SetState(PBST_ERROR);

        CString msg;
        msg.Format(L"An error occurred while creating the package: %s", static_cast<LPCTSTR>(m_lastError));
        AtlMessageBox(*this, msg.GetString(), nullptr, MB_ICONERROR);
        SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
    }

    return 0;
}

BOOL PAKWizBuildPage::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_progress = GetDlgItem(IDC_PROGRESS_PAKWIZ);
    ATLASSERT(m_progress.IsWindow());

    m_progress.SetPos(0);
    m_progress.SetStep(1);

    auto hThread = CreateThread(nullptr, 0, BuildProc, this, 0, nullptr);
    if (hThread == nullptr) {
        ATLTRACE(_T("Unable to create worker thread.\n"));
        return 0;
    }

    CloseHandle(hThread);

    return TRUE; // let the system set the focus
}

DWORD PAKWizBuildPage::BuildProc(LPVOID pv)
{
    auto* pThis = static_cast<PAKWizBuildPage*>(pv);
    ATLASSERT(pThis);

    PackageBuildData build{};
    build.version = PackageHeaderCommon::currentVersion;
    build.compression = pThis->m_pWiz->GetCompressionMethod();
    build.compressionLevel = LSCompressionLevel::DEFAULT;

    const auto& root = pThis->m_pWiz->GetRoot();
    auto genLoca = pThis->m_pWiz->GetGenerateLoca();
    auto genLSF = pThis->m_pWiz->GetGenerateLSF();
    auto targetFile = pThis->m_pWiz->GetPAKFile();

    PAKIgnore ignore(root.GetString());

    auto replaceExt = [](const std::string& s, const std::string& newExt) {
        fs::path p{s};
        p.replace_extension(newExt);
        return p.string();
    };

    WPARAM wParam = 0;

    try {
        std::unordered_set<std::string> seen;
        for_each_file(root.GetString(), [&](const fs::path& p) {
            auto relativePath = relative(p, root.GetString());
            if (ignore.IsIgnored(relativePath)) {
                return;
            }

            PackageBuildInputFile input;
            input.filename = p.string();
            input.name = relativePath.generic_string();

            auto ext = p.extension().string();
            std::ranges::transform(ext, ext.begin(), ::tolower);

            auto filename = fs::path(input.name).filename();

            // never package meta.lsf
            if (filename == "meta.lsf") {
                return;
            }

            // Convert .lsx to .lsf if needed (ignoring meta.lsx)
            if (genLSF && ext == ".lsx" && filename != "meta.lsx") {
                input.filename = replaceExt(input.filename, ".lsf");
                input.name = replaceExt(input.name, ".lsf");

                auto resource = ResourceUtils::loadResource(p.string().c_str(), LSX);

                ResourceUtils::saveResource(input.filename.c_str(), resource, LSF);
            }

            // Convert .xml to .loca if needed
            if (genLoca && ext == ".xml") {
                if (std::ranges::none_of(relativePath, [](const fs::path& part) {
                    return part == "Localization";
                })) {
                    return; // skip files not in a "Localization" folder
                }

                input.filename = replaceExt(input.filename, ".loca");
                input.name = replaceExt(input.name, ".loca");

                auto resource = LocaXmlReader::Read(p.string());

                LocaWriter::Write(input.filename, resource);
            }

            if (seen.emplace(input.name).second) { // Only add if not already seen
                build.files.push_back(std::move(input));
            }
        });

        pThis->PostMessage(WM_PAK_RANGE, 0, static_cast<LPARAM>(build.files.size()));

        auto utf8Filename = StringHelper::toUTF8(targetFile);
        PAKWriter writer(build, utf8Filename,
                         [pThis](size_t current, size_t total, const std::string& name) {
                             pThis->PostMessage(WM_PAK_PROGRESS, static_cast<WPARAM>(current),
                                                static_cast<LPARAM>(total));
                         });


        writer.write();
    } catch (const std::exception& e) {
        pThis->m_lastError = StringHelper::fromUTF8(e.what());
        wParam = -1;
    }

    pThis->PostMessage(WM_PAK_COMPLETE, wParam, 0);

    return 0;
}

void for_each_file(const fs::path& root, const std::function<void(const fs::path&)>& fn)
{
    for (const auto& entry :
         fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
        if (entry.is_regular_file()) {
            fn(entry.path());
        }
    }
}
