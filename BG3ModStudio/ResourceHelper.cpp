#include "stdafx.h"
#include "ResourceHelper.h"
#include "StringHelper.h"

CStringA ResourceHelper::LoadString(UINT nResourceID)
{
    auto hRes = FindResource(nullptr, MAKEINTRESOURCE(nResourceID), RT_RCDATA);
    if (!hRes) {
        return "";
    }

    auto hMem = LoadResource(nullptr, hRes);
    auto size = SizeofResource(nullptr, hRes);

    auto* pData = static_cast<const char*>(LockResource(hMem));

    return CStringA(pData, static_cast<int>(size));
}

CStringA ResourceHelper::ExpandTemplate(UINT nResourceID, const ResourceMap& map)
{
    auto templateStr = LoadString(nResourceID);
    if (templateStr.IsEmpty()) {
        return "";
    }

    for (const auto& [key, value] : map) {
        auto keyA = StringHelper::toUTF8(key.c_str());
        auto valueA = StringHelper::toUTF8(value.c_str());

        CStringA placeholder;
        placeholder.Format("${%s}", static_cast<LPCSTR>(keyA));

        int pos = 0;
        while ((pos = templateStr.Find(placeholder, pos)) != -1) {
            templateStr.Delete(pos, placeholder.GetLength());
            templateStr.Insert(pos, valueA);
            pos += valueA.GetLength();
        }
    }

    return templateStr;
}
