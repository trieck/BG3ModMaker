#pragma once

#include "PageableIterator.h"
#include "PAKReader.h"
#include "ProgressListener.h"
#include "Resource.h"
#include "Settings.h"

#include <DirectXTex.h>
#include <nlohmann/json.hpp>
#include <rocksdb/db.h>

class Iconizer
{
    Iconizer();
public:
    using Ptr = std::unique_ptr<Iconizer>;

    virtual ~Iconizer();

    static Ptr create();

    void open(const char* dbName);
    void openReadOnly(const char* dbName);

    void close();
    bool isOpen() const;
    PageableIterator::Ptr newIterator(size_t pageSize = 25);
    PageableIterator::Ptr newIterator(const char* key, size_t pageSize = 25);
    void iconize(const char* pakFile, const char* dbName, bool overwrite = false);
    void setProgressListener(IFileProgressListener* listener);

    DirectX::ScratchImage getIcon(const std::string& key);

private:
    DirectX::ScratchImage loadIconTexture(const std::string& path);
    void iconizeLSXFile(const PackagedFileInfo& file);
    void iconizeDDSFile(const PackagedFileInfo& file);

    PAKReader m_reader, m_iconReader;
    IFileProgressListener* m_listener = nullptr;
    rocksdb::DB* m_db = nullptr;
    rocksdb::WriteBatch m_batch;
    std::string m_gameDataPath;
    Settings m_settings;
};
