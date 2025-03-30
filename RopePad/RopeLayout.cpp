#include "stdafx.h"
#include "RopeLayout.h"

static constexpr auto LEVEL_SPACING_X = 100.0f;
static constexpr auto LEVEL_SPACING_Y = 100.0f;
static constexpr auto NODE_RADIUS = 30.0f;
static constexpr auto TOP_PADDING = NODE_RADIUS + 10.0f;

RopeLayout::RopeLayout()
{
}

void RopeLayout::Layout(D2D_SIZE_F dims, const Rope& rope)
{
    Reset();
    ComputeLayout(rope.root(), 0, 1.0f);

    m_svg.Begin(dims.width, dims.height);
    RenderSVG(rope.root(), {-1, -1});
    m_svg.End();
}

float RopeLayout::NodeRadius()
{
    return NODE_RADIUS;
}

void RopeLayout::Reset()
{
    m_nodePositions.clear();
    m_labels.clear();
    m_svg.Reset();
}

void RopeLayout::RenderSVG(const Rope::PNode& node, const D2D_POINT_2F& parent)
{
    if (!node) {
        return;
    }

    auto it = m_nodePositions.find(node);
    if (it == m_nodePositions.end()) {
        return;
    }

    const auto& pos = it->second;

    std::string text;
    std::string fontColor;

    if (node->value.isLeaf()) {
        // Estimate max chars that fit in circle (rough, can refine)
        constexpr int MaxTextChars = 6;
        text = node->value.text.substr(0, MaxTextChars);
        fontColor = node->value.isFrozen ? "black" : "white";
    }

    if (!text.empty()) {
        m_labels.emplace_back(NodeLabel{.pos = pos, .text = text, .color = fontColor});
    }

    if (parent.x >= 0 && parent.y >= 0) {
        m_svg.EdgeLine(parent.x, parent.y, pos.x, pos.y, NODE_RADIUS);
    }

    const bool isFrozen = node->value.isFrozen;
    const bool isLeaf = node->value.isLeaf();

    COLORREF fillColor = RGB(0x80, 0, 0); // default to dark red

    if (isLeaf) {
        if (isFrozen) {
            fillColor = RGB(173, 216, 230); // light blue (#ADD8E6)
        } else {
            fillColor = RGB(0, 128, 0);     // dark green
        }
    }

    m_svg.Circle(pos.x, pos.y, NODE_RADIUS, fillColor);

    RenderSVG(node->left, pos);
    RenderSVG(node->right, pos);
}

const SVG& RopeLayout::GetSVG() const
{
    return m_svg;
}

HRESULT RopeLayout::MakeDoc(ID2D1DeviceContext5* ctx, ID2D1SvgDocument** out)
{
    return m_svg.Make(ctx, out);
}

std::vector<NodeLabel> RopeLayout::GetLabels() const
{
    return m_labels;
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
