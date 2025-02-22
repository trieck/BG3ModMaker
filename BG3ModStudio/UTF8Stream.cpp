#include "stdafx.h"
#include "UTF8Stream.h"

#include "StringHelper.h"

HRESULT UTF8Stream::Reset() const
{
    ATLASSERT(m_pImpl != nullptr);

    ULARGE_INTEGER uli{};

    auto hr = m_pImpl->SetSize(uli);
    if (FAILED(hr)) {
        return hr;
    }

    LARGE_INTEGER li{};
    hr = m_pImpl->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

CStringA UTF8Stream::ReadString()
{
    ATLASSERT(m_pImpl != nullptr);

    STATSTG statstg;
    auto hr = Stat(&statstg, STATFLAG_NONAME);
    if (FAILED(hr)) {
        return "";
    }

    auto size = statstg.cbSize.QuadPart;

    CStringA strText;

    auto bufferLength = static_cast<int>(size) + 1;
    auto* buffer = strText.GetBuffer(bufferLength);

    LARGE_INTEGER li{};
    hr = Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        return "";
    }
    
    ULONG uRead;
    hr = Read(buffer, static_cast<ULONG>(size), &uRead);
    if (FAILED(hr)) {
        return "";
    }

    if (size != uRead) {
        return "";
    }

    buffer[uRead] = '\0';
    strText.ReleaseBuffer();

    return strText;
}

CStringW UTF8Stream::ReadUTF16String()
{
    auto utf8String = ReadString();
    return StringHelper::fromUTF8(utf8String);
}

ByteBuffer UTF8Stream::ReadBytes(size_t size)
{
    ATLASSERT(m_pImpl != nullptr);

    STATSTG statstg;
    auto hr = Stat(&statstg, STATFLAG_NONAME);
    if (FAILED(hr)) {
        return {};
    }

    auto cbSize = statstg.cbSize.QuadPart;
    if (size == static_cast<size_t>(-1)) {
        size = cbSize;
    }

    if (size > cbSize) {
        return {};
    }

    auto buffer = std::make_unique<uint8_t[]>(size);
    LARGE_INTEGER li{};
    hr = Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        return {};
    }

    ULONG uRead;
    hr = Read(buffer.get(), static_cast<ULONG>(size), &uRead);
    if (FAILED(hr)) {
        return {};
    }

    if (size != uRead) {
        return {};
    }

    return { std::move(buffer), size };
}

HRESULT UTF8Stream::Write(LPCWSTR text) const
{
    CStringA utf8Text = StringHelper::toUTF8(text);

    ULONG cb = static_cast<ULONG>(strlen(utf8Text));
    ULONG written;

    auto hr = m_pImpl->Write(utf8Text, cb, &written);

    return hr;
}

HRESULT UTF8Stream::Write(LPCSTR text) const
{
    return Write(text, strlen(text));
}

HRESULT UTF8Stream::Write(LPCSTR text, size_t length) const
{
    ULONG cb = static_cast<ULONG>(length);
    ULONG written;

    auto hr = m_pImpl->Write(text, cb, &written);

    return hr;
}


UTF8Stream::~UTF8Stream()
= default;

HRESULT UTF8Stream::FinalConstruct()
{
    m_pImpl = SHCreateMemStream(nullptr, 0);
    if (!m_pImpl) {
        return E_FAIL;
    }

    return S_OK;
}

void UTF8Stream::FinalRelease()
{
    if (m_pImpl) {
        m_pImpl.Release();
    }
}

HRESULT UTF8Stream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Read(pv, cb, pcbRead);
}

HRESULT UTF8Stream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Write(pv, cb, pcbWritten);
}

HRESULT UTF8Stream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Seek(dlibMove, dwOrigin, plibNewPosition);
}

HRESULT UTF8Stream::SetSize(ULARGE_INTEGER libNewSize)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->SetSize(libNewSize);
}

HRESULT UTF8Stream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->CopyTo(pstm, cb, pcbRead, pcbWritten);
}

HRESULT UTF8Stream::Commit(DWORD grfCommitFlags)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Commit(grfCommitFlags);
}

HRESULT UTF8Stream::Revert()
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Revert();
}

HRESULT UTF8Stream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->LockRegion(libOffset, cb, dwLockType);
}

HRESULT UTF8Stream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->UnlockRegion(libOffset, cb, dwLockType);
}

HRESULT UTF8Stream::Stat(STATSTG* pstatstg, DWORD grfStatFlag)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Stat(pstatstg, grfStatFlag);
}

HRESULT UTF8Stream::Clone(IStream** ppstm)
{
    ATLASSERT(m_pImpl);
    return m_pImpl->Clone(ppstm);
}
