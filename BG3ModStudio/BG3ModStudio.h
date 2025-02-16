#pragma once

class BG3ModStudio
{
public:
    BG3ModStudio() = default;
    ~BG3ModStudio();

    bool init();
    static int run(HINSTANCE hInstance, LPWSTR lpCmdLine, int nShowCmd);

private:
    HINSTANCE m_hInstRich = nullptr;
};
