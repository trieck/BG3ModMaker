#pragma once

#include "PAKReader.h"
#include "PageableIterator.h"
#include "Resource.h"
#include "Settings.h"

#include <DirectXTex.h>
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

    void open(const char* dbName);
    void close();
    bool isOpen() const;
    PageableIterator::Ptr newIterator(size_t pageSize = 25);
    PageableIterator::Ptr newIterator(const char* key, size_t pageSize = 25);
    void iconize(const char* pakFile, const char* dbName, bool overwrite = false);
    void setProgressListener(IIconizerProgressListener* listener);

    DirectX::ScratchImage getIcon(const std::string& key);

private:
    DirectX::ScratchImage loadIconTexture(const std::string& path);
    void iconizeLSXFile(const PackagedFileInfo& file);
    void iconizeLSFFile(const PackagedFileInfo& file);

    void iconizeRegion(const std::string& fileName, const Region::Ptr& region);
    void iconizeNodes(const std::string& filename, const std::vector<LSNode::Ptr>& nodes);
    void iconizeNode(const std::string& filename, const LSNode::Ptr& node);

    PAKReader m_reader, m_iconReader;
    IIconizerProgressListener* m_listener = nullptr;
    rocksdb::DB* m_db = nullptr;
    rocksdb::WriteBatch m_batch;
    std::string m_gameDataPath;
    Settings m_settings;
};
