#pragma once
#include <d2d1svg.h>
#include <d2d1_3.h>

#include "MemStream.h"

class SVG
{
public:
    SVG();
    ~SVG();

    void Begin(float w, float h);
    void Line(float x1, float y1, float x2, float y2);
    void Circle(float cx, float cy, float r, COLORREF fill);

    void End();

    void Write(const char* s);
    void Write(std::string_view s);

    template <typename... Args>
    void WriteF(std::format_string<Args...> fmt, Args&&... args);

    HRESULT Make(ID2D1DeviceContext5* ctx, ID2D1SvgDocument** out);

private:
    CComPtr<MemStream> m_stream;
};

template <typename ... Args>
void SVG::WriteF(std::format_string<Args...> fmt, Args&&... args)
{
    auto str = std::format(fmt, std::forward<Args>(args)...);
    (void)m_stream->WriteString(str.c_str());
}

