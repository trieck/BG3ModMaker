#pragma once

#include "Rope.h"
#include "SVG.h"

struct NodeLabel {
    D2D_POINT_2F pos;
    std::string text;
    std::string color;
};

class RopeLayout
{
public:
    explicit RopeLayout();
    ~RopeLayout() = default;

    const SVG& GetSVG() const;
    HRESULT MakeDoc(ID2D1DeviceContext5* ctx, ID2D1SvgDocument** out);
    static float NodeRadius();
    std::vector<NodeLabel> GetLabels() const;
    void Layout(D2D_SIZE_F dims, const Rope& rope);
    void Reset();
private:
    D2D_POINT_2F ComputeLayout(const Rope::PNode& node, int depth, float column);
    void RenderSVG(const Rope::PNode& node, const D2D_POINT_2F& parent);

    std::unordered_map<Rope::PNode, D2D_POINT_2F> m_nodePositions;
    std::vector<NodeLabel> m_labels;
    SVG m_svg;
};

