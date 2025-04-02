#pragma once

#include "RBTree.h"

struct IParserState {
    virtual ~IParserState() = default;

    using Ptr = std::shared_ptr<IParserState>;
};

class ParserStates : RBTree<size_t, IParserState::Ptr>
{
public:
    ParserStates() = default;
    virtual ~ParserStates() = default;

    void Add(size_t state, IParserState::Ptr parserState);
    IParserState::Ptr Get(size_t state) const;
};

