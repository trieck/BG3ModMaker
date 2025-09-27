#pragma once

#include <DirectXTex.h>

#include "PAKReader.h"
#include "PageableIterator.h"
#include "Settings.h"

#include <nlohmann/json.hpp>
#include <rocksdb/db.h>

class IIconizerProgressListener
{
public:
    virtual ~IIconizerProgressListener() = default;

    virtual void onStart(std::size_t totalEntries) = 0;
    virtual void onFileIconizing(std::size_t currentFile, const std::string& filename) = 0;
    virtual void onFinished(std::size_t iconizedEntries) = 0;
    virtual bool isCancelled() = 0;
    virtual void onCancel() = 0;
};

class Iconizer
{
public:
    Iconizer();
    virtual ~Iconizer();

    void close();
    bool isOpen() const;
    PageableIterator::Ptr newIterator(size_t pageSize = 25);
    PageableIterator::Ptr newIterator(const char* key, size_t pageSize = 25);
    void iconize(const char* pakFile, const char* dbName, bool overwrite = false);
    void open(const char* dbName);
    void setProgressListener(IIconizerProgressListener* listener);

private:
    DirectX::ScratchImage loadIconTexture(const std::string& path);
    void iconizeLSXFile(const PackagedFileInfo& file);
    void iconizeLSFFile(const PackagedFileInfo& file);

    PAKReader m_reader, m_iconReader;
    IIconizerProgressListener* m_listener = nullptr;
    rocksdb::DB* m_db = nullptr;
    rocksdb::WriteBatch m_batch;
    std::string m_gameDataPath;
    Settings m_settings;
};
