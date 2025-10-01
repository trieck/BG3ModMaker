#pragma once

#include <filesystem>
#include <set>

class PAKIgnore
{
public:
    explicit PAKIgnore(std::filesystem::path root);

    bool IsIgnored(const std::filesystem::path& path) const;

private:
    void Load();

    std::filesystem::path m_root;
    std::set<std::wstring> m_patterns;
};
