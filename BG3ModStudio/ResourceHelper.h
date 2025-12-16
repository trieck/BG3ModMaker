#pragma once

class ResourceHelper
{
    public:
        using ResourceMap = std::unordered_map<std::wstring, std::wstring>;

        static CStringA LoadString(UINT nResourceID);
        static CStringA ExpandTemplate(UINT nResourceID, const ResourceMap& map);
};

