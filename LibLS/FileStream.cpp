#include "pch.h"
#include "FileStream.h"
#include "Exception.h"

FileStream::FileStream() : m_file(INVALID_HANDLE_VALUE)
{
}

FileStream::~FileStream()
{
    Close();
}

void FileStream::Open(const char* path, const char* mode)
{
    Close();

    DWORD access = 0;
    DWORD creation = 0;

    if (strcmp(mode, "rb") == 0) {
        access = GENERIC_READ;
        creation = OPEN_EXISTING;
    } else if (strcmp(mode, "wb") == 0) {
        access = GENERIC_WRITE;
        creation = CREATE_ALWAYS;
    }

    m_file = CreateFileA(path, access, FILE_SHARE_READ, nullptr, creation, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_file == INVALID_HANDLE_VALUE) {
        throw Exception(GetLastError());
    }
}

void FileStream::Close()
{
    if (m_file != INVALID_HANDLE_VALUE) {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
}

void FileStream::Write(const void* data, size_t size) const
{
    const auto* buffer = static_cast<const char*>(data);

    while (size > 0) {
        auto chunk = (size > MAXDWORD) ? MAXDWORD : static_cast<DWORD>(size);

        DWORD written = 0;
        if (!WriteFile(m_file, buffer, chunk, &written, nullptr) || written == 0) {
            throw Exception(GetLastError());
        }

        buffer += written;
        size -= written;
    }
}

void FileStream::Read(void* data, size_t size) const
{
    auto* buffer = static_cast<char*>(data);

    while (size > 0) {
        auto chunk = (size > MAXDWORD) ? MAXDWORD : static_cast<DWORD>(size);
        DWORD read = 0;
        if (!ReadFile(m_file, buffer, chunk, &read, nullptr)) {
            throw Exception(GetLastError());
        }

        if (read == 0) {
            throw Exception(ERROR_HANDLE_EOF);
        }

        buffer += read;
        size -= read;
    }
}


void FileStream::Seek(int64_t offset, SeekMode mode) const
{
    LARGE_INTEGER li;
    li.QuadPart = offset;

    auto result = SetFilePointerEx(m_file, li, nullptr, static_cast<DWORD>(mode));
    if (!result) {
        throw Exception(GetLastError());
    }
}

size_t FileStream::Tell() const
{
    LARGE_INTEGER pos = {};
    if (!SetFilePointerEx(m_file, {}, &pos, FILE_CURRENT)) {
        throw Exception(GetLastError());
    }

    return static_cast<size_t>(pos.QuadPart);
}

size_t FileStream::Size() const
{
    LARGE_INTEGER size;
    if (!GetFileSizeEx(m_file, &size)) {
        throw Exception(GetLastError());
    }

    return static_cast<size_t>(size.QuadPart);
}

bool FileStream::IsOpen() const
{
    return m_file != INVALID_HANDLE_VALUE;
}


