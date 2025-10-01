#include "stdafx.h"
#include "PAKIgnore.h"

#include <fstream>

namespace fs = std::filesystem;

PAKIgnore::PAKIgnore(fs::path root) : m_root(std::move(root))
{
    Load();
}

bool PAKIgnore::IsIgnored(const std::filesystem::path& path) const
{
    auto s = path.wstring();

    // Normalize to backslashes (PathMatchSpecW expects this)
    for (auto& ch : s) {
        if (ch == L'/') {
            ch = L'\\';
        }
    }

    for (const auto& pat : m_patterns) {
        if (PathMatchSpecW(s.c_str(), pat.c_str())) {
            return true;
        }
    }

    return false;
}

void PAKIgnore::Load()
{
    auto ignoreFile = m_root / ".pakignore";
    if (!exists(ignoreFile)) {
        return;
    }

    std::wifstream in(ignoreFile);
    in.imbue(std::locale::classic()); // simple locale; use UTF-8 if needed

    std::wstring line;
    while (std::getline(in, line)) {
        // trim
        line.erase(0, line.find_first_not_of(L" \t\r\n"));

        if (line.empty() || line[0] == L'#') {
            continue;
        }

        line.erase(line.find_last_not_of(L" \t\r\n") + 1);

        m_patterns.insert(line);
    }
}
