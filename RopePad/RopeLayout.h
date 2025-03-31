#pragma once

#include "Rope.h"

#include <d2d1_3.h>

class RopeLayout
{
public:
    explicit RopeLayout();
    ~RopeLayout();

    D2D_SIZE_F GetBounds() const;
    HRESULT Init(ID2D1DeviceContext* ctx, IDWriteFactory* pWriteFactory);
    HRESULT Render(const Rope& rope, ID2D1CommandList** ppCommandList);

private:
    D2D_POINT_2F ComputeLayout(const Rope::PNode& node, int depth, float column);
    HRESULT CreateDevResources(ID2D1DeviceContext* ctx, IDWriteFactory* dwriteFactory);
    HRESULT InitBrushes();
    HRESULT InitTextFormat();
    void ComputeBounds();
    void DiscardDevResources();
    void DrawEdge(const D2D1_POINT_2F& p1, const D2D1_POINT_2F& p2, float radius);
    void Render(const Rope& rope);
    void Render(const Rope::PNode& node, const D2D_POINT_2F& parent);
    void Reset();

    CComPtr<ID2D1DeviceContext> m_pDeviceContext;
    CComPtr<IDWriteFactory> m_pDWriteFactory;

    CComPtr<ID2D1SolidColorBrush> m_pNodeFillBrush;
    CComPtr<ID2D1SolidColorBrush> m_pLeafFillBrush;
    CComPtr<ID2D1SolidColorBrush> m_pFrozenLeafFillBrush;
    CComPtr<ID2D1SolidColorBrush> m_pEdgeBrush;
    CComPtr<ID2D1SolidColorBrush> m_pWhiteTextBrush;
    CComPtr<ID2D1SolidColorBrush> m_pBlackTextBrush;
    CComPtr<IDWriteTextFormat> m_pTextFormat;

    std::unordered_map<Rope::PNode, D2D_POINT_2F> m_nodePositions;
    D2D_SIZE_F m_bounds{};
};
