#pragma once
#include "Direct2D.h"
#include "ScintillaLoader.h"

class BG3ModStudio
{
    BG3ModStudio() = default;

public:
    static BG3ModStudio& instance();
    BG3ModStudio(const BG3ModStudio&) = delete;
    ~BG3ModStudio();

    BOOL Init();
    static int Run(HINSTANCE hInstance, LPWSTR lpCmdLine, int nShowCmd);

    ID2D1Factory* GetD2DFactory() const;

private:
    ScintillaLoader m_scintillaLoader;
    Direct2D m_direct2D;
};
