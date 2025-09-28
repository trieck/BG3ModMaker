#pragma once

class COMInitializer
{
public:
    explicit COMInitializer(DWORD dwCoInit = COINIT_APARTMENTTHREADED, LPVOID pvReserved = nullptr);
    ~COMInitializer();
};
