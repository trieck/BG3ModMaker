#include "pch.h"
#include "CoMemory.h"

CoMemory::CoMemory() : m_pData(nullptr), m_cRef(0)
{
}

CoMemory::~CoMemory()
{
    Free();
}

ULONG CoMemory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CoMemory::Release()
{
    auto cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

HRESULT CoMemory::QueryInterface(const IID& riid, void** ppvObject)
{
    if (riid == IID_IUnknown) {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

uint8_t* CoMemory::Data() const
{
    return static_cast<uint8_t*>(m_pData);
}

void* CoMemory::Alloc(size_t size)
{
    Free();
    m_pData = CoTaskMemAlloc(size);
    return m_pData;
}

void* CoMemory::Realloc(size_t size)
{
    if (m_pData) {
        m_pData = CoTaskMemRealloc(m_pData, size);
    } else {
        m_pData = CoTaskMemAlloc(size);
    }
    return m_pData;
}

void CoMemory::Free()
{
    CoTaskMemFree(m_pData);
    m_pData = nullptr;
}
