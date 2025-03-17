#pragma once

#include <d2d1.h>
#include <dwrite.h>

class Direct2D
{
public:
    Direct2D() = default;
    ~Direct2D();

    HRESULT Initialize();
    void Terminate();
    ID2D1Factory* GetD2DFactory() const;
    IDWriteFactory* GetDWriteFactory() const;

private:
    CComPtr<ID2D1Factory> m_pD2DFactory;
    CComPtr<IDWriteFactory> m_pDWriteFactory;
};

