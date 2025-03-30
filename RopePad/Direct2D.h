#pragma once


#include <dwrite.h>
#include <d2d1_3.h>

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
