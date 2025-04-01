#pragma once
#include "Direct2D.h"
#include "RopeTreeFrame.h"

class RopePad
{
public:
    RopePad();
    ~RopePad();

    BOOL Init(HINSTANCE hInstance, LPSTR lpCmdLine);
    int Run(int nShowCmd = SW_SHOW);
    void Term();

    ID2D1Factory* GetD2DFactory() const;
    IDWriteFactory* GetDWriteFactory() const;
    const Rope& GetRope() const;

    void ToggleTreeView();
    void AddChar(int32_t pos, UINT nChar);
    void DeleteChar(int32_t pos);

private:
    Direct2D m_direct2D;
    RopeTreeFrame m_treeFrame;
    Rope m_rope;
};
