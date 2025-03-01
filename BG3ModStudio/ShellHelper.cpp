#include "stdafx.h"
#include "ShellHelper.h"

std::deque<CString> SplitPath(const CString& path)
{
    std::deque<CString> parts;

    auto* ppath = path.GetString();

    while (ppath) {
        auto* poldpath = ppath;
        ppath = PathFindNextComponent(ppath);

        if (ppath != nullptr) {
            auto len = static_cast<uint32_t>(ppath - poldpath);

            if (len && poldpath[len-1] == _T('\\')) {
                --len;
            }

            if (len) {
                parts.emplace_back(poldpath, len);
            }
        }
    }

    return parts;
}
