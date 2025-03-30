#include "stdafx.h"
#include "SVG.h"

SVG::SVG()
{
    m_stream = new MemStream();
}

SVG::~SVG()
{
}

void SVG::Begin(float w, float h)
{
    Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    WriteF("<svg xmlns='http://www.w3.org/2000/svg' width='{}' height='{}' viewBox='0 0 {} {}'>\n", w, h, w, h);
}

void SVG::Line(float x1, float y1, float x2, float y2)
{
    WriteF("<line x1='{}' y1='{}' x2='{}' y2='{}' stroke='black' stroke-width='1' />\n", x1, y1, x2, y2);
}

void SVG::Circle(float cx, float cy, float r, COLORREF fill)
{
    auto red = GetRValue(fill);
    auto green = GetGValue(fill);
    auto blue = GetBValue(fill);

    WriteF("<circle cx='{}' cy='{}' r='{}' fill='rgb({},{},{})' />\n", cx, cy, r, red, green, blue);
}

void SVG::End()
{
    Write("</svg>\n");
}

void SVG::Write(const char* s)
{
    (void)m_stream->WriteString(s);
}

void SVG::Write(std::string_view s)
{
    (void)m_stream->Write(s.data(), static_cast<ULONG>(s.size()), nullptr);
}

HRESULT SVG::Make(ID2D1DeviceContext5* ctx, ID2D1SvgDocument** out)
{
    if (!ctx || !out) {
        return E_INVALIDARG;
    }

    // Rewind stream to start
    LARGE_INTEGER zero{};
    auto hr = m_stream->Seek(zero, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    // For now, let's assume fixed viewport
    D2D1_SIZE_F viewport = D2D1::SizeF(800.0f, 600.0f);

    return ctx->CreateSvgDocument(m_stream, viewport, out);
}
