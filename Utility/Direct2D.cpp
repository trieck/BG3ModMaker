#include "pch.h"
#include "Direct2D.h"

Direct2D::~Direct2D()
{
    Terminate();
}

HRESULT Direct2D::Initialize()
{
    if (m_pD2DFactory) {
        return S_OK; // Already initialized
    }

    auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    if (FAILED(hr)) {
        return hr;
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                             reinterpret_cast<LPUNKNOWN*>(&m_pDWriteFactory));
    if (FAILED(hr)) {
        m_pD2DFactory.Release();
        return hr;
    }

    return hr;
}

void Direct2D::Terminate()
{
    m_pDWriteFactory.Release();
    m_pD2DFactory.Release();
}

ID2D1Factory* Direct2D::GetD2DFactory() const
{
    return m_pD2DFactory;
}

IDWriteFactory* Direct2D::GetDWriteFactory() const
{
    return m_pDWriteFactory;
}
