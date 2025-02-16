#pragma once

class MessageLoopEx : public CMessageLoop
{
public:
    MessageLoopEx() = default;
    ~MessageLoopEx() override = default;

    int Run();
};
