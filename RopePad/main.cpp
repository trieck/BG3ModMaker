#include "stdafx.h"
#include "Direct2D.h"
#include "RopePad.h"

CAppModule _Module;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    RopePad theApp;
    if (!theApp.Init(hInstance, lpCmdLine)) {
        return -11;
    }

    auto result = theApp.Run(nShowCmd);

    theApp.Term();

    return result;
}
