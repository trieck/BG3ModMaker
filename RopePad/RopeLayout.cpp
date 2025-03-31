#include "stdafx.h"
#include "RopeLayout.h"

#include <dwrite.h>

#include "ScopeGuard.h"
#include "StringHelper.h"

static constexpr auto LEVEL_SPACING_X = 125.0f;
static constexpr auto LEVEL_SPACING_Y = 125.0f;
static constexpr auto NODE_RADIUS = 50.0f;
static constexpr auto TOP_PADDING = NODE_RADIUS + 10.0f;
static constexpr auto FONT_SIZE = 16.0f;

RopeLayout::RopeLayout()
{
}

RopeLayout::~RopeLayout()
{
    DiscardDevResources();
}

HRESULT RopeLayout::Init(ID2D1DeviceContext* ctx, IDWriteFactory* pWriteFactory)
{
    if (!ctx || !pWriteFactory) {
        return E_POINTER;
    }

    auto hr = CreateDevResources(ctx, pWriteFactory);

    return hr;
}

HRESULT RopeLayout::InitBrushes()
{
    if (!m_pDeviceContext) {
        return E_POINTER;
    }

    auto hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pEdgeBrush);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteTextBrush);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackTextBrush);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &m_pNodeFillBrush);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGreen), &m_pLeafFillBrush);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightBlue), &m_pFrozenLeafFillBrush);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

HRESULT RopeLayout::InitTextFormat()
{
    if (!m_pDWriteFactory) {
        return E_POINTER;
    }

    auto hr = m_pDWriteFactory->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, FONT_SIZE, L"en-us", &m_pTextFormat);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    return hr;
}

HRESULT RopeLayout::Render(const Rope& rope, ID2D1CommandList** ppCommandList)
{
    if (!m_pDeviceContext || !m_pDWriteFactory || !ppCommandList) {
        return E_POINTER;
    }

    Reset();

    CComPtr<ID2D1CommandList> cmdList;
    auto hr = m_pDeviceContext->CreateCommandList(&cmdList);
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<ID2D1Image> previousTarget;
    ScopeGuardSimple targetGuard(
        [&] {
            m_pDeviceContext->GetTarget(&previousTarget);
            m_pDeviceContext->SetTarget(cmdList);
        },
        [&] { m_pDeviceContext->SetTarget(previousTarget); });

    m_pDeviceContext->BeginDraw();

    Render(rope);

    hr = m_pDeviceContext->EndDraw();
    if (FAILED(hr)) {
        return hr;
    }

    hr = cmdList->Close();
    if (FAILED(hr)) {
        return hr;
    }

    *ppCommandList = cmdList.Detach();

    return S_OK;
}

D2D_SIZE_F RopeLayout::GetBounds() const
{
    return m_bounds;
}

void RopeLayout::ComputeBounds()
{
    float left = 0;
    float top = 0;
    float right = 0;
    float bottom = 0;

    for (const auto& pos : m_nodePositions | std::views::values) {
        left = std::min(left, pos.x - NODE_RADIUS);
        right = std::max(right, pos.x + NODE_RADIUS);
        top = std::min(top, pos.y - NODE_RADIUS);
        bottom = std::max(bottom, pos.y + NODE_RADIUS);
    }

    // Optional padding
    constexpr float PADDING = 20.0f;

    m_bounds = {
        .width = right - left + 2 * PADDING,
        .height = bottom - top + 2 * PADDING
    };
}

D2D_POINT_2F RopeLayout::ComputeLayout(const Rope::PNode& node, int depth, float column)
{
    if (!node) {
        return {0.0f, column};
    }

    float xIndex;

    if (node->value.isLeaf()) {
        xIndex = column++;
    } else {
        D2D_POINT_2F left{}, right{};

        if (node->left) {
            left = ComputeLayout(node->left, depth + 1, column);
            column = left.y;
        }

        if (node->right) {
            right = ComputeLayout(node->right, depth + 1, column);
            column = right.y;
        }

        if (node->left && node->right) {
            xIndex = (left.x + right.x) / 2.0f;
        } else if (node->left) {
            xIndex = left.x + 1.0f;
        } else {
            xIndex = right.x - 1.0f;
        }
    }

    auto x = xIndex * LEVEL_SPACING_X;
    auto y = static_cast<float>(depth) * LEVEL_SPACING_Y + TOP_PADDING;

    m_nodePositions[node] = {.x = x, .y = y};

    return {xIndex, column};
}

void RopeLayout::DrawEdge(const D2D1_POINT_2F& p1, const D2D1_POINT_2F& p2, float radius)
{
    // Calculate vector (dx, dy) and length
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float len = std::sqrt(dx * dx + dy * dy);

    if (len == 0.0f) {
        return; // No edge to draw
    }

    // Normalize vector to get unit direction (ux, uy)
    float ux = dx / len;
    float uy = dy / len;

    // Calculate the new start and end points with radius applied
    D2D1_POINT_2F start = {p1.x + ux * radius, p1.y + uy * radius};
    D2D1_POINT_2F end = {p2.x - ux * radius, p2.y - uy * radius};

    // Draw the edge (line) between start and end points
    m_pDeviceContext->DrawLine(start, end, m_pEdgeBrush, 1.5f); // Adjust stroke width as needed
}

void RopeLayout::Render(const Rope::PNode& node, const D2D_POINT_2F& parent)
{
    if (!node || !m_pDeviceContext) {
        return;
    }

    auto it = m_nodePositions.find(node);
    if (it == m_nodePositions.end()) {
        return;
    }

    const auto& pos = it->second;

    ID2D1SolidColorBrush* fillBrush = m_pNodeFillBrush;
    if (node->value.isLeaf()) {
        fillBrush = node->value.isFrozen ? m_pFrozenLeafFillBrush : m_pLeafFillBrush;
    }

    D2D1_ELLIPSE ellipse = {pos, NODE_RADIUS, NODE_RADIUS};
    m_pDeviceContext->FillEllipse(ellipse, fillBrush);
    m_pDeviceContext->DrawEllipse(ellipse, m_pEdgeBrush);

    if (parent.x >= 0 && parent.y >= 0) {
        DrawEdge(parent, pos, NODE_RADIUS);
    }

    D2D1_RECT_F textBounds = {
        pos.x - NODE_RADIUS,
        pos.y - NODE_RADIUS,
        pos.x + NODE_RADIUS,
        pos.y + NODE_RADIUS
    };

    std::ostringstream ss;
    ss << "W: " << node->key.weight << " | S: " << node->size;

    if (node->value.isLeaf()) {
        constexpr auto MAX_TEXT_LENGTH = 10;
        ss << "\n\"" << node->value.text.substr(0, MAX_TEXT_LENGTH) << "\"";
    }

    auto textBrush = m_pWhiteTextBrush;
    if (node->value.isLeaf() && node->value.isFrozen) {
        textBrush = m_pBlackTextBrush;
    }

    auto wtext = StringHelper::fromUTF8(ss.str().c_str());
    
    m_pDeviceContext->DrawText(
        wtext,
        static_cast<UINT32>(wtext.GetLength()),
        m_pTextFormat,
        textBounds,
        textBrush,
        D2D1_DRAW_TEXT_OPTIONS_CLIP,
        DWRITE_MEASURING_MODE_NATURAL
    );

    Render(node->left, pos);
    Render(node->right, pos);
}

void RopeLayout::Reset()
{
    m_nodePositions.clear();
    m_bounds = {};
}

void RopeLayout::Render(const Rope& rope)
{
    if (!m_pDeviceContext || !m_pDWriteFactory) {
        return;
    }

    m_nodePositions.clear();

    ComputeLayout(rope.root(), 0, 1.f);
    ComputeBounds();

    Render(rope.root(), {-1.f, -1.f});
}

HRESULT RopeLayout::CreateDevResources(ID2D1DeviceContext* ctx, IDWriteFactory* dwriteFactory)
{
    if (!ctx || !dwriteFactory) {
        return E_POINTER;
    }

    DiscardDevResources();

    m_pDeviceContext = ctx;
    m_pDWriteFactory = dwriteFactory;

    auto hr = InitBrushes();
    if (FAILED(hr)) {
        return hr;
    }

    hr = InitTextFormat();
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

void RopeLayout::DiscardDevResources()
{
    m_pTextFormat.Release();
    m_pWhiteTextBrush.Release();
    m_pBlackTextBrush.Release();
    m_pEdgeBrush.Release();
    m_pNodeFillBrush.Release();
    m_pLeafFillBrush.Release();
    m_pFrozenLeafFillBrush.Release();

    m_pDWriteFactory.Release();
    m_pDeviceContext.Release();
}
