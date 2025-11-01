#pragma once

class IFileProgressListener
{
public:
    virtual ~IFileProgressListener() = default;

    virtual void onStart(std::size_t totalEntries) = 0;
    virtual void onFile(std::size_t index, const std::string& filename) = 0;
    virtual void onFinished(std::size_t entries) = 0;
    virtual bool isCancelled() = 0;
    virtual void onCancel() = 0;
};
