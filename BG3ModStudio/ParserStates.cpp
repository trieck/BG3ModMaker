#include "stdafx.h"
#include "ParserStates.h"

void ParserStates::Add(size_t state, IParserState::Ptr parserState)
{
    insert(state, std::move(parserState));
}

IParserState::Ptr ParserStates::Get(size_t state) const
{
    IParserState::Ptr parserState;
    find(state, parserState);
    return parserState;
}
