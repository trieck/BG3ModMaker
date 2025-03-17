#pragma once

#include <atlbase.h>
#include <atlapp.h>

class MessageLoopEx : public CMessageLoop
{
public:
    MessageLoopEx() = default;
    ~MessageLoopEx() override = default;

    int Run();
};
