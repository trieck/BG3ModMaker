#include "pch.h"
#include <atlcomcli.h>
#include "MemStream.h"

static auto constexpr DEFAULT_BUFFER_SIZE = 4096;

MemStream::MemStream() : m_pData(nullptr), m_pos(0), m_size(0), m_capacity(0), m_cRef(0)
{
    Alloc(DEFAULT_BUFFER_SIZE);
}

MemStream::MemStream(CoMemory* pData) : m_pos(0), m_size(0), m_capacity(0), m_cRef(0)
{
    m_pData = pData;    // will AddRef
}

MemStream::~MemStream()
{
    Free();
}

ULONG MemStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG MemStream::Release()
{
    auto cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

HRESULT MemStream::QueryInterface(const IID& riid, void** ppvObject)
{
    if (riid == IID_IUnknown || riid == IID_ISequentialStream || riid == IID_IStream) {
        *ppvObject = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;

    return E_NOINTERFACE;
}

HRESULT MemStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
    if (!(m_pData && pv)) {
        return STG_E_INVALIDPOINTER;
    }

    auto cbRead = std::min<ULONG>(cb, static_cast<ULONG>(m_size - m_pos));

    memcpy(pv, m_pData->Data() + m_pos, cbRead);

    m_pos += cbRead;

    if (pcbRead) {
        *pcbRead = cbRead;
    }

    return S_OK;
}

HRESULT MemStream::Write(const void* pv, ULONG cb, ULONG* pcbWritten)
{
    if (!(m_pData && pv)) {
        return STG_E_INVALIDPOINTER;
    }

    auto newSize = m_pos + cb;
    if (newSize > m_capacity) {
        Realloc(std::max({ newSize, static_cast<uint64_t>(DEFAULT_BUFFER_SIZE), m_capacity * 2 }));
    }

    memcpy(m_pData->Data() + m_pos, pv, cb);

    m_size = std::max(m_size, newSize);
    m_pos += cb;

    if (pcbWritten) {
        *pcbWritten = cb;
    }

    return S_OK;
}

HRESULT MemStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
    int64_t newPos;

    auto offset = dlibMove.QuadPart;

    switch (dwOrigin) {
    case STREAM_SEEK_SET:
        if (offset >= 0 && static_cast<uint64_t>(offset) <= m_size) {
            m_pos = offset;
        } else {
            return ResultFromScode(STG_E_SEEKERROR);
        }
        break;
    case STREAM_SEEK_CUR:
        newPos = static_cast<int64_t>(m_pos) + offset;
        if (newPos < 0 || static_cast<size_t>(newPos) > m_size) {
            return ResultFromScode(STG_E_SEEKERROR);
        }
        m_pos = newPos;
        break;
    case STREAM_SEEK_END:
        if (offset > 0) {
            m_pos = m_size; // clamp
        } else if (static_cast<uint64_t>(-offset) > m_size) {
            return ResultFromScode(STG_E_SEEKERROR);
        } else {
            m_pos = m_size + offset;
        }
        break;
    default:
        return ResultFromScode(STG_E_INVALIDFUNCTION);
    }

    if (plibNewPosition) {
        plibNewPosition->QuadPart = m_pos;
    }

    return S_OK;
}

HRESULT MemStream::SetSize(ULARGE_INTEGER libNewSize)
{
    if (libNewSize.QuadPart > m_capacity) {
        Realloc(libNewSize.QuadPart);
    }

    return S_OK;
}

HRESULT MemStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
{
    if (!pstm) {
        return STG_E_INVALIDPOINTER;
    }

    auto sz = std::min(cb.QuadPart, m_size - m_pos);

    ULONG written;
    auto hr = pstm->Write(m_pData->Data() + m_pos, static_cast<ULONG>(sz), &written);
    if (FAILED(hr)) {
        return hr;
    }

    m_pos += sz;

    if (pcbRead) {
        pcbRead->QuadPart = sz;
    }

    if (pcbWritten) {
        pcbWritten->QuadPart = written;
    }

    return S_OK;
}

HRESULT MemStream::Commit(DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}

HRESULT MemStream::Revert()
{
    return E_NOTIMPL;
}

HRESULT MemStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

HRESULT MemStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

HRESULT MemStream::Stat(STATSTG* pstatstg, DWORD grfStatFlag)
{
    if (!pstatstg) {
        return STG_E_INVALIDPOINTER;
    }

    ZeroMemory(pstatstg, sizeof(STATSTG));

    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = static_cast<ULONGLONG>(m_size);
    pstatstg->grfMode = STGM_READWRITE;
    pstatstg->clsid = CLSID_NULL;

    return S_OK;
}

HRESULT MemStream::Clone(IStream** ppstm)
{
    if (!ppstm) {
        return STG_E_INVALIDPOINTER;
    }

    auto* clone = new MemStream(m_pData);
    clone->m_pos = m_pos;
    clone->m_capacity = m_capacity;
    clone->m_size = m_size;

    clone->AddRef();
    *ppstm = clone;

    return S_OK;
}

void MemStream::Alloc(uint64_t size)
{
    Free();

    if (m_pData == nullptr) {
        m_pData = new CoMemory();
    }

    m_pData->Alloc(size);
    m_capacity = size;
    m_pos = 0;
    m_size = 0;
}

void MemStream::Realloc(uint64_t size)
{
    m_pData->Realloc(size);
    m_capacity = size;
}

void MemStream::Free()
{
    m_pData.Release();

    m_capacity = 0;
    m_pos = 0;
    m_size = 0;
}
